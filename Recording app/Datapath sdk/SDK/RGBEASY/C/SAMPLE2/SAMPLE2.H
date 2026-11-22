typedef struct
{
   HINSTANCE   HInstance;
   HWND        HWndParent;
   HRGB        HRGB;
   ULONG       Input;
   BOOL        BProgram;
   BOOL        HCurrentMode;
   HAUDIOINPUT HAudioInput;
}  WINDOW, *PWINDOW;

INT_PTR CALLBACK 
AudioSettingsDlgProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam );

INT_PTR CALLBACK
InputSettingsDlgProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam );

void
SetTrackBarPos (
   HWND  hCtl,
   LONG  pos,
   BOOL  bRedraw );

void
SetTrackBarRange (
   HWND  hCtl,
   LONG  min,
   LONG  max,
   BOOL  bRedraw );

static const TCHAR
   RGBWindowClass[]  = { TEXT("RGBSampleWndClass") },
   Property[]        = { TEXT("PWindow") },
   Caption[]         = { TEXT("RGB Sample 2") };