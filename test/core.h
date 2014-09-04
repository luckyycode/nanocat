//
//  Nanocat engine.
//
//  Game core.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef core_h
#define core_h

#include "consolevar.h"

#define MAX_SPRINTF_BUFFER 16384
#define CONSOLE_BUFFER_SIZE	16384

// Error types.
enum Error_t {
    ERC_FATAL,              // Fatal error ( probably unknown ).
    ERC_CRASH,              // Crash caused by code faults.
    ERC_NETWORK,            // Network errors.
    ERC_CVAR,               // Console variables failed to allocate etc.
    ERC_GL,                 // Graphics system error.
    ERC_FS,                 // File system error.
    ERC_ASSET,              // Missing asset, corrupted asset etc.
    ERC_SERVER,             // Server faults.
    ERC_SYSTEM
};

// Log types.
enum Logtype_t {
    // No prefix to use. Used for next line print.
    LOG_NONE,
    // Warning.
    LOG_WARN,
    // Error.
    LOG_ERROR,
    // Debug, shows only when 'log_verbose' is '1'.
    LOG_DEVELOPER,
    LOG_INFO                // Just an info.
};

class Core {
public:
    void Preload( const char *execpath );
    void Printhelp( void );
    void Initialize( void );
    void Loaded( void );
    void Frame( void );
    void Disconnect( void );
    void Shutdown( void );
    void Error( Error_t err, const char *msg, ... );
    void Print( Logtype_t type, char const *msg, ... );
    
    int Time;
    
    bool Initialized;
    bool UseGraphics;   // Mostly used for dedicated server.
    
    
    const char *GetVersionString( void );
private:
    int LastTime;
    int FrameTime;
};

extern Core _core;

#endif
