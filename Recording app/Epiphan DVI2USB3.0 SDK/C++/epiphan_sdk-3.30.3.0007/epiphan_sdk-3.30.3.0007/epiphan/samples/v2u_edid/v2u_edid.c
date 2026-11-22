/****************************************************************************
 *
 * $Id: v2u_edid.c 28179 2014-09-25 16:04:05Z pzeldin $
 *
 * Copyright (C) 2007-2013 Epiphan Systems Inc. All rights reserved.
 *
 * Reads and writes EDID information
 *
 ****************************************************************************/

#include <string.h>
#include "v2u_sys.h"
#include "v2u_util.h"

#ifdef _WIN32
#  define EDID_READ_FILE_MODE "rt"
#  define EDID_WRITE_FILE_MODE "wt"
#else
#  define EDID_READ_FILE_MODE "r"
#  define EDID_WRITE_FILE_MODE "w"
#endif


static const char * opt_help = "-h";
static const char * opt_help2 = "--help";
static const char * opt_usesn = "-u";
static const char * opt_usesn2 = "--use";
static const char * opt_edid = "-e";
static const char * opt_edid2 = "--edid";

/**
 * Finds -h (--help) option on the command line. Returns the index of the
 * first help option found (always >= 1 because the first element is the
 * prog name), zero otherwise.
 */
static int v2u_find_help_opt(int argc, char* argv[])
{
    int i;
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], opt_help) || !strcmp(argv[i], opt_help2)) {
            return i;
        }
    }
    return 0;
}

/**
 * Prints the usage
 */
static void v2u_usage()
{
    printf("Usage: v2u_edid [%s | %s SERIAL] [%s | %s SERIAL] [%s | %s EDIDFILE]\n",
           opt_help, opt_help2, opt_usesn, opt_usesn2, opt_edid, opt_edid2);
    printf("  If no EDIDFILE is given, EDID is read and dumped to stdout.\n");
    printf("  Otherwise, it's read from the file and written to the device.\n");
    printf("Options:\n");
    printf("  %s, %s            print this help and exit\n", opt_help, opt_help2);
    printf("  %s, %s SERIAL      specifies serial number of the card to use\n", opt_usesn, opt_usesn2);
    printf("  %s, %s EDIDFILE      EDID to write to the grabber\n", opt_edid, opt_edid2);
}

/**
 * Finds serial number specification on the command line. Returns the index
 * of the serial number found (always >= 1 because the first element is the
 * prog name), zero if the option wasn't found, negative number if multiple
 * serial numbers are specified.
 */
static int v2u_find_usesn_opt(int argc, char* argv[])
{
    int i, found = 0;
    for (i=1; i<argc-1; i++) {
        if (!strcmp(argv[i], opt_usesn) || !strcmp(argv[i], opt_usesn2)) {
            if (found) {
                printf("Multiple serial numbers on the command line");
                return -1;
            }
            found = i;
        }
    }
    return found;
}

/**
 * Finds EDID file specification. Returns the index
 * of the find name found (always >= 1 because the first element is the
 * prog name), zero if the option wasn't found, negative number if multiple
 * files are specified.
 */
static int v2u_find_edid_opt(int argc, char* argv[])
{
    int i, found = 0;
    for (i=1; i<argc-1; i++) {
        if (!strcmp(argv[i], opt_edid) || !strcmp(argv[i], opt_edid2)) {
            if (found) {
                printf("Multiple edid files on the command line");
                return -1;
            }
            found = i;
        }
    }
    return found;
}


/**
 * Finds device which supports EDID read/write. Returns device handle, NULL if
 * no devices with V2U_CAPS_EDID found.
 */
static V2U_DRIVER_HANDLE v2u_find_device(void)
{
    int i;
    for (i=MAX_VGA2USB_DEVICE_COUNT-1; i>=0; i--) {
        V2U_DRIVER_HANDLE handle = v2u_open_driver_idx(i);
        if (handle) {
            /* Check if this grabber has EDID */
            V2UPropertyValue v;
            if (v2u_get_property(handle, V2UKey_DeviceCaps, &v)) {
                if (v.int32 & V2U_CAPS_EDID) return handle;
            }
            v2u_close_driver(handle);
        }
    }
    return NULL;
}

/**
 * Entry point of the application
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    if (v2u_find_help_opt(argc, argv)) {
        v2u_usage();
    } else {
        int sn_idx;
        int edidfile_idx;
        int edid_size = 0;
        V2U_DRIVER_HANDLE d;
        V2UPropertyValue value;
        memset(value.blob, 0xff, sizeof(value.blob));

        edidfile_idx = v2u_find_edid_opt(argc, argv);
        if( edidfile_idx ) {
            /* Parse the EDID file */
            const char* fname = argv[edidfile_idx+1];
            FILE* f = fopen(fname, EDID_READ_FILE_MODE);
            if (f) {
                edid_size = v2u_edid_read_ext(f, value.blob);
                fclose(f);
                if (!edid_size) {
                    printf("Failed to parse %s\n",fname);
                    return 1;
                }
            } else {
                printf("Failed to open %s\n",fname);
                return 1;
            }
        }

        /* Open the driver */
        sn_idx = v2u_find_usesn_opt(argc, argv);
        if (sn_idx > 0) {
            d = v2u_open_driver_sn(argv[sn_idx+1]);
        } else if ( sn_idx == 0 ) {
            d = v2u_find_device();
        } else {
            return sn_idx;
        }
        
        /* Perform EDID operation */
        if (d) {
            if (edid_size) {
                /* Upload EDID to the device */
                V2UPropertyKey key = V2UKey_EDID;
                if (edid_size > V2U_EDID_SIZE) key = V2UKey_EEDID;
                if (v2u_set_property(d, key, &value)) {
                    printf("EDID uploaded successfully\n");
                } else {
                    printf("Failed to upload EDID to the device.\n");
                    rc = 3;
                }
            } else {
                /* Read EDID from the device. Driver didn't support V2UKey_EEDID
                 * property prior to version 3.27.14. So if it fails then retry
                 * with V2UKey_EDID but in this case we may get truncated EDID. */
                memset(value.blob, 0xff, sizeof(value.blob));
                if (v2u_get_property(d, V2UKey_EEDID, &value) ||
                    v2u_get_property(d, V2UKey_EDID, &value)) {
                    /* Use the short (128 bytes) format if the last 128 bytes
                     * are filled with 0xFF */
                    int i, size = V2U_EDID_SIZE;
                    for (i=V2U_EDID_SIZE; i<V2U_EEDID_SIZE; i++) {
                        if (value.blob[i] != 0xff) {
                            size = V2U_EEDID_SIZE;
                            break;
                        }
                    }
                    /* And dump it to stdout */
                    v2u_edid_write_ext(stdout, value.blob, size);
                } else {
                    printf("Unable to download EDID from the device.\n");
                    rc = 3;
                }
            }
            v2u_close_driver(d);
        } else {
            printf("No EDID capable Epiphan frame grabber found\n");
            rc = 2;
        }
    }
    return rc;
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
