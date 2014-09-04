//
//  Nanocat engine
//
//  Multiplayer: Game server.
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "core.h"
#include "server.h"
#include "network.h"
#include "command.h"
#include "console.h"
#include "renderer.h"
#include "ncstring.h"
#include "clientgame.h"
#include "system.h"

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
ConsoleVariable  server_maxclients( "sv", "maxclients", "Maximum server clients.", "512", CVAR_NEEDSREFRESH );
// Server public name.
ConsoleVariable  server_hostname( "sv", "name", "Server name.", DEFAULT_SERVER_NAME, CVAR_NONE );
// Is server running?
ConsoleVariable  server_running( "sv", "running", "Is server running?", "0", CVAR_NONE );
// Cheats enabled?
ConsoleVariable  server_cheats( "sv", "cheats", "Server cheats enabled?", "1", CVAR_NEEDSREFRESH );
// Server framerate.
ConsoleVariable  server_fps( "sv", "fps", "Server framerate.", "10", CVAR_NONE );
// Name which is going to be used in chat.
ConsoleVariable  server_sayname( "sv", "sayname", "Server name to be used in chat.", "Friskies", CVAR_NEEDSREFRESH );
// Default bot name.
ConsoleVariable  server_botname( "sv", "botname", "Server bot name.", "Whiskas", CVAR_NONE );
// Is server dedicated?
ConsoleVariable  server_dedi( "sv", "dedicated", "Is server dedicated?", "0", CVAR_NONE );
// In-active client timeout.
ConsoleVariable  server_clienttimeout( "sv", "clienttimeout", "Server client timeout.", "30", CVAR_NONE );
// Status print per x seconds
ConsoleVariable  server_statusperiod( "sv", "statusprint", "Print server status per X seconds.", "30", CVAR_NONE );
// ConsoleVariable server_cutebots( "sv", "cutebots", "Smiling and nice bots.", "1", CVAR_CHEAT );

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

/*
    Initialize server.
    Load only on application start. Never calls again.
*/
void ncServer::Initialize( void ) {
    float   t1, t2;                   // Load time.

    t1 = _system.Milliseconds();
    _core.Print( LOG_INFO, "Server initializing	...\n" );

    if( Initialized ) {
        _core.Print( LOG_DEVELOPER, "Uh, someone tried to call me, but I have already initialized.\n" );
        return;
    }
    
    // Initial values.
    Time = 0;
    TimeResidual = 0;
    LastInfoPrintTime = 0;
    State = SERVER_IDLE;
    
    zeromem( Response, sizeof(response_t) );

    // Execute server file.
    _gconsole.Execute( "exec server.cfg" );

    // Check server console variables.
    CheckParams();

    MaxClients              = server_maxclients.GetInteger();
    Port                    = network_port.GetInteger();
    ClientNum               = 0;
    _stringhelper.Copy( Name, server_hostname.GetString() );

    _core.Print( LOG_INFO, "Initializing server client data...\n" );

    // Initialize client slots.
    SetupClients( server_maxclients.GetInteger() );

    // Let user know some stuff.
    _core.Print( LOG_INFO, "Our server name is '%s'\n", server_hostname.GetString() );
    _core.Print( LOG_INFO, "Max clients allowed: '%i'\n", server_maxclients.GetInteger() );
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
    _core.Print( LOG_DEVELOPER, "Server is idle now.\n" );
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
    _gconsole.Execute( "exec server.cfg" );

    _core.Print( LOG_INFO, "Server name: %s\n", server_hostname.GetString() );

    // Will be reseted to default value if needed.
    SetupClients( server_maxclients.GetInteger() );

    _core.Print( LOG_NONE, "\n");
    _core.Print( LOG_INFO, "Our server name is '%s'\n", server_hostname.GetString() );

    // Some user information.
    _core.Print( LOG_INFO, "Max clients allowed: '%i'\n", server_maxclients.GetInteger() );
    _core.Print( LOG_INFO, "Our network port is '%i'\n", network_port.GetInteger() );
    _core.Print( LOG_NONE, "\n");

    // Reset server session id, so we don't get fcked up.
    serverId = 0xffff ^ _core.Time;

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
        server_maxclients.Set( "512" );

        _core.Print( LOG_INFO, "Maximum clients value set to 512\n" );
    }
    else if(maxclients < 1) {
        _core.Print( LOG_WARN, "Could not set max clients value to 0, minimum one client.\n" );
        server_maxclients.Set( "24" );
        
        _core.Print( LOG_INFO, "Maximum clients value set to 24 clients.\n" );
    }

    // We got nice checking here.
    if(!Clients)
        Clients = (ncServerClient*)malloc( server_maxclients.GetInteger() * sizeof(ncServerClient) );

    if(!Clients) {
        _core.Error( ERC_FATAL, "Could not allocate memory for %i clients.\n", server_maxclients.GetInteger() );
        return;
    }

    for( cl = 0; cl < server_maxclients.GetInteger(); cl ++ ) {
        if( !&Clients[cl] )
            _core.Error( ERC_SERVER, "Could not load client slots. No enough memory." );

        // Set initial values.
        memset( &Clients[cl], 0, sizeof(ncServerClient) );
    }

    _core.Print( LOG_DEVELOPER, "%i client slots loaded.\n", server_maxclients.GetInteger() );
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
void ncServer::CreateClient( netdata_t *from, int response, const char *name, const char *version ) {
    int     x, i;
    int		ping;

    for ( i = 0; i < MAX_RESPONSES; i++ ) {
        /* Compare addresses. */
        if ( _netmanager.CompareAddress(from, &Response[i].adr) ) {
            /* If number is right, continue! */
            if ( response == Response[i].response ) {
                break;
            }
        }
    }

    // This may happen.
    if ( i == MAX_RESPONSES ) {
        _netmanager.PrintOutOfBand( from, "print SERVER_BAD_RESPONSE" );
        return;
    }

    ping = (Time - Response[i].pingTime);
    
    _core.Print( LOG_DEVELOPER, "Client is connecting...\n" );
    Response[i].connected = true;

    // TODO: remove for loop.
    for( x = 0; x < server_maxclients.GetInteger(); x++ ) {
        ncServerClient *p_temp = (ncServerClient*)malloc( sizeof(ncServerClient) );

        /* This should never happen. */
        if( !p_temp ) {
            _core.Error( ERC_SERVER, "Could not create a new client. Out of memory.\n" );
            return;
        }

        _stringhelper.Copy(p_temp->version, version);
        _stringhelper.Copy(p_temp->name, name);

        p_temp->response                                = response;
        p_temp->clientnum                               = ClientNum;
        p_temp->address                                 = *from;

        _stringhelper.Copy( p_temp->address.ip, inet_ntoa(from->sockaddress.sin_addr) );
        
        p_temp->address.port                            = from->sockaddress.sin_port;
        p_temp->address.sockaddress                     = from->sockaddress;
        p_temp->state                                   = SVCL_CONNECTED;

        if( _netmanager.IsLanAddress( &p_temp->address ) && (network_port.GetInteger() == ntohs(p_temp->address.port))  ) {
            p_temp->type                                = ADDR_LOOPBACK;
            _core.Print( LOG_DEVELOPER, "Client has local address.\n" );
        }
        else
            p_temp->type                                = ADDR_IP;

        p_temp->lastCommandTime                         = Time;
        p_temp->lastMessageTime                         = Time;
        p_temp->lastConnectTime                         = Time;
        p_temp->lastMessageReceivedAt                   = Time;

        // Set initial client entity variables.
        p_temp->player.g_vEye = ncVec3( 5.0, 5.0, 5.0 );
        p_temp->player.g_vLook = ncVec3( -10.5, -0.5, -0.5 );
        p_temp->player.g_vRight= ncVec3( 1.0, 0.0, 0.0 );
        p_temp->player.g_vUp = ncVec3( 1.0, 1.0, 0.0 );

        // Create network channel.
        _netmanager.CreateChannel( &p_temp->channel, &p_temp->address );

        if(Clients[x].state != SVCL_CONNECTED) {
            Clients[x] = *p_temp;

            free(p_temp);
            p_temp = NULL;

            break;
        }
    }

    _netmanager.PrintOutOfBand( from, "connectResponse %i", ClientNum );
    _core.Print( LOG_INFO, "%s has joined the adventure.\n", name );

    ++ClientNum;
}

/*
    Server connectionless packets.
*/
void ncServer::Connectionless( netdata_t *from, byte *data ) {
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
void ncServer::ParseClients( netdata_t *from, ncBitMessage *packet ) {

    if( !server_running.GetInteger() )
        return;

    ncServerClient *p_temp;
    int command = 0;

    p_temp = GetClientByAddress( from->sockaddress );

    if( !p_temp )
        return;

    if( p_temp->state != SVCL_CONNECTED )
        return;

    packet->BeginReading();

    Clients[p_temp->clientnum].lastMessageReceivedAt = Time;
    Clients[p_temp->clientnum].lastAcknowledgedMessage = packet->ReadInt32();
    Clients[p_temp->clientnum].ackAcknowledged = packet->ReadInt32();

    while(1) {
        command = packet->ReadByte();
        
        // Do not remove.
        if( ( command == COMMANDHEADER_MOVE || command == COMMANDHEADER_ACK ) ) {

            switch(command) {
                case COMMANDHEADER_MOVE:
                {
                    int delta = packet->ReadInt32();
                    
                    // g_vLook
                    float g_vLx = packet->ReadFloat();
                    float g_vLy = packet->ReadFloat();
                    float g_vLz = packet->ReadFloat();

                    Clients[p_temp->clientnum].player.g_vLook.x = g_vLx;
                    Clients[p_temp->clientnum].player.g_vLook.y = g_vLy;
                    Clients[p_temp->clientnum].player.g_vLook.z = g_vLz;

                    Clients[p_temp->clientnum].player.g_vLook.Normalize();

                    Clients[p_temp->clientnum].player.g_vRight.Cross(Clients[p_temp->clientnum].player.g_vLook, Clients[p_temp->clientnum].player.g_vUp);
                    Clients[p_temp->clientnum].player.g_vRight.Normalize();

                    Clients[p_temp->clientnum].player.g_vUp.Cross(Clients[p_temp->clientnum].player.g_vRight, Clients[p_temp->clientnum].player.g_vLook);
                    Clients[p_temp->clientnum].player.g_vUp.Normalize();

                    ncVec3 tmpLook  = Clients[p_temp->clientnum].player.g_vLook;
                    ncVec3 tmpRight = Clients[p_temp->clientnum].player.g_vRight;

                    switch(delta) {
                        case 0: // Nothing, standing still.

                        break;

                        case 1: // Forward.
                        {
                            ncVec3 s = tmpLook * -3.0;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye - s;
                        }
                            break;
                            
                        case 2: // Backward.
                        {
                            ncVec3 s = tmpLook * -3.0;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye + s;
                        }
                            break;
                            
                        case 3:  // Left.
                        {
                            ncVec3 s = tmpRight * 3.0;
                            Clients[p_temp->clientnum].player.g_vEye = Clients[p_temp->clientnum].player.g_vEye - s;
                        }
                            break;
                            
                        case 4: // Right.
                        {
                            ncVec3 s = tmpRight * 3.0;
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
                break;
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
    int         clientCommands, i, j;

    ncBitMessage *msg = new ncBitMessage( MAX_SNAPSHOT_SIZE );
    msg->WriteInt32( cl->channel.sequenceOut );
    
    /* Get client command count. */
    clientCommands = sizeof(cl->ackCommands) / sizeof(cl->ackCommands[0]);

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
    for( j = 0; j < ClientNum; j++ ) {
        if( Clients[j].state != SVCL_CONNECTED )
            continue;
        
        msg->WriteInt32( j );
        
        msg->WriteCoord( Clients[j].player.g_vEye.x );
        msg->WriteCoord( Clients[j].player.g_vEye.y );
        msg->WriteCoord( Clients[j].player.g_vEye.z );
        
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

        SendFrame( &Clients[i] );
    }
}


/*
    Listen for incoming data.
*/
void ncServer::Process( netdata_t *from, byte *buffer ) {
    if( !server_running.GetInteger() )
        return;

    // Should never happen. But I am still afraid.
    if( strlen((const char*)buffer) > MAX_SERVER_COMMAND ) {
        _core.Print( LOG_DEVELOPER, "Too much data from address (%i)\n", MAX_SERVER_COMMAND );
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
*/
void ncServer::CheckParams( void ) {
    if( strlen( server_hostname.GetString() ) > 64 ) {
        _core.Print( LOG_WARN, "Current server name is too long. Resetting to the default. Sorry.\n" );
        server_hostname.Set( DEFAULT_SERVER_NAME );
    }
}

/*
    Clear the world.
*/
void ncServer::ClearWorld( const char *msg ) {
    if( !server_running.GetInteger() ) {
        _core.Print( LOG_WARN, "server_clearworld: I've tried to clear the world, but there's no server running!\n" );
        return;
    }

    _renderer.RemoveWorld("server_clearworld");
}

/*
    Print server information.
*/
void ncServer::PrintInfo( void ) {
    if( !server_running.GetInteger() )
        return;

    if( Time - LastInfoPrintTime > (server_statusperiod.GetInteger() * 1000) ) {
        LastInfoPrintTime = Time;

        _core.Print( LOG_INFO, "\n" );
        _core.Print( LOG_INFO, "------------------ Server status ---------------------\n" );
        _core.Print( LOG_INFO, "Server: \"%s\". We got %i users online.\n", server_hostname.GetString(), ClientNum );
        _core.Print( LOG_INFO, "We are alive for %i minute(s).\n", (Time / 60000) );
    }
}

/*
    Check client timeouts.
*/
void ncServer::CheckTimeouts( void ) {
    if( !server_running.GetInteger() )
        return;

    int i;
    for( i = 0; i < ClientNum; i++ ) {
        if( (Clients[i].state == SVCL_ZOMBIE || Clients[i].state == SVCL_FREE || Clients[i].type == ADDR_BOT ) )
            continue;

        if( !&Clients[i] )
            return;

        if (Clients[i].lastMessageTime > Time) {
             Clients[i].lastMessageTime = Time;
         }

        if( Time - Clients[i].lastMessageReceivedAt > server_clienttimeout.GetInteger() * 10000 ) {
            _core.Print( LOG_INFO, "%s timed out.\n", Clients[i].name );
            DisconnectClient( &Clients[i], "Connection timed out." );
        }
    }
}

/*
     Server frame function.
*/
void ncServer::Frame( int msec ) {
    int     frameMsec;

    if( !server_running.GetInteger() )
        return;

    if( server_fps.GetInteger() < 1 )
        server_fps.Set( "10" );

    /*
        Time wrap.
        24 hours reset.
    */
    if( Time > 0x86400000 ) {
        _core.Print( LOG_INFO, "Long time passed since server load. Server restarting.\n" );
        _gconsole.Execute("map_restart");
    }

    frameMsec = 1000 / server_fps.GetInteger();
    TimeResidual += msec;

    while( TimeResidual >= frameMsec ) {
        TimeResidual -= frameMsec;
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
        _core.Print( LOG_INFO, "usage: map <mapname>\n" );
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
    if( !server_dedi.GetInteger() ) {
            if( _clientgame.Loadmap( _commandManager.Arguments(0) )) {
            CreateSession();

            // Launch server network.
            //cvar_set("server_running", "1", true);
            //console_exec( "connect 127.0.0.1 4001" );
        }
        else {
            State   = SERVER_IDLE;
            Time    = 0;

            return;
        }
    } else { // override
        server_running.Set( "1" );
        mapname.Set(  _commandManager.Arguments(0) );
        
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
    server_running.Set( "1" );
    mapname.Set( "world " );
    

    CreateSession();
    State = SERVER_GAME;

    _core.Print( LOG_INFO, "World has been created and server spawned!\n" );
    _core.Print( LOG_NONE, "\n" );
}

/*
    Drop client.
*/
void ncServer::DisconnectClient( ncServerClient *client, const char *message ) {
    if( client->state == SVCL_ZOMBIE )
        return;

    SendAcknowledgeCommand( client, true, "disconnect \"%s\"", message );
    SendFrame( client );

    _core.Print( LOG_INFO, "%s disconnected. ( \"%s\" )\n", client->name, message );

    SendAcknowledgeCommand( NULL, "print \"%s disconnected (%s)\"", client->name, message );
    SendFrame( client );

    client->zombifiedAt = Time;
    client->state = SVCL_ZOMBIE;
}

/*
    Get response from client.
*/

void ncServer::GetResponse( netdata_t *from ) {
	int             i;
	int             oldest;
	int             oldestTime;
    
	response_t     *response;

	oldest = 0;
    
    /* Get nice value here. */
	oldestTime = 0x7fffffff;

	// Check if response already exists for this client.
	response = &Response[0];
	for ( i = 0 ; i < MAX_RESPONSES; i++, response++ ) {
		if ( !response->connected && _netmanager.CompareAddress( from, &response->adr ) ) {
			break;
		}
		if ( response->time < oldestTime ) {
			oldestTime = response->time;
			oldest = i;
		}
	}

	if ( i == MAX_RESPONSES ) {
		// Just connected.
		response = &Response[oldest];
        
        /* Generate nice random number for this response. */
		response->response = ( (rand() << 16) ^ rand() ) ^ Time;
		response->adr = *from;
		response->firstTime = Time;
		response->time = Time;
		response->connected = false;
		i = oldest;
	}

	if ( Time - response->firstTime > SERVER_CLIENTAUTHORIZE_TIME ) {
		_core.Print( LOG_INFO, "Authorizing client...\n", response->response );
        // TODO: authorizing
        // We simply connect client now without any check (yet).
        
		response->pingTime = Time;
		_netmanager.PrintOutOfBand( &response->adr, "requestResponse %i", response->response );
		return;
	}
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
    if( !server_running.GetInteger() ) {
        _core.Print( LOG_INFO, "No server running!\n" );
        return;
    }

    int i;
    const char *state;

    // Print server information first.
    _core.Print( LOG_INFO, "Our server name is '%s'\n", server_hostname.GetString() );
    _core.Print( LOG_INFO, "Our server address is '%s:%i'\n", network_ip.GetString(), network_port.GetInteger() );
    _core.Print( LOG_INFO, "We got %i clients online\n", ClientNum );
    _core.Print( LOG_NONE, "\n" );

    // Print player information now.

    _core.Print(LOG_NONE, "state num laty address         name\n");
    _core.Print(LOG_NONE, "----- --- ---- ----------------------------------\n");

    for(i = 0; i < ClientNum; i++) {
        state = "IDLE";

        switch( Clients[i].state ) {
        case SVCL_ZOMBIE:
            state = "ZMBI";
            break;
        case SVCL_CONNECTED:
            state = "OKAY";
            break;
        case SVCL_CONNECTING:
            state = "WAIT";
            break;
        default:
            state = "UNKN";
            break;
        }

        int percent = (Clients[i].channel.sequenceOut - Clients[i].lastAcknowledgedMessage) * 10;
        _core.Print(LOG_NONE, "%s   %i. %i   %s:%i -   %s\n", state, Clients[i].clientnum, percent, Clients[i].address.ip, ntohs(Clients[i].address.port), Clients[i].name );
    }
}

/*
    Get client by number.
*/
ncServerClient *ncServer::GetClientByNum( int num ) {
    int i;

    for( i = 0; i < server_maxclients.GetInteger(); i++ ) {
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
        if( !strcmp(Clients[s].address.ip, inet_ntoa(data.sin_addr)) ) {
            if(Clients[s].address.port == data.sin_port )
                return &Clients[s];
        }
    }

    return NULL;
}

/*
    Send Bye-bye message to all clients.
*/
void ncServer::SendFinalMessage( const char *msg ) {

}

/*
    Kick client(s) with a reason.
    Not done.
*/
void ncServer::KickClient( void ) {
    if( !server_running.GetInteger() ) {
        _core.Print( LOG_INFO, "No server running!\n" );
        return;
    }
    int x;

    if( _commandManager.ArgCount() < 1 ) {
        _core.Print(LOG_INFO, "usage: \n");
        _core.Print(LOG_INFO, " - kick all = kick everyone\n");
        _core.Print(LOG_INFO, " - kick bots = kick all bots\n");
        _core.Print(LOG_INFO, " - kick <client id> = kick chosen player\n");
        return;
    }

    // kick bots - kick bots only
    // kick all  - kick all clients ( including bots )
    // kick <num> - kick chosen client

    if(atoi(_commandManager.Arguments(0)) > (ClientNum - 1))                                   // -1, since one slot is being taken by local client
    {
        _core.Print(LOG_INFO, "Client with number '%i' does not exist\n", atoi(_commandManager.Arguments(0)));
        return;
    }

    if(!strcmp(_commandManager.Arguments(0), "all")) {
        for( x = ClientNum - 1; x > 0; x-- ) {
            if(Clients[x].type != ADDR_LOOPBACK) {
                _core.Print(LOG_INFO, "%s kicked..\n", Clients[x].name);
                DisconnectClient(&Clients[x], _commandManager.Arguments(1));
            }
        }
        return;
    }

    // Bots.
    if(!strcmp(_commandManager.Arguments(0), "bots"))
    {
        for( x = ClientNum-1; x > 0; x-- )
        {
            if(Clients[x].type == ADDR_BOT)
            {
                _core.Print(LOG_INFO, "%s kicked..\n", Clients[x].name);
                DisconnectClient(&Clients[x], _commandManager.Arguments (1));
            }
        }
        return;
    }
}

/*
    Add ack command.
*/
void ncServer::AddAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, char *message ) {
    uint unAcked = cl->ackSequence - cl->ackAcknowledged;
    int ackCommands = sizeof(cl->ackCommands) / sizeof(cl->ackCommands[0]);
    
    /* We can't clear ack buffer for now, so throw error. */
    if( ((long)unAcked > (long)((long)ackCommands)) && !isDisconnect ) {
        DisconnectClient( cl, "Server command overflow" );
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

    va_list             argptr;
    static char         message[1024];

    va_start(argptr, cmd);
    vsnprintf((char *)message, sizeof(message), cmd, argptr);
    va_end(argptr);

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
    if( !server_running.GetInteger() )
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
    if( !server_running.GetInteger() )
        return;

    // Clear the world and close all connections.
    RemoveBots();                                    // Remove all bots.
    SendFinalMessage( "Server quit.\n" );                // Kick all clients.
    ClearWorld( "Server disconnect" );               // Clear the world.

    server_running.Set( "0" );
    
    // Disconnect local client. ( If exists ).
    _core.Disconnect();//client_disconnect_f( "server disconnect", true );
}

/*
     Server shutdown.
     Called on application quit.
*/
void ncServer::Shutdown( const char *finalmsg ) {
    if( !server_running.GetInteger() ) {
        _core.Print( LOG_INFO, "No server running!\n" );
        return;
    }

    _core.Print( LOG_INFO, "Server shutting down...\n" );

    // Send last message to all clients and disconnect them.
    SendFinalMessage(finalmsg);
    Disconnect();

    free(Clients);
    
    // We must forget all information about clients.
    Clients = NULL;

    _core.Print( LOG_INFO, "Server quit after %i minute(s) since last launch.\n", ( Time / 60000 ) );

    Time    =   0;
    State   =   SERVER_IDLE;

    // Clear server struct.
    memset( &_server, 0, sizeof(_server) );
}

/*
     Add bot to server.
     Takes one client slot.
*/
void ncServer::AddBot( void ) {
    ncServerClient  *b_temp;
    int             x;

    if( !server_running.GetInteger() ) {
        _core.Print( LOG_INFO, "No server running!\n" );
        return;
    }

    if( ClientNum >= server_maxclients.GetInteger() ) {
        _core.Print( LOG_INFO, "Could not add bot. All slots are in use, type 'kick bots' to remove bots.\n" );
        return;
    }

    // We have to check all slots.
    for( x = 0; x < server_maxclients.GetInteger(); x++ )   {
        b_temp                                          = (ncServerClient*)malloc(sizeof(ncServerClient));

        // Keep defaults.
        _stringhelper.Copy(b_temp->version, _version);
        _stringhelper.Copy(b_temp->name, _stringhelper.STR("%s%i", server_botname.GetString(), x));

        b_temp->clientnum                               = x;
        
        // Since bots are local clients.
        _stringhelper.Copy(b_temp->address.ip,                     network_ip.GetString());
        b_temp->address.port                            = network_port.GetInteger();

        //b_temp->address.sockaddress                     = _client.cserver.sockaddress;
        b_temp->type                                    = ADDR_BOT;
        // ...and it always will be connected
        b_temp->state                                   = SVCL_CONNECTED;

        b_temp->lastCommandTime                         = 0;
        b_temp->lastMessageTime                         = 0;
        b_temp->lastConnectTime                         = Time;

        if(Clients[x].state != SVCL_CONNECTED) {
            
            if( !b_temp ) {
                // I don't know why I have wrote it.
                _core.Error( ERC_SERVER, "Fatal server error.\n" );
                return;
            }
            
            Clients[x] = *b_temp;

            free(b_temp);
            b_temp = NULL;

            ClientNum++;

            _core.Print( LOG_INFO, "%s has joined.\n", Clients[x].name );
            break;
        }
    }
}
