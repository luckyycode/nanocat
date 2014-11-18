//
//  Nanocat engine.
//
//  Console variable manager..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef consolevar_h
#define consolevar_h

#include "NCString.h"

// Maximum console variable space to be allocated.
#define MAX_CONSOLEVARIABLES   1024

#define CONSOLEVAR_NAMELEN  64
#define CONSOLEVAR_DESCLEN  64
#define CONSOLEVAR_GROUPLEN     16

#define MAX_CONSOLEVAR_STRINGLENGTH 512

enum CVFlag {
    CVFLAG_NONE = 1,            // No protection.
    CVFLAG_READONLY = 2,        // Read only.
    CVFLAG_NEEDSREFRESH = 4,    // Needs restart to apply.
    CVFLAG_LOCKED = 8,          // Locked. No changes allowed.
    CVFLAG_SYS = 16,            // System one.
    CVFLAG_KID = 32             // Cheats.
};

class ncConsoleVariable {
public:
    
    ~ncConsoleVariable() {

    }
    
    ncConsoleVariable() { }
    ncConsoleVariable( const NString group, const NString name, const NString desc, const NString value, CVFlag flags );
    ncConsoleVariable( const NString name, const NString value );
    
    void Lock( void );
    void Clear( void );
    
    void        Set( const NString value );
    
    const char  *GetString( void );
    const char  *GetDefaultValue( void );
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
    char        StringValue[MAX_CONSOLEVAR_STRINGLENGTH];
    char        DefaultValue[MAX_CONSOLEVAR_STRINGLENGTH];
    
    bool        Modified;
    
    int         Flag;
    
    float       FloatValue;
    int         Integer;
    
    int         ModificationCount;
    int         Index;
};

class ncConsoleVariableManager {
public:
    ncConsoleVariableManager();
    
    void Initialize( void );
    void Printlist( void );
    void Shutdown( void );
    
    ncConsoleVariable Find( const NString name );
    bool Exists( const NString var[] );
    
    void AddToList( ncConsoleVariable *var );
    
    ncConsoleVariable *consoleVariables[MAX_CONSOLEVARIABLES];
    unsigned int consoleVariableCount;
};

extern ncConsoleVariableManager _cvarmngr;

#endif
