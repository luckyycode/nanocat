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

struct ncFileChunk {
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
    void Initialize( const NString defaultpath );
    void GenerateConfigurationFile( void );
    void Shutdown( void );
    
    void LoadNFC( const NString nc );
    
    const NString GetFileName( const NString path );
    const NString GetFileExtension( const NString filename );
    
    void ReadConfiguration( void );
    void WriteLog( const NString msg );
    
    long GetFileLength( FILE *f );
    
    void Write( FILE *f, const void *buffer, int len );
    void Read( FILE *f, void *data, long len );
    
    FILE *OpenWrite( const NString filename );
    FILE *OpenRead( const NString filename );
    
    long Load( const NString filename, void **buffer );
};

extern ncFileSystem *c_FileSystem;

// FILE SYSTEM
extern ncConsoleVariable       Filesystem_Path;                           // Current game path.
extern ncConsoleVariable       Filesystem_Log;                            // Log filename.
extern ncConsoleVariable       Filesystem_Logging;                        // Is file logging enabled?


#endif
