//
//  Nanocat engine.
//
//  String helper.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef ncstring_h
#define ncstring_h

// Main string class.
class ncStringHelper {
public:
    void        SafeCopy( char *dest, const char *src, unsigned long size );
    void        Chomp( char *s );
    void        SkipCharacter( char *s, char a );
    void        SPrintf( char *dest, unsigned long size, const char *fmt, ...);
    void        RemoveChar( char *str, char garbage );
    
    const char  *STR( const char *msg, ...);
    bool        ContainsNextLine( char *s );
    char        *Copy(char *d, const char *s);
};

extern ncStringHelper _stringhelper;

#endif
