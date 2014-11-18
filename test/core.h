//
//  Nanocat engine.
//
//  Game core..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef core_h
#define core_h

#include "ConsoleVariable.h"
#include "NCString.h"

#define MAX_SPRINTF_BUFFER 16384
#define CONSOLE_BUFFER_SIZE	16384

// Some defines.
#define NC_LOG( text ) g_Core->Print( LOG_INFO, text )
#define NC_ASSERTWARN( what ) if( !what ) g_Core->Print( LOG_ASSERT, "Assert warning at line %i, %s function", __LINE__, __FUNCTION__ )
#define NC_TEXT _stringhelper.STR



// Global load states.
enum ncCoreGlobalLoadState {
    NCCLOAD_IDLE = 0,
    NCCLOAD_NETWORK,
    NCCLOAD_SERVER,
    NCCLOAD_CLIENT,
    NCCLOAD_RENDERER,
    NCCLOAD_CONSOLE,
    NCCLOAD_PREPARING,
    NCCLOAD_FILESYSTEM,
    NCCLOAD_ASSETMANAGER,
    NCCLOAD_OPENGL,
    NCCLOAD_SYSTEM,
    NCCLOAD_CONSOLEVARIABLES,
    NCCLOAD_LOCALGAME,
    NCCLOAD_INPUT
};

// Error types.
enum ncCoreErrorType {
    ERR_FATAL = 0,          // Fatal error ( probably unknown ).
    ERR_NETWORK,            // Network errors.
    ERR_OPENGL,             // Graphics system error.
    ERR_FILESYSTEM,         // File system error.
    ERR_ASSET,              // Missing asset, corrupted asset etc.
    ERR_SERVER,             // Server faults.
    ERR_CLIENT,             // Client error.
    ERR_CORE,               // Core error.
    ERR_LOGIC,              // Logic error.
    ERR_SYSTEM              // System stuff error.
};

// Log types.
enum ncCoreLogType {
    // No prefix to use. Used for next line print.
    LOG_NONE = 0,
    // Warning.
    LOG_WARN,
    // Error.
    LOG_ERROR,
    // Debug, shows only when 'log_verbose' is '1'.
    LOG_DEVELOPER,
    // NC_ASSERT
    LOG_ASSERT,
    LOG_INFO                // Just an info.
};

class ncCore {
public:

    void Preload( const NString execpath );
    void Printhelp( void );
    void Initialize( void );
    void Loaded( void );
    void Frame( void );
    void Disconnect( void );
    void Shutdown( void );
    void Error( ncCoreErrorType err, const NString msg, ... );
    
    void Print( const NString msg );
    void Print( ncCoreLogType type, char const *msg, ... );
    
    void DPrint( const NString msg );
    
    int Time;
    
    bool Initialized;
    bool UseGraphics;   // Mostly used for dedicated server.
    
    ncCoreGlobalLoadState LoadState;
    
    const NString GetVersioNString ( void );
    
    unsigned int GetVersionMajor( void );
    unsigned int GetVersionMinor( void );
    
    double GetVersionDouble( void );
    
private:
    int LastTime;
    int FrameTime;
};

extern ncCore   *g_Core;

#endif
