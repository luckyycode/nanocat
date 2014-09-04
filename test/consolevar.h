//
//  Nanocat engine.
//
//  Console variable manager.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef consolevar_h
#define consolevar_h

#include "ncstring.h"

// Maximum console variable space to be allocated.
#define MAX_CVARS                               1024

#define CVAR_NONE                               1               // No protection.
#define CVAR_READONLY                           2               // Read only.
#define CVAR_NEEDSREFRESH                            8               // Applies only after map/graphics/game restart.
#define CVAR_LOCKED                             16              // No changes allowed.
// You need to be gentle with it.
#define CVAR_SYS                                32              // System var.
#define CVAR_CHEAT                              64              // Cheat var.

#define CONSOLEVAR_NAMELEN 64
#define CONSOLEVAR_DESCLEN 64
#define CONSOLEVAR_GROUPLEN 16

#define MAX_CVAR_LEN                            512
#define MAX_CVAR_STRING_LEN                     256


class ConsoleVariable {
public:
    
    ~ConsoleVariable() {

    }
    
    ConsoleVariable() { }
    ConsoleVariable( const char *group, const char *name, const char *desc, const char *value, int flags );
    ConsoleVariable( const char *name, const char *value );
    
    void Lock( void );
    void Clear( void );
    
    void        Set( const char *value );
    
    const char  *GetString( void );
    int         GetInteger( void );
    bool        GetBool( void );
    float       GetFloat( void );
    bool        IsModified( void );

    void        SetModified( bool value );
    
    void        SetFlag( int flag );
    int         GetFlag( void );

    const char  *GetName( void );
    const char  *GetGroup( void );
    const char  *GetDescription( void );
    
protected:
    char        Group[CONSOLEVAR_GROUPLEN];
    char        Name[CONSOLEVAR_NAMELEN];
    char        Description[CONSOLEVAR_DESCLEN];
    
    // Fix me: could be more for path stuff.
    char        StringValue[MAX_CVAR_STRING_LEN];
    char        DefaultValue[MAX_CVAR_STRING_LEN];
    
    bool        Modified;
    
    int         Flag;
    
    float       FloatValue;
    int         Integer;
    
    int         ModificationCount;
    int         Index;
};

class CvarManager {
public:
    CvarManager();
    
    void Initialize( void );
    void Printlist( void );
    void Shutdown( void );
    
    ConsoleVariable Find( const char *name );
    bool Exists( const char *var[] );
    
    void AddToList( ConsoleVariable *var );
    
    ConsoleVariable *consoleVariables[MAX_CVARS];
    unsigned int consoleVariableCount;
};

extern CvarManager _cvarmngr;

#endif
