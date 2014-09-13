//
//  Nanocat engine.
//
//  Console variable manager..
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "ConsoleVariable.h"
#include "Core.h"
#include "ConsoleCommand.h"
#include "NCString.h"
#include "SystemShared.h"
#include "MultiplayerServer.h"

static void lazyVariablelist( void ) {
    _cvarmngr.Printlist();
}

ncConsoleVariableManager::ncConsoleVariableManager( void ) {
    consoleVariableCount = 0;
}

/*
    Initialize console variable stuff.
*/
void ncConsoleVariableManager::Initialize( void ) {
    _core.Print( LOG_INFO, "Preparing memory for %i console variables.\n", MAX_CONSOLEVARIABLES );

    _commandManager.Add( "cvarlist", lazyVariablelist );
}

void ncConsoleVariableManager::AddToList( ncConsoleVariable *var ) {
    consoleVariables[consoleVariableCount] = var;
    ++consoleVariableCount;
}

void ncConsoleVariable::Set( const char *value ) {
    _stringhelper.Copy(StringValue, value);
    FloatValue = atof(value);
    Integer = atoi(value);
}

ncConsoleVariable::ncConsoleVariable( const char *group, const char *name, const char *desc, const char *value, CVFlag flags ) {
    int     c_flag;

    if(flags < 1) c_flag = CVFLAG_NONE;
    else
        c_flag = flags;
    
    _stringhelper.Copy(Name, name);
    _stringhelper.Copy(Group, group);
    _stringhelper.Copy(Description, desc);
    
    _stringhelper.Copy(StringValue, value);
    _stringhelper.Copy(DefaultValue, value);
    
    Modified = false;
    ModificationCount = 0;
    
    FloatValue = atof(value);
    Integer = atoi(value);

    Flag = c_flag;
    
    _cvarmngr.AddToList( this );
}

int ncConsoleVariable::GetFlag( void ) {
    return Flag;
}

void ncConsoleVariable::SetFlag( int flag ) {
    Flag = flag;
}

const char *ncConsoleVariable::GetString( void ) {
    return StringValue;
}

int ncConsoleVariable::GetInteger( void ) {
    return Integer;
}

bool ncConsoleVariable::GetBool( void ) {
    return Integer; // Uh oh
}

float ncConsoleVariable::GetFloat( void ) {
    return FloatValue;
}

bool ncConsoleVariable::IsModified( void ) {
    return Modified;
}

void ncConsoleVariable::SetModified( bool value ) {
    Modified = value;
}

const char *ncConsoleVariable::GetName( void ) {
    return this->Name;
}

const char *ncConsoleVariable::GetGroup( void ) {
    return this->Group;
}

const char *ncConsoleVariable::GetDescription( void ) {
    return Description;
}

const char *ncConsoleVariable::GetDefaultValue( void ) {
    return DefaultValue;
}

/*
    List all existing console variables.
*/
void ncConsoleVariableManager::Printlist( void ) {
    // TODO.
}

/*
    Remove console variables from a memory
*/
void ncConsoleVariableManager::Shutdown( void ) {
    _core.Print( LOG_DEVELOPER, "Removing console variables..\n");

}

/*
    Lock console variable.
*/
void ncConsoleVariable::Lock( void ) {
    Flag = CVFLAG_LOCKED;
}

/*
    Check the command for existence.
*/


bool ncConsoleVariableManager::Exists( const char **var ) {
    
 
    return false;
}

