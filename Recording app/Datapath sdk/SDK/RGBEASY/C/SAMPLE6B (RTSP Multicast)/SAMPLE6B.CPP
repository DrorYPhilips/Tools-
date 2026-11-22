/*******************************************************************************
Copyright Datapath Ltd. 2015.

File:    Sample6B.cpp

History: 27 MAR 15    RL   Created.
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
   BUE( BTS & Scheduler ) :BasicUsageEnvironment( Scheduler )
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

#define MILLI_MS_SECOND 1000
#define MICRO_US_SECOND 1000000

class CRGBEasyH264FrameSource;

typedef struct _RGBEASYH264STREAM
{
   unsigned long           Input;
   uint32_t                Width;
   uint32_t                Height;
   uint32_t                FrameRate; // mHz
   DGCENCLEVEL             Level;
   DGCENCPROFILE           Profile;
   uint32_t                Bitrate; // bps
   uint32_t                KeyFrameInterval; // GOP Length
   ServerMediaSession      *PServerMediaSession;
   H264VideoStreamDiscreteFramer *PVideoDiscreteSource;
   RTCPInstance            *PRtcpInstance;
   Groupsock               *PRtpGroupsock;
   Groupsock               *PRtcpGroupsock;
   RTPSink                 *PVideoSink;
   BUE                     *_env;
} RGBEASYH264STREAM, *PRGBEASYH264STREAM;

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
      // DMJ bLastInSequence is a boolean to indicate that this NAL was the last NAL to be transferred from
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

// These have to be static as copies aren't made, references are taken instead.
std::string strName, strDesc;

uint32_t CreateServerMediaSession(BUE & _env, PRGBEASYH264STREAM pStream)
{
   std::stringstream sn, desc;

   sn << "input" << pStream->Input + 1 << "?resolution=" << pStream->Width << "x" << pStream->Height << "&rate=" << pStream->FrameRate;
   strName = sn.str( );
   desc << "RGBEasy H264 Multicast Session " << strName;
   strDesc = desc.str( );

   pStream->PServerMediaSession = ServerMediaSession::createNew( _env,
      strName.c_str(), 0, strDesc.c_str(), True /*SSM*/ );
   if ( pStream->PServerMediaSession )
   {
      pStream->PServerMediaSession->addSubsession ( 
         PassiveServerMediaSubsession::createNew(*pStream->PVideoSink, 
         pStream->PRtcpInstance));
      //*pServerMediaSession->addSubsession ( 
      //   PassiveServerMediaSubsession::createNew(*pStream->PAudioSink, 
      //   pStream->PRtcp));
      return 0;
   }
   return -1;
}

/******************************************************************************/

void DestroyServerMediaSession(PRGBEASYH264STREAM pStream)
{
   if ( pStream->PServerMediaSession != NULL )
   {
      pStream->PServerMediaSession->deleteAllSubsessions( );
   }
   // This doesn't appear to free the PServerMediaSession itself.  Oversight?
}

/******************************************************************************/

uint32_t CreateGroupSockets(BUE & _env, PRGBEASYH264STREAM pStream)
{
   // Create 'groupsocks' for RTP and RTCP:
   struct in_addr destinationAddress;
   destinationAddress.s_addr = chooseRandomIPv4SSMAddress(_env);
   // Note: This is a multicast address.  If you wish instead to stream
   // using unicast, then you should use the "testOnDemandRTSPServer"
   // test program - not this test program - as a model. See Sample6A.
#if 0  
   const unsigned short rtpPortNum = 18888;
   const unsigned short rtcpPortNum = rtpPortNum+1;
#else
   const unsigned short rtpPortNum = 18888+(USHORT)pStream->Input;
   const unsigned short rtcpPortNum = 28888+(USHORT)pStream->Input;
#endif
   const unsigned char ttl = 255;

   const Port rtpPort(rtpPortNum);
   const Port rtcpPort(rtcpPortNum);

   pStream->PRtpGroupsock = new Groupsock(_env, 
      destinationAddress, rtpPort, ttl);
   pStream->PRtpGroupsock->multicastSendOnly(); // we're a SSM source
   pStream->PRtcpGroupsock = new Groupsock(_env, 
      destinationAddress, rtcpPort, ttl);
   pStream->PRtcpGroupsock->multicastSendOnly(); // we're a SSM source

   // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
   pStream->PVideoSink = H264VideoRTPSink::createNew(_env, 
      pStream->PRtpGroupsock, 96);
   if( pStream->PVideoSink )
   {
      // Create (and start) a 'RTCP instance' for this RTP sink:
      const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP
      const unsigned maxCNAMElen = 100;
      unsigned char CNAME[maxCNAMElen+1];
      gethostname((char*)CNAME, maxCNAMElen);
      CNAME[maxCNAMElen] = '\0'; // just in case
      pStream->PRtcpInstance = RTCPInstance::createNew(_env, 
         pStream->PRtcpGroupsock,
		   estimatedSessionBandwidth, CNAME,
		   pStream->PVideoSink, NULL /* we're a server */,
		   True /* we're a SSM source */);
      return 0;
   }
   return -1;
}

/******************************************************************************/

void Play ( PRGBEASYH264STREAM pStream);

void AfterPlaying(void* pClientData) 
{
   // This should not occur for a live source.
   PRGBEASYH264STREAM pStream = (PRGBEASYH264STREAM)pClientData;

   *(pStream->_env) << "...EOF reached\n";
   pStream->PVideoSink->stopPlaying();

   Medium::close(pStream->PVideoDiscreteSource);
   // Try playing once again:
   Play( pStream );  // Why do we do this when the stream stopped?  In the case of Ctrl-C this doesn't work.
}

/******************************************************************************/

void Play( PRGBEASYH264STREAM pStream )
{
   try
   {
      pStream->PVideoDiscreteSource = H264VideoStreamDiscreteFramer::createNew( *pStream->_env,
               new CRGBEasyH264FrameSource( *pStream->_env, pStream->Input, pStream->Width,
      pStream->Height, pStream->FrameRate, pStream->Level, pStream->Profile, 
                                             pStream->Bitrate, pStream->KeyFrameInterval ) );
      pStream->PVideoSink->startPlaying( *pStream->PVideoDiscreteSource,
                                         AfterPlaying, pStream );
   }
   catch ( uint32_t e )
   {
      //*(pStream->_env) << "Couldn't create FrameSource for input " << pStream->Input << " threw error: " << static_cast<uint32_t>e << "\n";
      printf( "Couldn't create Object: %d (out of encoder headroom?)\n", e );
   }
}

/******************************************************************************/

BOOL WINAPI HandlerRoutine( DWORD dwCtrlType )
{
   gStop = 1;
   return TRUE;
}

/******************************************************************************/

int main( int argc, TCHAR **argv )
{
   HRGBDLL  hDLL;

   uint32_t inputCount, h264Count;
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
         PRGBEASYH264STREAM pStreamList = (PRGBEASYH264STREAM)malloc (sizeof(RGBEASYH264STREAM)*
                                                                   h264Count);
         if ( pStreamList )
         {
            unsigned long mode_it = 0;
            memset ( pStreamList, 0, sizeof(RGBEASYH264STREAM)*h264Count);
            for ( uint32_t input_it=0; input_it< inputCount; input_it++)
            {
               if ( pInputList[input_it] ) // if input supports h264
               {
                  PRGBEASYH264STREAM pStream = &pStreamList[mode_it];

                  pStream->_env = &_env;
                  pStream->Input = input_it;
                  pStream->Level = DGCENC_H264_LEVEL_4_2;
                  pStream->Profile = DGCENC_H264_PROFILE_MAIN;
                  pStream->Bitrate = 50000000;
                  pStream->KeyFrameInterval = 90;
                  // Get 'on the wire' defaults or use above defaults.
                  if ( GetInputSignalType ( input_it, &pStream->Width, &pStream->Height,
                        &pStream->FrameRate ))
                  {
                     pStream->Width = 1920;
                     pStream->Height = 1080;
                     pStream->FrameRate = 30000;
                  }
                  if ( CreateGroupSockets (_env, pStream) == 0 )
                  {
                     if ( CreateServerMediaSession (_env, pStream) == 0 )
                     {
                        // Add the media session to the RTSP server
                        rtspServer->addServerMediaSession(pStream->PServerMediaSession);
                        _env << "using url \"" << rtspServer->rtspURL(
                           pStream->PServerMediaSession) << "\"\n";
                        Play(pStream);
                     }
                     else
                        _env << "ERROR - creating media session\n";
                  }
                  else
                     _env << "ERROR - creating group sockets\n";
                  mode_it++;
               }
               _env << "\n";
            }

            // Add a CTRL-C handler routine.
            SetConsoleCtrlHandler( HandlerRoutine, TRUE );
            gStop = 0;
            _env << "Beginning to stream in 5 seconds...\n";
            // Chance to read the URI's!
            Sleep(5000);
            _env << "Streaming started\n";
	         _env.taskScheduler().doEventLoop( &gStop );
            
            for ( uint32_t stream_it = 0; stream_it < inputCount; stream_it++ )
            {
               PRGBEASYH264STREAM pStream = &pStreamList[stream_it];
               AfterPlaying( pStream );
               try
               {
                  _env << "Destroying " << pStream->PServerMediaSession->streamName( ) << "\n";
               }
               catch ( ... )
               {
                  printf( "Failed to get stream name from %08.8X\n", pStream->PServerMediaSession );
               }
               rtspServer->removeServerMediaSession( pStream->PServerMediaSession);
               // DMJ - it appears that DestroyServerMediaSession is superfluous; by the time
               // the call is made, there isn't a list of subsessions anymore.  This causes
               // an access violation as the CPU attempts to dereference memory through a
               // pointer which has been invalidated (set to value 0xddddddd by Live555).
               //DestroyServerMediaSession( &pStream[stream_it] );
               if ( pStream->PRtpGroupsock )
                  delete pStream->PRtpGroupsock;
               if ( pStream->PRtcpGroupsock )
                  delete pStream->PRtcpGroupsock;
            }

            // This is the officially sanctioned way of closing an RTSP server,
            // it cleans up all the sessions that it manages, making code a lot simpler
            // Test that this works, and use it instead of the hokeyness above.
            Medium::close( rtspServer );
            // Note that not calling this means that the process leaks an RTSPServer object.
            // Not the end of the world for an application just about to quit, but for
            // an API provider, this could be a big deal over time.

            free( pStreamList );
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
#endif

   Sleep(3000);

	return 0;
}

/******************************************************************************/
