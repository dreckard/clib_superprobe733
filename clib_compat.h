//Cross platform compatibility
#ifndef CLIB_COMPAT_H
#define CLIB_COMPAT_H

#include <string>

#ifdef WIN32
    #include <windows.h>
    typedef HANDLE CP_FILE;
#elif __unix__
    #include <unistd.h>
    #include <fcntl.h>
    typedef int CP_FILE;
#endif

void cp_sleep( unsigned int ms );
ssize_t cp_write( CP_FILE file, const char* bytes, unsigned int count );
ssize_t cp_read( CP_FILE file, char* dst, unsigned int count );

#endif
