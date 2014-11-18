//
//  ncstring.cpp
//
//  String helper.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "NCString.h"
#include "SystemShared.h"

ncStringHelper _stringhelper;

/*
    Copy string to destination..
*/
NString ncStringHelper::Copy( NString d, const NString s ) {
    
    if( !d ) {
        g_Core->Error( ERR_FATAL, "ncStringHelper::Copy failed, destination is null." );
    }
    
    if( !s ) {
        g_Core->Error( ERR_FATAL, "ncStringHelper::Copy failed, source is null." );
    }
    
    NString dest = d;
    
    while( *s ) {
        *d++ = *s++;
    }
    
    *d = 0;
    
    return dest;
}

/*
    Remove character from string.
*/
void ncStringHelper::RemoveChar( NString str, char garbage ) {
    NString src;
    NString dst;
    
    for( src = dst = str; *src != '\0'; src++ ) {
        *dst = *src;
        if( *dst != garbage ) dst++;
    }
    
    *dst = '\0';
}

/*
    Safe copy string.
*/
void ncStringHelper::SafeCopy( NString dest, const NString src, unsigned long size ) {
    
    if ( !src ) {
        g_Core->Error( ERR_FATAL, "ncStringHelper::SafeCopy failed, null source." );
    }
        
    if ( size < 1 ) {
        g_Core->Error( ERR_FATAL, "ncStringHelper::SafeCopy failed, size is negative. :(" );
    }
    
    strncpy( dest, src, size - 1 );
    dest[size - 1] = 0;
}

/*
    Safe copy character array.
*/
void ncStringHelper::SPrintf( NString dest, ulong size, const NString fmt, ...) {
    
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
void ncStringHelper::Chomp(NString s) {
    while( *s && *s != '\n' && *s != '\r' ) s++;
    
    *s = 0;
}

/*
    Skip the chosen character.
*/
void ncStringHelper::SkipCharacter( NString s, char a ) {
    while(*s != a) s++;
    
    *s = 0;
}

/*
    Combine the parts in one string.
*/
const NString ncStringHelper::STR( const NString msg, ... ) {
    va_list     argptr;
    char        text[MAX_SPRINTF_BUFFER];
    
    va_start( argptr, msg );
    
    vsnprintf( text, sizeof(text), msg, argptr );
    
    va_end( argptr );
    
    // Fix-me.
    
#pragma mark - Fix me
    return text;
}

/*
    Check if string contains next line.
*/
bool ncStringHelper::ContainsNextLine( NString s ) {
    int i;
    
    for( i = 0; i < strlen(s); i++ ) {
        if( s[i] == '\n' ) {
            return true;
        }
    }
    
    return false;
}