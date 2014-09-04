//
//  ncstring.cpp
//
//  String helper.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "ncstring.h"
#include "systemshared.h"

ncStringHelper _stringhelper;

/*
    Copy string to destination.
*/
char *ncStringHelper::Copy( char *d, const char *s ) {
    
    if( !d ) {
        _core.Error( ERC_FATAL, "ncStringHelper::Copy failed, destination is null." );
    }
    
    if( !s ) {
        _core.Error( ERC_FATAL, "ncStringHelper::Copy failed, source is null." );
    }
    
    char *saved = d;
    
    while( *s ) {
        *d++ = *s++;
    }
    
    *d = 0;
    
    return saved;
}

/*
    Remove character from string.
*/
void ncStringHelper::RemoveChar( char *str, char garbage ) {
    char *src, *dst;
    
    for( src = dst = str; *src != '\0'; src++ ) {
        *dst = *src;
        if( *dst != garbage ) dst++;
    }
    
    *dst = '\0';
}

/*
    Safe copy string.
*/
void ncStringHelper::SafeCopy( char *dest, const char *src, unsigned long size ) {
    
    if ( !src )
        _core.Error( ERC_FATAL, "ncStringHelper::SafeCopy failed, null source." );
    
    if ( size < 1 )
        _core.Error( ERC_FATAL, "ncStringHelper::SafeCopy failed, size is negative. :(" );
    
    strncpy( dest, src, size - 1 );
    dest[size - 1] = 0;
}

/*
    Safe copy character array.
*/
void ncStringHelper::SPrintf( char *dest,
                    ulong size, const char *fmt, ...) {
    
    int len;
    va_list	argptr;
    static char	bigbuffer[MAX_SPRINTF_BUFFER];
    
    va_start( argptr, fmt );
    
    len = vsprintf( bigbuffer, fmt, argptr );
    
    va_end( argptr );
    
    // Copy now.
    SafeCopy( dest, bigbuffer, size );
}

/*
    Skip all next lines and return simple one lined string.
*/
void ncStringHelper::Chomp(char *s) {
    while( *s && *s != '\n' && *s != '\r' ) s++;
    
    *s = 0;
}

/*
    Skip the chosen character.
*/
void ncStringHelper::SkipCharacter( char *s, char a ) {
    while(*s != a) s++;
    
    *s = 0;
}

/*
    Combine the parts in one string.
*/
const char *ncStringHelper::STR( const char *msg, ... ) {
    va_list     argptr;
    char        text[MAX_SPRINTF_BUFFER];
    
    va_start( argptr, msg );
    vsnprintf( text, sizeof(text), msg, argptr );
    va_end( argptr );
    
    // Fix-me.
    return text;
}

/*
    Check if string contains next line.
*/
bool ncStringHelper::ContainsNextLine( char *s ) {
    int i;
    
    for( i = 0; i < strlen(s); i++ )
        if( s[i] == '\n' )
            return true;
    
    return false;
}