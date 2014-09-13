//
//  Nanocat engine.
//
//  File system..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef files_h
#define files_h

#include "SystemShared.h"
#include "ConsoleVariable.h"

struct filechunk_t {
    int offset;
    int length;
};

struct ncFCHeader {
    int header;
    long files;
};

class ncFileContainer {
public:
    ncFCHeader header;
};

class ncFileSystem {
public:
    void Initialize( const char *defaultpath );
    void GenerateConfigurationFile( void );
    void Shutdown( void );
    
    void LoadNFC( const char *nc );
    
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
extern ncConsoleVariable       Filesystem_Path;                           // Current game path.
extern ncConsoleVariable       Filesystem_Log;                            // Log filename.
extern ncConsoleVariable       Filesystem_Logging;                        // Is file logging enabled?


#endif
