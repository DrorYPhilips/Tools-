/*******************************************************************************

Copyright Datapath Ltd. 1997, 2015.

File:    api.c

Purpose: Implementation of functions to help with loading DLLs and obtaining
         pointers to functions within those DLLs.

History:
         20 MAY 97    JH   Created.
         05 FEB 98    JH   Prevented MessageBox being called.
         05 DEC 01    JH   Added APILoadLibrary, APIFreeLibrary and
                           APILoadInterface.
         23 JUN 04    JH   In Win32 user mode, if APILoadLibrary fails to load
                           the DLL, it returns the value from GetLastError.
         30 MAY 06    JH   Ported to Unicode.
         16 FEB 15    SB   For debug builds only, print name of missing export
                           to debugger output.

*******************************************************************************/

#include <windows.h>

#ifdef KERNEL_MODE
#if (defined WIN32) && (WINVER >= 0x400)
#include "twntkrnl.h"
#endif
#else
#include <strsafe.h>
#endif

#include "api.h"

/******************************************************************************/

int
APILoadFunctions (
   HINSTANCE   hInstance,
   LPAPIFNLIST lpFnList,
   void        *pReserved )
/*
 * Summary: Loads a set of API functions from a DLL.
 *
 * Purpose:
 *
 * Return:  If successful, TRUE is returned; otherwise, FALSE is returned.
 */
{
   int   i = 0;
   UNREFERENCED_PARAMETER( pReserved);

   while ( lpFnList[i].PFnPtr )
   {
      *lpFnList[i].PFnPtr = GetProcAddress ( hInstance, lpFnList[i].FnName );
      if ( *lpFnList[i].PFnPtr == NULL )
      {
#ifndef KERNEL_MODE
#ifdef _DEBUG
         // GetProcAddress is ANSI only
         char buffer[256];
         StringCchPrintfA(buffer, sizeof(buffer) / sizeof(buffer[0]),
            "API Load Error: %s\n", lpFnList[i].FnName);
         OutputDebugStringA(buffer);
#endif
#endif
         return FALSE;
      }
      i++;
   }
   return TRUE;
}

/******************************************************************************/

int
APILoadFunctionsEx (
   HINSTANCE   hInstance,
   LPAPIFNLIST lpFnList,
   BOOL        bFailOnLoad )
/*
 * Summary: Loads a set of API functions from a DLL.
 *
 * Purpose:
 *
 * Return:  If successful, TRUE is returned; otherwise, FALSE is returned.
 */
{
   int   i = 0;

   while ( lpFnList[i].PFnPtr )
   {
      *lpFnList[i].PFnPtr = GetProcAddress ( hInstance, lpFnList[i].FnName );
      if ( ( *lpFnList[i].PFnPtr == NULL ) && bFailOnLoad )
      {
#ifndef KERNEL_MODE
#ifdef _DEBUG
         // GetProcAddress is ANSI only
         char buffer[256];
         StringCchPrintfA(buffer, sizeof(buffer) / sizeof(buffer[0]),
            "API Load Error: %s\n", lpFnList[i].FnName);
         OutputDebugStringA(buffer);
#endif
#endif
         return FALSE;
      }
      i++;
   }
   return TRUE;
}

/******************************************************************************/

unsigned long
APILoadLibrary (
   LPCAPICHAR  pDriverName,
   HINSTANCE   FAR *pHInstance )
/*
 * Summary: Loads a DLL.
 *
 * Purpose:
 *
 * Return:  If successful, zero is returned and the instance handle of the DLL
 *          is stored in *pHInstance; otherwise, one of the API_ERROR values
 *          defined in API.H is returned.
 */
{
   HINSTANCE   hInstance;

#if ( defined WIN32 ) && ( defined KERNEL_MODE )
   hInstance = EngLoadImage ( (LPWSTR)pDriverName );
#else
   hInstance = LoadLibrary ( pDriverName );
#endif
#if ( defined WIN32 )
   if ( hInstance )
#else
   if ( hInstance > HINSTANCE_ERROR )
#endif
   {
      *pHInstance = hInstance;
      return 0;
   }
#if ( defined WIN32 ) && ( !defined KERNEL_MODE )
   return GetLastError ( );
#else
   return API_ERROR_UNABLE_TO_LOAD_DLL;
#endif
}

/******************************************************************************/

unsigned long
APIFreeLibrary (
   HINSTANCE   hInstance )
/*
 * Summary: Frees a DLL.
 *
 * Purpose: Frees a DLL loaded with APILoadDLL or APILoadInterface.
 *
 * Return:  N/A.
 */
{
#ifndef KERNEL_MODE
   FreeLibrary ( hInstance );
#else
   EngUnloadImage ( hInstance );
#endif
   return 0;
}

/******************************************************************************/

unsigned long
APILoadInterface (
   LPCAPICHAR  lpSectionName,
   LPCAPICHAR  lpValueName,
   LPAPIFNLIST lpFnList,
   HINSTANCE   *pHInstance )
/*
 * Summary: Loads an interface DLL.
 *
 * Purpose: Reads the Registry entry specifying the path of a DLL with a
 *          standard interface. The DLL is loaded and the functions of the
 *          interface are located.
 *
 *          lpSectionName
 *             The name of the Registry key in which to find the value
 *             specified below. For example "SOFTWARE\\Datapath\\I2C".
 *
 *          lpValueName
 *             The name of the REG_SZ entry that contains the interface
 *             DLL name. For example "I2CDLL".
 *
 *          lpFnList
 *             A pointer to an APIFNLIST, a list of function names and
 *             pointers to locations where the addresses of the functions
 *             loaded from the DLL may be stored.
 *
 *          pHInstance
 *             A pointer to the location where the HINSTANCE of the loaded
 *             interface DLL is to be stored.
 *
 * Return:  If successful, zero is returned and the instance handle of the DLL
 *          is stored in *pHInstance; otherwise, one of the API_ERROR values
 *          defined in API.H is returned.
 */
{
#ifndef KERNEL_MODE
   unsigned long  error = 0;
   HKEY           hKey;

   /* Open the Registry key. */
   if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
         lpSectionName, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
   {
      DWORD dwSize, dwType;
      TCHAR driverName[MAX_PATH];
      LONG  lError;

      /* Read the name of the interface DLL. */
      dwSize = sizeof ( driverName );
      lError = RegQueryValueEx ( hKey, lpValueName, 0,
            &dwType, (LPBYTE)driverName, &dwSize );
      RegCloseKey ( hKey );

      if (  ( lError == ERROR_SUCCESS ) &&
            ( dwType == REG_SZ ) &&
            ( dwSize > sizeof ( TCHAR )))
      {
         HINSTANCE   hInstance;

         /* Load the interface DLL. */
         error = APILoadLibrary ( driverName, &hInstance );
         if ( error == 0 )
         {
            /* Find each of the functions exported by the interface. */
            if ( APILoadFunctions ( hInstance, lpFnList, NULL ))
            {
               *pHInstance = hInstance;
            }
            else
            {
               APIFreeLibrary ( hInstance );
               error = API_ERROR_INCOMPATIBLE_API;
            }
         }
      }
      else 
      {
         error = API_ERROR_UNABLE_TO_READ_VALUE;
      }
   }
   else
   {
      error = API_ERROR_UNABLE_TO_OPEN_KEY;
   }

   return error;
#else
   /* TODO: Implement kernel mode version. */
   return -1;
#endif
}

/******************************************************************************/

