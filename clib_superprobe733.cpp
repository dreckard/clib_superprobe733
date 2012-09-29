#include <fstream>
#include <sstream>
#include "clib_superprobe733.h"
#include "clib_serial.h"
#include "clib_compat.h"

HANDLE hCon = NULL;
std::ofstream logFile;
bool clib_init( int iPort, const std::string& strLog /*=std::string()*/, HANDLE hConsole /*= NULL*/ )
{
    if ( !strLog.empty() )
        clib_enablelog( strLog );
    hCon = hConsole;
    return clib_serial_init( iPort );
}

void clib_close( void )
{
    clib_serial_close();
    clib_closelog();
}

bool clib_commstest( std::string testStr )
{
	return clib_serial_writecmd( testStr );
}

void clib_linescan( float flX1, float flY1, float flZ1, float flX2, float flY2, float flZ2,
                    int iNumPts, float flDwellTime, int iBlock )
{
    if ( !clib_serial_open() )
        return;

    std::stringstream s;
    s << "A F1(2),F2(2)\r";
    s.setf( std::ios::fixed, std::ios::floatfield ); s.precision( 3 );
    s << "S F1=" << flX1 << '\\' << flY1 << '\\' << flZ1 << '\r';
    s << "S F2=" << flX2 << '\\' << flY2 << '\\' << flZ2 << '\r';
    s << "S NP=" << iNumPts << ';' << "S FT=" << flDwellTime << '\r';
    s << "G " << iBlock << '\r';

    clib_serial_writestr_bytemeal( s.str(), true, CHAR_DELAY, NEWLINE_DELAY );

    /*Sleep( NEWLINE_DELAY );
    clib_serial_writecmd( "A F1(2),F2(2),NP,FT\r" );
    Sleep( NEWLINE_DELAY );

    std::stringstream s;
    s.setf( std::ios::fixed, std::ios::floatfield ); s.precision( 3 );
    s << "S F1=" << flX1 << '\\' << flY1 << '\\' << flZ1 << '\r';
    clib_serial_writecmd( s.str() );
    Sleep( NEWLINE_DELAY );

    s.str(std::string());
    s << "S F2=" << flX2 << '\\' << flY2 << '\\' << flZ2 << '\r';
    clib_serial_writecmd( s.str() );
    Sleep( NEWLINE_DELAY );

    s.str(std::string());
    s << "S NP=" << iNumPts << ';' << "S FT=" << flDwellTime << '\r';
    clib_serial_writecmd( s.str() );
    Sleep( NEWLINE_DELAY );

    s.str(std::string());
    s << "G " << iBlock << '\r';
    clib_serial_writecmd( s.str() );*/
}

void clib_grabpoint( float flX, float flY, float flZ, float flDwellTime, int iBlock )
{
    //TODO: Implement this

}

void clib_blankbeam( bool bBlank )
{
    //Blank: $SE(0,CHKVAR,3,0)
    //Unblank: $SE(0,CHKVAR,3,1)
    std::stringstream s; s << "A BB;" << "$SE(0,BB,3," << int(!bBlank) << ")\r";
    //clib_serial_writecmd( s.str() );
    clib_serial_writestr_bytemeal( s.str(), true, CHAR_DELAY, NEWLINE_DELAY );
    cp_sleep( NEWLINE_DELAY );
}

bool clib_enablelog( std::string strFile )
{
    logFile.open( strFile.c_str(), std::ios_base::out | std::ios_base::app );
    return !logFile.fail();
}

bool clib_writelog( const std::string& strEntry, bool bWriteConsole /*= true,*/, bool bTimestamp /*= true*/ )
{
    return WriteLog( strEntry, bWriteConsole, bTimestamp );
}

void clib_closelog( void )
{
    if ( logFile.is_open() )
        logFile.close();
}

std::ofstream& clib_getlogfile( void )
{
    return logFile;
}

void clib_clearconsole( void )
{
    hCon = NULL;
}

HANDLE clib_getconsole( void )
{
    return hCon;
}

BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        break;

      case DLL_PROCESS_DETACH:
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
