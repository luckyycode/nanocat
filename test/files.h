//
//  Nanocat engine.
//
//  File system.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef files_h
#define files_h

#include "systemshared.h"
#include "consolevar.h"

struct filechunk_t {
    int offset;
    int length;
};

class ncFileSystem {
public:
    void Initialize( const char *defaultpath );
    void GenerateConfigurationFile( void );
    void Shutdown( void );
    
    const char *GetFileName( const char *path );
    const char *GetFileExtension( const char *filename );
    
    void ReadConfiguration( void );
    void WriteLog( const char *msg );
    
    long GetFileLength( FILE *f );
    
    void Write( FILE *f, const void *buffer, int len );
    void Read( FILE *f, void *data, long len );
    
    FILE *OpenWrite( const char *filename );
    FILE *OpenRead( const char *filename );
    
    long Load( const char *filename, void **buffer );
};

extern ncFileSystem _filesystem;

// FILE SYSTEM
extern ConsoleVariable       filesystem_path;                           // Current game path.
extern ConsoleVariable       filesystem_log;                            // Log filename.
extern ConsoleVariable       filesystem_logging;                        // Is file logging enabled?


#endif
