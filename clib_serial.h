#ifndef CLIB_SERIAL_H_
#define CLIB_SERIAL_H_

#include <string>

#define FLEXTRAN_ERRORSTR "ERROR-"

bool clib_serial_init( int iPort );
void clib_serial_close( void );
bool clib_serial_open( void );
HANDLE clib_serial_gethandle( void );
bool clib_serial_writebyte( const char cByte );
bool clib_serial_writestr( const std::string& str, bool bFlushRecv = true );
bool clib_serial_writestr_bytemeal( const std::string& str, bool bFlushRecv, unsigned int iCharDelay, unsigned int iLineDelay );
bool clib_serial_writecmd( const std::string& str, bool bErrorChk = false, bool bValidate = false );
char clib_serial_readbyte( void );
bool clib_serial_readbytes( char* cBuf, unsigned int iCount );
bool clib_serial_readstr( std::string& str );
bool clib_serial_readcmd( std::string& str );
bool clib_serial_flextranerror( void );
bool clib_serial_gettimeout( COMMTIMEOUTS& timeouts );
bool clib_serial_settimeout( COMMTIMEOUTS& timeouts );

bool WriteLog( const std::string& strEntry, bool bWriteConsole = true, bool bTimestamp = true );

#endif
