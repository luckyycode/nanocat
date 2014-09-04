//
//  Nanocat engine.
//
//  Console variable manager.
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "consolevar.h"
#include "core.h"
#include "command.h"
#include "ncstring.h"
#include "systemshared.h"
#include "server.h"

static void lazyVariablelist( void ) {
    _cvarmngr.Printlist();
}

CvarManager::CvarManager( void ) {
    consoleVariableCount = 0;
}

/*
    Initialize console variable stuff.
*/
void CvarManager::Initialize( void ) {
    _core.Print( LOG_INFO, "Preparing memory for %i console variables.\n", MAX_CVARS );

    _commandManager.Add( "cvarlist", lazyVariablelist );
}

void CvarManager::AddToList( ConsoleVariable *var ) {
    consoleVariables[consoleVariableCount] = var;
    ++consoleVariableCount;
}

void ConsoleVariable::Set( const char *value ) {
    _stringhelper.Copy(StringValue, value);
    FloatValue = atof(value);
    Integer = atoi(value);
}

ConsoleVariable::ConsoleVariable( const char *group, const char *name, const char *desc, const char *value, CVFlag flags ) {
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

int ConsoleVariable::GetFlag( void ) {
    return Flag;
}

void ConsoleVariable::SetFlag( int flag ) {
    Flag = flag;
}

const char *ConsoleVariable::GetString( void ) {
    return StringValue;
}

int ConsoleVariable::GetInteger( void ) {
    return Integer;
}

bool ConsoleVariable::GetBool( void ) {
    return Integer; // Uh oh
}

float ConsoleVariable::GetFloat( void ) {
    return FloatValue;
}

bool ConsoleVariable::IsModified( void ) {
    return Modified;
}

void ConsoleVariable::SetModified( bool value ) {
    Modified = value;
}

const char *ConsoleVariable::GetName( void ) {
    return this->Name;
}

const char *ConsoleVariable::GetGroup( void ) {
    return this->Group;
}

const char *ConsoleVariable::GetDescription( void ) {
    return Description;
}

const char *ConsoleVariable::GetDefaultValue( void ) {
    return DefaultValue;
}

/*
    List all existing console variables.
*/
void CvarManager::Printlist( void ) {
    // TODO.
}

/*
    Remove console variables from a memory
*/
void CvarManager::Shutdown( void ) {
    _core.Print( LOG_DEVELOPER, "Removing console variables..\n");

}

/*
    Lock console variable.
*/
void ConsoleVariable::Lock( void ) {
    Flag = CVFLAG_LOCKED;
}

/*
    Check the command for existence.
*/


bool CvarManager::Exists( const char **var ) {
    
 
    return false;
}

