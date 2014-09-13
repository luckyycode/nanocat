//
//  Nanocat engine
//
//  Multiplayer: Game server..
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "MultiplayerServer.h"
#include "Network.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "Renderer.h"
#include "NCString.h"
#include "LocalGame.h"
#include "System.h"

/*
    Server Side.

    * How it works? Simple!
    * Client sends looking direction *g_vLook* and move delta command ( 1, 2, 3, 4 ).
    * Server calculates client position and sends it back to all clients.
    * Client is server entity. So entities work by the same way.

    * No need to load map files or whatever.
 
    Server messages:
        * SERVER_HELLO - Test.
        * SERVER_BAD_RESPONSE - Bad client response.
*/

ncServer _server;

// Maximum clients on server.
ncConsoleVariable  Server_Maxclients( "server", "maxclients", "Maximum server clients.", "2", CVFLAG_NEEDSREFRESH );
// Server public name.
ncConsoleVariable  Server_Name( "server", "name", "Server name.", DEFAULT_SERVER_NAME, CVFLAG_NONE );
// Is server running?
ncConsoleVariable  Server_Active( "server", "running", "Is server running?", "0", CVFLAG_NONE );
// Cheats enabled?
ncConsoleVariable  Server_Fun( "server", "cheats", "Server cheats enabled?", "1", CVFLAG_NEEDSREFRESH );
// Server framerate.
ncConsoleVariable  Server_Maxfps( "server", "fps", "Server framerate.", "10", CVFLAG_NONE );
// Name which is going to be used in chat.
ncConsoleVariable  Server_Sayname( "server", "sayname", "Server name to be used in chat.", "Friskies", CVFLAG_NEEDSREFRESH );
// Default bot name.
ncConsoleVariable  Server_Botname( "server", "botname", "Server bot name.", "Whiskas", CVFLAG_NONE );
// Is server dedicated?
ncConsoleVariable  Server_Dedicated( "server", "dedicated", "Is server dedicated?", "0", CVFLAG_NONE );
// In-active client timeout.
ncConsoleVariable  Server_Clienttimeout( "server", "clienttimeout", "Server client timeout.", "30", CVFLAG_NONE );
// Status print per x seconds
ncConsoleVariable  Server_Statusperiod( "server", "statusprint", "Print server status per X seconds.", "30", CVFLAG_NONE );
// ncConsoleVariable server_cutebots( "server", "cutebots", "Smiling and nice bots.", "1", CVFLAG_KID );

/*
    Lazy functions.
*/
void server_tests (void) {
    //_client.SendAc("Hello, Server!\n");
}

void lazyServerMap( void ) {
    _server.Loadmap();
}

void lazyServerKick( void ) {
    _server.KickClient();
}

void lazyServerPrintStatus( void ) {
    _server.PrintStatus();
}

void lazyServerAddbot( void ) {
    _server.AddBot();
}

void lazyServerLaunch( void ) {
    _server.Launchmap();
}

void lazyServerShutdown( void ) {
    _server.Shutdown("Game quit.\n");
}

ncServerClient::ncServerClient( void ) {
    lastMessageTime = 0;
    lastCommandTime = 0;
    lastConnectTime = 0;
    lastReceivedackCommand = 0;
    lastExecutedackCommand = 0;
    lastAcknowledgedMessage = 0;
    lastMessageReceivedAt = 0;

    ackSequence = 0;
    ackAcknowledged = 0;
    zombifiedAt = 0;
    response = 0;
}

/*
    Initialize server.
    Load only on application start. Never calls again.
*/
void ncServer::Initialize( void ) {
    float   t1, t2; // Load time.

    t1 = _system.Milliseconds();
    _core.Print( LOG_INFO, "Server initializing	...\n" );

    if( Initialized ) {
        _core.Print( LOG_DEVELOPER, "Uh, someone tried to call me, but I have already initialized.\n" );
        return;
    }
    
    // Initial values.
    Time = 0;
    LastTime = 0;
    LastInfoPrintTime = 0;
    State = SERVER_IDLE;
    
    zeromem( Response, sizeof(ncServerResponseData) );

    // Execute server file.
    _gconsole.Execute( "readconfig server.nconf" );

    // Check server console variables.
    CheckParams();

    MaxClients = Server_Maxclients.GetInteger();
    Port  = network_port.GetInteger();
    ClientNum = 0;
    
    _stringhelper.Copy( Name, Server_Name.GetString() );

    _core.Print( LOG_INFO, "Initializing server client data...\n" );

    // Initialize client slots.
    SetupClients( Server_Maxclients.GetInteger() );

    // Let user know some stuff.
    _core.Print( LOG_INFO, "Our server name is '%s'\n", Server_Name.GetString() );
    _core.Print( LOG_INFO, "Maximum client allowed - '%i'\n", Server_Maxclients.GetInteger() );
    _core.Print( LOG_INFO, "Our network port is '%i'\n\n", network_port.GetInteger() );

    // Add server commands.
    _commandManager.Add( "map",             lazyServerMap );
    _commandManager.Add( "kick",            lazyServerKick );
    _commandManager.Add( "status",          lazyServerPrintStatus );
    _commandManager.Add( "addbot",          lazyServerAddbot );
    _commandManager.Add( "launch",          lazyServerLaunch );
    _commandManager.Add( "s",               server_tests );
    _commandManager.Add( "lazykillserver",  lazyServerShutdown );

    Initialized      = true;
    t2 = _system.Milliseconds();

    _core.Print( LOG_INFO, "Server took %4.2f ms to initialize.\n", t2 - t1 );
}

/*
    Create server.
*/
void ncServer::CreateSession( void ) {
    if( !Initialized ) {
        _core.Print( LOG_WARN, "Server was not initialized while I was creating session, initializing server..\n" );
        Initialize();
    }

    _core.Print( LOG_INFO, "Creating server session..\n" );

    // Default server configuration.
    _gconsole.Execute( "readconfig server.nconf" );

    _core.Print( LOG_INFO, "Server name: %s\n", Server_Name.GetString() );

    // Will be reseted to default value if needed.
    SetupClients( Server_Maxclients.GetInteger() );

    _core.Print( LOG_NONE, "\n");
    _core.Print( LOG_INFO, "Our server name is '%s'\n", Server_Name.GetString() );

    // Some user information.
    _core.Print( LOG_INFO, "Max clients allowed - '%i'\n", Server_Maxclients.GetInteger() );
    _core.Print( LOG_INFO, "Our network port is '%i'\n", network_port.GetInteger() );
    _core.Print( LOG_NONE, "\n");

    // Reset server session id, so we don't get fcked up.
    ServerIdentificator = 0xFFFF ^ _core.Time;

    // Everything went okay, set game to be active.
    _core.Print( LOG_DEVELOPER, "Server is active now.\n" );

    // Time warp fix.
    State   = SERVER_GAME;
    Time    = 0;
}

/*
     Set maximum clients to be allowed.
*/
void ncServer::SetupClients( int maxclients ) {
    if( Clients )
        return;

    int cl;

    if( maxclients >= MAX_CLIENTS_NUM ) {
        _core.Print( LOG_WARN, "Could not set max clients value, maximum %i clients.\n", MAX_CLIENTS_NUM );
        Server_Maxclients.Set( "512" );

        _core.Print( LOG_INFO, "Maximum clients value set to 512\n" );
    }
    else if(maxclients < 1) {
        _core.Print( LOG_WARN, "Could not set max clients value to 0, minimum one client.\n" );
        Server_Maxclients.Set( "24" );
        
        _core.Print( LOG_INFO, "Maximum clients value set to 24 clients.\n" );
    }

    // We got nice checking here.
    if(!Clients)
        Clients = new ncServerClient[Server_Maxclients.GetInteger()];

    if(!Clients) {
        _core.Error( ERC_FATAL, "Could not allocate memory for %i clients.\n", Server_Maxclients.GetInteger() );
        return;
    }

    for( cl = 0; cl < Server_Maxclients.GetInteger(); cl ++ ) {
        if( !&Clients[cl] )
            _core.Error( ERC_SERVER, "Could not load client slots. No enough memory." );

        // Set initial values.
        memset( &Clients[cl], 0, sizeof(ncServerClient) );
    }

    _core.Print( LOG_DEVELOPER, "%i client slots loaded.\n", Server_Maxclients.GetInteger() );
}

/*
    Process client messages and commands.
*/
void ncServer::ProcessClientMessage( ncServerClient *client, ncBitMessage *msg ) {

    int commandSequence = msg->ReadInt32();
    char *command = msg->ReadString();
    
    if( commandSequence > client->lastExecutedackCommand ) {
    
        // Probably bad, fixme.
        int     i;
        char    *p;
        const char    *token[8];

        // Three parameters.
        for ( i = 0; i < 8; i++ )
            token[i] = "";

        i = 0;
        p = strtok (command, " ");

        while (p != NULL) {
            token[i++] = p;
            p = strtok (NULL, " ");
        }
        
        // Process client commands now.
        if( !strcmp( token[0], "disconnect" ) ) {
            DisconnectClient( client, "User quit" );
        }

        if( !strcmp( token[0], "say" ) ) {
            _core.Print( LOG_NONE, "%s said \"%s\"\n", client->name, token[1] );
        }

        client->lastReceivedackCommand = commandSequence;
        client->lastExecutedackCommand = commandSequence;
    }
}

/*
    Create a new client.
*/
void ncServer::CreateClient( ncNetdata *from, int response, const char *name, const char *version ) {
    int     x, i;
    int		ping;

    for ( i = 0; i < MAX_RESPONSES; i++ ) {
        // Compare addresses.
        if ( _netmanager.CompareAddress(from, &Response[i].Address) ) {
            // We found what we need.
            if ( response == Response[i].Response ) {
                _core.Print( LOG_DEVELOPER, "ncServer::CreateClient - Found connect data for %s\n", inet_ntoa(from->SockAddress.sin_addr) );
                break;
            }
        }
    }

    // This may happen.
    if ( i == MAX_RESPONSES ) {
        _netmanager.PrintOutOfBand( from, "print SERVER_BAD_RESPONSE" );
        return;
    }

    ping = (Time - Response[i].PingAt);
    
    _core.Print( LOG_DEVELOPER, "Client is connecting...\n" );
    Response[i].Connected = true;

    // TODO: remove loop.
    for( x = 0; x < Server_Maxclients.GetInteger(); x++ ) {
        ncServerClient *p_temp = new ncServerClient();

        if( !p_temp ) {
            _core.Error( ERC_SERVER, "Could not create a new client. Out of memory.\n" );
            return;
        }

        // Keep this first.
        p_temp->address = *from;
        
        _stringhelper.Copy( p_temp->version, version );
        _stringhelper.Copy( p_temp->name, name );
        _stringhelper.Copy( p_temp->address.IPAddress, inet_ntoa(from->SockAddress.sin_addr) );
        
        p_temp->response = response;
        p_temp->clientnum = ClientNum;
        
        p_temp->address.Port = from->SockAddress.sin_port;
        p_temp->address.SockAddress = from->SockAddress;
        p_temp->state = SVCL_CONNECTED;

        if( _netmanager.IsLanAddress( &p_temp->address )
           && (network_port.GetInteger() == ntohs(p_temp->address.Port))  ) {
            
            p_temp->type = ADDR_LOOPBACK;
            _core.Print( LOG_DEVELOPER, "ncServer::CreateClient - Client has local address.\n" );
        }
        else
            p_temp->type = ADDR_IP;

        p_temp->lastCommandTime = Time;
        p_temp->lastMessageTime = Time;
        p_temp->lastConnectTime = Time;
        p_temp->lastMessageReceivedAt = Time;

        // Set initial client entity variables.
        p_temp->player.g_vEye = ncVec3( 5.0, 5.0, 5.0 );
        p_temp->player.g_vLook = ncVec3( -10.5, -0.5, -0.5 );
        p_temp->player.g_vRight= ncVec3( 1.0, 0.0, 0.0 );
        p_temp->player.g_vUp = ncVec3( 1.0, 1.0, 0.0 );

        // Create network channel.
        _netmanager.CreateChannel( &p_temp->channel, &p_temp->address );

        if( Clients[x].state != SVCL_CONNECTED ) {
            Clients[x] = *p_temp;

            delete p_temp;
            p_temp = NULL;

            break;
        }
    }


    _netmanager.PrintOutOfBand( from, "connectResponse %i", ClientNum );
    _core.Print( LOG_INFO, "%s has joined the adventure. \n", name );
    
    memset( &Response[i], 0, sizeof( ncServerResponseData ) );

    ++ClientNum;
}

/*
    Server connectionless packets.
*/
void ncServer::Connectionless( ncNetdata *from, byte *data ) {
    int     i;
    char    *p;
    const char    *token[8];

    // Skip first four bytes.
    // Use memcpy next time.
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

    if( !strcmp(token[0], "getresponse" ) )
        GetResponse(from);

    // connect "response id" "Username" "Version"
    if( !strcmp(token[0], "connect" ) )
        CreateClient(from, atoi(token[1]), token[2], token[3]);
}

/*
    Parse client commands.
*/
void ncServer::ParseClients( ncNetdata *from, ncBitMessage *packet ) {

    if( !Server_Active.GetInteger() )
        return;
    
    // Should never happen.
    if( !packet ) {
        _core.Print( LOG_ERROR, "ncServer::ParseClient - empty message packet pointed.\n" );
        return;
    }

    ncServerClient *p_temp;
    int command = 0;

    p_temp = GetClientByAddress( from->SockAddress );

    if( !p_temp ) {
        _core.Print( LOG_INFO, "Couldn't find client by ip %s\n", inet_ntoa( from->SockAddress.sin_addr) );
        return;
    }

    if( p_temp->state != SVCL_CONNECTED ) {
        _core.Print( LOG_INFO, "Found client, but user isn't connected.\n" );
        return;
    }

    packet->BeginReading();

    Clients[p_temp->clientnum].lastMessageReceivedAt = Time;
    Clients[p_temp->clientnum].lastAcknowledgedMessage = packet->ReadInt32();
    Clients[p_temp->clientnum].ackAcknowledged = packet->ReadInt32();
    
    while(1) {
        command = packet->ReadByte();
        
        // Do not remove.
        if( ( command == COMMANDHEADER_MOVE || command == COMMANDHEADER_ACK ) ) {

            switch(command) {
                case COMMANDHEADER_MOVE: {
                    int delta = packet->ReadInt32();
                    
                    // g_vLook
                    float g_vLx = packet->ReadFloat();
                    float g_vLy = packet->ReadFloat();
                    float g_vLz = packet->ReadFloat();

                    // Probably not that good place where we should
                    // calculate the view for client entity.
                    Clients[p_temp->clientnum].player.g_vLook.x = g_vLx;
                    Clients[p_temp->clientnum].player.g_vLook.y = g_vLy;
                    Clients[p_temp->clientnum].player.g_vLook.z = g_vLz;

                    Clients[p_temp->clientnum].player.g_vLook.Normalize();

                    Clients[p_temp->clientnum].player.g_vRight.Cross( Clients[p_temp->clientnum].player.g_vLook, Clients[p_temp->clientnum].player.g_vUp );
                    Clients[p_temp->clientnum].player.g_vRight.Normalize();

                    Clients[p_temp->clientnum].player.g_vUp.Cross( Clients[p_temp->clientnum].player.g_vRight, Clients[p_temp->clientnum].player.g_vLook );
                    Clients[p_temp->clientnum].player.g_vUp.Normalize();

                    ncVec3 tmpLook  = Clients[p_temp->clientnum].player.g_vLook;
                    ncVec3 tmpRight = Clients[p_temp->clientnum].player.g_vRight;

                    float speed = cam_speed.GetFloat();
                    
                    switch( delta ) {
                        case 0: // Nothing, standing still.

                        break;

                        case 1: // Forward.
                        {
                            ncVec3 s = tmpLook * -speed;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye - s;
                        }
                            break;
                            
                        case 2: // Backward.
                        {
                            ncVec3 s = tmpLook * -speed;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye + s;
                        }
                            break;
                            
                        case 3:  // Left.
                        {
                            ncVec3 s = tmpRight * speed;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye - s;
                        }
                            break;
                            
                        case 4: // Right.
                        {
                            ncVec3 s = tmpRight * speed;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye + s;
                        }
                            break;


                        // Oops
                        default:
                            DisconnectClient( &Clients[p_temp->clientnum], "Unknown client delta move command.\n" );
                        break;
                    }

                }
                break;
                
                case COMMANDHEADER_ACK:
                    ProcessClientMessage( p_temp, packet );
                break;
                }
            }
            else
                break; // No way. Packet fully received.
        }
}

/*
    Send current game frame to client.
*/
void ncServer::SendFrame( ncServerClient *cl ) {

    if( !cl )
        return;

    // Since bots are useless now.
    if( cl->type == ADDR_BOT )
        return;
    
    int clientCommands, i, j;

    ncBitMessage *msg = new ncBitMessage( MAX_SNAPSHOT_SIZE );
    msg->WriteInt32( cl->channel.sequenceOut );
    
    /* Get client command count. */
    clientCommands = sizeof( cl->ackCommands ) / sizeof( cl->ackCommands[0] );

    /* Send ack ack. */
    for ( i = cl->ackAcknowledged + 1; i <= cl->ackSequence; i++ ) {
        msg->WriteByte( COMMANDHEADER_ACK );
        msg->WriteInt32( i );
        msg->WriteString( cl->ackCommands[i & (clientCommands - 1)] );
    }

    /* Server entities. */
    msg->WriteByte( COMMANDHEADER_SERVERENTITY );
    msg->WriteInt32( Time );
    msg->WriteInt32( cl->lastReceivedackCommand );

    // Server world entities.
    // TODO: Make clients to entities.
    
    // Using client data, okay for now, BUT
    // make me into server entity.
    
    for( j = 0; j < ClientNum; j++ ) {
        if( Clients[j].state != SVCL_CONNECTED )
            continue;
        
        // Entity number.
        msg->WriteInt32( j );
        
        // Position.
        msg->WriteCoord( Clients[j].player.g_vEye.x );
        msg->WriteCoord( Clients[j].player.g_vEye.y );
        msg->WriteCoord( Clients[j].player.g_vEye.z );
        
        // Rotation.
        msg->WriteFloat( Clients[j].player.g_vLook.x );
        msg->WriteFloat( Clients[j].player.g_vLook.y );
        msg->WriteFloat( Clients[j].player.g_vLook.z );
    }

    /* End byte for entity data. */
    msg->WriteInt32( MAX_SERVER_ENTITIES );

    _netmanager.SendMessageChannel( &cl->channel, msg );
}

/*
    Send current game server world to all clients.
*/
void ncServer::SendFrames( void ) {
    int i;
    for( i = 0; i < ClientNum; i++ ) {
        if( Clients[i].state == SVCL_FREE )
            continue;

        if( !&Clients[i] )
            continue;
        
        SendFrame( &Clients[i] );
    }
}

/*
    Listen for incoming data.
*/
void ncServer::Process( ncNetdata *from, byte *buffer ) {
    if( !Server_Active.GetInteger() )
        return;
    
    // Should never happen. But I am still afraid.
    if( strlen((const char*)buffer) > MAX_SERVER_COMMAND ) {
        _core.Print( LOG_DEVELOPER, "Exceeded packet length from %s.\n", from->IPAddress );
        return;
    }

    // Client out of band packets.
    if( strlen((const char*)buffer) >= 4 && *(int*)buffer == -1 ) {
        Connectionless( from, buffer );
        return;
	}

    // Parse existing clients.
    ncBitMessage message = ncBitMessage( buffer, MAX_UDP_PACKET );
    ParseClients( from, &message );
}


/*
    Check server console variables.
 
    Note: we check client maximum number in SetupClients.
*/
void ncServer::CheckParams( void ) {
    if( strlen( Server_Name.GetString() ) > 64 ) {
        _core.Print( LOG_WARN, "Current server name is too long. Resetting to the default.\n" );
        Server_Name.Set( DEFAULT_SERVER_NAME );
    }
    
    // Already checked in SetupClients, but nevermind...
    if( Server_Maxclients.GetInteger() ) {
        _core.Print( LOG_INFO, "Server has too much client number, resetting..\n" );
        Server_Maxclients.Set( _stringhelper.STR( "%i", SERVER_DEFAULT_CLIENTNUM ) );
        return;
    }
}

/*
     Clear the world.
*/
void ncServer::ClearWorld( const char *msg ) {
    if( !Server_Active.GetInteger() ) {
        _core.Print( LOG_WARN, "ClearWorld - I've tried to clear the world, but there's no server running!\n" );
        return;
    }

    _renderer.RemoveWorld( "Server" );
}

/*
    Print server information.
*/
void ncServer::PrintInfo( void ) {
    if( !Server_Active.GetInteger() )
        return;

    if( Time - LastInfoPrintTime > (Server_Statusperiod.GetInteger() * 1000) ) {
        LastInfoPrintTime = Time;

        _core.Print( LOG_INFO, "\n" );
        _core.Print( LOG_INFO, "------------------ Server status ---------------------\n" );
        _core.Print( LOG_INFO, "Server: \"%s\". We got %i users online.\n", Server_Name.GetString(), ClientNum );
        _core.Print( LOG_INFO, "We are alive for %i minute(s).\n", (Time / 60000) );
    }
}

/*
    Check client timeouts.
*/
void ncServer::CheckTimeouts( void ) {
    if( !Server_Active.GetInteger() )
        return;

    int i;
    for( i = 0; i < ClientNum; i++ ) {
        if( (Clients[i].state == SVCL_ZOMBIE || Clients[i].state == SVCL_FREE || Clients[i].type == ADDR_BOT ) )
            continue;

        if( !&Clients[i] )
            return;

        if( Clients[i].lastMessageTime > Time ) {
             Clients[i].lastMessageTime = Time;
         }

        if( Time - Clients[i].lastMessageReceivedAt > Server_Clienttimeout.GetInteger() * 10000 ) {
            _core.Print( LOG_INFO, "%s timed out.\n", Clients[i].name );
            DisconnectClient( &Clients[i], "Connection timed out." );
        }
    }
}

/*
     Server frame function.
*/
void ncServer::Frame( int msec ) {
    int frameMsec;

    if( !Server_Active.GetInteger() )
        return;

    if( Server_Maxfps.GetInteger() < 1 )
        Server_Maxfps.Set( "10" );

    /*
        Time wrap.
        Twentyfour hours reset.
    */
    if( Time > 0x86400000 ) {
        _core.Print( LOG_INFO, "Long time passed since server load. Server restarting.\n" );
        _gconsole.Execute("map_restart");
    }

    frameMsec = 1000 / Server_Maxfps.GetInteger();
    LastTime += msec;

    while( LastTime >= frameMsec ) {
        LastTime -= frameMsec;
        Time += frameMsec;

        // Send the simulated world to all clients.
        SendFrames();

        // Check zombie clients.
        CheckZombies();

        // Check client timeouts.
        CheckTimeouts();

        // Print server information per X seconds.
        PrintInfo();
    }
}

/*
    Load server map.
*/
void ncServer::Loadmap( void ) {
    if(_commandManager.ArgCount() < 1) {
        _core.Print( LOG_INFO, "USAGE: map <World_Name>\n" );
        return;
    }

    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "SERVER LOADING...\n" );
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "\n" );

    _core.Print( LOG_INFO, "Loading server map...\n" );
    _core.Print( LOG_INFO, "Requested map: %s\n", _commandManager.Arguments(0) );

    State = SERVER_LOADING;

    _renderer.RemoveWorld( "server_map" );

    // Note:
    // Don't load the map file while server is dedicated.
    if( !Server_Dedicated.GetInteger() ) {
            if( _clientgame.Loadmap( _commandManager.Arguments(0) )) {
            CreateSession();
        }
        else {
            State   = SERVER_IDLE;
            Time    = 0;

            return;
        }
    } else {
        Server_Active.Set( "1" );
        World_Name.Set(  _commandManager.Arguments(0) );
        
        CreateSession();
    }

    _core.Print( LOG_INFO, "....world has been created!\n" );
    _core.Print( LOG_NONE, "\n" );
}

/*
    Launch server.
    Different from server_spawnserver and server_map.
*/
void ncServer::Launchmap( void )  {
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "Server loading...\n" );
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_NONE, "\n" );

    _core.Print( LOG_INFO, "Creating world...\n" );
    
    Server_Active.Set( "1" );
    World_Name.Set( "world" );
    
    // Create server session.
    CreateSession();
    State = SERVER_GAME;

    _core.Print( LOG_INFO, "World has been created and server spawned!\n" );
    _core.Print( LOG_NONE, "\n" );
}

/*
    Disconnect client with a reason.
*/
void ncServer::DisconnectClient( ncServerClient *client, const char *message ) {
    if( client->state == SVCL_ZOMBIE )
        return;
    
    if( !message )
        message = "No reason.";

    SendAcknowledgeCommand( client, true, "disconnect \"%s\"", message );
    SendFrame( client );

    _core.Print( LOG_INFO, "%s disconnected from server. %s\n", client->name, message );

    SendAcknowledgeCommand( NULL, "print \"%s disconnected. %s\"", client->name, message );
    SendFrame( client );

    client->zombifiedAt = Time;
    client->state = SVCL_ZOMBIE;
}

/*
    Get auth response from client.
*/
void ncServer::GetResponse( ncNetdata *from ) {
    int i;
    int previous = 0;
    int previousTime;
    
    ncServerResponseData *response;

    /* Get nice value here. */
    previousTime = 0x7FFFFFFF;

    // Check if response already exists for this client.
    response = &Response[0];
    
    // Don't overflow.
    for ( i = 0 ; i < MAX_RESPONSES; i++, response++ ) {
        if ( !response->Connected && _netmanager.CompareAddress( from, &response->Address ) )
            break;
		
        if ( response->Time < previousTime ) {
            previousTime = response->Time;
            previous = i;
        }
    }

    if ( i == MAX_RESPONSES ) {
        
        // Just connected.
        response = &Response[previous];
        
        /* Generate nice random number for this response. */
        response->Response = ( (rand() << 16) ^ rand() ) ^ Time;
        response->Address = *from; // TODO: use memcpy here?
        response->FirstAt = Time;
        response->Time = Time;
        response->Connected = false;
        
        i = previous;
    }

    if ( Time - response->FirstAt > SERVER_CLIENTAUTHORIZE_TIME ) {
            _core.Print( LOG_INFO, "Authorizing client... ( %s, %i )\n", inet_ntoa(response->Address.SockAddress.sin_addr), response->Response );
        
            // TODO: authorizing on server database!
        
        
            response->PingAt = Time;
            _netmanager.PrintOutOfBand( &response->Address, "requestResponse %i", response->Response );
            return;
    }
    
    // Kick client if get here.
    _core.Print( LOG_WARN, "Couldn't authorize %s ( %i authtime )\n", inet_ntoa(response->Address.SockAddress.sin_addr), Time - response->FirstAt );
    // zeromem( response, sizeof(ncResponse) );
}

/*
    Kick stuck players.
*/
void ncServer::CheckZombies( void ) {
    int i;
    for ( i = 0; i < ClientNum; i++ ) {
        if( &Clients[i] && Clients[i].state == SVCL_ZOMBIE ) {
            if( Time - Clients[i].zombifiedAt > 1000 ) {
                
                zeromem( &Clients[i], sizeof(ncServerClient) );
                
                Clients[i].state = SVCL_FREE;
                ClientNum--;
            }
        }
    }
}

/*
    Get clients status and server information.
*/
void ncServer::PrintStatus( void )
{
    if( !Server_Active.GetInteger() ) {
        _core.Print( LOG_INFO, "Server is not active!\n" );
        return;
    }

    int i;
    const char *state;

    // Print server information first.
    _core.Print( LOG_INFO, "Our server name is '%s'\n", Server_Name.GetString() );
    _core.Print( LOG_INFO, "Our server address is '%s:%i'\n", network_ip.GetString(), network_port.GetInteger() );
    _core.Print( LOG_INFO, "We got %i clients online\n", ClientNum );
    _core.Print( LOG_NONE, "\n" );

    // Print player information now.

    for(i = 0; i < ClientNum; i++) {
        state = "idle";

        switch( Clients[i].state ) {
        case SVCL_ZOMBIE:
            state = "No response";
            break;
        case SVCL_CONNECTED:
            state = "Okay";
            break;
        case SVCL_CONNECTING:
            state = "Connecting";
            break;
        default:
            state = "Unknown";
            break;
        }

        int percent = (Clients[i].channel.sequenceOut - Clients[i].lastAcknowledgedMessage) * 10;
        _core.Print( LOG_NONE, "%i. %s - %s (%i)\n", Clients[i].clientnum, Clients[i].name, state, percent );
    }
}

/*
    Get client by number.
*/
ncServerClient *ncServer::GetClientByNum( int num ) {
    int i;

    for( i = 0; i < Server_Maxclients.GetInteger(); i++ ) {
        if(Clients[i].clientnum == num)
            return &Clients[i];
    }

    return NULL;
}

/*
    Get client by an ip address.
*/
ncServerClient *ncServer::GetClientByAddress( struct sockaddr_in data ) {
    int s;

    for( s = 0; s < ClientNum; s++ ) {
        if( !strcmp(Clients[s].address.IPAddress, inet_ntoa(data.sin_addr)) ) {
            if( Clients[s].address.Port == data.sin_port )
                return &Clients[s];
        }
    }

    return NULL;
}

/*
    Send Bye-bye message to all clients.
*/
void ncServer::SendByeMessage( const char *msg ) {
    _core.Print( LOG_INFO, "SendByeMessage: Implement me!\n" );
}

/*
    Kick client(s) with a reason.
*/
void ncServer::KickClient( void ) {
    
    if( !Server_Active.GetInteger() ) {
        _core.Print( LOG_INFO, "Server is not active.\n" );
        return;
    }

    if( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "USAGE for 'kick' command: \n" );
        
        _core.Print( LOG_INFO, " * Use 'kick all' to disconnect every client on server.\n" );
        _core.Print( LOG_INFO, " * Use 'kick <client id>' to disconnect selected client.\n" );
        _core.Print( LOG_INFO, "You can also use third parameter to set reason.\n" );
        return;
    }
    
    
}

/*
    Add ack command.
*/
void ncServer::AddAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, char *message ) {
    uint unAcked = cl->ackSequence - cl->ackAcknowledged;
    int ackCommands = sizeof(cl->ackCommands) / sizeof( cl->ackCommands[0] );
    
    /* We can't clear ack buffer for now, so throw error. */
    if( ((long)unAcked > (long)((long)ackCommands)) && !isDisconnect ) {
        DisconnectClient( cl, "Too much server commands sent." );
    }

    cl->ackSequence += 1;
    cl->ackCommands[cl->ackSequence & (ackCommands-1)] = message;
}

/*
    Send ack command to client(s).

    Use NULL as first parameter to send ack command to
    all clients.
*/

void ncServer::SendAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, const char *cmd, ... ) {

    va_list argptr;
    static char message[1024];

    va_start( argptr, cmd );
    vsnprintf( (char *)message, sizeof(message), cmd, argptr );
    va_end( argptr );

    if( !cl ) {
        int i;
        for( i = 0; i < ClientNum; i++ ) {
            if( &Clients[i] ) {
                AddAcknowledgeCommand( &Clients[i], isDisconnect, message );
            }
        }
    }
    else  {
        AddAcknowledgeCommand( cl, isDisconnect, message );
    }
}

/*
    Remove the bots.
*/
void ncServer::RemoveBots( void ) {
    if( !Server_Active.GetInteger() )
        return;

    _core.Print( LOG_INFO, "Removing all bots from the game.\n" );

    int x;

    for( x = ClientNum-1; x > 0; x-- ) {
        if(Clients[x].type == ADDR_BOT) {
            _core.Print( LOG_INFO, "%s kicked..\n", Clients[x].name );
            DisconnectClient(&Clients[x], "svr");
        }
    }
}

/*
    Close the server.
*/
void ncServer::Disconnect( void ) {
    if( !Server_Active.GetInteger() )
        return;

    // Clear the world and close all connections.
    RemoveBots();                                    // Remove all bots.
    SendByeMessage( "Server quit.\n" );              // Kick all clients.
    ClearWorld( "Server disconnect" );               // Clear the world.

    Server_Active.Set( "0" );
    
    // Disconnect local client. ( If exists ).
    _core.Disconnect();
}

/*
     Server shutdown.
     Called on application quit.
*/
void ncServer::Shutdown( const char *finalmsg ) {
    if( !Server_Active.GetInteger() ) {
        _core.Print( LOG_INFO, "No server running!\n" );
        return;
    }

    _core.Print( LOG_INFO, "Server shutting down...\n" );

    // Send last message to all clients and disconnect them.
    SendByeMessage(finalmsg);
    Disconnect();

    // We must forget all information about clients.
    delete [] Clients;

    _core.Print( LOG_INFO, "Server quit after %i minute(s) since last session launch.\n", ( Time / 60000 ) );

    Time    =   0;
    State   =   SERVER_IDLE;

    // Clear server data.
    zeromem( &_server, sizeof(_server) );
}

/*
     Add bot to server.
     Takes one client slot.
*/
void ncServer::AddBot( void ) {
    _core.Print( LOG_INFO, "Implement me!\n" );
}
