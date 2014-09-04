//
//  Nanocat engine.
//
//  Command manager.
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "core.h"
#include "command.h"
#include "console.h"
#include "ncstring.h"

ncCommandManager _commandManager;
ncConsoleCommand *cmds;

static int argc;

/*
    Add command, not its variable.
*/
void ncCommandManager::Add( const char *cmd, functioncall_t command ) {
    ncConsoleCommand   *c_temp;
    c_temp = &cmds[CommandCount];

    c_temp->function = command;
    _stringhelper.SPrintf( c_temp->name, strlen(cmd) + 1, cmd );

    CommandCount++;
}

/*
    Get parameter at index
*/
char *ncCommandManager::Arguments( int c ) {
    if( c > MAX_COMMAND_TOKENS )
        return NULL;

    // fix
    return _gconsole.lastBuffer[c + 1];
}

/*
    Get parameter count.
*/
int ncCommandManager::ArgCount( void ) {
    return argc;
}

/*
    Initialize command stuff.
*/
void ncCommandManager::Initialize( void ) {
    CommandCount = 0;

    cmds = (ncConsoleCommand*)malloc( sizeof( ncConsoleCommand ) * MAX_CVARS );

    if( !cmds )
        _core.Error( ERC_FATAL, "No enough memory to allocate memory for %i commands.\n", MAX_CVARS );
}

/*
    Execute command or a console variable if found.
*/
void ncCommandManager::Execute( const char *cmd[] ) {
    if( !cmd[0] )
        return;

    int g_argc, g_cmd;

    // Clear last buffer.
    zeromem( _gconsole.lastBuffer, sizeof(_gconsole.lastBuffer) );
    argc = 0;

    /*
        Oooh great look at following codes...
    */

    // Fill last buffer.
    for( g_argc = 0; g_argc < MAX_COMMAND_TOKENS; g_argc ++ ) {
        if( cmd[g_argc] != NULL ) {
            _stringhelper.SPrintf( _gconsole.lastBuffer[g_argc], strlen(cmd[g_argc]) + 1, cmd[g_argc] );
            argc ++;
        }
    }

    // Nano Cat listener
    //NC_Listen();

    argc = 0;

    // Find a command and do what it needs..
    for( g_cmd= 0; g_cmd < CommandCount; g_cmd++ ) {
        if( !strcmp( cmd[0], cmds[g_cmd].name ) ) {
            for( g_argc = 0; g_argc < MAX_COMMAND_TOKENS; g_argc ++ ) {
                if( cmd[g_argc] != NULL ) {
                    _stringhelper.SPrintf( _gconsole.lastBuffer[g_argc], strlen(cmd[g_argc]) + 1, cmd[g_argc] );
                    argc ++;
                }
            }

            cmds[g_cmd].function();
            return;
        }
    }

    // Console variables.
    static char g_message[1024];
    
    sprintf( (char*)g_message, "%s", cmd[0] );
    
    int     i;
    char    *p;
    const char    *token[8];
    
    // This is retarded.
    for( i = 0; i < 8; i++ ) token[i] = "";

    i = 0;
    p = strtok( (char*)g_message, "." );
    
    while( p != NULL ) {
        
        token[i++] = p;
        p = strtok( NULL, "." );
    }
    
    if( strlen(token[1]) < 1  ) {
        _core.Print( LOG_INFO, "Available commands.\n" );
        for( i = 0; i < _cvarmngr.consoleVariableCount; i++ ) {
            if( !strcmp( _cvarmngr.consoleVariables[i]->GetGroup(), token[0] ) ) {
                
                _core.Print( LOG_INFO, "%s - %s\n", _cvarmngr.consoleVariables[i]->GetName(), _cvarmngr.consoleVariables[i]->GetDescription() );
            }
        }
        return;
    }
    
    if( !cmd[1] ) {
        _core.Print( LOG_INFO, "USAGE: group.command <value>\n" );
        return;
    }
    
    for( i = 0; i < _cvarmngr.consoleVariableCount; i++ ) {
        
        if( !strcmp( _cvarmngr.consoleVariables[i]->GetGroup(), token[0] ) ) {
            if( !strcmp( _cvarmngr.consoleVariables[i]->GetName(), token[1] )) {
                
                // Okay, we got this variable.
                _cvarmngr.consoleVariables[i]->Set( cmd[1] );
                return;
            }
        }
    }
    
    
    // _core.Print( LOG_INFO, "Unknown command \"%s\".\n", cmd[0] );
}



