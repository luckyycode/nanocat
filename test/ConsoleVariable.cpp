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
    
    g_Core->LoadState = NCCLOAD_CONSOLEVARIABLES;
    g_Core->Print( LOG_INFO, "Preparing memory for %i console variables.\n", MAX_CONSOLEVARIABLES );

    c_CommandManager->Add( "cvarlist", lazyVariablelist );
}

void ncConsoleVariableManager::AddToList( ncConsoleVariable *var ) {
    consoleVariables[consoleVariableCount] = var;
    ++consoleVariableCount;
}

void ncConsoleVariable::Set( const NString value ) {
    g_stringHelper->Copy(StringValue, value);
    
    FloatValue = atof(value);
    Integer = atoi(value);
}

ncConsoleVariable::ncConsoleVariable( const NString group, const NString name, const NString desc, const NString value, CVFlag flags ) {
    int     c_flag;

    if(flags < 1) c_flag = CVFLAG_NONE;
    else
        c_flag = flags;
    
    g_stringHelper->Copy(Name, name);
    g_stringHelper->Copy(Group, group);
    g_stringHelper->Copy(Description, desc);
    
    g_stringHelper->Copy(StringValue, value);
    g_stringHelper->Copy(DefaultValue, value);
    
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

const NString ncConsoleVariable::GetString( void ) {
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

const NString ncConsoleVariable::GetName( void ) {
    return this->Name;
}

const NString ncConsoleVariable::GetGroup( void ) {
    return this->Group;
}

const NString ncConsoleVariable::GetDescription( void ) {
    return Description;
}

const NString ncConsoleVariable::GetDefaultValue( void ) {
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


bool ncConsoleVariableManager::Exists( const NString *var ) {
    
 
    return false;
}

