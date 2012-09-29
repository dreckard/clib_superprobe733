#include <cstdio>
#include <fstream>
#include <time.h>
#include <windows.h>
#include "clib_serial.h"
#include "clib_superprobe733.h"
#include "clib_compat.h"

static HANDLE hSerial = INVALID_HANDLE_VALUE;
COMMTIMEOUTS stdTimeouts = {0};
COMMTIMEOUTS flushTimeouts = {0};
bool clib_serial_init( int iPort )
{
	if ( hSerial != INVALID_HANDLE_VALUE )
		clib_serial_close();

    #ifdef WIN32
        char szPort[8];
        snprintf( szPort, 8, "COM%i", iPort );

        hSerial = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if ( hSerial == INVALID_HANDLE_VALUE )
            return false;

        DCB dcbParams = {0};
        if ( !GetCommState( hSerial, &dcbParams ) )
           return false;

        dcbParams.DCBlength = sizeof(dcbParams);
        dcbParams.BaudRate = CBR_1200;
        dcbParams.ByteSize = 8;
        dcbParams.StopBits = ONESTOPBIT;
        dcbParams.Parity = NOPARITY;
        dcbParams.fAbortOnError = true;

        if ( !SetCommState( hSerial, &dcbParams ) )
           return false;

        //All values in ms
        stdTimeouts.ReadIntervalTimeout = 50; //Was working with 500, 250, 500
        stdTimeouts.ReadTotalTimeoutConstant = 50;
        stdTimeouts.ReadTotalTimeoutMultiplier = 50;
        stdTimeouts.WriteTotalTimeoutConstant = 100;
        stdTimeouts.WriteTotalTimeoutMultiplier = 10;

        flushTimeouts.ReadIntervalTimeout = MAXDWORD;
        flushTimeouts.ReadTotalTimeoutConstant = 0;
        flushTimeouts.ReadTotalTimeoutMultiplier = 0;
        flushTimeouts.WriteTotalTimeoutConstant = 0;
        flushTimeouts.WriteTotalTimeoutMultiplier = 0;

        if ( !SetCommTimeouts( hSerial, &stdTimeouts ) )
           return false;
    #elif defined __unix__
        int fd;
        fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
        if ( fd == -1 )
            return false;
        fnctl( fd, F_SETFL, 0 );

        struct termios options;
        tcgetattr(fd, &options);
        cfsetispeed(&options, B1200);
        cfsetospeed(&options, B1200);

        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~CNEW_RTSCTS;

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;

        tcsetattr(fd, TCSANOW, &options);
    #endif

	WriteLog( "Initialized\n" );

	return true;
}

bool clib_serial_gettimeout( COMMTIMEOUTS& timeouts )
{
    return GetCommTimeouts( hSerial, &timeouts );
}

bool clib_serial_settimeout( COMMTIMEOUTS& timeouts )
{
    return SetCommTimeouts( hSerial, &timeouts );
}

void clib_serial_close( void )
{
	if ( hSerial != INVALID_HANDLE_VALUE )
        CloseHandle( hSerial );

    #ifdef WIN32
        CloseHandle( hSerial );
    #elif __unix__
        close(fd);
    #endif
	hSerial = INVALID_HANDLE_VALUE;

	WriteLog( "Closed\n\n" );
}

bool clib_serial_open( void )
{
    return hSerial != INVALID_HANDLE_VALUE;
}

HANDLE clib_serial_gethandle( void )
{
    return hSerial;
}

bool clib_serial_writebyte( const char cByte )
{
	//DWORD dwBytesWritten;
	//return ( WriteFile( hSerial, &cByte, sizeof(cByte), &dwBytesWritten, NULL ) && dwBytesWritten > 0 );
	return cp_write( hSerial, &cByte, 1 ) > 0;
}

bool clib_serial_flushrecv( void )
{
    DWORD dwBytes;
    if ( !SetCommTimeouts( hSerial, &flushTimeouts ) )
        return false;
    char dummy[32];
    while ( ReadFile( hSerial, dummy, 32, &dwBytes, NULL ) && dwBytes >= 32 ) ;

    if ( !SetCommTimeouts( hSerial, &stdTimeouts ) )
        return false;
    return true;
}

//Length comes from std::string; bFlushRecv flushes the receive buffer before transmitting
bool clib_serial_writestr( const std::string& str, bool bFlushRecv /*=true*/ )
{
	//DWORD dwBytes;

	//Remove any trailing newline
	std::string logStr = str;
	if ( logStr[logStr.length()-1] == '\r' || logStr[logStr.length()-1] == '\n' )
	   logStr.erase( logStr.length()-1 );

	if ( bFlushRecv && !clib_serial_flushrecv() )
	{
        WriteLog( "Failed to flush recv for write: " + logStr + '\n' );
	    return false;
    }
    //if ( WriteFile( hSerial, str.data(), str.length(), &dwBytes, NULL ) && dwBytes >= str.length() )
    if ( cp_write( hSerial, str.data(), str.length() ) >= str.length() )
    {
        WriteLog( "<< " + logStr + '\n' );
        return true;
    }
    WriteLog( "Failed to write: " + logStr + '\n' );
    return false;
}

bool clib_serial_writestr_bytemeal( const std::string& str, bool bFlushRecv, unsigned int iCharDelay, unsigned int iLineDelay )
{
    std::string logStr = str;
	if ( logStr[logStr.length()-1] == '\r' || logStr[logStr.length()-1] == '\n' )
	   logStr.erase( logStr.length()-1 );

	if ( bFlushRecv && !clib_serial_flushrecv() )
	{
        WriteLog( "Failed to flush recv for write: " + logStr + '\n' );
	    return false;
    }

    for ( unsigned int i = 0; i < str.length(); i++ )
    {
        char cByte = str[i];
        if ( !clib_serial_writebyte( cByte ) )
            return false;

        if ( cByte == '\r' )
            cp_sleep( iLineDelay );
        else
            cp_sleep( iCharDelay );
    }

    return true;
}

//Arguments
//  str: command
//  bErrorChk - if true, reads from serial to check for flextran error
//  bValidate - if true, reads echo (must be on) from 5500 to validate command before sending \r
bool clib_serial_writecmd( const std::string& str, bool bErrorChk /*=false*/, bool bValidate /*=false*/ )
{
    if ( bValidate )
    {
        std::string recv( 128, 1 );
        for ( int i=0; i < 5; i++ )
        {
            if ( !clib_serial_writestr( str ) )
                return false;
            cp_sleep( 10*str.length() );
            if ( !clib_serial_readstr( recv ) )
                return false;
            if ( recv.compare( 0, str.length(), str ) == 0 )
            {
                if ( !clib_serial_writebyte( '\r' ) )
                    return false;
                if ( bErrorChk && clib_serial_flextranerror() )
                    return false;
                return true;
            }
            else
            {
                clib_probe_abortexec();
                cp_sleep( 250 );
            }
        }
        return false;
    }

    if ( !clib_serial_writestr( str ) )
        return false;

    if ( bErrorChk && clib_serial_flextranerror() )
        return false;

    return true;
}

char clib_serial_readbyte( void )
{
	char cRet = 0;
    //DWORD dwBytesRead = 0;
	//if ( ReadFile( hSerial, &cRet, 1, &dwBytesRead, NULL ) && dwBytesRead > 0 )
	if ( cp_read( hSerial, &cRet, 1 ) > 0 )
	   return cRet;
	return -1;
}

bool clib_serial_readbytes( char* cBuf, unsigned int iCount )
{
    //DWORD dwBytesRead = 0;
	//if ( ReadFile( hSerial, cBuf, iCount, &dwBytesRead, NULL ) && dwBytesRead > iCount )
    //  return true;
	return cp_read( hSerial, cBuf, iCount ) != -1;
}

//This just uses the std::string length for reading, so set it before you pass
bool clib_serial_readstr( std::string& str )
{
    size_t len = str.length();
    char cstr[len];

	//DWORD dwBytesRead = 0;
    //if ( ReadFile( hSerial, cstr, len, &dwBytesRead, NULL ) && dwBytesRead > 0 )
    if ( cp_read( hSerial, cstr, len ) > 0 )
    {
        str = cstr;
        WriteLog( ">> " + str + '\n' );
        return true;
    }
    return false;
}

bool clib_serial_readcmd( std::string& str )
{
    if ( !clib_serial_readstr( str ) )
        return false;
    if ( clib_serial_flextranerror() )
        return false;

    //The last two characters should be \r* with echo on
    //str.erase( str.length()-2 );

    return true;
}

//Returns true if error string is found
bool clib_serial_flextranerror( void )
{
    std::string recv( 16, 1 );
    if ( clib_serial_readstr( recv ) && recv.find( FLEXTRAN_ERRORSTR ) != std::string::npos )
        return true;
    return false;
}

bool WriteLog( const std::string& strEntry, bool bWriteConsole /*=true*/, bool bTimestamp /*=true*/ )
{
    std::ofstream& ofs = clib_getlogfile();
    //DWORD dwBytesWritten = 0;
	HANDLE hConsole = clib_getconsole();
	bool bLog = ( !ofs.fail() && ofs.is_open() );
    if ( !bLog && !hConsole )
        return false;
    if ( bTimestamp )
    {
        char szTime[64];
        time_t stTime = time(NULL);
        strncpy( szTime, ctime(&stTime), 64 );
	    szTime[strlen(szTime)-1] = '\0';
	    std::string szOut = szTime;
	    szOut += ": " + strEntry;
		if ( bLog )
			ofs << szOut;
	    if ( hConsole && bWriteConsole )
            cp_write( hConsole, szOut.c_str(), szOut.length() );
            //WriteFile( hConsole, szOut.c_str(), szOut.length(), &dwBytesWritten, NULL );
	    //ofs << szTime << ": " + strEntry;
    }
    else
    {
		if ( bLog )
			ofs << strEntry;
        if ( hConsole && bWriteConsole )
            //WriteFile( hConsole, strEntry.c_str(), strEntry.length(), &dwBytesWritten, NULL );
            cp_write( hConsole, strEntry.c_str(), strEntry.length() );
    }
	if ( bLog )
		ofs.flush();
    return ( ( bLog && !ofs.fail() ) || ( hConsole && bWriteConsole ) );
}
