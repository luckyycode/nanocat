//
//  Nanocat engine.
//
//  Multiplayer: Client side..
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "System.h"
#include "MultiplayerServer.h"
#include "MultiplayerClient.h"
#include "Core.h"
#include "Network.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "Camera.h"
#include "Renderer.h"
#include "NCString.h"
#include "GameMath.h"
#include "LocalGame.h"
#include "ncBitMessage.h"

ncClient _client;

/*
    Client side.
*/

// Is client running?
ncConsoleVariable      Client_Running( "client", "running", "Is client running?", "0", CVFLAG_READONLY );
// User name.
ncConsoleVariable      NameVar( "client", "name", "Client username.", "A cat", CVFLAG_NONE );
// Server timeout. ( In seconds )
ncConsoleVariable      Client_ServerTimeout( "client", "servertimeout", "Server timeout.", "30", CVFLAG_NEEDSREFRESH );
// Maximum packets to be send per frame.
ncConsoleVariable      Client_MaximumPackets( "client", "maxpackets", "Maximum packets to be send per frame.", "33", CVFLAG_NEEDSREFRESH );

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
void ncClient::Disconnect( void ) {
    // Forced disconnect.
    _client.DisconnectForced( "User disconnect.", true );
}

/*
    Initialize client stuff.
*/
void ncClient::Initialize() {
    _core.Print( LOG_INFO, "Client initializing...\n" );

    NameVar.Set( _system.GetCurrentUsername() );
    State = CLIENT_IDLE;

    // Commands.
    _commandManager.Add( "connect", lazyConnect );
    _commandManager.Add( "name", lazyName );
    _commandManager.Add( "say", lazySay );

    _stringhelper.Copy( Name, NameVar.GetString() );

    AckAcknowledged = 0;
    AckSequence = 0;

    latestPosition = ncVec3( 0.0, 10.0, 0.0 );
}

/*
    Add ack command.
*/
void ncClient::SendAcknowledge( const char *command ) {
    
    int unacknowledged = AckSequence - AckAcknowledged;
    int commandCount = sizeof( AckCommands ) / sizeof( AckCommands[0] );

    if( unacknowledged > commandCount ) {
        _core.Print( LOG_INFO, "ncClient::SendAcknowledge - Error is going to happen. Command overflow ( Got %i )\n", unacknowledged );
        _core.Error( ERC_FATAL, "ncClient::SendAcknowledge - Client command stack overflow." );
        return;
    }

    AckSequence++;
    AckCommands[AckSequence & commandCount - 1] = (char*)command;
}

/*
    Check client timeout.
*/
void ncClient::CheckTimeout( void ) {
    if( State == CLIENT_IDLE )
        return;

    if( Time - LastMessageReceivedAt > ( Client_ServerTimeout.GetInteger() * 10000.0 ) ) {
        _core.Print( LOG_INFO, "Active connection timed out.\n" );
        
        // Disconnect from server and clear info.
        Disconnect();
    }
}

/*
    Connect to the server.
*/
void ncClient::Connect( void ) {
    
    if( Server_Dedicated.GetInteger() ) {
        _core.Print( LOG_INFO, "Not available in server dedicated mode.\n" );
        return;
    }

    if( _commandManager.ArgCount() < 2 ) {
        _core.Print( LOG_INFO, "USAGE: connect <address> <port>\n" );
        return;
    }

    if( Server_Active.GetInteger() ) {
        _core.Print( LOG_WARN, "You need to shutdown the server first.\n" );
        return;
    }

    struct sockaddr_in _server;
    int socket;
    
    const char *addr = _commandManager.Arguments(0);
    const int port = atoi(_commandManager.Arguments(1));
    
    _core.Print( LOG_INFO, "Connecting to %s:%i...\n", addr, port );

    // FIXME: This is ugly
    if(( !strcmp( "localhost", addr ) ||
         !strcmp( "127.0.0.1", addr ) ) &&
         port == network_port.GetInteger() ) {
        
        _core.Print( LOG_INFO, "Local client - address type is loopback.\n" );
        Server_Active.Set( "1" );
    }
    else
        Server_Active.Set( "0" );
    
    
    // Create temp sockaddr.
    zeromem( &_server, sizeof( _server ) );

    _server.sin_family = AF_INET;
    _server.sin_port = htons( port );
    _server.sin_addr.s_addr = inet_addr( addr );

    socket = _netmanager.GetSocket();
    
    CurrentServer = new ncNetdata( _server, port, socket );
    
    _stringhelper.Copy( CurrentServer->IPAddress, addr );
    
    // Resolve the network address.
    // If it changes then re-assign it.
    if( _netmanager.Resolve( CurrentServer, addr ) )
        _stringhelper.Copy( CurrentServer->IPAddress, inet_ntoa( CurrentServer->SockAddress.sin_addr ) );
    
    // Create network channel.
    _netmanager.CreateChannel( &Channel, CurrentServer );
    
    // Set client to be active.
    Client_Running.Set( "1" );
    
    State = CLIENT_PRECONNECTING;
    
    LastMessageReceivedAt = Time;
    LastConnectPacketTime = -999999;

    ClientID = _core.Time ^ 0xFFFFFF;
    
    // Temp.
    _clientgame.Loadmap( "demo" );
}

/*
    Quick local connect.
*/
void ncClient::LoopbackConnect( void ) {
    if( Server_Dedicated.GetInteger() )
        return;
    
    _gconsole.Execute( "connect 127.0.0.1 4004" );
}

/*
    Process server ack commands.
*/
void ncClient::ProcessAcknowledgeCommands( ncBitMessage *msg ) {

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
void ncClient::SendCommandPacket( void ) {

    if( !Client_Running.GetInteger() )
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

void ncClient::CheckCommands( void ) {
    TimeSinceLastPacket += _core.Time;
    
    if( TimeSinceLastPacket > ( 1000 / Client_MaximumPackets.GetInteger() ) ) {
        SendCommandPacket();
        TimeSinceLastPacket = 0;
    }
}

void ncClient::ParseCommands( ncNetdata *from, ncBitMessage *buffer ) {

    if( !Client_Running.GetInteger() )
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
void ncClient::Connectionless( ncNetdata *from, byte *data ) {

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
void ncClient::Process( ncNetdata *from, byte *buffer ) {
    if( !Client_Running.GetInteger() )
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
void ncClient::Reconnect( void ) {
    // TODO
}

/*
    Disconnect the client from current server ( also clear server info ).
*/
void ncClient::DisconnectForced( const char *msg, bool forced ) {

    if( !Client_Running.GetInteger() )
        return;

    if( State == CLIENT_IDLE )
        return;

    SendAcknowledge( "disconnect ");
    SendCommandPacket();
    SendAcknowledge( "disconnect" );
    SendCommandPacket();

    _renderer.RemoveWorld( "Client disconnect." ); // Remove current world.

    State = CLIENT_IDLE;  // Set our state to IDLE.

    Client_Running.Set( "0" );

    // Causes some errors, I don't know why.
    //delete CurrentServer;
    
    zeromem( CurrentServer, sizeof(CurrentServer) );
    
    memset( AckCommands, 0, sizeof( AckCommands ) );
    
    _core.Print( LOG_INFO, "Disconnected from server.\n" );
    _core.Print( LOG_INFO, "\"%s\"\n", msg );
}

/*
    Change client name.
*/
void ncClient::ChangeName( void ) {
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
    NameVar.Set( _commandManager.Arguments(0) );

    _core.Print( LOG_INFO, "Your name was successfully changed to '%s'. Awesome!\n", NameVar.GetString() );
}

/*
    Check connect command.
*/
void ncClient::CheckConnect( void ) {

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
                _netmanager.PrintOutOfBand( CurrentServer, "connect %i \"%s\" %s", Response, NameVar.GetString(), _version );
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
void ncClient::Interpolate( void ) {
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
void ncClient::Frame( int msec ) {
    
    if( !Client_Running.GetInteger() )
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
void ncClient::Say( void ) {

    if( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "USAGE: say <message>\n" );
        return;
    }

    if( Server_Dedicated.GetInteger() ) {
        _core.Print( LOG_NONE, "%s: %s\n", Server_Sayname.GetString(), _commandManager.Arguments(0) );
        _server.SendAcknowledgeCommand( NULL, false, "print %s", _commandManager.Arguments(0) );
        return;
    }

    if( Client_Running.GetInteger() ) {
        SendAcknowledge( _stringhelper.STR("say %s", _commandManager.Arguments(0)) );
    }
}


