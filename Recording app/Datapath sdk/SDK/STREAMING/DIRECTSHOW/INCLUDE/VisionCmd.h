/*******************************************************************************

Copyright Datapath Ltd. 2009.

File:    DGCCmd.h

Purpose: 

History:
         31 JAN 09    RL   Created.

*******************************************************************************/

#define CMD_FLAGS_SUBTYPE_RGB8               0x00000001
#define CMD_FLAGS_SUBTYPE_RGB565             0x00000002
#define CMD_FLAGS_SUBTYPE_RGB555             0x00000004
#define CMD_FLAGS_SUBTYPE_RGB32              0x00000008
#define CMD_FLAGS_SUBTYPE_RGB24              0x00000010
#define CMD_FLAGS_SUBTYPE_YUY2               0x00000020
#define CMD_FLAGS_SUBTYPE_UYVY               0x00000040
#define CMD_FLAGS_SUBTYPE_YVYU               0x00000080
#define CMD_FLAGS_SUBTYPE_NV12               0x00000100
#define CMD_FLAGS_SUBTYPE_YV12               0x00000200
// Space left for new subtypes
#define CMD_FLAGS_CAPTURE_RATE               0x00001000
#define CMD_FLAGS_BUFFER_SIZE                0x00002000
#define CMD_FLAGS_WINDOW_POSITION            0x00004000
#define CMD_FLAGS_INPUT_SOURCE               0x00008000
#define CMD_FLAGS_CHANNEL_SOURCE             0x00020000

/******************************************************************************/

typedef struct
{
   unsigned long  flags;

   POINT          buffer;        /* width, height. */
   RECT           window;        /* Top, left, width, height. */
   GUID           subformat;
   unsigned long  input;
   unsigned long  channel;
   unsigned long  fps;

}  COMMANDLINE, *PCOMMANDLINE;

/******************************************************************************/

BOOL
ProcessCommandLine(
   LPCTSTR        lpStrCommandLine,
   PCOMMANDLINE   pCommandLine);

BOOL
SetCommandLine(
   HWND           hWnd,
   LPCTSTR        pTitle,
   PCOMMANDLINE   pCommandLine,
   IBaseFilter    *pSrcFilter,
   LPCTSTR        pPinName);

/******************************************************************************/
