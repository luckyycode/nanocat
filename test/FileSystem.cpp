//
//  Nanocat engine.
//
//  File system..
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "FileSystem.h"
#include "NCString.h"
#include "LocalGame.h"

ncConsoleVariableManager _cvarmngr;

ncConsoleVariable  Filesystem_Log("filesystem", "log", "Log file to write.", "logfile.txt", CVFLAG_NONE );
ncConsoleVariable  Filesystem_Path("filesystem", "path", "Current directory.", "", CVFLAG_NEEDSREFRESH );
ncConsoleVariable  Filesystem_Logging("filesystem", "logging", "Is file logging enabled?", "0", CVFLAG_NEEDSREFRESH );

ncFileSystem local_filesystem;
ncFileSystem *c_FileSystem = &local_filesystem;

// Lazy functions.
void lazyFileGenerateConfig( void ) {
    c_FileSystem->GenerateConfigurationFile();
}


/*
    Initialize file system.
*/
void ncFileSystem::Initialize( const NString defaultpath ) {
    
    g_Core->LoadState = NCCLOAD_FILESYSTEM;
    g_Core->Print( LOG_INFO, "Loading file system..\n" );

    if( strlen(defaultpath) > MAX_PATH ) {
        g_Core->Error( ERR_FILESYSTEM, "Nanocat is placed far far away. Please make a path shorter.\n", MAX_PATH );
        return;
    }
    
    Filesystem_Path.Set( _stringhelper.STR("%s/%s", defaultpath, DEFAULT_EXEC_PATH) );
    
    /* Notify */
    g_Core->Print( LOG_INFO, "Current directory: \"%s\"\n", defaultpath );

    /* Check file logging. */
    if( Filesystem_Logging.GetInteger() ) {
        g_Core->Print( LOG_INFO, "Logging to file is enabled ( %s ).\n", Filesystem_Log.GetString() );
        if( !( g_Console->logFile = fopen( _stringhelper.STR("%s/%s", defaultpath, Filesystem_Log.GetString() ), "ab+" ) ) ) {
            g_Core->Print( LOG_ERROR, "Couldn't write or create the '%s' file for logging. Please check permissions.\n");
            g_Core->Print( LOG_INFO, "Logging is disabled.\n");
            g_Core->Print( LOG_NONE, "\n");
            
            Filesystem_Logging.Set( "0" );
            Filesystem_Logging.Lock();
         }
    }
    else g_Core->Print( LOG_INFO, "File logging is disabled.\n" );


    c_CommandManager->Add( "writeconfig", lazyFileGenerateConfig );
}

/*
    Generate configuration file.
    If 'raw' is true - generate a new file with default values.
*/
void ncFileSystem::GenerateConfigurationFile( void ) {
    if ( c_CommandManager->ArgCount() < 1 ) {
        g_Core->Print( LOG_INFO, "USAGE: writeconfig <raw>\n" );
        g_Core->Print( LOG_INFO, "raw: \"1\" - write default values\n" );
        g_Core->Print( LOG_INFO, "raw: \"0\" - write used values\n" );
        return;
    }
    
    int szArg = atoi( c_CommandManager->Arguments(0) );

    if( szArg < 0 || szArg > 1 )
        return;

    FILE     *_cfg;
    int      i;

    // Create new file.
    if ( !( _cfg = OpenWrite( _stringhelper.STR("%s/config.profile", Filesystem_Path.GetString()) ) ) ) {
        g_Core->Print( LOG_ERROR, "Couldn't generate configuration file.\n" );
        g_Core->Print( LOG_INFO, "Please check folder permissions.\n" );
        return;
    }

    // *sign it*
    fprintf( _cfg, "# Nanocat configuration profile.\n"  );
    fprintf( _cfg, "# For %s's profile.\n", NameVar.GetString() );
    fprintf( _cfg, "\n" );

    for( i = 0; i < _cvarmngr.consoleVariableCount; i++ ) {
        fprintf( _cfg, "%s %s\n", _cvarmngr.consoleVariables[i]->GetName(), szArg == 1 ? _cvarmngr.consoleVariables[i]->GetDefaultValue() : _cvarmngr.consoleVariables[i]->GetString() );
    }

    g_Core->Print( LOG_INFO, "Done writing the configuration file for %s's profile. :)\n", NameVar.GetString() );
    
    fclose( _cfg );
}


/*
    Shutdown the file system.
*/
void ncFileSystem::Shutdown( void ) {

    g_Core->Print( LOG_INFO, "File system shutting down...\n" );

    // Close the log file.
    if( g_Console->logFile )
        fclose( g_Console->logFile );
}

/*
    Get file name from a path.
*/
const NString ncFileSystem::GetFileName( const NString path ) {
    const NString filename;

    // Loop thru every slash.
    filename = strrchr(path, '\\');

    if (filename == NULL)
        filename = path;
    else
        filename++;

    return filename;
}

/*
    Get file extension.
*/
const NString ncFileSystem::GetFileExtension( const NString filename ) {
    const NString dot;

    dot = strrchr( filename, '.' );

    if( !dot || dot == filename )
        return "";

    // ok
    return dot + 1;
}

/*
    Read per configuration file line.
    Needs a bit improvements.
*/

#define CONFIGLINE_SZ 1024
void ncFileSystem::ReadConfiguration( void ) {
    FILE    *cfg;

    if( c_CommandManager->ArgCount() < 1 ) {
        g_Core->Print( LOG_INFO, "Read configuration file and load it.\n" );
        g_Core->Print( LOG_INFO, "USAGE: readconfig <configfile>\n" );
        return;
    }

    cfg = OpenRead( _stringhelper.STR("%s/%s.nconf", Filesystem_Path.GetString(), c_CommandManager->Arguments(0)) );

    if( !cfg ) {
        g_Core->Print( LOG_INFO, "Could not read %s configuration file.\n", c_CommandManager->Arguments(0) );
        return;
    }
    else {
        char buff[CONFIGLINE_SZ];
        while( fgets (buff, CONFIGLINE_SZ, cfg) ) {

            // Remove some useless symbols.
            int i;
            for( i = 0; i < CONFIGLINE_SZ; i++ ) {
                if( (buff[i] == '\n' || buff[i] == '\r' || buff[i] == '/' ) )
                    buff[i] = '\0';
            }

            g_Console->Execute( buff );
        }
    }

    fclose( cfg );
}

/*
    Write to the log file.
*/
void ncFileSystem::WriteLog( const NString msg ) {
    if ( !g_Console->logFile )
        return;

    // Gosh.
    if( !msg || strlen( msg ) < 1 )
        return;
    
    fprintf( g_Console->logFile, "%s", msg );
}

/*
    Get file size.
*/
long ncFileSystem::GetFileLength( FILE *f ) {
    ulong f_pos;
    ulong f_end;

    f_pos = ftell( f );
    fseek ( f, 0, SEEK_END );

    f_end = ftell( f );
    fseek ( f, f_pos, SEEK_SET );

    return f_end;
}

/*
    Write to file.
*/
void ncFileSystem::Write( FILE *f, const void *buffer, int len ) {
    if ( fwrite (buffer, 1, len, f) != (size_t)len )
        g_Core->Error ( ERR_FILESYSTEM, "File system failed to write." );
}

/*
    Open file for writing. ( Nor create it )
*/
FILE *ncFileSystem::OpenWrite( const NString filename ) {
    FILE   *f;

    f = fopen( filename, "wb" );

    if (!f)
        g_Core->Print( LOG_ERROR, "File system failed to open \"%s\".\n", filename );

    return f;
}

/*
    Open file for read.
*/
FILE *ncFileSystem::OpenRead( const NString filename ) {
    FILE    *f;

    f = fopen( filename, "rb" );

    // Also returns null if fopen failed.
    return f;
}

/*
    Read file and get its data.
*/
void ncFileSystem::Read( FILE *f, void *data, long len ) {
    if ( fread( data, 1, len, f ) != (size_t)len )
        g_Core->Error( ERR_FILESYSTEM, "ncFileSystem::Read failed to read file." );
}

/*
    Load file and get its data.
*/
long ncFileSystem::Load( const NString filename, void **buffer ) {
    FILE        *f;
    void        *data;
    long        len;

    f = OpenRead( filename );

    if( !f )
        return -1;

    len     = GetFileLength( f );
    data    = malloc( len + 1 );

    ((char*)data)[len] = 0;

    Read( f, data, len );
    fclose( f );

    *buffer = data;
    return len;
}

void ncFileSystem::LoadNFC( const NString name ) {
    
}
