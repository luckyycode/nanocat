//
//  Nanocat engine.
//
//  Game console..
//
//  Created by Neko Vision on 03/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


#include "MultiplayerClient.h"
#include "Core.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "FileSystem.h"
#include "CoreFont.h"
#include "Camera.h"
#include "System.h"
#include "Input.h" // For console input.
#include "GameMath.h" // Colors.

ncGameConsole _gconsole;

/*
    Console input from external/internal sources.
*/

void ncGameConsole::Execute( const char *msg, ... ) {
    va_list             argptr;
    static char         g_message[MAX_COMMAND_SIZE];

    va_start( argptr, msg );
    vsnprintf( (char*)g_message, sizeof(g_message), msg, argptr );
    va_end( argptr );

    if( strlen(g_message) > MAX_COMMAND_SIZE ) {
        _core.Print( LOG_DEVELOPER, "Too long command.\n", MAX_COMMAND_SIZE );
        return;
    }

    // We need to make our input string as char array.
    const char *cmds[MAX_COMMAND_TOKENS];

    int i;
    char *p;

    zeromem( cmds, sizeof(cmds) );

    i = 0;
    p = strtok (g_message, " ");

    while ( p ) {
        cmds[i++] = p;
        p = strtok (NULL, " ");
    }

    // Execute our commands now.
    _commandManager.Execute( cmds );
}

/*
    Render console and text.
*/
void ncGameConsole::Render( void )
{
    int i;

    if( !IsShown() ) {
        if( !IsUIShown() ) {
            _font.Print2D( COLOR_WHITE, 10, 15, 10, "Nanocat, %s", _core.GetVersionString() );
        }
        else {
            _font.Print2D( COLOR_GREEN, 10, 45, 10, "look: x: %4.2f y: %4.2f z: %4.2f", _camera.g_vLook.x, _camera.g_vLook.y, _camera.g_vLook.z );
            _font.Print2D( COLOR_GREEN, 10, 35, 10, "eye: x: %4.2f y: %4.2f z: %4.2f", _camera.g_vEye.x, _camera.g_vEye.y, _camera.g_vEye.z );
        }
    }
    else {
        // Version > buffer
        _font.Print2D( COLOR_RED, 10, 70, 8, "%s > %s", Prefix, Buffer );

        // Console log.
        for( i = 0; i < logCount; i++ ) {
            // Causes strange symbol on line end.
            strtok( Log[i + 1], "\n" );
            
            _font.Print2D( COLOR_WHITE, 10, 470 - ( i * CONSOLE_LINE_SKIP ), 8, "%s", Log[i + 1] );
        }
    }
}

/*
    Change console prefix.
*/
void ncGameConsole::SetPrefix( const char *prefix ) {
    if( !prefix ) {
        _core.Print( LOG_WARN, "Empty console prefix set.\n" );
        Prefix = "Nanocat";
        
        return;
    }

    Prefix = prefix;
}

/*
    Clear console, external and internal.
*/
void ncGameConsole::Clear( void ) {
    int h, j;

    // Write a log piece to the log file.
    if( Filesystem_Logging.GetInteger() )
        for( j = 0; j < logFill; j++ )
            _filesystem.WriteLog( Log[j] );

    // Clean the console buffer.
    for( h = 0; h < 256; h++ )
        for( j = 0; j < 512; j++ )
            Log[h][j] = '\0';

    logCount = 1;
    logFill = 0;

    if( _commandManager.ArgCount() > 1 ) {
        #ifdef _WIN32 // Clear Windows console.
            SendMessage( _con32.hwndBuffer, EM_SETSEL, 0, -1 );
            SendMessage( _con32.hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
            UpdateWindow( _con32.hwndBuffer );
        #endif // _WIN32
    }
}

void ncGameConsole::Initialize( void ) {
    int h, j;
    
    _core.Print( LOG_INFO, "Game console initializing...\n" );
    
    // Initialize console buffer - looks ugly but it's okaaay.
    for(  h = 0; h < CONSOLELOG_LINES; h++ )
        for( j = 0; j < CONSOLELOG_LINE_CHAR; j++ )
            Log[h][j] = '\0';
    
    // Some initial values.
    logCount    = 0;
    logFill     = 0;
    logErrors   = 0;
    logWarnings = 0;
    uiInfoShown = false;
    charFill    = 0;

}

void ncGameConsole::KeyInput( char key ) {
    
    // Console visibility.
    if( isShown ){
        if( key != KEY_ENTER ) {
            Buffer[charFill]  = key;//_input.GetKeyFromNum(key);
            
            if( key == KEY_BACKSPACE ) {
                charFill--;
                Buffer[charFill] = '\0';
                
                if( charFill < 0 )
                    charFill = 0;
            }
            else charFill++;
        }
    }
    else charFill = 0;
    
    
    switch( key ) {
        // Show/hide console.
        case '~':
            isShown = !isShown;
            zeromem( Buffer, MAX_COMMAND_SIZE );
            break;
        
        // Execute command buffer.
        case KEY_ENTER:
            if( isShown ) {
                _core.Print( LOG_NONE, "> %s\n", Buffer );
                
                Execute( Buffer );
                zeromem( Buffer, MAX_COMMAND_SIZE );
                
                charFill = 0;
            }
            break;
        default:
            break;
    }

}

void ncGameConsole::SwitchVisibility( void ) {
    isShown = !isShown;
}

bool ncGameConsole::IsShown( void ) {
    return isShown;
}

void ncGameConsole::SetVisible( bool visible ) {
    isShown = visible;
}

int ncGameConsole::GetLogLineCount( void ) {
    return logCount;
}

int ncGameConsole::GetLogFillCount( void ) {
    return logFill;
}

bool ncGameConsole::IsUIShown( void ) {
    return uiInfoShown;
}

FILE *ncGameConsole::GetLogFile( void ) {
    return logFile;
}


/*
    Add text to console window.
*/
void ncGameConsole::PrintExternal( const char *msg ) {
    
    static char buffer[CONSOLE_BUFFER_SIZE * 2];
    char *b = buffer;
    
    int i = 0;
    
    // Fix next line symbols.
    while ( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) ) {
        if ( msg[i] == '\n' ) {
            b[0] = '\r'; b[1] = '\n';
            b += 2;
        }
        else if ( msg[i] == '\r' ) {
            b[0] = '\r';
            b[1] = '\n';
            b += 2;
        }
        else if ( msg[i] == '\n' ) {
            b[0] = '\r';
            b[1] = '\n';
            b += 2;
        }
        else {
            *b= msg[i];
            b++;
        }
        i++;
    }
    *b = 0;
    
#ifdef _WIN32
    SendMessage( _con32.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
    SendMessage( _con32.hwndBuffer, EM_SCROLLCARET, 0, 0 );
    SendMessage( _con32.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
#else
    /*
        TODO something.
    */
#endif
}



/*
    Common stuff.
*/

