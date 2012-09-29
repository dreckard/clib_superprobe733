#include "clib_compat.h"

void cp_sleep( unsigned int ms )
{
    #ifdef WIN32
        Sleep( ms );
    #elif defined __unix__
        usleep( ms*1000 );
    #endif
}

ssize_t cp_write( CP_FILE file, const char* bytes, size_t count )
{
    #ifdef WIN32
        DWORD dwRet = 0;
        if ( WriteFile( file, bytes, count, &dwRet, NULL ) )
            return dwRet;
    #elif defined __unix__
        return write( file, bytes, count );
    #endif
    return -1;
}

ssize_t cp_read( CP_FILE file, char* dst, size_t count )
{
    #ifdef WIN32
        DWORD dwRet = 0;
        if ( ReadFile( file, dst, count, &dwRet, NULL ) )
            return dwRet;
    #elif defined __unix__
        return read( file, dst, count );
    #endif
    return -1;
}
