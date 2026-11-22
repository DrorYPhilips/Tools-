using System;
using System.Runtime.InteropServices;

namespace Datapath.RGBEasy
{
   public enum CAPTURECARD
   {
      DGC103 = 0,
      DGC133 = 1,
   }

   public enum CAPTURESTATE
   {
      CAPTURING = 0,
      NOSIGNAL = 1,
      INVALIDSIGNAL = 2,
      PAUSED = 3,
      ERROR = 4,
   }

   public enum PIXELFORMAT
   {
      AUTO = 0,
      RGB555 = 1,
      RGB565 = 2,
      RGB888 = 3,
      GREY = 4,
      RGB24 = 5,
      YUY2 = 6,
   }

   public enum DEINTERLACE
   {
      WEAVE = 0,
      BOB = 1,
      FIELD_0 = 2,
      FIELD_1 = 3,
   }

   public enum ANALOG_INPUT_TYPE
   {
      VGA = 0,
      VIDEO = 1,
   }

   public enum SIGNALTYPE
   {
      NOSIGNAL = 0,
      VGA = 1,
      DVI = 2,
      COMPONENT = 3,
      COMPOSITE = 4,
      SVIDEO = 5,
      OUTOFRANGE = 6,
   }

   public enum OSD_TYPE
   {
      OSD_TYPE_DISABLED = 0,
      OSD_TYPE_TEXT = 1,
   }

   public enum OSD_ALIGNMENT
   {
      OSD_HOR_LEFT = 0,
      OSD_HOR_CENTRE = 1,
      OSD_HOR_RIGHT = 2,

      OSD_VER_TOP = 0,
      OSD_VER_CENTRE = 1,
      OSD_VER_BOTTOM = 2,
   }

   public enum ROTATIONANGLE
   {
      ROTATIONANGLE_0 = 0,
      ROTATIONANGLE_90 = 1,
      ROTATIONANGLE_180 = 2,
      ROTATIONANGLE_270 = 3,
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBDRIVERVER
   {
      public uint Major;
      public uint Minor;
      public uint Micro;
      public uint Revision;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBLOCATION
   {
      public uint Bus;
      public uint Device;
      public uint Function;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBINPUTINFO
   {
      public uint Size;
      RGBDRIVERVER Driver;
      RGBLOCATION Location;
      public uint Firmware;
      public uint VHDL;
      public ulong Identifier;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBMODECHANGEDINFO
   {
      public uint Size;
      public uint RefreshRate;
      public uint LineRate;
      public uint TotalNumberOfLines;
      public int BInterlaced;
      public int BDVI;
      public ANALOG_INPUT_TYPE AnalogType;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBMODEINFO
   {
      public uint Size;
      public CAPTURESTATE State;
      public uint RefreshRate;
      public uint LineRate;
      public uint TotalNumberOfLines;
      public int BInterlaced;
      public int BDVI;
      public ANALOG_INPUT_TYPE AnalogType;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBQUAD
   {
      public byte rgbBlue;
      public byte rgbGreen;
      public byte rgbRed;
      private byte rgbReserved;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct BITMAPINFOHEADER
   {
      public uint biSize;
      public int biWidth;
      public int biHeight;
      public ushort biPlanes;
      public ushort biBitCount;
      public uint biCompression;
      public uint biSizeImage;
      public int biXPelsPerMeter;
      public int biYPelsPerMeter;
      public uint biClrUsed;
      public uint biClrImportant;
   }

   [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto, Pack = 1)]
   public class LOGFONT
   {
      public const int LF_FACESIZE = 32;
      public int lfHeight;
      public int lfWidth;
      public int lfEscapement;
      public int lfOrientation;
      public int lfWeight;
      public byte lfItalic;
      public byte lfUnderline;
      public byte lfStrikeOut;
      public byte lfCharSet;
      public byte lfOutPrecision;
      public byte lfClipPrecision;
      public byte lfQuality;
      public byte lfPitchAndFamily;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = LF_FACESIZE)]
      public string lfFaceName;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public class COLORREF
   {
      public byte red, green, blue;
      private byte unused;

      public COLORREF(byte r, byte g, byte b)
      {
         red = r;
         green = g;
         blue = b;
         unused = 0;
      }

      public uint ToUint()
      {
         return (uint)blue << 16 | (uint)green << 8 | (uint)red;
      }
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct BITMAPINFO
   {
      public BITMAPINFOHEADER bmiHeader;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
      public RGBQUAD[] bmiColors;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct SIGNEDVALUE
   {
      public int BChanged;
      public int Value;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct UNSIGNEDVALUE
   {
      public int BChanged;
      public uint Value;
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct RGBVALUECHANGEDINFO
   {
      public uint Size;
      public SIGNEDVALUE HorPosition;
      public UNSIGNEDVALUE HorScale;
      public UNSIGNEDVALUE VerPosition;
      public SIGNEDVALUE CaptureWidth;
      public UNSIGNEDVALUE CaptureHeight;
      public SIGNEDVALUE Brightness;
      public SIGNEDVALUE Contrast;
      public SIGNEDVALUE BlackLevel;
      public SIGNEDVALUE Phase;
      public UNSIGNEDVALUE RedGain;
      public UNSIGNEDVALUE GreenGain;
      public UNSIGNEDVALUE BlueGain;
      public UNSIGNEDVALUE RedOffset;
      public UNSIGNEDVALUE GreenOffset;
      public UNSIGNEDVALUE BlueOffset;
      public UNSIGNEDVALUE Saturation;
      public UNSIGNEDVALUE Hue;
      public UNSIGNEDVALUE VideoStandard;
   }

   public delegate void RGBFRAMECAPTUREDFN(IntPtr hWnd, IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits, IntPtr userData);

   public delegate void RGBMODECHANGEDFN(IntPtr hWnd, IntPtr hRGB, ref RGBMODECHANGEDINFO modeChangedInfo, IntPtr userData);

   public delegate void RGBNOSIGNALFN(IntPtr hWnd, IntPtr hRGB, IntPtr userData);

   public delegate void RGBDRAWNOSIGNALFN(IntPtr hWnd, IntPtr hRGB, IntPtr hDC, IntPtr userData);

   public delegate void RGBINVALIDSIGNALFN(IntPtr hWnd, IntPtr hRGB, uint horClock, uint verClock, IntPtr userData);

   public delegate void RGBDRAWINVALIDSIGNALFN(IntPtr hWnd, IntPtr hRGB, IntPtr hDC, uint horClock, uint verClock, IntPtr userData);

   public delegate void RGBERRORFND(IntPtr hWnd, IntPtr hRGB, uint error, IntPtr userData, ref IntPtr reserved);

   public delegate void RGBVALUECHANGEDFN(IntPtr hWnd, IntPtr hRGB, ref RGBVALUECHANGEDINFO valueChangedInfo, IntPtr userData);

   public class RGB
   {
      public static RGBERROR Load(out IntPtr hRGBDLL)
      {
         return NativeMethods.RGBLoad(out hRGBDLL);
      }

      public static RGBERROR LoadEx(out IntPtr hRGBDLL, CAPTURECARD captureCard)
      {
         return NativeMethods.RGBLoadEx(out hRGBDLL, captureCard);
      }

      public static RGBERROR Free(IntPtr hRGBDLL)
      {
         return NativeMethods.RGBFree(hRGBDLL);
      }

      public static RGBERROR GetCaptureCard(out CAPTURECARD captureCard)
      {
         return NativeMethods.RGBGetCaptureCard(out captureCard);
      }

      public static RGBERROR GetNumberOfInputs(out uint NumberOfInputs)
      {
         return NativeMethods.RGBGetNumberOfInputs(out NumberOfInputs);
      }

      public static RGBERROR GetInputInfo(uint uInput, [In, Out] ref RGBINPUTINFO inputInfo)
      {
         return NativeMethods.RGBGetInputInfo(uInput, ref inputInfo);
      }

      public static RGBERROR OpenInput(uint uInput, out IntPtr hRGB)
      {
         return NativeMethods.RGBOpenInput(uInput, out hRGB);
      }

      public static RGBERROR CloseInput(IntPtr hRGB)
      {
         return NativeMethods.RGBCloseInput(hRGB);
      }

      public static RGBERROR CloseInputs(IntPtr phRGBArray, uint numberOfInputs)
      {
         return NativeMethods.RGBCloseInputs(phRGBArray, numberOfInputs);
      }

      public static RGBERROR DetectInput(IntPtr hRGB)
      {
         return NativeMethods.RGBDetectInput(hRGB);
      }

      public static RGBERROR SetInput(IntPtr hRGB, uint input)
      {
         return NativeMethods.RGBSetInput(hRGB, input);
      }

      public static RGBERROR GetInput(IntPtr hRGB, out uint input)
      {
         return NativeMethods.RGBGetInput(hRGB, out input);
      }

      public static RGBERROR SetWindow(IntPtr hRGB, IntPtr hWnd)
      {
         return NativeMethods.RGBSetWindow(hRGB, hWnd);
      }

      public static RGBERROR GetWindow(IntPtr hRGB, out IntPtr hWnd)
      {
         return NativeMethods.RGBGetWindow(hRGB, out hWnd);
      }

      public static RGBERROR ResetCapture(IntPtr hRGB)
      {
         return NativeMethods.RGBResetCapture(hRGB);
      }

      public static RGBERROR GetHorPositionMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHorPositionMinimum(hRGB, out value);
      }

      public static RGBERROR GetHorPositionMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHorPositionMaximum(hRGB, out value);
      }

      public static RGBERROR GetHorPositionDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHorPositionDefault(hRGB, out value);
      }

      public static RGBERROR GetHorPosition(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHorPosition(hRGB, out value);
      }

      public static RGBERROR SetHorPosition(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetHorPosition(hRGB, value);
      }

      public static RGBERROR GetHorScaleMinimum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetHorScaleMinimum(hRGB, out value);
      }

      public static RGBERROR GetHorScaleMaximum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetHorScaleMaximum(hRGB, out value);
      }

      public static RGBERROR GetHorScaleDefault(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetHorScaleDefault(hRGB, out value);
      }

      public static RGBERROR GetHorScale(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetHorScale(hRGB, out value);
      }

      public static RGBERROR SetHorScale(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBSetHorScale(hRGB, value);
      }

      public static RGBERROR GetCaptureWidthMinimum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureWidthMinimum(hRGB, out value);
      }

      public static RGBERROR GetCaptureWidthMaximum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureWidthMaximum(hRGB, out value);
      }

      public static RGBERROR GetCaptureWidthDefault(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureWidthDefault(hRGB, out value);
      }

      public static RGBERROR GetCaptureWidth(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureWidth(hRGB, out value);
      }

      public static RGBERROR TestCaptureWidth(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBTestCaptureWidth(hRGB, value);
      }

      public static RGBERROR SetCaptureWidth(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBSetCaptureWidth(hRGB, value);
      }

      public static RGBERROR GetVerPositionMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetVerPositionMinimum(hRGB, out value);
      }

      public static RGBERROR GetVerPositionMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetVerPositionMaximum(hRGB, out value);
      }

      public static RGBERROR GetVerPositionDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetVerPositionDefault(hRGB, out value);
      }

      public static RGBERROR GetVerPosition(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetVerPosition(hRGB, out value);
      }

      public static RGBERROR SetVerPosition(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetVerPosition(hRGB, value);
      }

      public static RGBERROR GetCaptureHeightMinimum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureHeightMinimum(hRGB, out value);
      }

      public static RGBERROR GetCaptureHeightMaximum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureHeightMaximum(hRGB, out value);
      }

      public static RGBERROR GetCaptureHeightDefault(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureHeightDefault(hRGB, out value);
      }

      public static RGBERROR GetCaptureHeight(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetCaptureHeight(hRGB, out value);
      }

      public static RGBERROR SetCaptureHeight(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBSetCaptureHeight(hRGB, value);
      }

      public static RGBERROR GetBrightnessMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBrightnessMinimum(hRGB, out value);
      }

      public static RGBERROR GetBrightnessMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBrightnessMaximum(hRGB, out value);
      }

      public static RGBERROR GetBrightnessDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBrightnessDefault(hRGB, out value);
      }

      public static RGBERROR GetBrightness(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBrightness(hRGB, out value);
      }

      public static RGBERROR SetBrightness(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetBrightness(hRGB, value);
      }

      public static RGBERROR GetContrastMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetContrastMinimum(hRGB, out value);
      }

      public static RGBERROR GetContrastMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetContrastMaximum(hRGB, out value);
      }

      public static RGBERROR GetContrastDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetContrastDefault(hRGB, out value);
      }

      public static RGBERROR GetContrast(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetContrast(hRGB, out value);
      }

      public static RGBERROR SetContrast(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetContrast(hRGB, value);
      }

      public static RGBERROR GetBlackLevelMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBlackLevelMinimum(hRGB, out value);
      }

      public static RGBERROR GetBlackLevelMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBlackLevelMaximum(hRGB, out value);
      }

      public static RGBERROR GetBlackLevelDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBlackLevelDefault(hRGB, out value);
      }

      public static RGBERROR GetBlackLevel(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetBlackLevel(hRGB, out value);
      }

      public static RGBERROR SetBlackLevel(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetBlackLevel(hRGB, value);
      }

      public static RGBERROR GetPhaseMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetPhaseMinimum(hRGB, out value);
      }

      public static RGBERROR GetPhaseMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetPhaseMaximum(hRGB, out value);
      }

      public static RGBERROR GetPhaseDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetPhaseDefault(hRGB, out value);
      }

      public static RGBERROR GetPhase(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetPhase(hRGB, out value);
      }

      public static RGBERROR SetPhase(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetPhase(hRGB, value);
      }

      public static RGBERROR GetFrameDroppingMinimum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetFrameDroppingMinimum(hRGB, out value);
      }

      public static RGBERROR GetFrameDroppingMaximum(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetFrameDroppingMaximum(hRGB, out value);
      }

      public static RGBERROR GetFrameDroppingDefault(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetFrameDroppingDefault(hRGB, out value);
      }

      public static RGBERROR GetFrameDropping(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetFrameDropping(hRGB, out value);
      }

      public static RGBERROR SetFrameDropping(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBSetFrameDropping(hRGB, value);
      }

      public static RGBERROR GetFrameRate(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetFrameRate(hRGB, out value);
      }

      public static RGBERROR IsCroppingEnabled(IntPtr hRGB, out uint enabled)
      {
         return NativeMethods.RGBIsCroppingEnabled(hRGB, out enabled);
      }

      public static RGBERROR EnableCropping(IntPtr hRGB, uint enable)
      {
         return NativeMethods.RGBEnableCropping(hRGB, enable);
      }

      public static RGBERROR GetCroppingMinimum(IntPtr hRGB, out int top, out int left, out uint width, out uint height)
      {
         return NativeMethods.RGBGetCroppingMinimum(hRGB, out top, out left, out width, out height);
      }

      public static RGBERROR GetCroppingMaximum(IntPtr hRGB, out int top, out int left, out uint width, out uint height)
      {
         return NativeMethods.RGBGetCroppingMaximum(hRGB, out top, out left, out width, out height);
      }

      public static RGBERROR GetCroppingDefault(IntPtr hRGB, out int top, out int left, out uint width, out uint height)
      {
         return NativeMethods.RGBGetCroppingDefault(hRGB, out top, out left, out width, out height);
      }

      public static RGBERROR GetCropping(IntPtr hRGB, out int top, out int left, out uint width, out uint height)
      {
         return NativeMethods.RGBGetCropping(hRGB, out top, out left, out width, out height);
      }

      public static RGBERROR TestCropping(IntPtr hRGB, int top, int left, uint width, uint height)
      {
         return NativeMethods.RGBTestCropping(hRGB, top, left, width, height);
      }

      public static RGBERROR SetCropping(IntPtr hRGB, int top, int left, uint width, uint height)
      {
         return NativeMethods.RGBSetCropping(hRGB, top, left, width, height);
      }

      public static RGBERROR PauseCapture(IntPtr hRGB)
      {
         return NativeMethods.RGBPauseCapture(hRGB);
      }

      public static RGBERROR ResumeCapture(IntPtr hRGB)
      {
         return NativeMethods.RGBResumeCapture(hRGB);
      }

      public static RGBERROR GetCaptureState(IntPtr hRGB, out CAPTURESTATE captureState)
      {
         return NativeMethods.RGBGetCaptureState(hRGB, out captureState);
      }

      public static RGBERROR GetMessageDelay(IntPtr hRGB, out int showMessages, out uint delay)
      {
         return NativeMethods.RGBGetMessageDelay(hRGB, out showMessages, out delay);
      }

      public static RGBERROR SetMessageDelay(IntPtr hRGB, int showMessages, uint delay)
      {
         return NativeMethods.RGBSetMessageDelay(hRGB, showMessages, delay);
      }

      public static RGBERROR GetPixelformat(IntPtr hRGB, out PIXELFORMAT pixelFormat)
      {
         return NativeMethods.RGBGetPixelFormat(hRGB, out pixelFormat);
      }

      public static RGBERROR SetPixelformat(IntPtr hRGB, PIXELFORMAT pixelFormat)
      {
         return NativeMethods.RGBSetPixelFormat(hRGB, pixelFormat);
      }

      public static RGBERROR SaveCurrentFrame(IntPtr hRGB, string fileName)
      {
         return NativeMethods.RGBSaveCurrentFrameW(hRGB, fileName);
      }

      public static RGBERROR IsDirectDMASupported(out int supported)
      {
         return NativeMethods.RGBIsDirectDMASupported(out supported);
      }

      public static RGBERROR GetDMADirect(IntPtr hRGB, out int direct)
      {
         return NativeMethods.RGBGetDMADirect(hRGB, out direct);
      }

      public static RGBERROR SetDMADirect(IntPtr hRGB, int direct)
      {
         return NativeMethods.RGBSetDMADirect(hRGB, direct);
      }

      public static RGBERROR GetDownScaling(IntPtr hRGB, out int fastScaling)
      {
         return NativeMethods.RGBGetDownScaling(hRGB, out fastScaling);
      }

      public static RGBERROR SetDownScaling(IntPtr hRGB, int fastScaling)
      {
         return NativeMethods.RGBSetDownScaling(hRGB, fastScaling);
      }

      public static RGBERROR SetFrameCapturedFn(IntPtr hRGB, RGBFRAMECAPTUREDFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetFrameCapturedFn(hRGB, pFn, userData);
      }

      public static RGBERROR DrawFrame(IntPtr hRGB)
      {
         return NativeMethods.RGBDrawFrame(hRGB);
      }

      public static RGBERROR SaveBitmap(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits, string filename)
      {
         return NativeMethods.RGBSaveBitmapW(hRGB, bitmapInfo, bitmapBits, filename);
      }

      public static RGBERROR SetModeChangedFn(IntPtr hRGB, RGBMODECHANGEDFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetModeChangedFn(hRGB, pFn, userData);
      }

      public static RGBERROR SetNoSignalFn(IntPtr hRGB, RGBNOSIGNALFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetNoSignalFn(hRGB, pFn, userData);
      }

      public static RGBERROR NoSignal(IntPtr hRGB)
      {
         return NativeMethods.RGBNoSignal(hRGB);
      }

      public static RGBERROR SetDrawNoSignalFn(IntPtr hRGB, RGBDRAWNOSIGNALFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetDrawNoSignalFn(hRGB, pFn, userData);
      }

      public static RGBERROR SetInvalidSignalFn(IntPtr hRGB, RGBINVALIDSIGNALFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetInvalidSignalFn(hRGB, pFn, userData);
      }

      public static RGBERROR InvalidSignal(IntPtr hRGB, uint horClock, uint verClock)
      {
         return NativeMethods.RGBInvalidSignal(hRGB, horClock, verClock);
      }

      public static RGBERROR SetDrawInvalidSignalFn(IntPtr hRGB, RGBDRAWINVALIDSIGNALFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetDrawInvalidSignalFn(hRGB, pFn, userData);
      }

      public static RGBERROR SetErrorFn(IntPtr hRGB, RGBERRORFND pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetErrorFn(hRGB, pFn, userData);
      }

      public static RGBERROR SetValueChangedFn(IntPtr hRGB, RGBVALUECHANGEDFN pFn, IntPtr userData)
      {
         return NativeMethods.RGBSetValueChangedFn(hRGB, pFn, userData);
      }

      public static RGBERROR StartCapture(IntPtr hRGB)
      {
         return NativeMethods.RGBStartCapture(hRGB);
      }

      public static RGBERROR StopCapture(IntPtr hRGB)
      {
         return NativeMethods.RGBStopCapture(hRGB);
      }

      public static RGBERROR GetColourBalanceMinimum(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue)
      {
         return NativeMethods.RGBGetColourBalanceMinimum(hRGB, out brightnessRed, out brightnessGreen, out brightnessBlue, out contrastRed, out contrastGreen, out contrastBlue);
      }

      public static RGBERROR GetColourBalanceMaximum(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue)
      {
         return NativeMethods.RGBGetColourBalanceMaximum(hRGB, out brightnessRed, out brightnessGreen, out brightnessBlue, out contrastRed, out contrastGreen, out contrastBlue);
      }

      public static RGBERROR GetColourBalanceDefault(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue)
      {
         return NativeMethods.RGBGetColourBalanceDefault(hRGB, out brightnessRed, out brightnessGreen, out brightnessBlue, out contrastRed, out contrastGreen, out contrastBlue);
      }

      public static RGBERROR GetColourBalance(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue)
      {
         return NativeMethods.RGBGetColourBalance(hRGB, out brightnessRed, out brightnessGreen, out brightnessBlue, out contrastRed, out contrastGreen, out contrastBlue);
      }

      public static RGBERROR SetColourBalance(IntPtr hRGB, int brightnessRed, int brightnessGreen, int brightnessBlue, int contrastRed, int contrastGreen, int contrastBlue)
      {
         return NativeMethods.RGBSetColourBalance(hRGB, brightnessRed, brightnessGreen, brightnessBlue, contrastRed, contrastGreen, contrastBlue);
      }

      public static RGBERROR GetModeInfo(IntPtr hRGB, out RGBMODEINFO modeInfo)
      {
         return NativeMethods.RGBGetModeInfo(hRGB, out modeInfo);
      }

      public static RGBERROR SetOutputSize(IntPtr hRGB, uint width, uint height)
      {
         return NativeMethods.RGBSetOutputSize(hRGB, width, height);
      }

      public static RGBERROR GetOutputSize(IntPtr hRGB, out uint width, out uint height)
      {
         return NativeMethods.RGBGetOutputSize(hRGB, out width, out height);
      }

      public static RGBERROR UseOutputBuffers(IntPtr hRGB, int enable)
      {
         return NativeMethods.RGBUseOutputBuffers(hRGB, enable);
      }

      public static RGBERROR ChainOutputBuffer(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits)
      {
         return NativeMethods.RGBChainOutputBuffer(hRGB, bitmapInfo, bitmapBits);
      }

      public static RGBERROR SetDeinterlace(IntPtr hRGB, DEINTERLACE value)
      {
         return NativeMethods.RGBSetDeinterlace(hRGB, value);
      }

      public static RGBERROR GetDeinterlace(IntPtr hRGB, out DEINTERLACE value)
      {
         return NativeMethods.RGBGetDeinterlace(hRGB, out value);
      }

      public static RGBERROR IsYUVSupported(out int value)
      {
         return NativeMethods.RGBIsYUVSupported(out value);
      }

      public static RGBERROR IsDeinterlaceSupported(out int value)
      {
         return NativeMethods.RGBIsDeinterlaceSupported(out value);
      }

      public static RGBERROR GetVideoStandard(IntPtr hRGB, out uint value)
      {
         return NativeMethods.RGBGetVideoStandard(hRGB, out value);
      }

      public static RGBERROR TestVideoStandard(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBTestVideoStandard(hRGB, value);
      }

      public static RGBERROR SetVideoStandard(IntPtr hRGB, uint value)
      {
         return NativeMethods.RGBSetVideoStandard(hRGB, value);
      }

      public static RGBERROR GetSaturationMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetSaturationMinimum(hRGB, out value);
      }

      public static RGBERROR GetSaturationMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetSaturationMaximum(hRGB, out value);
      }

      public static RGBERROR GetSaturationDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetSaturationDefault(hRGB, out value);
      }

      public static RGBERROR GetSaturation(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetSaturation(hRGB, out value);
      }

      public static RGBERROR SetSaturation(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetSaturation(hRGB, value);
      }

      public static RGBERROR GetHueMinimum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHueMinimum(hRGB, out value);
      }

      public static RGBERROR GetHueMaximum(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHueMaximum(hRGB, out value);
      }

      public static RGBERROR GetHueDefault(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHueDefault(hRGB, out value);
      }

      public static RGBERROR GetHue(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBGetHue(hRGB, out value);
      }

      public static RGBERROR SetHue(IntPtr hRGB, int value)
      {
         return NativeMethods.RGBSetHue(hRGB, value);
      }

      public static RGBERROR GetInputSignalType(uint uInput, out SIGNALTYPE signalType, out uint captureWidth, out uint captureHeight, out uint refreshRate)
      {
         return NativeMethods.RGBGetInputSignalType(uInput, out signalType, out captureWidth, out captureHeight, out refreshRate);
      }

      public static RGBERROR InputIsVGASupported(uint uInput, out int value)
      {
         return NativeMethods.RGBInputIsVGASupported(uInput, out value);
      }

      public static RGBERROR InputIsDVISupported(uint uInput, out int value)
      {
         return NativeMethods.RGBInputIsDVISupported(uInput, out value);
      }

      public static RGBERROR InputIsComponentSupported(uint uInput, out int value)
      {
         return NativeMethods.RGBInputIsComponentSupported(uInput, out value);
      }

      public static RGBERROR InputIsCompositeSupported(uint uInput, out int value)
      {
         return NativeMethods.RGBInputIsCompositeSupported(uInput, out value);
      }

      public static RGBERROR InputIsSVideoSupported(uint uInput, out int value)
      {
         return NativeMethods.RGBInputIsSVideoSupported(uInput, out value);
      }

      public static RGBERROR SetNoSignalText(string text)
      {
         return NativeMethods.RGBSetNoSignalTextW(text);
      }

      public static RGBERROR SetInvalidSignalText(string text)
      {
         return NativeMethods.RGBSetInvalidSignalTextW(text);
      }

      public static RGBERROR CreateOSD(out IntPtr hOSD)
      {
         return NativeMethods.RGBCreateOSD(out hOSD);
      }

      public static RGBERROR DeleteOSD(IntPtr hOSD)
      {
         return NativeMethods.RGBDeleteOSD(hOSD);
      }

      public static RGBERROR AttachOSD(IntPtr hRGB, IntPtr hOSD)
      {
         return NativeMethods.RGBAttachOSD(hRGB, hOSD);
      }

      public static RGBERROR DetachOSD(IntPtr hRGB, IntPtr hOSD)
      {
         return NativeMethods.RGBDetachOSD(hRGB, hOSD);
      }

      public static RGBERROR GetFirstOSD(IntPtr hRGB, out IntPtr hOSD)
      {
         return NativeMethods.RGBGetFirstOSD(hRGB, out hOSD);
      }

      public static RGBERROR GetNextOSD(IntPtr hRGB, IntPtr hOSD, out IntPtr hOSDNext)
      {
         return NativeMethods.RGBGetNextOSD(hRGB, hOSD, out hOSDNext);
      }

      public static RGBERROR GetOSDType(IntPtr hOSD, out OSD_TYPE type)
      {
         return NativeMethods.RGBGetOSDType(hOSD, out type);
      }

      public static RGBERROR SetOSDType(IntPtr hOSD, OSD_TYPE type)
      {
         return NativeMethods.RGBSetOSDType(hOSD, type);
      }

      public static RGBERROR GetOSDScaling(IntPtr hOSD, out int fixedSize)
      {
         return NativeMethods.RGBGetOSDScaling(hOSD, out fixedSize);
      }

      public static RGBERROR SetOSDScaling(IntPtr hOSD, int fixedSize)
      {
         return NativeMethods.RGBSetOSDScaling(hOSD, fixedSize);
      }

      public static RGBERROR GetOSDBackground(IntPtr hOSD, out COLORREF background, out int transparent)
      {
         return NativeMethods.RGBGetOSDBackground(hOSD, out background, out transparent);
      }

      public static RGBERROR SetOSDBackground(IntPtr hOSD, COLORREF background, int transparent)
      {
         return NativeMethods.RGBSetOSDBackground(hOSD, background.ToUint(), transparent);
      }

      public static RGBERROR GetOSDTextLength(IntPtr hOSD, out uint count)
      {
         return NativeMethods.RGBGetOSDTextLength(hOSD, out count);
      }

      public static RGBERROR GetOSDText(IntPtr hOSD, out string text, out uint count)
      {
         return NativeMethods.RGBGetOSDTextW(hOSD, out text, out count);
      }

      public static RGBERROR SetOSDText(IntPtr hOSD, string text)
      {
         return NativeMethods.RGBSetOSDTextW(hOSD, text);
      }

      public static RGBERROR GetOSDWrapping(IntPtr hOSD, out int wrapText)
      {
         return NativeMethods.RGBGetOSDWrapping(hOSD, out wrapText);
      }

      public static RGBERROR SetOSDWrapping(IntPtr hOSD, int wrapText)
      {
         return NativeMethods.RGBSetOSDWrapping(hOSD, wrapText);
      }

      public static RGBERROR GetOSDFont(IntPtr hOSD, LOGFONT font, COLORREF foreground)
      {
         return NativeMethods.RGBGetOSDFontW(hOSD, font, foreground);
      }

      public static RGBERROR SetOSDFont(IntPtr hOSD, LOGFONT font, COLORREF foreground)
      {
         return NativeMethods.RGBSetOSDFontW(hOSD, font, foreground.ToUint());
      }

      public static RGBERROR GetOSDMargins(IntPtr hOSD, out int top, out int left, out int bottom, out int right)
      {
         return NativeMethods.RGBGetOSDMargins(hOSD, out top, out left, out bottom, out right);
      }

      public static RGBERROR SetOSDMargins(IntPtr hOSD, int top, int left, int bottom, int right)
      {
         return NativeMethods.RGBSetOSDMargins(hOSD, top, left, bottom, right);
      }

      public static RGBERROR GetOSDAlignment(IntPtr hOSD, out OSD_ALIGNMENT horizontal, out OSD_ALIGNMENT vertical)
      {
         return NativeMethods.RGBGetOSDAlignment(hOSD, out horizontal, out vertical);
      }

      public static RGBERROR SetOSDAlignment(IntPtr hOSD, OSD_ALIGNMENT horizontal, OSD_ALIGNMENT vertical)
      {
         return NativeMethods.RGBSetOSDAlignment(hOSD, horizontal, vertical);
      }

      public static RGBERROR IsOSDAccelerated(out int value)
      {
         return NativeMethods.RGBIsOSDAccelerated(out value);
      }

      public static RGBERROR GetKeyColour(IntPtr hOSD, out COLORREF keyColour)
      {
         return NativeMethods.RGBGetKeyColour(hOSD, out keyColour);
      }

      public static RGBERROR SetKeyColour(IntPtr hOSD, COLORREF keyColour)
      {
         return NativeMethods.RGBSetKeyColour(hOSD, keyColour);
      }

      public static RGBERROR GetRotation(IntPtr hRGB, out ROTATIONANGLE rotationAngle)
      {
         return NativeMethods.RGBGetRotation(hRGB, out rotationAngle);
      }

      public static RGBERROR SetRotation(IntPtr hRGB, ROTATIONANGLE rotationAngle)
      {
         return NativeMethods.RGBSetRotation(hRGB, rotationAngle);
      }

      public static RGBERROR IsRotationSupported(IntPtr hRGB, out int value)
      {
         return NativeMethods.RGBIsRotationSupported(hRGB, out value);
      }
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct AudioData
   {
      public uint Size;
      public ulong StartTime;     /* Captured data start time. */
      public ulong EndTime;       /* Captured data end time. */
      public IntPtr PBuffer;       /* Pointer to the PCM bits. */
   }

   public enum AUDIOCAPTURECHANNELS
   {
      AUDIOCAPTURECHANNELS_STEREO = 2,
   }

   public enum AUDIOCAPTURESAMPLESPERSEC
   {
      AUDIOCAPTURESAMPLESPERSEC_32000 = 32000,
      AUDIOCAPTURESAMPLESPERSEC_44100 = 44100,
      AUDIOCAPTURESAMPLESPERSEC_48000 = 48000,
      AUDIOCAPTURESAMPLESPERSEC_96000 = 96000,
   }

   public enum AUDIOCAPTURESAMPLEDEPTH
   {
      AUDIOCAPTURESAMPLEDEPTH_16BPS = 16,
      AUDIOCAPTURESAMPLEDEPTH_24PS = 24,
   }

   [StructLayout(LayoutKind.Sequential, Pack = 1)]
   public struct AudioCaps
   {
      public uint Size;

      public AUDIOCAPTURECHANNELS Channels;
      public AUDIOCAPTURESAMPLESPERSEC SamplesPerSec;
      public AUDIOCAPTURESAMPLEDEPTH SampleDepth;
   }

   public enum AUDIOCAPTURESTATE
   {
      STOP,
      ACQUIRE,
   }
   public enum AUDIOCAPTURETYPE
   {
      AUDIOCAPTURETYPE_EMBEDDED = 0,
      AUDIOCAPTURETYPE_EXTERNAL
   }

   public enum AUDIOCAPTURESOURCE
   {
      AUDIOCAPTURESOURCE_SDI = 1,
      AUDIOCAPTURESOURCE_HDMI,
      AUDIOCAPTURESOURCE_ANALOGUE,
      AUDIOCAPTURESOURCE_HDMI_OR_ANALOGUE,
      AUDIOCAPTURESOURCE_SDI_OR_ANALOGUE
   }

   /// <summary>
   /// Audio callback function
   /// </summary>
   /// <param name="audioHandle">Audio capture handle</param>
   /// <param name="audioData">AudioData structure that describes the audio buffer</param>
   /// <param name="userData">An application defined value set in OpenAudioInput</param>
   public delegate void AUDIOCAPTUREDFN(IntPtr audioHandle, ref AudioData audioData, IntPtr userData);

   /// <summary>
   /// Audio wrapper for the Vision capture cards
   /// </summary>
   public class RGBAUDIO
   {
      
      /// <summary>
      /// Checks if an input supports audio.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR IsAudioSupported(uint input, out bool isSupported)
      {
         return NativeMethods.RGBAudioIsAudioSupported(input, out isSupported);
      }

      /// <summary>
      /// Checks if an input supports HDMI audio.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports HDMI audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR IsHDMISupported(uint input, out bool isSupported)
      {
         return NativeMethods.RGBAudioIsHDMISupported(input, out isSupported);
      }

      /// <summary>
      /// Checks if an input supports SDI audio.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports SDI audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR IsSDISupported(uint input, out bool isSupported)
      {
         return NativeMethods.RGBAudioIsSDISupported(input, out isSupported);
      }

      /// <summary>
      /// Checks if an input supports analogue audio.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports analogue audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR IsAnalogueSupported(uint input, out bool isSupported)
      {
         return NativeMethods.RGBAudioIsAnalogueSupported(input, out isSupported);
      }

      /// <summary>
      /// Returns the number of supported audio formats for an input.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="count">Set to the number of avalaible formats.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetCapabilitiesCount(uint input, out uint count)
      {
         return NativeMethods.RGBAudioGetCapabilitiesCount(input, out count);
      }

      /// <summary>
      /// Returns a specfic audio format.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="index">The audio format index.</param>
      /// <param name="capability">An AudioCaps struct for audio format <paramref name="index"/>.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetCapabilities(uint input, uint index, ref AudioCaps capability)
      {
         return NativeMethods.RGBAudioGetCapabilities(input, index, ref capability);
      }

      /// <summary>
      /// Sets a specfic audio format.
      /// </summary>
      /// <param name="input">The input number.</param>
      /// <param name="index">The audio format index.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetCapabilities(uint input, uint index)
      {
         return NativeMethods.RGBAudioSetCapabilities(input, index);
      }

      /// <summary>
      /// Opens a RGB Input for Audio capture
      /// </summary>
      /// <param name="callback">Specifies the audio callback function for applications that capture
      /// audio through <code>ChainOutputBuffer</code>. Applications that capture audio using
      /// <code>LoadOutputBuffer</code> should set <paramref name="callback"/> to zero</param>
      /// <param name="userData">An application defined value that is passed to the <paramref name="callback"/> audio callback function </param>
      /// <param name="input">The input number.</param>
      /// <param name="audioHandle">Set to a handle that identifes the audio capture.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR OpenInput(AUDIOCAPTUREDFN callback, IntPtr userData, uint input, out IntPtr audioHandle)
      {
         return NativeMethods.RGBAudioOpenInput(callback, userData, input, out audioHandle);
      }

      /// <summary>
      /// Closes a Audio capture.
      /// </summary>
      /// <param name="audioHandle">Audio capture handle.</param>
      /// <returns></returns>
      public static RGBERROR CloseInput(IntPtr audioHandle)
      {
         return NativeMethods.RGBAudioCloseInput(audioHandle);
      }

      /// <summary>
      /// Adds an audio buffer to a driver managed queue. Multiple buffers can added
      /// through repeated calls calls to this function. Filled audio buffers are
      /// returned through the audio callback function specifed in <code>OpenAudioInput</code>
      /// </summary>
      /// <param name="audioHandle">Audio capture handle.</param>
      /// <param name="bufferLength">Number of <b>bytes</b> of audio to capture into the audio buffer.</param>
      /// <param name="bufferPointer">Pointer to a pinned audio buffer.</param>
      /// <example>
      /// var audioBuffer = new byte[1024];
      /// var pinnedHandle = GCHandle.Alloc(audioBuffer, GCHandleType.Pinned);
      /// Intptr bufferPointer = pinnedHandle.AddrOfPinnedObject()
      /// </example>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR ChainOutputBuffer(IntPtr audioHandle, uint bufferLength, IntPtr bufferPointer)
      {
         return NativeMethods.RGBAudioChainOutputBuffer(audioHandle, bufferLength, 0, bufferPointer);
      }

      /// <summary>
      /// Fills a buffer with avaialble audio data. 
      /// </summary>
      /// <param name="audioHandle">Audio capture handle</param>
      /// <param name="bufferLength">Number of <b>bytes</b> of audio to capture into the audio buffer.</param>
      /// <param name="bytesWritten">Number of <b>bytes</b> of audio written to the buffer.
      /// This could be less than the <paramref name="bufferLength"/> and could be zero if no additional audio has beed
      /// captured since the last call to <code>LoadOutputBuffer</code>. Unused byes in the buffer will be set to zero</param>
      /// <param name="bufferPointer">Pointer to a pinned audio buffer.</param>
      /// <example>
      /// var audioBuffer = new byte[1024];
      /// var pinnedHandle = GCHandle.Alloc(audioBuffer, GCHandleType.Pinned);
      /// Intptr bufferPointer = pinnedHandle.AddrOfPinnedObject()
      /// </example>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR LoadOutputBuffer(IntPtr audioHandle, uint bufferLength, ref uint bytesWritten, IntPtr bufferPointer)
      {
         return NativeMethods.RGBAudioLoadOutputBuffer(audioHandle, bufferLength, 0, ref bytesWritten, bufferPointer);
      }

      /// <summary>
      /// Removes audio all audio buffers from the driver managed queue.
      /// The audio callback function will not be called for these buffers.
      /// </summary>
      /// <param name="audioHandle">Audio capture handle.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR ReleaseOutputBuffers(IntPtr audioHandle)
      {
         return NativeMethods.RGBAudioReleaseOutputBuffers(audioHandle);
      }

      /// <summary>
      /// Sets the state of an audio capture.
      /// </summary>
      /// <param name="audioHandle">Audio capture handle.</param>
      /// <param name="state">The AUDIOCAPTURESTATE value to set.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetState(IntPtr audioHandle, AUDIOCAPTURESTATE state)
      {
         return NativeMethods.RGBAudioSetState(audioHandle, state);
      }

      /// <summary>
      /// Gets the state of an audio capture.
      /// </summary>
      /// <param name="audioHandle">Audio capture handle.</param>
      /// <param name="state">The AUDIOCAPTURESTATE value set</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetState(IntPtr audioHandle, out AUDIOCAPTURESTATE state)
      {
         return NativeMethods.RGBAudioGetState(audioHandle, out state);
      }

      /// <summary>
      /// Sets the audio capture type. 
      /// The capture type cannot be set with audio running.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="type">The AUDIOCAPTURETYPE value to set.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetCaptureType(uint input, AUDIOCAPTURETYPE type)
      {
         return NativeMethods.RGBAudioSetCaptureType(input, type);
      }

      /// <summary>
      /// Gets the current audio capture type.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="type">The current AUDIOCAPTURETYPE.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetCaptureType(uint input, out AUDIOCAPTURETYPE type)
      {
         return NativeMethods.RGBAudioGetCaptureType(input, out type);
      }

      /// <summary>
      /// Sets which digital channel pair to capture.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="pair">The channel pair.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetDigitalChannelPair(uint input, uint pair)
      {
         return NativeMethods.RGBAudioSetDigitalChannelPair(input, pair);
      }

      /// <summary>
      /// Gets the current digital channel pair.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="pair">The channel pair.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetDigitalChannelPair(uint input, out uint pair)
      {
         return NativeMethods.RGBAudioGetDigitalChannelPair(input, out pair);
      }

      /// <summary>
      /// Mutes or unmutes the digital audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">set true to Mute, false to unmute.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetDigitalMute(uint input, bool mute)
      {
         return NativeMethods.RGBAudioSetDigitalMute(input, mute);
      }

      /// <summary>
      /// Gets the current digital channel mute value.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">mute value.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetDigitalMute(uint input, out bool mute)
      {
         return NativeMethods.RGBAudioGetDigitalMute(input, out mute);
      }

      /// <summary>
      /// Sets the analogue balanced (XLR) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetAnalogueBalancedGain(uint input, int gain)
      {
         return NativeMethods.RGBAudioSetAnalogueBalancedGain(input, gain);
      }

      /// <summary>
      /// Gets the analogue balanced (XLR) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGain(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGain(input, out gain);
      }

      /// <summary>
      /// Gets the minimum analogue balanced (XLR) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGainMinimum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGainMinimum(input, out gain);
      }

      /// <summary>
      /// Gets the maximum analogue balanced (XLR) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGainMaximum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGainMaximum(input, out gain);
      }

      /// <summary>
      /// Gets the default analogue balanced (XLR) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGainDefault(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGainDefault(input, out gain);
      }

      /// <summary>
      /// Gets the scale factor for the balanced (XLR) gain used to convert the gain value to decibels.
      /// dB = (val * scale) / 1000
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGainScale(uint input, out uint scale)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGainScale(input, out scale);
      }

      /// <summary>
      /// Sets the analogue balanced (XLR) gain boost.
      /// When enabled +20dB of gain is added into the balanced audio path.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetAnalogueBalancedGainBoost(uint input, bool boost)
      {
         return NativeMethods.RGBAudioSetAnalogueBalancedGainBoost(input, boost);
      }

      /// <summary>
      /// Gets the analogue balanced (XLR) gain boost.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedGainBoost(uint input, out bool boost)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedGainBoost(input, out boost);
      }

      /// <summary>
      /// Mutes or unmutes the analogue balanced (XLR) audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">set true to Mute, false to unmute.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetAnalogueBalancedMute(uint input, bool mute)
      {
         return NativeMethods.RGBAudioSetAnalogueBalancedMute(input, mute);
      }

      /// <summary>
      /// Gets the current analogue balanced (XLR) mute value.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">mute value.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueBalancedMute(uint input, out bool mute)
      {
         return NativeMethods.RGBAudioGetAnalogueBalancedMute(input, out mute);
      }

      /// <summary>
      /// Sets the analogue balanced (RCA) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetAnalogueUnbalancedGain(uint input, int gain)
      {
         return NativeMethods.RGBAudioSetAnalogueUnbalancedGain(input, gain);
      }

      /// <summary>
      /// Gets the analogue Unbalanced (RCA) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedGain(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedGain(input, out gain);
      }

      /// <summary>
      /// Gets the minimum analogue Unbalanced (RCA) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedGainMinimum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedGainMinimum(input, out gain);
      }

      /// <summary>
      /// Gets the maximum analogue Unbalanced (RCA) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedGainMaximum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedGainMaximum(input, out gain);
      }

      /// <summary>
      /// Gets the default analogue Unbalanced (RCA) gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedGainDefault(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedGainDefault(input, out gain);
      }

      /// <summary>
      /// Gets the scale factor for the Unbalanced (RCA) gain used to convert the gain value to decibels.
      /// dB = (val * scale) / 1000
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedGainScale(uint input, out uint scale)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedGainScale(input, out scale);
      }

      /// <summary>
      /// Mutes or unmutes the analogue Unbalanced (RCA) audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">set true to Mute, false to unmute.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetAnalogueUnbalancedMute(uint input, bool mute)
      {
         return NativeMethods.RGBAudioSetAnalogueUnbalancedMute(input, mute);
      }

      /// <summary>
      /// Gets the current analogue Unbalanced (RCA) mute value.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">mute value.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAnalogueUnbalancedMute(uint input, out bool mute)
      {
         return NativeMethods.RGBAudioGetAnalogueUnbalancedMute(input, out mute);
      }

      /// <summary>
      /// Sets the ADC gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetADCGain(uint input, int gain)
      {
         return NativeMethods.RGBAudioSetADCGain(input, gain);
      }

      /// <summary>
      /// Gets the ADC gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCGain(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetADCGain(input, out gain);
      }

      /// <summary>
      /// Gets the minimum ADC gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCGainMinimum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetADCGainMinimum(input, out gain);
      }

      /// <summary>
      /// Gets the maximum value for the ADC gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCGainMaximum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetADCGainMaximum(input, out gain);
      }

      /// <summary>
      /// Gets the default value for the ADC gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCGainDefault(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetADCGainDefault(input, out gain);
      }

      /// <summary>
      /// Gets the scale factor for the ADC used to convert the gain value to decibels.
      /// dB = (val * scale) / 1000
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCGainScale(uint input, out uint scale)
      {
         return NativeMethods.RGBAudioGetADCGainScale(input, out scale);
      }

      /// <summary>
      /// Mutes or unmutes the ADC audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">set true to Mute, false to unmute.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetADCMute(uint input, bool mute)
      {
         return NativeMethods.RGBAudioSetADCMute(input, mute);
      }

      /// <summary>
      /// Gets the current ADC mute value.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">mute value.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetADCMute(uint input, out bool mute)
      {
         return NativeMethods.RGBAudioGetADCMute(input, out mute);
      }

      /// <summary>
      /// Checks if an input supports line out audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports line out audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR IsLineOutSupported(uint input, out bool isSupported)
      {
         return NativeMethods.RGBAudioIsLineOutSupported(input, out isSupported);
      }

      /// <summary>
      /// Sets the line out audio source.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports line out audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetLineOutSource(uint input, AUDIOCAPTURESOURCE source)
      {
         return NativeMethods.RGBAudioSetLineOutSource(input, source);
      }

      /// <summary>
      /// Gets the line out audio source.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="isSupported">Set to <c>true</c> if the input supports line out audio.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutSource(uint input, out AUDIOCAPTURESOURCE source)
      {
         return NativeMethods.RGBAudioGetLineOutSource(input, out source);
      }

      /// <summary>
      /// Sets the Line Out gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetLineOutGain(uint input, int gain)
      {
         return NativeMethods.RGBAudioSetLineOutGain(input, gain);
      }

      /// <summary>
      /// Gets the Line Out gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutGain(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetLineOutGain(input, out gain);
      }

      /// <summary>
      /// Gets the minimum Line Out gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutGainMinimum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetLineOutGainMinimum(input, out gain);
      }

      /// <summary>
      /// Gets the maximum Line Out gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutGainMaximum(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetLineOutGainMaximum(input, out gain);
      }

      /// <summary>
      /// Gets the default Line Out gain.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutGainDefault(uint input, out int gain)
      {
         return NativeMethods.RGBAudioGetLineOutGainDefault(input, out gain);
      }

      /// <summary>
      /// Gets the default Line Out scale factor.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="gain">gain.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutGainScale(uint input, out uint scale)
      {
         return NativeMethods.RGBAudioGetLineOutGainScale(input, out scale);
      }

      /// <summary>
      /// Mutes or unmutes the Line Out audio.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">set true to Mute, false to unmute.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SetLineOutMute(uint input, bool mute)
      {
         return NativeMethods.RGBAudioSetLineOutMute(input, mute);
      }

      /// <summary>
      /// Gets the current Line Out mute value.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="mute">mute value.</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetLineOutMute(uint input, out bool mute)
      {
         return NativeMethods.RGBAudioGetLineOutMute(input, out mute);
      }

      /// <summary>
      /// Saves the audio configuration of an input to the Windows Registry so that
      /// it persists acrooss reboots.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR SaveConfigurationToRegistry(uint input)
      {
         return NativeMethods.RGBAudioSaveConfigurationToRegistry(input);
      }

      /// <summary>
      /// Delete the audio configuration of an input from the Windows Registry
      /// </summary>
      /// <param name="input">The input number</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR DeleteRegistryConfiguration(uint input)
      {
         return NativeMethods.RGBAudioDeleteRegistryConfiguration(input);
      }

      /// <summary>
      /// Opens a WASAPI session and starts playing audio from the selected input to the default audio device
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="audioSession">Handle to the WASAPI audio session</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR OpenSharedAudio(uint input, out IntPtr audioSession)
      {
         return NativeMethods.OpenSharedAudio(input, out audioSession);
      }

      /// <summary>
      /// Stops playing audio and and closes the WASAPI session.
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="audioSession">Handle to the WASAPI audio session</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR CloseSharedAudio(IntPtr audioSession)
      {
         return NativeMethods.CloseSharedAudio(audioSession);
      }

      /// <summary>
      /// Returns the audio configuration of the WASAPI session.
      /// The index returned can be passed into the RGBAUDIO.GetCapabilities function to
      /// retrieve the sample rate and sample depth
      /// </summary>
      /// <param name="input">The input number</param>
      /// <param name="audioSession">Handle to the WASAPI audio session</param>
      /// <returns>RGBERROR.</returns>
      public static RGBERROR GetAudioFormatIndex(IntPtr audioSession, out uint index)
      {
         return NativeMethods.GetAudioFormatIndex(audioSession, out index);
      }
   }

   internal static class NativeMethods
   {
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBLoad(out IntPtr hRGBDLL);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBLoadEx(out IntPtr hRGBDLL, CAPTURECARD captureCard);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBFree(IntPtr hRGBDLL);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureCard(out CAPTURECARD captureCard);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetNumberOfInputs(out uint NumberOfInputs);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetInputInfo(uint uInput, [In, Out] ref RGBINPUTINFO inputInfo);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBOpenInput(uint uInput, out IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBCloseInput(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBCloseInputs(IntPtr phRGBArray, uint uInputs);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBDetectInput(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetInput(IntPtr hRGB, uint uInput);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetInput(IntPtr hRGB, out uint uInput);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetWindow(IntPtr hRGB, IntPtr hWnd);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetWindow(IntPtr hRGB, out IntPtr hWnd);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBResetCapture(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorPositionMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorPositionMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorPositionDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorPosition(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetHorPosition(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorScaleMinimum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorScaleMaximum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorScaleDefault(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHorScale(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetHorScale(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureWidthMinimum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureWidthMaximum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureWidthDefault(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureWidth(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBTestCaptureWidth(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetCaptureWidth(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetVerPositionMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetVerPositionMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetVerPositionDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetVerPosition(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetVerPosition(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureHeightMinimum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureHeightMaximum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureHeightDefault(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureHeight(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetCaptureHeight(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBrightnessMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBrightnessMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBrightnessDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBrightness(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetBrightness(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetContrastMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetContrastMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetContrastDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetContrast(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetContrast(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBlackLevelMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBlackLevelMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBlackLevelDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetBlackLevel(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetBlackLevel(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetPhaseMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetPhaseMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetPhaseDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetPhase(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetPhase(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFrameDroppingMinimum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFrameDroppingMaximum(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFrameDroppingDefault(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFrameDropping(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetFrameDropping(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFrameRate(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsCroppingEnabled(IntPtr hRGB, out uint bEnabled);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBEnableCropping(IntPtr hRGB, uint bEnable);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCroppingMinimum(IntPtr hRGB, out int top, out int left, out uint width, out uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCroppingMaximum(IntPtr hRGB, out int top, out int left, out uint width, out uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCroppingDefault(IntPtr hRGB, out int top, out int left, out uint width, out uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCropping(IntPtr hRGB, out int top, out int left, out uint width, out uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBTestCropping(IntPtr hRGB, int top, int left, uint width, uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetCropping(IntPtr hRGB, int top, int left, uint width, uint height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBPauseCapture(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBResumeCapture(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetCaptureState(IntPtr hRGB, out CAPTURESTATE captureState);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetMessageDelay(IntPtr hRGB, out int bShowMessages, out uint delay);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetMessageDelay(IntPtr hRGB, int bShowMessages, uint delay);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetPixelFormat(IntPtr hRGB, out PIXELFORMAT value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetPixelFormat(IntPtr hRGB, PIXELFORMAT value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Unicode)]
      internal static extern RGBERROR RGBSaveCurrentFrameW(IntPtr hRGB, string fileName);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsDirectDMASupported(out int bIsSupported);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetDMADirect(IntPtr hRGB, out int bDMADirect);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetDMADirect(IntPtr hRGB, int bDMADirect);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetDownScaling(IntPtr hRGB, out int fastScaling);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetDownScaling(IntPtr hRGB, int fastScaling);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetFrameCapturedFn(IntPtr hRGB, RGBFRAMECAPTUREDFN pFrameCapturedFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBDrawFrame(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Unicode)]
      internal static extern RGBERROR RGBSaveBitmapW(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits, string filename);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetModeChangedFn(IntPtr hRGB, RGBMODECHANGEDFN pModeChangedFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetNoSignalFn(IntPtr hRGB, RGBNOSIGNALFN pNoSignalFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBNoSignal(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetDrawNoSignalFn(IntPtr hRGB, RGBDRAWNOSIGNALFN pDrawFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetInvalidSignalFn(IntPtr hRGB, RGBINVALIDSIGNALFN pInvalidSignalFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInvalidSignal(IntPtr hRGB, uint horClock, uint verClock);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetDrawInvalidSignalFn(IntPtr hRGB, RGBDRAWINVALIDSIGNALFN pDrawFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetErrorFn(IntPtr hRGB, RGBERRORFND pErrorFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetValueChangedFn(IntPtr hRGB, RGBVALUECHANGEDFN pValueChangedFn, IntPtr userData);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBStartCapture(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBStopCapture(IntPtr hRGB);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetColourBalanceMinimum(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetColourBalanceMaximum(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetColourBalanceDefault(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetColourBalance(IntPtr hRGB, out int brightnessRed, out int brightnessGreen, out int brightnessBlue, out int contrastRed, out int contrastGreen, out int contrastBlue);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetColourBalance(IntPtr hRGB, int brightnessRed, int brightnessGreen, int brightnessBlue, int contrastRed, int contrastGreen, int contrastBlue);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetModeInfo(IntPtr hRGB, out RGBMODEINFO modeInfo);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOutputSize(IntPtr hRGB, uint width, uint Height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOutputSize(IntPtr hRGB, out uint width, out uint Height);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBUseOutputBuffers(IntPtr hRGB, int enable);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBChainOutputBuffer(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetDeinterlace(IntPtr hRGB, DEINTERLACE value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetDeinterlace(IntPtr hRGB, out DEINTERLACE value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsYUVSupported(out int supported);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsDeinterlaceSupported(out int supported);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetVideoStandard(IntPtr hRGB, out uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBTestVideoStandard(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetVideoStandard(IntPtr hRGB, uint value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetSaturationMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetSaturationMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetSaturationDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetSaturation(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetSaturation(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHueMinimum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHueMaximum(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHueDefault(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetHue(IntPtr hRGB, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetHue(IntPtr hRGB, int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetInputSignalType(uint uInput, out SIGNALTYPE signalType, out uint captureWidth, out uint captureHeight, out uint refreshRate);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInputIsVGASupported(uint uInput, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInputIsDVISupported(uint uInput, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInputIsComponentSupported(uint uInput, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInputIsCompositeSupported(uint uInput, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBInputIsSVideoSupported(uint uInput, out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetNoSignalTextW(string text);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetInvalidSignalTextW(string text);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBCreateOSD(out IntPtr hOSD);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBDeleteOSD(IntPtr hOSD);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAttachOSD(IntPtr hRGB, IntPtr hOSD);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBDetachOSD(IntPtr hRGB, IntPtr hOSD);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetFirstOSD(IntPtr hRGB, out IntPtr hOSD);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetNextOSD(IntPtr hRGB, IntPtr hOSD, out IntPtr hOSDNext);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDType(IntPtr hOSD, out OSD_TYPE type);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDType(IntPtr hOSD, OSD_TYPE type);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDScaling(IntPtr hOSD, out int fixedSize);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDScaling(IntPtr hOSD, int fixedSize);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDBackground(IntPtr hOSD, out COLORREF background, out int transparent);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDBackground(IntPtr hOSD, uint background, int transparent);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDTextLength(IntPtr hOSD, out uint count);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDTextW(IntPtr hOSD, out string text, out uint count);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDTextW(IntPtr hOSD, string text);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDWrapping(IntPtr hOSD, out int wrapText);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDWrapping(IntPtr hOSD, int wrapText);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDFontW(IntPtr hOSD, [In, Out] LOGFONT font, COLORREF foreground);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDFontW(IntPtr hOSD, [In, MarshalAs(UnmanagedType.LPStruct)] LOGFONT font, uint foreground);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDMargins(IntPtr hOSD, out int top, out int left, out int bottom, out int right);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDMargins(IntPtr hOSD, int top, int left, int bottom, int right);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetOSDAlignment(IntPtr hOSD, out OSD_ALIGNMENT horizontal, out OSD_ALIGNMENT vertical);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetOSDAlignment(IntPtr hOSD, OSD_ALIGNMENT horizontal, OSD_ALIGNMENT vertical);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsOSDAccelerated(out int value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetKeyColour(IntPtr hOSD, out COLORREF keyColour);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetKeyColour(IntPtr hOSD, COLORREF keyColour);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBGetRotation(IntPtr hRGB, out ROTATIONANGLE rotationAngle);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBSetRotation(IntPtr hRGB, ROTATIONANGLE rotationAngle);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBIsRotationSupported(IntPtr hRGB, out int value);
      
      //Audio
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioIsAudioSupported(uint input, out bool value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioIsHDMISupported(uint input, out bool value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioIsSDISupported(uint input, out bool value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioIsAnalogueSupported(uint input, out bool value);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetCapabilitiesCount(uint input, out uint count);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetCapabilities(uint input, uint index, ref AudioCaps pAudioCaps);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetCapabilities(uint input, uint index);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioOpenInput(AUDIOCAPTUREDFN pNotifyFn, IntPtr notifyArg, uint input, out IntPtr pAudioHandle);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioCloseInput(IntPtr audioHandle);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioChainOutputBuffer(IntPtr audioHandle, uint numberOfBytes, uint bufferPitch, IntPtr pBuffer);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioLoadOutputBuffer(IntPtr audioHandle, uint numberOfBytes, uint bufferPitch, ref uint pBytesWritten, IntPtr pBuffer);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioReleaseOutputBuffers(IntPtr audioHandle);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetState(IntPtr audioHandle, AUDIOCAPTURESTATE state);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetState(IntPtr audioHandle, out AUDIOCAPTURESTATE pState);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetCaptureType(uint input, AUDIOCAPTURETYPE type);

      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetCaptureType(uint input, out AUDIOCAPTURETYPE pType);

      // DIGITAL
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetDigitalChannelPair(uint input, uint pair);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetDigitalChannelPair(uint input, out uint pair);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetDigitalMute(uint input, bool mute);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetDigitalMute(uint input, out bool mute);

      // BALANCED 
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetAnalogueBalancedGain(uint input, int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGain(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGainMinimum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGainMaximum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGainDefault(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGainScale(uint input, out uint scale);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetAnalogueBalancedGainBoost(uint input, bool boost);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedGainBoost(uint input, out bool boost);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetAnalogueBalancedMute(uint input, bool mute);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueBalancedMute(uint input, out bool mute);

      // UNBALANCED 
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetAnalogueUnbalancedGain(uint input, int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedGain(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedGainMinimum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedGainMaximum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedGainDefault(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedGainScale(uint input, out uint scale);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetAnalogueUnbalancedMute(uint input, bool mute);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetAnalogueUnbalancedMute(uint input, out bool mute);

      // ADC 
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetADCGain(uint input, int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCGain(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCGainMinimum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCGainMaximum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCGainDefault(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCGainScale(uint input, out uint scale);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetADCMute(uint input, bool mute);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetADCMute(uint input, out bool mute);

      // Line Out
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioIsLineOutSupported(uint input, out bool isSupported);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetLineOutSource(uint input, AUDIOCAPTURESOURCE source);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutSource(uint input, out AUDIOCAPTURESOURCE source);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetLineOutGain(uint input, int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutGain(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutGainMinimum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutGainMaximum(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutGainDefault(uint input, out int gain);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutGainScale(uint input, out uint scale);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSetLineOutMute(uint input, bool mute);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioGetLineOutMute(uint input, out bool mute);

      // Registry
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioSaveConfigurationToRegistry(uint input);
      [DllImport("rgbeasy.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR RGBAudioDeleteRegistryConfiguration(uint input);


      // WASPI
      [DllImport("rgbwasapi.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR OpenSharedAudio(uint input, out IntPtr audioSession);

      [DllImport("rgbwasapi.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR CloseSharedAudio(IntPtr audioSession);

      [DllImport("rgbwasapi.dll", CharSet = CharSet.Auto)]
      internal static extern RGBERROR GetAudioFormatIndex(IntPtr audioSession, out uint index);
   }

   public enum RGBERROR
   {
      // RGBERROR.H
      NO_ERROR = 0,
      DRIVER_NOT_FOUND = 0x00010000,
      UNABLE_TO_LOAD_DRIVER = 0x00010001,
      HARDWARE_NOT_FOUND = 0x00010002,
      INVALID_INDEX = 0x00010003,
      DEVICE_IN_USE = 0x00010004,
      INVALID_HRGBCAPTURE = 0x00010005,
      INVALID_POINTER = 0x00010006,
      INVALID_SIZE = 0x00010007,
      INVALID_FLAGS = 0x00010008,
      INVALID_DEVICE = 0x00010009,
      INVALID_INPUT = 0x0001000A,
      INVALID_FORMAT = 0x0001000D,
      INVALID_VDIF_CLOCKS = 0x0001000E,
      INVALID_PHASE = 0x00010011,
      INVALID_BRIGHTNESS = 0x00010013,
      INVALID_CONTRAST = 0x00010014,
      INVALID_BLACKLEVEL = 0x00010015,
      INVALID_SAMPLERATE = 0x00010016,
      INVALID_PIXEL_FORMAT = 0x00010017,
      INVALID_HWND = 0x00010018,
      INSUFFICIENT_RESOURCES = 0x00010019,
      INSUFFICIENT_BUFFERS = 0x0001001A,
      INSUFFICIENT_MEMORY = 0x0001001B,
      SIGNAL_NOT_DETECTED = 0x0001001C,
      INVALID_SYNCEDGE = 0x0001001D,
      OLD_FIRMWARE = 0x0001001E,
      HWND_AND_FRAMECAPTUREDFN = 0x0001001F,
      HSCALED_OUT_OF_RANGE = 0x00010020,
      VSCALED_OUT_OF_RANGE = 0x00010021,
      SCALING_NOT_SUPPORTED = 0x00010022,
      BUFFER_TOO_SMALL = 0x00010023,
      HSCALE_NOT_WORD_DIVISIBLE = 0x00010024,
      HSCALE_NOT_DWORD_DIVISIBLE = 0x00010025,
      HORADDRTIME_NOT_WORD_DIVISIBLE = 0x00010026,
      HORADDRTIME_NOT_DWORD_DIVISIBLE = 0x00010027,
      VERSION_MISMATCH = 0x00010028,
      ACC_REALLOCATE_BUFFERS = 0x00010029,
      BUFFER_NOT_VALID = 0x0001002A,
      BUFFERS_STILL_ALLOCATED = 0x0001002B,
      NO_NOTIFICATION_SET = 0x0001002C,
      CAPTURE_DISABLED = 0x0001002D,
      INVALID_PIXELFORMAT = 0x0001002E,
      ILLEGAL_CALL = 0x0001002F,
      CAPTURE_OUTSTANDING = 0x00010030,
      MODE_NOT_FOUND = 0x00010031,
      CANNOT_DETECT = 0x00010032,
      NO_MODE_DATABASE = 0x00010033,
      CANT_DELETE_MODE = 0x00010034,
      MUTEX_FAILURE = 0x00010035,
      THREAD_FAILURE = 0x00010036,
      NO_COMPLETION = 0x00010037,
      INSUFFICIENT_RESOURCES_HALLOC = 0x00010038,
      INSUFFICIENT_RESOURCES_RGBLIST = 0x00010039,
      DEVICE_NOT_READY = 0x0001003a,
      HORADDRTIME_NOT_QWORD_DIVISIBLE = 0x0001003b,
      HSCALE_NOT_QWORD_DIVISIBLE = 0x0001003c,
      AOI_NOT_QWORD_ALIGNED = 0x0001003d,
      AOI_NOT_DWORD_ALIGNED = 0x0001003e,
      INVALID_HTIMINGS = 0x0001003f,
      INVALID_PITCH = 0x00010040,
      INVALID_PIXELCOUNT = 0x00010041,
      FLASH_ONLY = 0x00010042,
      CPU_CODE_LOAD_FAILED = 0x00010043,
      IRQ_LINE_FAILED = 0x00010044,
      CPU_CODE_LOAD_ERROR = 0x00010045,
      CPU_STARTUP_UNSIGNALLED = 0x00010046,
      HW_INTEGRITY_VIOLATION = 0x00010047,
      MEMORY_TEST_FAILED = 0x00010048,
      PCI_BUS_FAILURE = 0x00010049,
      CPU_NO_HANDSHAKE = 0x0001004a,
      INVALID_VTIMINGS = 0x0001004b,
      INVALID_ENVIRONMENT = 0x0001004c,
      FILE_NOT_FOUND = 0x0001004d,
      INVALID_GAIN = 0x0001004e,
      INVALID_OFFSET = 0x0001004f,
      CANT_ADJUST_DVI = 0x00010050,
      INCOMPATIBLE_INTERFACE = 0x00010051,
      FLASH_INPROGRESS = 0x00010052,
      INVALID_SATURATION = 0x00010053,
      INVALID_HUE = 0x00010054,
      INVALID_VIDEOSTANDARD = 0x00010055,
      INVALID_DEINTERLACE = 0x00010056,
      ROTATION_NOT_SUPPORTED = 0x00010057,
      INVALID_ROTATION_ANGLE = 0x00010058,
      INVALID_EDID = 0x0001005C,
      EDID_NOT_SUPPORTED = 0x00010059,
      EDID_VERIFY = 0x0001005D,
      EDID_I2C_STUCK = 0x0001005E,
      EDID_I2C_NOACK = 0x0001005F,
      INVALID_EQUALISATION = 0x00010060,
      INTERNAL_DEVICE_ERROR = 0x00010061,
      INVALID_HWINDOW = 0x00010061,
      NOACCESS = 0x00010063,
      NOT_INITIALISED = 0x00010064,
      INVALID_HANDLE = 0x00010065,
      NO_AUDIO_AVAILABLE = 0x00010066,
      ALREADY_ACTIVE = 0x00010067,
      DIFFERENT_GPUDESCRIPTOR = 0x00010068,
      INVALID_COLOURDOMAIN = 0x00010069,
      OFFLINE_LOAD_FAILURE = 0x00010070,
      OFFLINE_LOAD_ABORTED = 0x00010071,
      INVALID_LIVESTREAM = 0x00010072,
      INVALID_LINKRATE = 0x00010073,
      INVALID_STATE = 0x00010074,
      INPUT_UNAVAILABLE = 0x00010075,
      KEY_VERIFY_FAIL = 0x00010076,
      KEY_ALREADY_PRESENT = 0x00010077,
      OVERLAY_UNSUPPORTED = 0x00010078,
      OVERLAY_INVALID_TIMEOUT = 0x00010079,
      GANGING_UNSUPPORTED = 0x0001007A,
      GANGINGTYPE_UNSUPPORTED = 0x0001007B,
      HDCP_NOT_SUPPORTED = 0x0001007C,
      INVALID_PIXEL_GAMUT = 0x0001007D,
      INVALID_PIXEL_RANGE = 0x0001007E,
      AUDIOCAPTURETYPE_NOT_ACTIVE = 0x0001007F,
      AUDIOCAPTURETYPE_UNSUPPORTED = 0x00010080,
      OVERLAY_INVALID_OFFSET = 0x00010081,
      OUTPUT_UNSUPPORTED = 0x00010082,
      OUTPUT_NOTMAPPED = 0x00010083,
      OUTPUT_CONVERT_ASPECT_UNSUPPORTED = 0x00010084,
      UPGRADE_INPROGRESS = 0x00010085,
      VSCALE_NOT_EVEN = 0x00010086,
      SIGNAL_OUT_OF_RANGE = 0x00010087,


      // RGB.H
      UNKNOWN = 0x01160000,
      OUTOFRANGE = 0x01160001,
      INVALIDINPUT = 0x01160002,
      WINDOWINUSE = 0x01160003,
      OSDATTACHED = 0x01160004,
      DETECTVIDEOMODE = 0x01160005,
      INVALIDBUFFER = 0x01160006,
      INVALIDHANDLE = 0x01160007,
      UNSUPPORTED = 0x01160008,
      INVALIDOSD = 0x01160009,
      INVALIDDATA = 0x0116000A,
      AUDIOINPUT = 0x0116000B,

      // HANDLE.H
      HNDLTBLE_INVALID_SIZE = 0x01170001,
      HNDLTBLE_ALREADY_INITIALIZED = 0x01170002,
      HNDLTBLE_INVALID_HANDLE = 0x01170003,
      HNDLTBLE_LOCK_VIOLATION = 0x01170004,
      HNDLTBLE_NOT_ENOUGH_MEMORY = 0x01170005,
      HNDLTBLE_ILLEGAL_VALUE = 0x01170006,
      HNDLTBLE_NO_RESOURCES = 0x01170007,
      HNDLTBLE_PTR_MISMATCH = 0x01170008,

      // API.H
      API_ERROR_UNABLE_TO_OPEN_KEY = 0x00050000,
      API_ERROR_UNABLE_TO_READ_VALUE = 0x00050001,
      API_ERROR_UNABLE_TO_LOAD_DLL = 0x00050002,
      API_ERROR_INCOMPATIBLE_API = 0x00050003,
   }
}
