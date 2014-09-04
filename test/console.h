//
//  Nanocat engine.
//
//  Console manager.
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef console_h
#define console_h

#include "systemshared.h"

// Maximum *in-game* console log lines to render.
#define MAX_CONSOLE_LOG_LINES                   36

#define CONSOLE_LINE_SKIP 11

#define CONSOLELOG_LINES 256
#define CONSOLELOG_LINE_CHAR 512
#define MAX_COMMAND_SIZE                        64              // Max console command length.
// Maximum console command tokens.
#define MAX_COMMAND_TOKENS                      5

#define SHOW_CONSOLE_LINE_PREFIX

class ncGameConsole {

public:
    void Execute( const char *msg, ... );
    void Render( void );
    void SetPrefix( const char *prefix );
    void Clear( void );
    void KeyInput( uint key );
    void Initialize( void );
    void PrintExternal( const char *msg );
    
    bool    IsShown( void );
    void    SwitchVisibility( void );
    void    SetVisible( bool visible );
    int     GetLogLineCount( void );
    int     GetLogFillCount( void );
    bool    IsUIShown( void );
    FILE    *GetLogFile( void );
    
    // Log file to write our log.
    FILE        *logFile;
    
    // Fix me: to do more?
    char        Buffer[32];
    
    // Internal console log. [lines][characters]
    char        Log[CONSOLELOG_LINES][CONSOLELOG_LINE_CHAR];
    
    // Last executed buffer ( without the command itself ).
    char        lastBuffer[MAX_COMMAND_TOKENS - 1][MAX_COMMAND_SIZE];
    
    // How many chars were written.
    int         charFill;
    
    int         logWarnings;
    int         logErrors;
    int         logCount;
    int         logFill;
    
private:
    bool        isShown;
    
    // Show some info at main hud.
    bool        uiInfoShown;


    // Usually it's "prefix > command buffer".
    const char  *Prefix;
};

extern ncGameConsole _gconsole;

#endif
