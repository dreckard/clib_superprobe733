#include "clib_superprobe733.h"
#include "clib_serial.h"
#include "clib_compat.h"
#include <string.h>
#include <cstdio>
#include <cmath>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

bool FltEq( const float flA, const float flB, const float flEpsilon )
{
    return fabs(flA-flB) < flEpsilon;
}

//TN-5500 Notes:
    //Use \r (015) for return
    //CAUTION: Exceeding 16 characters or so without sending \r causes string comms to become unreliable

    //FLEXTRAN
    //P - Print function
    //  Syntax: P <xw,d>expr1,<...>expr2...
    //  x: F = Fixed point, E = Exponential, O = Octal
    //  w: Field width; d: Places to print after decimal
    //A - Allocate
    //  Allocates a variable/array with name of 1 or 2 chars
    //  Syntax: A name(num)
    //  NOTE: num+1 will be allocated, i.e. "A X(3)" creates a 4 member array
    //Errors:
    //  The Flextran interpreter will print ERROR-x with a code 0-10

//There should really be better error feedback on these than just succeed or fail
//The TN-5600 displays 3 places past the decimal for stage positions
bool clib_probe_readpos( Float3& fPos )
{
    char szCmd[32];
    for ( int i = 0; i < 3; i++ )
    {
        snprintf( szCmd, 32, "P <F10,3>$RE(%i,I,1)\r", 1+i );
        clib_serial_writecmd( szCmd );
        std::string recv( 16, 1 );
        if ( !clib_serial_readcmd( recv ) )
            return false;
        fPos[i] = atof( recv.c_str() );
    }
    return true;
}

bool clib_probe_posrel( const Float3& fPos )
{
    char szCmd[32];
    for ( int i = 0; i < 3; i++ )
    {
        if ( FltEq(fPos.fValues[i], 0, 0.0001f ) )
            continue;

        snprintf( szCmd, 32, "$AX(%i,-1,%.3f,I,7)\r", 1+i, fPos.fValues[i] );
        if ( !clib_serial_writecmd( szCmd, true ) )
            return false;
    }
    return true;
}
bool clib_probe_posabs( const Float3& fPos )
{
    char szCmd[32];
    for ( int i = 0; i < 3; i++ )
    {
        //Negative values are skipped
        if ( fPos.fValues[i] < 0.0f )
            continue;

        snprintf( szCmd, 32, "$AX(%i,1,%.3f,I,7)\r", 1+i, fPos.fValues[i] );
        if ( !clib_serial_writecmd( szCmd, true ) )
            return false;
    }
    return true;
}

//The TN-5600 displays 2 places past the decimal for current
bool clib_probe_readabscurrent( float& flCurrent )
{
    clib_serial_writecmd( "P <F10,2>$RE(0,I,1)\r" );

    std::string recv( 16, 1 );
    if ( !clib_serial_readcmd( recv ) )
        return false;

    flCurrent = atof( recv.c_str() );
    return true;
}

bool clib_probe_allocflextranvars( void )
{
    //!!FJ(3) allocates a 4 member zero based array!!
    return clib_serial_writecmd( "A I,FJ(3);S I=0;S FJ=0\\0\\0\\0\r", true );
}

bool clib_probe_countspectrometers( const float& flTime )
{
    char szCmd[32];
    for ( int i = 0; i < 4; i++ )
    {
        snprintf( szCmd, 32, "$AQ(%i,1,%.2f,I,FJ(%i))\r", 16+i, flTime, i );
        if ( !clib_serial_writecmd( szCmd, true ) )
            return false;
    }
    return true;
}

//The TN-5600 demands a float for spectrometer counts (for range presumably) despite it being an inherently integer quantity
bool clib_probe_readspectrometercounts( Float4& nCounts )
{
    std::string recv( 16, 1 );
    for ( int i = 0; i < 4; i++ )
    {
        char szCmd[32];
        snprintf( szCmd, 32, "P <F16,4>FJ(%i)\r", i );
        if ( !clib_serial_writecmd( szCmd ) )
            return false;
        if ( !clib_serial_readcmd( recv ) )
            return false;
        nCounts[i] = atoi(recv.c_str());
    }
    return true;
}

//This function is NOT reentrant
TN5500_Data rxData = {};
DWORD dwByteCtr = 0;
void clib_probe_listen( clib_callback_class* pObj, float flTimeout /*=0.0f*/ )
{
    if ( !clib_serial_open() )
        return;

    HANDLE hSerial = clib_serial_gethandle();

    clock_t endtime = clock();
    if ( flTimeout > 0.0f )
        endtime += flTimeout * CLOCKS_PER_SEC;
    else
        endtime += 1;

    //COMMTIMEOUTS oldTimeouts, listenTimeouts;

    //listenTimeouts.ReadIntervalTimeout = 50; //Was working with 500, 250, 500
	//listenTimeouts.ReadTotalTimeoutConstant = 50;
	//listenTimeouts.ReadTotalTimeoutMultiplier = 50;
	//listenTimeouts.WriteTotalTimeoutConstant = 100;
	//listenTimeouts.WriteTotalTimeoutMultiplier = 10;

    //clib_serial_gettimeout( oldTimeouts );
    //clib_serial_settimeout( listenTimeouts );

    while ( endtime > clock() )
    {
        DWORD dwBytesRead = 0;
        if ( ReadFile( hSerial, (char*)((&rxData)+dwByteCtr), sizeof(rxData)-dwByteCtr, &dwBytesRead, NULL ) )
        {
            dwByteCtr += dwBytesRead;
            if ( dwByteCtr >= sizeof(rxData) )
            {
                pObj->dataReceived( rxData );
                endtime = clock() + flTimeout * CLOCKS_PER_SEC;
                dwByteCtr = 0;
            }
        }

        /*if ( clib_serial_readbytes( (char*)(&rxData), sizeof(rxData)-cByteCtr ) )
        {
			pObj->dataReceived( rxData );
            endtime = clock() + flTimeout * CLOCKS_PER_SEC;
        }*/
    }

    //clib_serial_settimeout( oldTimeouts );
}
void clib_probe_listen_reset( void )
{
    dwByteCtr = 0;
}

//Sends the ESC (033) key to abort current execution
bool clib_probe_abortexec( void )
{
    bool bRet = clib_serial_writebyte( 033 );
    cp_sleep( ABORT_DELAY );
    return bRet;
}

//Toggle echo with CTRL-A (001)
bool clib_probe_toggleecho( void )
{
    return clib_serial_writebyte( 001 );
}

#define TEST_STR "PING"
bool clib_probe_echooff( void )
{
    //char outStr[9] = TEST_STR;
    if ( !clib_serial_writecmd( TEST_STR ) )
        return false;

    std::string inStr( 16, 1 );
    if ( clib_serial_readstr( inStr ) || inStr.find( FLEXTRAN_ERRORSTR ) != std::string::npos )
    {
        if ( inStr.find( TEST_STR ) != std::string::npos )
            clib_probe_toggleecho();
    }

    //for ( int i = 0; i < 8; i++ )
    //    clib_serial_writebyte( 0177 ); //DEL
    return clib_probe_abortexec();
}

bool clib_probe_sendfile( std::string fileName, int iCharDelay, int iLineDelay /*= NEWLINE_DELAY*/ )
{
    using namespace std;
    ifstream ifs( fileName.c_str() );
    if ( !ifs.good() )
        return false;
    char cByte;
    char cPrevByte = '\r';

    while ( 1 )
    {
        ifs.get( cByte );
        if ( !ifs.good() )
            break;
        if ( cByte == '\n' )
            cByte = '\r';

        if ( cByte == '\r' && cPrevByte == '\r' ) //No reason to send blank lines
            continue;
        if ( cByte == '%' && cPrevByte == '\r' )
        {
            while ( 1 )
            {
                char tmp = ifs.get();
                if ( (tmp == '\r' || tmp == '\n') && ifs.peek() != '\r' && ifs.peek() != '\n' )
                    break;
            }
            continue;
        }
        clib_serial_writebyte( cByte );
        if ( cByte == '\r' )
            cp_sleep( iLineDelay );
        else
            cp_sleep( iCharDelay );
        cPrevByte = cByte;
    }

    //Write a terminating newline if there isn't one already
    if ( cPrevByte != '\r' )
        clib_serial_writebyte( '\r' );

    ifs.close();
    return !ifs.fail();
}
