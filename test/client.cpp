//
//  Nanocat engine.
//
//  Multiplayer: Client side.
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "system.h"
#include "server.h"
#include "client.h"
#include "core.h"
#include "network.h"
#include "command.h"
#include "console.h"
#include "camera.h"
#include "renderer.h"
#include "ncstring.h"
#include "gmath.h"
#include "clientgame.h"
#include "bitset.h"

Client _client;

/*
    Client side.
*/

// Is client running?
ConsoleVariable      client_running( "client", "running", "Is client running?", "0", CVAR_READONLY );
// User name.
ConsoleVariable      namevar( "client", "name", "Client username.", "A cat", CVAR_NONE );
// Server timeout. ( In seconds )
ConsoleVariable      client_servertimeout( "client", "servertimeout", "Server timeout.", "30", CVAR_NEEDSREFRESH );
// Maximum packets to be send per frame.
ConsoleVariable      client_maxpackets( "client", "maxpackets", "Maximum packets to be send per frame.", "33", CVAR_NEEDSREFRESH );

// Uh oh, global variables.
ncVec3 latestPosition;
ncVec3 latestRotation;
float latestPositionTime = 0.0f;

// Lazy commands.
void lazyConnect() {
    _client.Connect();
}

void lazyName() {
    _client.ChangeName();
}

void lazySay() {
    _client.Say();
}

/*
    Disconnect from current server.
*/
void Client::Disconnect( void ) {
    _client.DisconnectForced( "User disconnect.", true );
}

/*
    Initialize client stuff.
*/
void Client::Initialize() {
    _core.Print( LOG_INFO, "Client initializing...\n" );

    namevar.Set( _system.GetCurrentUsername() );
    State = CLIENT_IDLE;

    _commandManager.Add( "connect", lazyConnect );
    _commandManager.Add( "name", lazyName );
    _commandManager.Add( "say", lazySay );

    _stringhelper.Copy( Name, namevar.GetString() );

    AckAcknowledged = 0;
    AckSequence = 0;

    latestPosition = ncVec3( 0.0, 10.0, 0.0 );
}

/*
    Add ack command.
*/
void Client::SendAcknowledge( const char *command ) {
    
    int unacknowledged = AckSequence - AckAcknowledged;
    int commandCount = sizeof( AckCommands ) / sizeof( AckCommands[0] );

    if( unacknowledged > commandCount ) {
        _core.Error( ERC_FATAL, "Client command overflow" );
        return;
    }

    AckSequence++;
    AckCommands[AckSequence & commandCount - 1] = (char*)command;
}

/*
    Check client timeout.
*/
void Client::CheckTimeout( void ) {
    if( State == CLIENT_IDLE )
        return;

    if( Time - LastMessageReceivedAt > ( client_servertimeout.GetInteger() * 10000.0 ) ) {
        _core.Print( LOG_INFO, "Connection timed out.\n" );
        
        // Disconnect from server and clear info.
        Disconnect();
    }
}

/*
    Connect to the server.
*/
void Client::Connect( void ) {
    
    if( server_dedi.GetInteger() ) {
        _core.Print( LOG_INFO, "Not available in server dedicated mode.\n" );
        return;
    }

    if( _commandManager.ArgCount() < 2 ) {
        _core.Print( LOG_INFO, "Usage: connect <ip> <port>\n" );
        return;
    }

    if( server_running.GetInteger() ) {
        _core.Print( LOG_WARN, "You need to shutdown the server first.\n" );
        return;
    }

    struct sockaddr_in _server;
    
    const char *addr = _commandManager.Arguments(0);
    const int port = atoi(_commandManager.Arguments(1));
    
    _core.Print( LOG_INFO, "Connecting to %s:%i...\n", addr, port );

    // FIXME: This is ugly
    if(( !strcmp( "localhost", addr ) ||
         !strcmp( "127.0.0.1", addr ) ) &&
         port == network_port.GetInteger() ) {
        
        _core.Print( LOG_INFO, "Local client - address type is loopback.\n" );
        server_running.Set( "1" );
    }
    else
        server_running.Set( "0" );
    
    
    // Create temp sockaddr.
    zeromem( &_server, sizeof( _server ) );

    _server.sin_family = AF_INET;
    _server.sin_port = htons( port );
    _server.sin_addr.s_addr = inet_addr( addr );

    CurrentServer = new netdata_t();
    CurrentServer->sockaddress = _server;
    CurrentServer->port = atoi(  _commandManager.Arguments(1) );
    CurrentServer->sc = _netmanager.GetSocket();
    _stringhelper.Copy( CurrentServer->ip, addr );
    
    // Resolve the network address.
    // If it changes then re-assign it.
    if( _netmanager.Resolve( CurrentServer, addr ) )
        _stringhelper.Copy( CurrentServer->ip, inet_ntoa( CurrentServer->sockaddress.sin_addr ) );
    
    // Create network channel.
    _netmanager.CreateChannel( &Channel, CurrentServer );
    
    // Set client to be active.
    client_running.Set( "1" );
    
    State = CLIENT_PRECONNECTING;
    
    LastMessageReceivedAt = Time;
    LastConnectPacketTime = -999999;

    // Temp.
    _clientgame.Loadmap("demo");
}

/*
    Quick local connect.
*/
void Client::LoopbackConnect( void ) {
    if( server_dedi.GetInteger() )
        return;
    
    _gconsole.Execute( "connect %s 4004", network_localip.GetString() );
}

/*
    Process server ack commands.
*/
void Client::ProcessAcknowledgeCommands( ncBitMessage *msg ) {

    int commandSequence = msg->ReadInt32();
    char *command = msg->ReadString();

    if( commandSequence > LastExecutedack ) {

        /*
            Gooosh, this is very bad.
        */

        int     i;
        char    *p;
        const char    *token[8];

        // Three parameters.
        for ( i = 0; i < 8; i++ )
            token[i] = "";

        i = 0;
        p = strtok (command, " ");

        while( p != NULL ) {
            token[i++] = p;
            p = strtok (NULL, " ");
        }

        if( !strcmp( token[0], "disconnect" ) ) {
            DisconnectForced(token[1], false);
        }
        else if( !strcmp( token[0], "print") ) {
            _core.Print( LOG_INFO, "Server: %s\n", token[1] );
        }

        LastExecutedack = commandSequence;
        LastackMessage = commandSequence;
    }
}

/*
    Send command packet to server.
*/
#define BASE_PACKET_SIZE 4096
void Client::SendCommandPacket( void ) {

    if( !client_running.GetInteger() )
        return;

    if( State != CLIENT_CONNECTED )
        return;

    int i, clientCommands;

    clientCommands = sizeof(AckCommands) / sizeof(AckCommands[0]);

    ncBitMessage *msg = new ncBitMessage( BASE_PACKET_SIZE );
    msg->WriteInt32( LastServerMessage );
    msg->WriteInt32( LastackMessage );

    for ( i = AckAcknowledged + 1; i <= AckSequence; i++ ) {
        msg->WriteByte( COMMANDHEADER_ACK );
        msg->WriteInt32( i );
        msg->WriteString( AckCommands[i & clientCommands - 1] );
    }

    msg->WriteByte( COMMANDHEADER_MOVE );

    // Fuuuu
    // We're sending g_vLook, server will
    // calculate g_vUp and g_vRight,
    // also g_vEye for client view.

    msg->WriteInt32( _camera.deltaMove );
    msg->WriteFloat( _camera.g_vLook.x );
    msg->WriteFloat( _camera.g_vLook.y );
    msg->WriteFloat( _camera.g_vLook.z );
    
    _netmanager.SendMessageChannel( &Channel, msg );

    CommandSequence++;
}

void Client::CheckCommands( void ) {
    TimeSinceLastPacket += _core.Time;
    
    if( TimeSinceLastPacket > ( 1000 / client_maxpackets.GetInteger() ) ) {
        SendCommandPacket();
        TimeSinceLastPacket = 0;
    }
}

void Client::ParseCommands( netdata_t *from, ncBitMessage *buffer ) {

    if( !client_running.GetInteger() )
        return;

    if( State != CLIENT_CONNECTED )
        return;

    buffer->BeginReading();

    LastMessageReceivedAt = Time;
    LastServerMessage = buffer->ReadInt32();

    int cmd;
    while( 1 ) {

        cmd = buffer->ReadByte();
        if( (cmd == COMMANDHEADER_SERVERACK  || cmd == COMMANDHEADER_SERVERENTITY ) ) {

        switch(cmd) {
            case COMMANDHEADER_SERVERENTITY:
            {
                int temp = buffer->ReadInt32();
                AckAcknowledged = buffer->ReadInt32();

                if ( AckAcknowledged < AckSequence - 128 ) {
                    AckAcknowledged = AckSequence;
                }

                TimeBase = (int)(_core.Time - Time);

                int i;
                for( i = buffer->ReadInt32(); i != MAX_SERVER_ENTITIES; i = buffer->ReadInt32() ){
                    float x = buffer->ReadCoord();
                    float y = buffer->ReadCoord();
                    float z = buffer->ReadCoord();

                    float ax = buffer->ReadFloat();
                    float ay = buffer->ReadFloat();
                    float az = buffer->ReadFloat();

                    // a + (b - a) * t;
                    latestPosition = ncVec3( x, y, z );
                    latestRotation = ncVec3( ax, ay, az );
                    
                    latestPositionTime = Time;
                }
            }
            break;

            case COMMANDHEADER_SERVERACK:
                ProcessAcknowledgeCommands( buffer );
            break;
            }
        }
        else break;
    }
}

/*
    Client connectionless packets.
*/
void Client::Connectionless( netdata_t *from, byte *data ) {

    /* It is not good method to split messages, but well... */

    int     i;
    char    *p;
    const char    *token[8];

    data += 4;
    // Three parameters.
    for ( i = 0; i < 8; i++ )
        token[i] = "";

    i = 0;
    p = strtok ((char*)data, " ");

    while (p != NULL) {
        token[i++] = p;
        p = strtok (NULL, " ");
    }

    if( !strcmp( token[0], "requestResponse" ) ) {
        Response = atoi(token[1]);
        State = CLIENT_CONNECTING;
        LastConnectPacketTime = -99999;
        CurrentServer = from;
    }

    if( !strcmp( token[0], "print" ) ) {
        // Server sends messages like SERVER_HELLO so make a parser for it.
        _core.Print( LOG_INFO, "%s\n", token[1] );
    }

    if( !strcmp( token[0], "connectResponse" ) ) {
        _core.Print( LOG_INFO, "Connected.\n" );

        State = CLIENT_CONNECTED;
    }
}

/*
    Parse all incoming information from server.
*/
void Client::Process( netdata_t *from, byte *buffer ) {
    if( !client_running.GetInteger() )
        return;

    if( strlen((char*)buffer) >= 4 && *(int*)buffer == -1 ) {
        Connectionless( from, buffer );
        return;
    }

    ncBitMessage s = ncBitMessage( buffer, MAX_UDP_PACKET );
    ParseCommands( from, &s );
}

/*
    Works like disconnect.
*/
void Client::Reconnect( void ) {
    // TODO
}

/*
    Disconnect the client from current server ( also clear server info ).
*/
void Client::DisconnectForced( const char *msg, bool forced ) {

    if( !client_running.GetInteger() )
        return;

    if( State == CLIENT_IDLE )
        return;

    SendAcknowledge( "disconnect ");
    SendCommandPacket();
    SendAcknowledge( "disconnect" );
    SendCommandPacket();

    _renderer.RemoveWorld( "Client disconnect." ); // Remove current world.

    State = CLIENT_IDLE;  // Set our state to IDLE.

    client_running.Set( "0" );

    delete [] CurrentServer;
    
    memset( AckCommands, 0, sizeof( AckCommands ) );
    
    _core.Print( LOG_INFO, "Disconnected from server.\n" );
    _core.Print( LOG_INFO, "\"%s\"\n", msg );
}

/*
    Change client name.
*/
void Client::ChangeName( void ) {
    if(  _commandManager.ArgCount() < 2 ) {
        _core.Print( LOG_NONE, "\n" );
        _core.Print( LOG_INFO, "Usage: name <username>" );
        _core.Print( LOG_INFO, "Choose any name which you would like to use in game.\n" );
        _core.Print( LOG_WARN, "No empty names allowed. Maximum name length is %i characters.\n", MAX_PLAYER_NAME_LEN );
        _core.Print( LOG_INFO, "Try typing *random name* to get random nickname. :)\n" );
        return;
    }

    if( strlen( _commandManager.Arguments(0)) > MAX_PLAYER_NAME_LEN ) {
        _core.Print( LOG_INFO, "Too long name, maximum %i symbols.\n", MAX_PLAYER_NAME_LEN );
        return;
    }

    _stringhelper.SPrintf( Name, MAX_PLAYER_NAME_LEN + 1,  _commandManager.Arguments(0));
    namevar.Set( _commandManager.Arguments(0) );

    _core.Print( LOG_INFO, "Your name was successfully changed to '%s'. Awesome!\n", namevar.GetString() );
}

/*
    Check connect command.
*/
void Client::CheckConnect( void ) {

    /* Do not check connect packet while in game or idle. */
    if ( State != CLIENT_CONNECTING && State != CLIENT_PRECONNECTING )
		return;

    switch( State ) {
        case CLIENT_PRECONNECTING:
            if ( Time - LastConnectPacketTime > RESPONSE_REPEAT ) {
                _netmanager.PrintOutOfBand( CurrentServer, "getresponse" );
                LastConnectPacketTime = Time;

                _core.Print( LOG_INFO, "Awaiting server response..\n" );
            }
            break;

        case CLIENT_CONNECTING:
            if ( Time - LastConnectPacketTime > RESPONSE_REPEAT ) {
                _netmanager.PrintOutOfBand( CurrentServer, "connect %i \"%s\" %s", Response, namevar.GetString(), _version );
                LastConnectPacketTime = Time;

                _core.Print( LOG_INFO, "Awaiting connect response..\n" );
            }
            break;
        default:
            _core.Error( ERC_FATAL, "Unexpected client state in CheckCommand\n" );
            break;
    }
}

/*
    Not prediction at all.
    Made for smooth movement.
*/
void Client::Interpolate( void ) {
    if( State != CLIENT_CONNECTED )
        return;

    if( latestPositionTime <= 0.0f )
        return;

    // Interpolation.
    // Some kind of magic, but works very smooth. Oh yeah.
    // Anyways: fix me.

    float t = 1.0f - ( (latestPositionTime - Time) / 0.3 );

    _camera.g_vEye.x = Math_Lerpf2( _camera.g_vEye.x, latestPosition.x, (t / 30.0f) / 25.0f );
    _camera.g_vEye.y = Math_Lerpf2( _camera.g_vEye.y, latestPosition.y, (t / 30.0f) / 25.0f );
    _camera.g_vEye.z = Math_Lerpf2( _camera.g_vEye.z, latestPosition.z, (t / 30.0f) / 25.0f );
}

/*
    Client update function.
*/
void Client::Frame( int msec ) {
    
    if( !client_running.GetInteger() )
        return;

    // Client movement.
    Interpolate();

    // Client time.
	Frametime = msec;
	Time += Frametime;

    // Check incoming commands.
    CheckCommands();

    // Check server timeout.
    CheckTimeout();

    // Check connection command.
    CheckConnect();
}


/*          CLIENT COMMANDS             */

/*
    "Say" command.
*/
void Client::Say( void ) {

    if(  _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "USAGE: say <message>\n" );
        return;
    }

    if( server_dedi.GetInteger() ) {
        _core.Print( LOG_NONE, "%s: %s\n", server_sayname.GetString(), _commandManager.Arguments(0) );
        _server.SendAcknowledgeCommand( NULL, false, "print %s", _commandManager.Arguments(0) );
        return;
    }

    if( client_running.GetInteger() ) {
        SendAcknowledge( _stringhelper.STR("say %s", _commandManager.Arguments(0)) );
    }
}


