#ifndef CLIBSUPERPROBE733_H_
#define CLIBSUPERPROBE733_H_

#include <string>
#include <cmath>

#ifdef WIN32
    #include "windows.h"

    #ifdef BUILDING_DLL
        #define DLLIMPORT __declspec (dllexport)
    #elif defined CLIB_SUPERPROBE733_STATICLIB //Note that static lib will not work unless the version of gcc used to build lib is same as qt's
        #define DLLIMPORT
    #else
        #define DLLIMPORT __declspec (dllimport)
    #endif
#else
    #define DLLIMPORT
    typedef void* HANDLE;
#endif

const int ABORT_DELAY = 1500; //Delay specifically for abort (ms); it seems to take a little while for the 5500 to recover sometimes
const int NEWLINE_DELAY = 500; //Delay after newline is transmitted (ms)
const int CHAR_DELAY = 50; //Delay between characters transmitted (ms)
class Float3
{
    public:
    float fValues[3];

    Float3(float flInitialVal) { for ( int i=0; i<3;i++ ) fValues[i] = flInitialVal; }
    Float3(float fX, float fY, float fZ)
    { fValues[0] = fX; fValues[1] = fY; fValues[2] = fZ; }
    float& operator[](unsigned char idx) { return fValues[idx]; }
    const float& operator[](unsigned char idx) const { return fValues[idx]; }
};

class Float4
{
    public:
    Float4(float flInitialVal) { for ( int i=0; i<4;i++ ) fValues[i] = flInitialVal; }
    float fValues[4];
    float& operator[](unsigned char idx) { return fValues[idx]; }
};

//The Flextran program on the TN5500 will transmit data in this format
//sizeof(short) assumed to be 2 bytes for a total of 10 bytes/point
//NOTE: This is the raw data, needs a little processing
struct TN5500_Data
{
    short sCounts[4];
    short sCurrent;
} __attribute__ ((packed));

//These translate the raw data into proper values
inline unsigned short TN5500_Data_Counts( short sCounts )
{
    return (unsigned short)(sCounts); //+ 32768);
}
inline float TN5500_Data_Current( short sCurrent )
{
    return float(sCurrent) / 100.0f;
}

class clib_callback_class
{
public:
    virtual void dataReceived( const TN5500_Data& data ) = 0;
};

DLLIMPORT bool clib_init( int iPort, const std::string& strLog = std::string(), HANDLE hConsole = NULL );
DLLIMPORT bool clib_commstest( std::string strTest );
DLLIMPORT void clib_close( void );

//These functions will return when the TN-5500 is expected to be ready for new input (some hundreds of ms after xmit)
DLLIMPORT void clib_linescan( float flX1, float flY1, float flZ1, float flX2, float flY2, float flZ2,
                             int iNumPts, float flDwellTime, int iBlock );
DLLIMPORT void clib_grabpoint( float flX, float flY, float flZ, float flDwellTime, int iBlock );
DLLIMPORT void clib_blankbeam( bool bBlank );
DLLIMPORT bool clib_probe_abortexec( void );

//Flush receive buffer
DLLIMPORT bool clib_serial_flushrecv( void );

DLLIMPORT bool clib_probe_posrel( const Float3& fPos );

//Any abs. position parameters < 0 will be ignored
DLLIMPORT bool clib_probe_posabs( const Float3& fPos );

DLLIMPORT bool clib_probe_readabscurrent( float& flCurrent );
DLLIMPORT bool clib_probe_readpos( Float3& fPos );

DLLIMPORT bool clib_probe_allocflextranvars( void );
DLLIMPORT bool clib_probe_countspectrometers( const float& flTime );
DLLIMPORT bool clib_probe_readspectrometercounts( Float4& nCounts );

DLLIMPORT void clib_probe_listen( clib_callback_class* pObj, float flTimeout = 0.0f );
DLLIMPORT void clib_probe_listen_reset( void );

DLLIMPORT bool clib_probe_echooff( void );
DLLIMPORT bool clib_probe_toggleecho( void );

DLLIMPORT bool clib_probe_sendfile( std::string fileName, int iCharDelay, int iLineDelay = NEWLINE_DELAY );

//An existing file will be appended to
DLLIMPORT bool clib_enablelog( std::string strFile );
DLLIMPORT void clib_closelog( void );
DLLIMPORT bool clib_writelog( const std::string& strEntry, bool bWriteConsole = true, bool bTimestamp = true );
DLLIMPORT std::ofstream& clib_getlogfile( void );

//FreeConsole and such are not called, simply ceases to attempt to write to the console
DLLIMPORT void clib_clearconsole( void );
DLLIMPORT HANDLE clib_getconsole( void );

#endif /* _CLIBSUPERPROBE733_H_ */
