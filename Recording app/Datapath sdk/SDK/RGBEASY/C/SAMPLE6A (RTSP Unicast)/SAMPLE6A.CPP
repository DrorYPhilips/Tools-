/*******************************************************************************
Copyright Datapath Ltd. 2015.

File:    Sample6A.cpp

History: 30 OCT 14    RL   Created.
         12 OCT 15    DC   Added further hard-coded URLs.
                           Included GOP Length as an option in URLs.

*******************************************************************************/

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include <string>
#include <sstream>

#include <rgb.h>
#include <rgbapi.h>
#include <rgbh264nal.h>
#include <rgberror.h>
#include <stdio.h>
#include <tchar.h>

//#define _LOCAL_HEAP_DEBUG
#ifdef _LOCAL_HEAP_DEBUG
#include <CRTDBG.h>
#endif

/******************************************************************************/

// New: A couple of helper classes to make lifetime management of the Live555
// classes somewhat more intuitive, and allows the application to exit without
// leaving memory like a sieve - which would be A Bad Thing if this application
// ever forms the backbone of the StreamServer API ;)
// This is round 1 of the C++ cleanup.
class BTS : public BasicTaskScheduler
{
public:
   BTS( ) : BasicTaskScheduler( 10000 )
   {
   }
   BTS( unsigned granularity ) : BasicTaskScheduler( granularity )
   {
   }
   ~BTS( )
   {
   }
};

class BUE : public BasicUsageEnvironment
{
public:
   BUE( BTS & Scheduler ):BasicUsageEnvironment( Scheduler )
   {
      // We know we're passed a dereferenced new'd object,
      // so store a pointer to it internally, and then
      // we can free it at BUE destruction time.
      m_pScheduler = &Scheduler;
   }
   ~BUE( )
   {
      if ( m_pScheduler )
      {
         delete m_pScheduler;
      }
   }
   BUE & operator << ( const char * pString )
   {
      if ( pString )
      {
         printf( "%s", pString );
      }
      return *this;
   }
   BUE & operator << ( const WCHAR * pString )
   {
      if ( pString )
      {
         wprintf( L"%s", pString );
      }
      return *this;
   }
private:
   BTS * m_pScheduler;
};


// OUE *_env = 0;

// Required for busy 1080p I frames
// #define MAX_PACKET_SIZE 1500000

#define MILLI_MS_SECOND 1000
#define MICRO_US_SECOND 1000000

class CRGBEasyH264FrameSource;

typedef struct _tagEncInfo
{
   uint32_t       Width;
   uint32_t       Height;
   uint32_t       FrameRate; // mHz
   DGCENCLEVEL    Level;
   DGCENCPROFILE  Profile;
   uint32_t       Bitrate; // bps
   uint32_t       KeyFrameInterval; // GOP Length
} ENCINFO;

typedef struct _RGBEASYH264STREAM
{
   unsigned long           Input;
   ENCINFO ei;
   CRGBEasyH264FrameSource *PFrameSourceVideo;
   ServerMediaSession      *PServerMediaSession;
} RGBEASYH264STREAM, *PRGBEASYH264STREAM;

const ENCINFO Modes[] =
{
   { 640,  480, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  50000000, 90},
   { 640,  480, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   { 640,  480, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   { 640,  480, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   { 640,  480, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   { 640,  480, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   { 640,  480, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   { 800,  600, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   { 800,  600, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   { 800,  600, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   { 800,  600, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   { 800,  600, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   { 800,  600, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   { 800,  600, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1024,  768, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1024,  768, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   {1024,  768, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   {1024,  768, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   {1024,  768, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   {1024,  768, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   {1024,  768, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1280,  720, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1280,  720, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   {1280,  720, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   {1280,  720, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   {1280,  720, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   {1280,  720, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   {1280,  720, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1280, 1024, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1280, 1024, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   {1280, 1024, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   {1280, 1024, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   {1280, 1024, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   {1280, 1024, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1600, 1200, 15000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1600, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1600, 1200, 60000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1920, 1080, 15000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1920, 1080, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   {1920, 1080, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   {1920, 1080, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   {1920, 1080, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   {1920, 1080, 30000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   {1920, 1080, 60000, DGCENC_H264_LEVEL_4_2, DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   {1920, 1200, 15000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 90},
   {1920, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 90},
//   {1920, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 60},
//   {1920, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 45},
//   {1920, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 30},
//   {1920, 1200, 30000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_MAIN,  50000000, 15},
   {1920, 1200, 60000, DGCENC_H264_LEVEL_5,   DGCENC_H264_PROFILE_HIGH,  62500000, 90},
   // Add more modes to this list.
};
uint32_t numModes = sizeof( Modes ) / sizeof( Modes[0] );
char gStop = 0;

/******************************************************************************/
#if 0
char const* nal_unit_description_h264[32] = {
  "Unspecified", //0
  "Coded slice of a non-IDR picture", //1
  "Coded slice data partition A", //2
  "Coded slice data partition B", //3
  "Coded slice data partition C", //4
  "Coded slice of an IDR picture", //5
  "Supplemental enhancement information (SEI)", //6
  "Sequence parameter set", //7
  "Picture parameter set", //8
  "Access unit delimiter", //9
  "End of sequence", //10
  "End of stream", //11
  "Filler data", //12
  "Sequence parameter set extension", //13
  "Prefix NAL unit", //14
  "Subset sequence parameter set", //15
  "Reserved", //16
  "Reserved", //17
  "Reserved", //18
  "Coded slice of an auxiliary coded picture without partitioning", //19
  "Coded slice extension", //20
};
#endif

/******************************************************************************/

class CRGBEasyH264FrameSource : public FramedSource
{
   CRGBEasyH264 *m_pRGBEasyH264;
   void *m_pToken;
   uint32_t m_input;
   uint32_t m_error;
   uint64_t m_timeoffset;
public:
   CRGBEasyH264FrameSource( UsageEnvironment &env,
                             int input,
                             uint32_t width,
                             uint32_t height,
                             uint32_t frameRate,
                             DGCENCLEVEL level,
                             DGCENCPROFILE profile,
                             uint32_t bitrate,
                             uint32_t keyframeinterval ) : FramedSource( env )
	{
      uint32_t Error = 0;
		m_pToken = NULL;
      m_input = input;
      m_error = 0;
      m_timeoffset = 0;
      m_pRGBEasyH264 = new CRGBEasyH264 ( input );
      if ( !m_pRGBEasyH264 )
      {
         Error = RGBERROR_INSUFFICIENT_MEMORY;
         throw Error;
      }
      if((Error = m_pRGBEasyH264->RGBEasyH264Start ( width, height, frameRate,
                                        level, profile, bitrate,
                                        keyframeinterval )) == 0)
      {
         OutPacketBuffer::maxSize = (width * height * 3) / 2; // NV12 data format from encoder, assume no compression.
      }
      else
      {
         // We're in a constructor, so extraordinary measures are needed to fail this call.
         // Caller must wrap in a try{} block to discover the error, otherwise whole application unwinds and faults in main()
         delete m_pRGBEasyH264;
         throw Error;
      }
	}

	~CRGBEasyH264FrameSource ()
	{
      if ( m_pToken )
      {
			envir().taskScheduler().unscheduleDelayedTask(m_pToken);
		}
      m_pRGBEasyH264->RGBEasyH264Stop ( );
      delete m_pRGBEasyH264;
	}

protected:
   // Overridden from derived FrameSource class
	virtual void doGetNextFrame ()
	{
      RealGetNextFrame();
	}

private:
	static void getNextFrame (void *ptr)
	{
      // Static to run-time class adapter call.
		((CRGBEasyH264FrameSource*)ptr)->RealGetNextFrame();
	}

   // It would appear that it is incumbent upon this function to - at some point in the future - return a NAL
   // from the source.  In the case that there isn't already a NAL waiting on the queue, this function should
   // schedule a call to itself again, to make sure that the next delivered NAL is picked up in a timely
   // manner.
   // Inspection of the code doesn't show that this function actually returns a frame.  It returns a NAL.
   // Now, it could be that a NAL is also known as a frame, but to me a frame means "of video".  And given that
   // the reader process doesn't process NALs quickly enough such that over time the list willl grow and keep
   // growing, I'm wondering if there's a bug here?
	void RealGetNextFrame ()
   {
      
      uint64_t timeStamp = 0, Sequence=0, bLastInSeq=0;
      m_pToken = NULL;

      //
      // bLastInSequence is a boolean to indicate that this NAL was the last NAL to be transferred from
      // the encoder in a single payload.  It is after this NAL which the fDurationInMicroseconds needs to
      // be set to the frame interval's time for this stream, with all non-bLastInSequence NALs set to a
      // duration of 0.
      // As a courtesy, this function also gets the sequence number so that we can see the sequence numbers
      // for debugging purposes (easier to tie up additions to the list in RGBH264NAL.CPP's printf()s).
      //
      m_error = m_pRGBEasyH264->RGBEasyH264GetNAL ( fTo, fMaxSize, 
            &fNumTruncatedBytes, &fFrameSize, &timeStamp, &Sequence, &bLastInSeq );

      if ( m_error == 0 )
      {
         // NAL returned from CRGBEasyH264 class.
         if ( DoesH264NALUnitBeginNewAccessUnit(fTo) )
         {
#if 0
            printf("CRGBEasyH264FrameSource::RealGetNextFrame: NEW AU - timeStamp(%lu)\n", timeStamp);
#endif
            // If the NAL begins a new frame, we recalculate (and cache) the new frame presentation time
            // also, take the opportunity to cope with the timestamp values wrapping round.
            // It would also be good to check the magnitude of clock-drift here and adjust our time deltas
            // according to how much drift there is on the SQX clock.
            if ( m_timeoffset == 0 )
            {
               struct timeval tvCurTime = { 0 };
               uint64_t iCurTime = 0;
               gettimeofday( &tvCurTime, 0 );
               iCurTime = ((uint64_t)tvCurTime.tv_sec) * 1000000;
               iCurTime += ((uint64_t)tvCurTime.tv_usec);
               m_timeoffset = iCurTime - timeStamp;
            }
            timeStamp += m_timeoffset;
            fPresentationTime.tv_sec =  ( long ) ( timeStamp / ( ( uint64_t ) 1000000 ) );
            fPresentationTime.tv_usec = timeStamp % 1000000;

            //printf( "RealGetNextFrame() calculates fPresentationTime as %lu.%06.6lu\n", fPresentationTime.tv_sec, fPresentationTime.tv_usec );
            //printf( "RealGetNextFrame() calculates fDurationInMicroseconds to be: %lu for NAL sequence %lld type %02.2d\n", fDurationInMicroseconds, Sequence, (int)GetNALType(fTo) );
         } else {
            //printf( "RealGetNextFrame() considers NAL sequence %lld to be a continuation NAL (type: %02.2d)\n", Sequence, GetNALType(fTo) );
         }
         if ( bLastInSeq )
         {
            fDurationInMicroseconds = MICRO_US_SECOND / ( m_pRGBEasyH264->m_StreamInfo.FPS / MILLI_MS_SECOND );
         } else {
            fDurationInMicroseconds = 0;
         }
         //printf( "RealGetNextFrame() set fDurationinMicroseconds to %lu\n", fDurationInMicroseconds );
         afterGetting(this);
         return;
      }
      else
      {
         // No NAL returned from CRGBEasyH264 class, wait a frame time and try again.
         int64_t microseconds = 0;
         
         // Has CRGBEasyH264 class finished initialising with a signal attached?
         if ( m_pRGBEasyH264->m_StreamInfo.FPS )
         {
            microseconds = MICRO_US_SECOND / ( 
               m_pRGBEasyH264->m_StreamInfo.FPS / MILLI_MS_SECOND );
            // microseconds = 1;
         }
         else // wait 16.6ms
         {
            microseconds = MICRO_US_SECOND / ( 60000 / MILLI_MS_SECOND ); 
         }

         m_pToken = envir().taskScheduler().scheduleDelayedTask(
               microseconds, getNextFrame, this);

         //printf( " ***** WAITING ***** - %ld us\n", microseconds );
         return;     
	   }
   }
};

/******************************************************************************/

// A 'ServerMediaSubsession' object that creates a new, unicast, 
// "RTPSink"s on demand.
class myRTSPSinkOndemandMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	// Static to run-time class adapter call.
   static myRTSPSinkOndemandMediaSubsession *createNew (
      UsageEnvironment &env, FramedSource *source, int input,
      uint32_t width,
      uint32_t height,
      uint32_t frameRate,
      DGCENCLEVEL level,
      DGCENCPROFILE profile,
      uint32_t bitrate,
      uint32_t keyframeinterval,
      const char* streamName ) // Everything else in the source is ASCII.
	{
		return new myRTSPSinkOndemandMediaSubsession(env, source, input,
                                                    width, height, frameRate,
                                                    level, profile, bitrate,
                                                    keyframeinterval, streamName );
	}

protected:
   // This will create only one source that will be shared by all the sinks.
	myRTSPSinkOndemandMediaSubsession (
         UsageEnvironment &env,
         FramedSource *pFrameSource,
         int input,
         uint32_t width,
         uint32_t height,
         uint32_t frameRate,
         DGCENCLEVEL level,
         DGCENCPROFILE profile,
         uint32_t bitrate,
         uint32_t keyframeinterval,
         const char* streamName) // Everything is ASCII in the rest of the source.
         : OnDemandServerMediaSubsession(env, True)
	{
		m_pFrameSource = pFrameSource;
		m_pSDP = NULL;
      m_input = input;
      m_width = width;
      m_height = height;
      m_frameRate = frameRate;
      m_level = level;
      m_profile = profile;
      m_bitrate = bitrate;
      m_keyframeinterval = keyframeinterval;
      m_streamName = streamName;
	}

	~myRTSPSinkOndemandMediaSubsession ()
	{
		if (m_pSDP) 
         free(m_pSDP);
	}

private:
	static void afterPlayingDummy (void *ptr)
	{
		myRTSPSinkOndemandMediaSubsession *This = 
            (myRTSPSinkOndemandMediaSubsession*)ptr;
		This->m_done = 127;
	}

	static void chkForAuxSDPLine (void *ptr)
	{
		myRTSPSinkOndemandMediaSubsession *This = 
            (myRTSPSinkOndemandMediaSubsession *)ptr;
		This->RealchkForAuxSDPLine();
	}

	void RealchkForAuxSDPLine ()
	{
      envir( ) << "chkForAuxSDPLine: " << m_streamName.c_str( ) << "\n";
      if (m_pDummy_RTPSink->auxSDPLine())
      {
         m_done = 127;
         m_pSDP = _strdup(m_pDummy_RTPSink->auxSDPLine());
         m_pDummy_RTPSink->stopPlaying();
      }
      else 
      {
         int delay = MILLI_MS_SECOND*100;
         nextTask() = envir().taskScheduler().scheduleDelayedTask(
               delay, chkForAuxSDPLine, this);
      }
	}

protected:

	virtual const char *getAuxSDPLine (RTPSink *sink, FramedSource *source)
	{
		if (m_pSDP) 
         return m_pSDP;

		m_pDummy_RTPSink = sink;
		m_pDummy_RTPSink->startPlaying(*source, 0, 0);
		chkForAuxSDPLine(this);
		m_done = 0;
		envir().taskScheduler().doEventLoop(&m_done);
      envir( ) << "getAuxSDPLine: Got SPS/PPS - " << m_streamName.c_str( )<< "\n";

		return m_pSDP;
	}

	virtual RTPSink *createNewRTPSink(
         Groupsock *rtpsock, uint_least8_t type, FramedSource *source)
	{
      UNREFERENCED_PARAMETER( source );
		return H264VideoRTPSink::createNew(envir(), rtpsock, type);
	}

	virtual FramedSource *createNewStreamSource (
         unsigned sid, unsigned &estBitrate)
	{
      UNREFERENCED_PARAMETER( sid );
		estBitrate = m_bitrate; // Was MAX_PACKET_SIZE, which we've dispensed with
      try {
		   return H264VideoStreamDiscreteFramer::createNew(
               envir(), new CRGBEasyH264FrameSource(envir(), m_input,
               m_width, m_height, m_frameRate, m_level, m_profile, m_bitrate,
               m_keyframeinterval ) );
      }
      catch (uint32_t e)
      {
         // The encoder couldn't be opened, or couldn't start.
         envir() << "Cannot start RGBEasy encoder (input already open elsewhere?) - Error:" << e << "\n";
         return NULL;
      }
	}

private:
	FramedSource *m_pFrameSource;
	int_fast8_t *m_pSDP;
	RTPSink *m_pDummy_RTPSink;
	int_fast8_t m_done;
   uint32_t m_input;
   uint32_t m_width;
   uint32_t m_height;
   uint32_t m_frameRate;
   DGCENCLEVEL m_level;
   DGCENCPROFILE m_profile;
   uint32_t m_bitrate;
   uint32_t m_keyframeinterval;
   std::string m_streamName;
};

/******************************************************************************/

ServerMediaSession * createServerMediaSession( UsageEnvironment &env,
                                               FramedSource *source,
                                               uint32_t input,
                                               const ENCINFO &ei)
{
   // C++-ified and cleaned up string handling.
   ServerMediaSession *p;
   std::string strName, strDesc;
   std::stringstream sn, desc;

   sn << "input" << input + 1 << "?resolution=" << ei.Width << "x" << ei.Height << "&rate=" << ei.FrameRate;
   strName = sn.str( );
   desc << "RGBEasy H264 Unicast Session " << sn;
   strDesc = desc.str( );

   p = ServerMediaSession::createNew( env, strName.c_str( ), 0, strDesc.c_str( ) );

   // Insert into the media session a video stream.
   p->addSubsession(
      myRTSPSinkOndemandMediaSubsession::createNew( env, source, input,
                                                    ei.Width, ei.Height, ei.FrameRate, ei.Level, ei.Profile, 
                                                    ei.Bitrate, ei.KeyFrameInterval, strName.c_str( ) ) );

   // Insert into the media session an audio stream.
   //p->addSubsession(
   //   myRTSPSinkOndemandMediaSubsession::createNew( env, source, input ) );

   return p;
}

/******************************************************************************/

#if 0
void DestroyServerMediaSession(PRGBEASYH264STREAM pStream)
{
   if ( ( pStream != NULL ) && ( pStream->PServerMediaSession != NULL ) )
   {
      pStream->PServerMediaSession->deleteAllSubsessions( );
      pStream->PServerMediaSession->deleteWhenUnreferenced( );
   }
}
#endif

/******************************************************************************/

BOOL WINAPI HandlerRoutine( DWORD dwCtrlType )
{
   UNREFERENCED_PARAMETER( dwCtrlType );
   gStop = 1;
   return TRUE;
}

/******************************************************************************/

int main( int argc, TCHAR **argv )
{
   HRGBDLL  hDLL;
   UNREFERENCED_PARAMETER( argc );
   UNREFERENCED_PARAMETER( argv );
   uint32_t inputCount, h264Count, input_it;
   uint32_t *pInputList;

   unsigned long error;

   // BUE is a BasicUsageEnvironment wrapper class, which allows us to create
   // an instance of the class directly, thereby making lifetimme management
   // of this instance of the class the compiler's job (it's now an auto class
   // variable).  I've also made it free the BTS class which it is passed. 
   // the BTS class is a wrapper for BasicTaskScheduler so that it can liekwise be
   // instantiated by the compiler and its lifetime managed sensibly - in this
   // case, the BTS reference is freed by the BUE class destructor.
   BUE _env( *new BTS );

#ifdef _LOCAL_HEAP_DEBUG
   {
      int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

      // Turn on leak-checking bit.
      tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

      // Turn off CRT block checking bit.
      tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

      // Set flag to the new value.
      _CrtSetDbgFlag( tmpFlag );
   }
#endif

   /* Load the RGBEASY API. */
   error = RGBLoad(&hDLL);
   if (error)
   {
         _env << "ERROR - loading RGBEasy dll\n";

         Sleep(3000);

         return 0;
   }

   if ( GetSupportedH264Inputs (&pInputList, &inputCount, &h264Count) == 0 )
   {   
      UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
     // To implement client access control to the RTSP server, do the following:
     authDB = new UserAuthenticationDatabase;
     authDB->addUserRecord("username1", "password1"); // replace these with real strings
     // Repeat the above with each <username>, <password> that you wish to allow
     // access to the server.
#endif

      // Create the RTSP server.  Try first with the default port number (554),
      // and then with any alternative port numbers
      RTSPServer* rtspServer = NULL;
      uint32_t port=554;
      do 
      {
         rtspServer = RTSPServer::createNew( _env, (portNumBits)port, authDB );
         if(port==554) 
            port=8553;
         port++;
      } while((rtspServer == NULL) && (port < 9555));

	   if (rtspServer) 
      {
         // Set up each of the possible streams that can be served by the
         // RTSP server.  Each such stream is implemented using a
         // "ServerMediaSession" object, plus one or more
         // "ServerMediaSubsession" objects for each audio/video substream.
         PRGBEASYH264STREAM pStreamList = (PRGBEASYH264STREAM)malloc (
            sizeof(RGBEASYH264STREAM)*h264Count*numModes);
         if ( pStreamList )
         {
            unsigned long stream_it = 0;

            for ( input_it=0; input_it< inputCount; input_it++)
            {
               if ( pInputList[input_it] ) // if input supports h264
               {
                  unsigned long mode_it;
                  for ( mode_it = 0; mode_it < numModes; mode_it++ )
                  {
                     PRGBEASYH264STREAM pStream = &pStreamList[stream_it];
                     const ENCINFO & EncInfo = Modes[mode_it];
                     pStream->Input = input_it;
                     pStream->ei = EncInfo;
                     // Specify a unicast, reuse same source RTP sink class handler.
                     pStream->PFrameSourceVideo = NULL;
                     // Create and add the description of a media session.

                     pStream->PServerMediaSession = createServerMediaSession(
                        _env, pStream->PFrameSourceVideo, pStream->Input,
                        EncInfo);
                     if ( pStream->PServerMediaSession )
                     {
                        // Add the media session to the RTSP server
                        rtspServer->addServerMediaSession( pStream->PServerMediaSession);
                        _env << "using url \"" <<
                           rtspServer->rtspURL(pStream->PServerMediaSession) << 
                           "\"\n";
                     }
                     else
                        _env << "ERROR - creating media session\n";
                     stream_it++;
                  }
                  _env << "\n";
               }
            }

            // Add a CTRL-C handler routine.
            SetConsoleCtrlHandler( HandlerRoutine, TRUE );
            // run loop
            gStop = 0;
	         _env.taskScheduler().doEventLoop( &gStop );
#if 0
            // Deprecate this method of cleanup..
            for (input_it = 0; input_it< inputCount; input_it++)
            {
               rtspServer->removeServerMediaSession(pStreamList[stream_it].PServerMediaSession);
               DestroyServerMediaSession(&pStreamList[input_it]);
            }
#else
            // This is the officially sanctioned way of closing an RTSP server,
            // it cleans up all the sessions that it manages, making code a lot simpler
            // Test that this works, and use it instead of the hokeyness above.
            Medium::close( rtspServer );
            // Note that not calling this means that the process leaks an RTSPServer object.
            // Not the end of the world for an application just about to quit, but for
            // an API provider, this could be a big deal over time.
#endif
            free (pStreamList);
         }
      }
      else
         _env << "ERROR - creating RTSPServer\n";

      free( pInputList );
   }
   else
      _env << "ERROR - No H264 inputs supported\n";

   /* Unload the RGBEASY API. */
   RGBFree(hDLL);

#ifdef _LOCAL_HEAP_DEBUG
   _CrtDumpMemoryLeaks( );
   // None, now.  Or certainly, fewer.
#endif

   Sleep(3000);

   return 0;
}

/******************************************************************************/
