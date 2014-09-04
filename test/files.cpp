//
//  Nanocat engine.
//
//  File system.
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "command.h"
#include "console.h"
#include "files.h"
#include "ncstring.h"
#include "clientgame.h"

CvarManager _cvarmngr;

ConsoleVariable  filesystem_log("filesystem", "log", "Log file to write.", "logfile.txt", CVFLAG_NONE );
ConsoleVariable  filesystem_path("filesystem", "path", "Current directory.", "", CVFLAG_NEEDSREFRESH );
ConsoleVariable  filesystem_logging("filesystem", "logging", "Is file logging enabled?", "0", CVFLAG_NEEDSREFRESH );

ncFileSystem _filesystem;

// #define FILESYSTEM_LOGERRORS // Print errors such as "filesystem_openforread failed to read" etc...

/*
    Initialize file system.
*/

void lazyFileGenerateConfig( void ) {
    _filesystem.GenerateConfigurationFile();
}


void ncFileSystem::Initialize( const char *defaultpath ) {
    _core.Print( LOG_INFO, "Loading file system..\n" );

    if( strlen(defaultpath) > MAX_PATH ) {
        _core.Error( ERC_FS, "Nanocat is placed far far away. Please make a path shorter.\n", MAX_PATH );
        return;
    }
    
    filesystem_path.Set( _stringhelper.STR("%s/%s", defaultpath, DEFAULT_EXEC_PATH) );
    
    /* Notify */
    _core.Print( LOG_INFO, "Current directory: \"%s\"\n", defaultpath );

    /* Check file logging. */
    if( filesystem_logging.GetInteger() ) {
        _core.Print( LOG_INFO, "Logging to file is enabled ( %s ).\n", filesystem_log.GetString() );
        if( !( _gconsole.logFile = fopen( _stringhelper.STR("%s/%s", defaultpath, filesystem_log.GetString() ), "ab+" ) ) ) {
            _core.Print( LOG_ERROR, "Couldn't write or create the '%s' file for logging. Please check permissions.\n");
            _core.Print( LOG_INFO, "Logging is disabled.\n");
            _core.Print( LOG_NONE, "\n");
            
            filesystem_logging.Set( "0" );
            filesystem_logging.Lock();
         }
    }
    else _core.Print( LOG_INFO, "File logging is disabled.\n" );


    _commandManager.Add( "writeconfig", lazyFileGenerateConfig );
}

/*
    Generate configuration file.
    If 'raw' is true - generate a new file with default values.
*/
void ncFileSystem::GenerateConfigurationFile( void ) {
    if ( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "usage: writeconfig <raw>\n" );
        _core.Print( LOG_INFO, "raw: \"1\" - write default values\n" );
        _core.Print( LOG_INFO, "raw: \"0\" - write used values\n" );
        return;
    }
    
    int szArg = atoi( _commandManager.Arguments(0) );

    if( szArg < 0 )
        return;

    if( szArg > 1 )
        return;

    FILE     *_cfg;
    int      i;

    // Create new file.
    if ( !( _cfg = OpenWrite( _stringhelper.STR("%s/config.profile", filesystem_path.GetString()) ) ) ) {
        _core.Print( LOG_ERROR, "Couldn't generate configuration file.\n" );
        _core.Print( LOG_INFO, "Please check folder permissions.\n" );
        return;
    }

    // *sign it*
    fprintf( _cfg, "# Nanocat configuration profile.\n"  );
    fprintf( _cfg, "# For %s's profile.\n", namevar.GetString() );
    fprintf( _cfg, "\n" );

    for( i = 0; i < _cvarmngr.consoleVariableCount; i++ ) {
        fprintf( _cfg, "%s %s\n", _cvarmngr.consoleVariables[i]->GetName(), szArg == 1 ? _cvarmngr.consoleVariables[i]->GetDefaultValue() : _cvarmngr.consoleVariables[i]->GetString() );
    }

    _core.Print( LOG_INFO, "Done writing the configuration file for %s's profile. :)\n", namevar.GetString() );
    
    fclose( _cfg );
}


/*
    Shutdown the file system.
*/
void ncFileSystem::Shutdown( void ) {

    _core.Print( LOG_INFO, "File system shutting down...\n" );

    // Close the log file.
    if( _gconsole.logFile )
        fclose( _gconsole.logFile );
}

/*
    Get file name from a path.
*/
const char *ncFileSystem::GetFileName( const char *path ) {
    const char *filename;

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
const char *ncFileSystem::GetFileExtension( const char *filename ) {
    const char *dot;

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

    if( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "Read configuration file and load it.\n" );
        _core.Print( LOG_INFO, "USAGE: readconfig <configfile>\n" );
        return;
    }

    cfg = OpenRead( _stringhelper.STR("%s/%s.nconf", filesystem_path.GetString(), _commandManager.Arguments(0)) );

    if( !cfg ) {
        _core.Print( LOG_INFO, "Could not read %s configuration file.\n", _commandManager.Arguments(0) );
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

            _gconsole.Execute( buff );
        }
    }

    fclose( cfg );
}

/*
    Write to the log file.
*/
void ncFileSystem::WriteLog( const char *msg ) {
    if ( !_gconsole.logFile )
        return;

    // Gosh.
    if( !msg || strlen( msg ) < 1 )
        return;
    
    fprintf( _gconsole.logFile, "%s", msg );
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
        _core.Error ( ERC_FS, "File system failed to write." );
}

/*
    Open file for writing. ( Nor create it )
*/
FILE *ncFileSystem::OpenWrite( const char *filename ) {
    FILE   *f;

    f = fopen( filename, "wb" );

    if (!f)
        _core.Print( LOG_ERROR, "File system failed to open \"%s\".\n", filename );

    return f;
}

/*
    Open file for read.
*/
FILE *ncFileSystem::OpenRead( const char *filename ) {
    FILE    *f;

    f = fopen( filename, "rb" );

#ifdef FILESYSTEM_LOGERRORS
    if (!f)
        _core.Print( LOG_ERROR, "File system failed to read \"%s\"\n", filename );
#endif
    return f;
}

/*
    Read file and get its data.
*/
void ncFileSystem::Read( FILE *f, void *data, long len ) {
    if ( fread( data, 1, len, f ) != (size_t)len )
        _core.Error( ERC_FS, "ncFileSystem::Read failed to read file." );
}

/*
    Load file and get its data.
*/
long ncFileSystem::Load( const char *filename, void **buffer ) {
    FILE        *f;
    void        *data;
    long        len;

    f = OpenRead( filename );

    if( !f )
        return -1;

    len     = GetFileLength( f );
    data    = malloc( len + 1 );    // Sometimes it doesn't fit..fuu

    ((char *)data)[len] = 0;

    Read( f, data, len );
    fclose( f );

    *buffer = data;
    return len;
}
