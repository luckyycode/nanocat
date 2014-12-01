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
#include "ImageManager.h"
#include "Camera.h"

ncCore local_core;
ncCore *g_Core = &local_core;

ncConsoleVariable      core_version("core", "version", "Game version", "6q", CVFLAG_NEEDSREFRESH);
ncConsoleVariable      core_maxfps("core", "maxfps", "Maximum game framerate.", "60", CVFLAG_NEEDSREFRESH);
ncConsoleVariable      core_timemultiplier("core", "timescale", "Time scale.", "1.0", CVFLAG_KID);
ncConsoleVariable      log_verbose("log", "verbose", "Show more detailed log?", "1", CVFLAG_NONE);

/*
    Lazy functions. 
*/
static void lazyReadConfig( void ) {
    c_FileSystem->ReadConfiguration();
}

static void lazyShutdown( void ) {
    g_Core->Shutdown();
}

static void lazyPrinthelp( void ) {
    g_Core->Printhelp();
}

static void lazyDisconnect( void ) {
    g_Core->Disconnect();
}

static void lazyError( void ){
    cg_LocalGame->Error();
}

static void lazyConvertModel( void ) {
    ncUtils::OBJtoSM( c_CommandManager->Arguments(0) );
}

const uint _randomWords[] = {
    0xfeedc0de,
    0x1337d05e,
    0xdeadbeef,
    0x1337c0de,
};

Byte Sharedsecret[] =
{   0x3f,0x58,0xf0,0xbe,0xba,0x68,0x54,0x07,0xe5,0xc7,
    0x6b,0x23,0x05,0x33,0x00,0x18
};

void temp_tokenize( void ) {
    
    g_Core->Print( LOG_ERROR, "version: %1.1f\n", g_Core->GetVersionDouble() );
    
    NC_ASSERTWARN( false );
}

static void lazyConsoleClear( void ) {
    g_Console->Clear();
}

void lazyConnectA() {
    n_client->LoopbackConnect();
}

/*
    Pre-load stuff, even before context & graphic stuff load.
*/
void ncCore::Preload( const NString execpath ) {
    
    // O

    // Reset game time.
    Time = 0;
    LastTime = 0;
    FrameTime = 0;

    g_Console->Initialize();
    
    
    LoadState = NCCLOAD_PREPARING;
    Initialized = false;

    // Server initialization isn't called yet.
    n_server->Initialized = false;

    // Commands.
    c_CommandManager->Initialize();

    // Initialize console variables - moved to preload
    // since we have some console variables to initialize first for
    // filesystem etc...
    _cvarmngr.Initialize();

    // Initialize file system.
    c_FileSystem->Initialize( execpath );

    // System stuff.
    c_coreSystem->Initialize();

    // Common commands.
    c_CommandManager->Add( "local", lazyConnectA );
    c_CommandManager->Add( "clear", lazyConsoleClear );
    c_CommandManager->Add( "error", lazyError );
    c_CommandManager->Add( "quit", lazyShutdown );
    c_CommandManager->Add( "disconnect", lazyDisconnect );
    c_CommandManager->Add( "readconfig", lazyReadConfig );
    c_CommandManager->Add( "?", lazyPrinthelp );
    c_CommandManager->Add( "help", lazyPrinthelp );
    c_CommandManager->Add( "t", temp_tokenize );
}

/*
    Some help.
*/
void ncCore::Printhelp( void ) {
    g_Core->Print( LOG_NONE, "Nanocat running, version %s\n", _version );
    g_Core->Print( LOG_NONE, "\n" );
    g_Core->Print( LOG_NONE, "Server running: %s\n", Server_Active.GetInteger() ? "Yes" : "No" );
}

/*
    Game core, load only on application launch!
*/
void ncCore::Initialize( void )
{
    float  t1, t2;

    t1 = c_coreSystem->Milliseconds();
    
    gl_Core->ShowInfo();

    // Default stuff.
    g_Console->SetPrefix( core_version.GetString() );

    // Notify us.
    g_Core->Print( LOG_NONE, "\n" );
    g_Core->Print( LOG_INFO, "Loading core...\n" );
    g_Core->Print( LOG_INFO, "OS: %s, Version: %s, Build date: %s\n", _osname, _version, __DATE__ );

    // Developer stuff.
    c_CommandManager->Add( "cm", lazyConvertModel ); // Model converation tool.

    // Client game. ( Do not bother with server-client ).
    cg_LocalGame->Initialize();
    
    // Build version info.
    core_version.Set( NC_TEXT("%s %s %s", _osname, _version, __DATE__) );
    
    // User input.
    g_Input->Initialize();

    // Initialize network.
    g_networkManager->Initialize();

    // Initialize client system.
    n_client->Initialize();

    // Create server.
    n_server->Initialize();
    
    // Default parameters.
    g_Console->Execute("readconfig config.nconf");

    // Ok!
    Initialized = true; t2 = c_coreSystem->Milliseconds();
    g_Core->Print( LOG_INFO, "Core took %.1f msecs to load\n", t2 - t1 );
}

/*
    All systems got loaded, so do something after it.
*/
void ncCore::Loaded( void ) {
    g_Core->Print( LOG_NONE, "\n" );

    g_Core->Print( LOG_NONE, "\n" );
    g_Core->Print( LOG_INFO, "All systems were loaded.\n" );
    g_Core->Print( LOG_DEVELOPER, "%i errors and %i warnings found while initializing.\n", g_Console->logErrors, g_Console->logWarnings );
    g_Core->Print( LOG_NONE, "\n" );

    g_Core->Print( LOG_NONE, "\n" );

    if( Server_Dedicated.GetInteger() ) {
        g_Console->Execute("launch");
        g_Core->Print( LOG_INFO, "See server configuration file to edit parameters.\n" );
        g_Core->Print( LOG_INFO, "Server has been successfully created.\n" );
    }

    // Keep console clean.
    g_Console->Execute( "clear" );
}


/*
    Game process.
    Let the everything create/think/move/load/do/read/write etc..
*/
void ncCore::Frame( void ) {

    int msec, minMsec;
    float scale;

    // Main game time count.
    if ( core_maxfps.GetInteger() > 1 ) {
		minMsec = 1000 / core_maxfps.GetInteger();
	} else {
		minMsec = 1;
	}
    
	do {
		FrameTime = c_coreSystem->Milliseconds();
		if ( LastTime > FrameTime ) {
			LastTime = FrameTime;
		}
        msec = FrameTime - LastTime;
	} while ( msec < minMsec );

    LastTime = FrameTime;

    scale = core_timemultiplier.GetFloat();
    if( scale < 1 )
        core_timemultiplier.Set( "1.0" );

    msec = (uint)(msec * scale);

    if( msec < 1 )
        msec = 1;
    else if( msec > 5000 )
        msec = 5000;

    if( msec > 500 )
        g_Core->Print( LOG_INFO, "Nanocat overloaded.. or just stuck for %i ms\n", msec );

    Time += msec;
    
    // Some system functions.
    c_coreSystem->Frame();

    // Network manager.
    g_networkManager->Frame();

    // Server and client framing.
    n_server->Frame( msec );
    n_client->Frame( msec );

    // Render our world!
    
    // Rendering on iOS is done from
    // its view controller for some purposes.
#ifndef iOS_BUILD
    g_mainRenderer->Render( msec );
#endif
    
    g_playerCamera->Frame( msec );
}

/*
    Smart disconnect.
*/
void ncCore::Disconnect( void ) {
    if( Client_Running.GetInteger() ) {
        n_client->Disconnect();
    }

    if( Server_Active.GetInteger() ) {
        n_server->Disconnect();
    }
}

/*
    Calls only on application quit.
*/
void ncCore::Shutdown( void ) {
    const NString msg;

    if( c_CommandManager->ArgCount() < 1 ) {
        g_Core->Print( LOG_WARN, "No quit reason given.\n" );
        msg = "No reason given.";
    }

    msg = c_CommandManager->Arguments(0);

    c_coreSystem->Quit( msg );

    printf( "Bye, %s", c_coreSystem->GetCurrentUsername() );
}

/*  
    Some useless stuff.
*/
void ncCore::DPrint( const NString msg ) {
    if( !msg ) {
        return;
    }
    
    printf( "Core::DPrint - %s\n", msg );
}

/*
    Error.
*/
#define MAX_ERROR_TEXT_LENGTH 1024
void ncCore::Error( ncCoreErrorType err, const NString msg, ... ) {
    system("COLOR 0C");
    
    if( ( strlen(msg) < 1 || !msg ) ) {
        g_Core->Print( LOG_DEVELOPER, "No reason given.\n" );
        msg = "No reason";
    }
    
    va_list argptr;
    static char text[MAX_ERROR_TEXT_LENGTH];
    
    va_start( argptr, msg );
    vsnprintf( (char*)text, sizeof(text), msg, argptr );
    va_end( argptr );
    
    // Message box for Windows
#ifdef _WIN32
    
    MessageBox( NULL, NC_TEXT("%s\nError code: 0x%x\n%s", text, err, ERROR_REPORT ), "Error", MB_OK | MB_ICONINFORMATION );
    
    HANDLE hOut;
    
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY );
    
#elif __APPLE__ // and Cocoa
    MassageBox( NC_TEXT( "%s (errorcode: 0x%x)\n%s", text, err, ERROR_REPORT ) );
#endif // _WIN32
    
    g_Core->Print( LOG_NONE, "\n" );
    g_Core->Print( LOG_NONE, "Oops, errors were made:\n" );
    g_Core->Print( LOG_NONE, "%s (error code: 0x%x) :(\n", text, err );
    g_Core->Print( LOG_NONE, "\n" );
    printf("\n");
    
    g_Core->Print( LOG_NONE, "\n" );
    
    /*
        I love arts.
    */
    const NString oh_no[12]={
        
        "                                           \n",
        "        :\\     /;                _        \n",
        "       ;  \\___/  ;              ; ; Oops..\n",
        "      ,:-\"'   `\"-:.            / ;       \n",
        " _   /,---.   ,---.\\   _     _; /         \n",
        " _:>((  |  ) (  |  ))<:_ ,-""_,\"          \n",
        "     \\`````   `````/""""\",-\"            \n",
        "      '-.._ v _..-'      )                 \n",
        "        / ___   ____,..  \\                \n",
        "       / /   | |   | ( \\. \\              \n",
        "      / /    | |    | |  \\ \\             \n",
        "      `\"     `\"     `\"    `\"           \n"
    };
    
    int i;
    for( i = 0; i < 12; ++i ) {
        g_Core->Print( LOG_NONE, oh_no[i] );
    }
    
    g_Core->Print( LOG_NONE, "Bye! :>\n" );
    
    // Shutdown the game now.
    c_coreSystem->Quit( NC_TEXT( "Error - 0x%i\n", err ) );
}


/*
    Print to console and to the log file if necessary.
*/
void ncCore::Print( const NString message ) {
    g_Core->Print( LOG_INFO, message );
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
    
    const NString logtype;
    
    switch( type ) {
        case LOG_DEVELOPER:
            logtype = "[DEBUG]: ";
            break;
        case LOG_ERROR:
            logtype = "[ERROR]: ";
            ++g_Console->logErrors;
            break;
        case LOG_INFO:
            logtype = "[INFO]: ";
            break;
        case LOG_NONE:
            logtype = "";
            break;
        case LOG_WARN:
            logtype = "[WARNING]: ";
            ++g_Console->logWarnings;
            break;
        default:
            logtype = "[UNKNOWN]: ";
            break;
    }
    
    ++g_Console->logCount;
    
    if( g_Core->Initialized && gl_Core->Initialized ) {
        if( g_Console->logCount > MAX_CONSOLE_LOG_LINES )
            g_Console->Execute("clear ext");
    }
    
    if(!g_stringHelper->ContainsNextLine(text)) {
        strcat(g_Console->Log[g_Console->logFill], NC_TEXT("%s%s", logtype, text));
    }
    else {
        ++g_Console->logFill;
        sprintf(g_Console->Log[g_Console->logFill], "%s%s", logtype, text);
    }
    
    // External console.
#ifdef SHOW_CONSOLE_LINE_PREFIX
    
    fputs( NC_TEXT("%s%s", logtype, text), stdout );
    g_Console->PrintExternal( NC_TEXT("%s%s", logtype, text) );
    
#else
    
    fputs( text, stdout );
    g_Console->PrintExternal( text );
    
#endif
    
    // Log will be written to the file on console clear.
    c_FileSystem->WriteLog( text );
    
    fflush( stdout );
}

const NString ncCore::GetVersioNString () {
    return core_version.GetString();
}

double ncCore::GetVersionDouble() {
    double versionMinor = CORE_VERSION_MINOR / 10.0;
    double versionMajor = (double)CORE_VERSION_MAJOR;
    
    double version = versionMajor + versionMinor;
    
    return version;
}

unsigned int ncCore::GetVersionMajor() {
    return CORE_VERSION_MAJOR;
}

unsigned int ncCore::GetVersionMinor() {
    return CORE_VERSION_MINOR;
}
