//
//  Nanocat engine.
//
//  Game core..
//
//  Created by Neko Vision on 10/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "MultiplayerServer.h"
#include "MultiplayerClient.h"
#include "System.h"
#include "Network.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "Input.h"
#include "Renderer.h"
#include "FileSystem.h"
#include "NCString.h"
#include "OpenGL.h"
#include "LocalGame.h"
#include "Utils.h"


ncCore _core;

ncConsoleVariable      core_version("core", "version", "Game version", "6q", CVFLAG_NEEDSREFRESH);
ncConsoleVariable      core_maxfps("core", "maxfps", "Maximum game framerate.", "90", CVFLAG_NEEDSREFRESH);
ncConsoleVariable      core_timemultiplier("core", "timescale", "Time scale.", "1.0", CVFLAG_KID);
ncConsoleVariable      log_verbose("log", "verbose", "Show more detailed log?", "1", CVFLAG_NONE);

/*
    Lazy functions. 
*/
static void lazyReadConfig( void ) {
    _filesystem.ReadConfiguration();
}

static void lazyShutdown( void ) {
    _core.Shutdown();
}

static void lazyPrinthelp( void ) {
    _core.Printhelp();
}

static void lazyDisconnect( void ) {
    _core.Disconnect();
}

static void lazyError( void ){
    _clientgame.Error();
}

static void lazyConvertModel( void ) {
    ncUtils::OBJtoSM( _commandManager.Arguments(0) );
}

const uint32 _randomWords[] = {
    0xfeedc0de,
    0x1337d05e,
    0xdeadbeef,
    0x1337c0de,
};

unsigned char Sharedsecret[] =
{   0x3f,0x58,0xf0,0xbe,0xba,0x68,0x54,0x07,0xe5,0xc7,
    0x6b,0x23,0x05,0x33,0x00,0x18
};

void temp_tokenize( void ) {
    _core.Print( LOG_INFO, "%x %x\n", _randomWords[0], _randomWords[1] );
    
    ncNetdata temp;
    _netmanager.Resolve( &temp, "stackoverflow.com" );
}

static void lazyConsoleClear( void ) {
    _gconsole.Clear();
}

void lazyConnectA() {
    _client.LoopbackConnect();
}

/*
    Pre-load stuff, even before context & graphic stuff load.
*/
void ncCore::Preload( const char *execpath ) {
    
    // O
   
    
    // Reset game time.
    Time = 0;
    LastTime = 0;
    FrameTime = 0;

    _gconsole.Initialize();
    
    Initialized = false;

    // Server initialization isn't called yet.
    _server.Initialized = false;


    // Commands.
    _commandManager.Initialize();

    // Initialize console variables - moved to preload
    // since we have some console variables to initialize first.
    _cvarmngr.Initialize();


    // Initialize file system.
    _filesystem.Initialize( execpath );

    // System stuff.
    _system.Initialize();

    // Common commands.
    _commandManager.Add( "local", lazyConnectA );
    _commandManager.Add( "clear", lazyConsoleClear );
    _commandManager.Add( "error", lazyError );
    _commandManager.Add( "quit", lazyShutdown );
    _commandManager.Add( "disconnect", lazyDisconnect );
    _commandManager.Add( "readconfig", lazyReadConfig );
    _commandManager.Add( "?", lazyPrinthelp );
    _commandManager.Add( "help", lazyPrinthelp );
    _commandManager.Add( "t", temp_tokenize );
}

/*
    Some help.
*/
void ncCore::Printhelp( void ) {
    _core.Print( LOG_NONE, "Nanocat running, version %s\n", _version );
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "Server running: %s\n", Server_Active.GetInteger() ? "Yes" : "No" );
}

/*
    Game core, load only on application launch!
*/
void ncCore::Initialize( void )
{
    float  t1, t2;

    t1 = _system.Milliseconds();
    
    _opengl.ShowInfo();

    // Default stuff.
    _gconsole.SetPrefix( core_version.GetString() );

    // Notify us.
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_INFO, "Loading core...\n" );
    _core.Print( LOG_INFO, "OS: %s, Version: %s, Build date: %s\n", _osname, _version, __DATE__ );

    // Developer stuff.
    _commandManager.Add( "cm", lazyConvertModel ); // Model converation tool.

    // Client game. ( Do not bother with server-client ).
    _clientgame.Initialize();
    
    // Build version info.
    core_version.Set( _stringhelper.STR("%s %s %s", _osname, _version, __DATE__) );
    
    // User input.
    _input.Initialize();

    // Initialize network.
    _netmanager.Initialize();

    // Initialize client system.
    _client.Initialize();

    // Create server.
    _server.Initialize();
    
    // Default parameters.
    _gconsole.Execute("readconfig config.nconf");

    // Ok!
    Initialized = true; t2 = _system.Milliseconds();
    _core.Print( LOG_INFO, "Core took %.1f msecs to load\n", t2 - t1 );
}

/*
    All systems got loaded, so do something after it.
*/
void ncCore::Loaded( void ) {
    _core.Print( LOG_NONE, "\n" );

    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_INFO, "All systems were loaded.\n" );
    _core.Print( LOG_DEVELOPER, "%i errors and %i warnings found while initializing.\n", _gconsole.logErrors, _gconsole.logWarnings );
    _core.Print( LOG_NONE, "\n" );

    _core.Print( LOG_NONE, "\n" );

    if( Server_Dedicated.GetInteger() ) {
        _gconsole.Execute("launch");
        _core.Print( LOG_INFO, "See server configuration file to edit parameters.\n" );
        _core.Print( LOG_INFO, "Server has been successfully created.\n" );
    }

    _gconsole.Execute("clear");
}


/*
    Game process.
    Let the everything create/think/move/load/do/read/write etc..
*/
void ncCore::Frame( void ) {

    int		msec, minMsec;
    float   scale;

    // Main game time count.
    if ( core_maxfps.GetInteger() > 0 ) {
		minMsec = 1000 / core_maxfps.GetInteger();
	} else {
		minMsec = 1;
	}
	do {
		FrameTime = _system.Milliseconds();
		if ( LastTime > FrameTime ) {
			LastTime = FrameTime;
		}
        msec = FrameTime - LastTime;
	} while ( msec < minMsec );

    LastTime = FrameTime;

    scale = core_timemultiplier.GetFloat();
    if( scale < 1 )
        core_timemultiplier.Set( "1.0" );

    msec = (unsigned int)(msec * scale);

    if( msec < 1 )
        msec = 1;
    else if( msec > 5000 )
        msec = 5000;

    if( msec > 500 )
        _core.Print( LOG_INFO, "Nanocat overloaded.. or just stuck for %i ms\n", msec );

    Time += msec;
    
    // Some system functions.
    _system.Frame();

    // Network manager.
    _netmanager.Frame();

    // Server and client framing.
    _server.Frame( msec );
    _client.Frame( msec );

    // Render our world!
    _renderer.Render( msec );
}

/*
    Smart disconnect.
*/
void ncCore::Disconnect( void ) {
    if( Client_Running.GetInteger() ) {
        _client.Disconnect();
    }

    if( Server_Active.GetInteger() ) {
        _server.Disconnect();
    }
}
/*
    Calls only on application quit.
*/
void ncCore::Shutdown( void )
{
    const char *msg;

    if( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_WARN, "No quit reason given.\n" );
        msg = "no reason";
    }

    msg = _commandManager.Arguments(0);

    _system.Quit( msg );

    printf( "Bye, %s", _system.GetCurrentUsername() );
}


/*  
    Developer print.
*/
void ncCore::DPrint( const char *msg ) {
    if( !msg ) {
        return;
    }
    
    printf( "Core::DPrint - %s\n", msg );
}

/*
    Error.
*/
void ncCore::Error( ncCoreErrorType err, const char *msg, ... ) {
    system("COLOR 0C");
    
    if( ( strlen(msg) < 1 || !msg ) ) {
        _core.Print( LOG_DEVELOPER, "No reason given.\n" );
        msg = "No reason";
    }
    
   	va_list         argptr;
    static char		text[128];
    
    va_start (argptr, msg);
    vsnprintf ((char *)text, sizeof(text), msg, argptr );
    va_end (argptr);
    
    // Message box for Windows
#ifdef _WIN32
    MessageBox( NULL, _stringhelper.STR("%s (errorcode: 0x%x)\n%s", text, err, ERROR_REPORT ), "Error", MB_OK | MB_ICONINFORMATION );
    
    HANDLE hOut;
    
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY );
#elif __APPLE__ // and Cocoa
    MassageBox( _stringhelper.STR( "%s (errorcode: 0x%x)\n%s", text, err, ERROR_REPORT ) );
#endif // _WIN32
    
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "Oops, errors were made:\n" );
    _core.Print( LOG_NONE, "%s (error code: 0x%x) :(\n", text, err );
    _core.Print( LOG_NONE, "\n" );
    printf("\n");
    
    _core.Print( LOG_NONE, "\n" );
    
    /*
        I love arts.
    */
    const char *oh_no[12]={
        
        "                                                      \n",
        "        :\\     /;               _         \n",
        "       ;  \\___/  ;             ; ;   Oops..      \n",
        "      ,:-\"'   `\"-:.            / ;         \n",
        " _   /,---.   ,---.\\   _     _; /         \n",
        " _:>((  |  ) (  |  ))<:_ ,-""_,\"           \n",
        "     \\`````   `````/""""\",-\"         \n",
        "      '-.._ v _..-'      )         \n",
        "        / ___   ____,..  \\          \n",
        "       / /   | |   | ( \\. \\         \n",
        "      / /    | |    | |  \\ \\        \n",
        "      `\"     `\"     `\"    `\"        \n"
    };
    
    int i;
    for( i = 0; i < 12; i++) {
        _core.Print( LOG_NONE, oh_no[i] );
    }
    
    _core.Print( LOG_NONE, "Bye! :>\n" );
    
    // Shutdown the game now.
    _system.Quit( _stringhelper.STR( "Error - 0x%i\n", err ) );
}


/*
    Print to console and to the log file if necessary.
*/
void ncCore::Print( const char *message ) {
    _core.Print( LOG_INFO, message );
}

void ncCore::Print( ncCoreLogType type, char const *msg, ... ) {
    
    // Developer stuff.
    if( (type == LOG_DEVELOPER && !log_verbose.GetInteger()) )
        return;
    
    if( !msg )
        return;
    
    va_list argptr;
    static char text[MAX_SPRINTF_BUFFER];

    va_start( argptr, msg );
    vsnprintf( text, sizeof(text), msg, argptr );
    va_end( argptr );
    
    const char *logtype;
    
    switch( type ) {
        case LOG_DEVELOPER:
            logtype     = "[DEBUG]: ";
            break;
        case LOG_ERROR:
            logtype     = "[ERROR]: ";
            ++_gconsole.logErrors;
            break;
        case LOG_INFO:
            logtype     = "[INFO]: ";
            break;
        case LOG_NONE:
            logtype     = "";
            break;
        case LOG_WARN:
            logtype     = "[WARNING]: ";
            ++_gconsole.logWarnings;
            break;
        default:
            logtype     = "[UNKNOWN]: ";
            break;
    }
    
    ++_gconsole.logCount;
    
    if( _core.Initialized && _opengl.Initialized ) {
        if( _gconsole.logCount > MAX_CONSOLE_LOG_LINES )
            _gconsole.Execute("clear ext");
    }
    
    if(!_stringhelper.ContainsNextLine(text)) {
        strcat(_gconsole.Log[_gconsole.logFill], _stringhelper.STR("%s%s", logtype, text));
    }
    else {
        ++_gconsole.logFill;
        sprintf(_gconsole.Log[_gconsole.logFill], "%s%s", logtype, text);
    }
    
    // External console.
#ifdef SHOW_CONSOLE_LINE_PREFIX
    fputs( _stringhelper.STR("%s%s", logtype, text), stdout );
    _gconsole.PrintExternal( _stringhelper.STR("%s%s", logtype, text) );
#else
    fputs( text, stdout );
    _gconsole.PrintExternal( text );
#endif
    
    // Log will be written to the file on console clear.
    _filesystem.WriteLog( text );
    
    fflush(stdout);
}

const char *ncCore::GetVersionString() {
    return core_version.GetString();
}

double ncCore::GetVersionDouble() {
    return CORE_VERSION_MM;
}

unsigned int ncCore::GetVersionMajor() {
    return CORE_VERSION_MAJOR;
}

unsigned int ncCore::GetVersionMinor() {
    return CORE_VERSION_MINOR;
}
