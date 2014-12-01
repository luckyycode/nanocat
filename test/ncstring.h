//
//  Nanocat engine.
//
//  String helper..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef ncstring_h
#define ncstring_h

#include "SystemShared.h"

// Main string class.
class ncStringHelper {
public:
    void        SafeCopy( NString dest, const NString src, unsigned long size );
    void        Chomp( NString s );
    void        SkipCharacter( NString s, char a );
    void        SPrintf( NString dest, unsigned long size, const NString fmt, ...);
    void        RemoveChar( NString str, char garbage );
    
    const char  *STR( const NString msg, ...);
    bool        ContainsNextLine( NString s );
    char        *Copy(NString d, const NString s);
};

class ncString {
    friend class ncStringHelper;
    
public:
    ncString( const NString text );
    
    void CopyTo( ncString *to );
    void CopyFrom( ncString *to );
    
    ncString operator=(const NString s) const;
    
    const NString Owner;
};

extern ncStringHelper *g_stringHelper;

#endif
