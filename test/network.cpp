//
//  Nanocat engine.
//
//  Game network manager..
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//


#include "Network.h"
#include "MultiplayerServer.h"
#include "MultiplayerClient.h"

#include "NCString.h"

ncConsoleVariable    Network_Port( "net", "port", "Network port", "4004", CVFLAG_NEEDSREFRESH );
ncConsoleVariable    Network_IPAddress( "net", "ip", "Network IP address.", "0.0.0.0", CVFLAG_NEEDSREFRESH );
ncConsoleVariable    Network_Active( "net", "active", "Is network active?", "0", CVFLAG_NEEDSREFRESH );
ncConsoleVariable    Network_AddressType( "net", "addrtype", "Address type.", "0", CVFLAG_NEEDSREFRESH );
ncConsoleVariable    Network_LocalIPAddress( "net", "localip", "Local IP address.", "0.0.0.0", CVFLAG_NEEDSREFRESH );
ncConsoleVariable    Network_NotAvailable( "net", "nonet", "Is internet connection available?", "0", CVFLAG_NEEDSREFRESH );

ncNetwork local_netmanager;
ncNetwork *g_networkManager = &local_netmanager;

// Last received message.
byte network_message_buffer[MAX_UDP_PACKET];

/*
    Initialize network.
*/

void ncNetwork::Initialize( void ) {

    int port, i;
    int flags;
    
    bool bound, g_allow;
    
    struct hostent *hp;
    static char n_hostname[MAX_NET_HOSTNAME_LENGTH];

    g_Core->LoadState = NCCLOAD_NETWORK;
    g_Core->Print( LOG_INFO, "Initializing network..\n" );

    i = 0;
    bound = false;

#ifdef _WIN32
    // We have to initialize socket on Windows.
    int  w;
    if( WSAStartup( WINSOCKET_STARTUP_CODE, &w ) != 0 ) {
        g_Core->Error( ERC_NETWORK, "Failed to initialize Windows socket. Initialization code: %s.\n", WINSOCKET_STARTUP_CODE );
        return;
    }
#endif

    g_Core->Print( LOG_INFO, "Creating network socket using udp protocol..\n" );
    if( ( NetworkSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP )) == - 1 ) {
        g_Core->Print( LOG_ERROR, "%s\n", strerror(errno) );
        g_Core->Error( ERR_NETWORK, "Could not initialize network socket. %s", strerror(errno) );
        return;
    }
    
    // Get local data.
    if( gethostname( n_hostname, sizeof(n_hostname) ) == -1 ) {
        g_Core->Error( ERR_NETWORK, "ncNetwork::Init - Could not get host name.\n" );
        return;
    }
    
    const NString n_localIP;
    
#ifndef NETWORK_IGNORE_GETHOSTBYNAME
    struct hostent *host = gethostbyname( n_hostname );

    if( !host ) {
        g_Core->Error( ERC_FATAL, "ncNetwork::Init - Couldn't get host by name. %s\n", strerror( errno ) );
        return;
    }
    n_localIP = inet_ntoa( *(struct in_addr *)*host->h_addr_list );
#else 
    n_localIP = "null";
#endif
    
    
    g_Core->Print( LOG_INFO, "Our local host name is '%s'\n", n_hostname );
    g_Core->Print( LOG_INFO, "Our local address is %s\n", n_localIP );
    
    Network_LocalIPAddress.Set( n_localIP );
    
    memset( &DataAddr, 0, sizeof(struct sockaddr_in) );

    port = Network_Port.GetInteger();

    // Prevent the network socket blocking.
#ifdef _WIN32
    
    long blockmode;
    blockmode = 1;
    
    if( ioctlsocket( _network.socket, FIONBIO, &blockmode ) == -1 ) {
        g_Core->Error( ERC_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
        return;
    }
    
#else // Mac, and another *nix systems.
    
    flags = fcntl( NetworkSocket, F_GETFL );
    flags |= O_NONBLOCK;
    fcntl( NetworkSocket, F_SETFL, flags );

    if( ioctl( NetworkSocket, FIONBIO, &g_allow ) == -1 ) {
        g_Core->Error( ERR_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
		return;
	}
    
#endif

	if( setsockopt( NetworkSocket, SOL_SOCKET, SO_BROADCAST, (NString )&i, sizeof(i) ) == -1 ) {
        g_Core->Error( ERR_NETWORK, "Failed to setup socket broadcasting.\n" );
		return;
	}

    // Scan for available ports.
    g_Core->Print( LOG_INFO, "Binding socket with parameters..\n" );
    
    while( true ) {
        memset( &DataAddr, 0, sizeof(struct sockaddr_in) );
        
        DataAddr.sin_family = AF_INET;
        DataAddr.sin_port = htons( port );
        
        switch( Network_AddressType.GetInteger() ) {
            case 0: // Any.
                g_Core->Print( LOG_INFO, "Our network type is any.\n" );
                DataAddr.sin_addr.s_addr = INADDR_ANY;
                break;
            case 1: // Set automatically.
                g_Core->Print( LOG_INFO, "Our network type is automatic.\n" );
                
                hp = gethostbyname(n_hostname);
                if( !hp ) {
                    g_Core->Error( ERR_NETWORK, "Couldn't resolve %s", n_hostname );
                }
                
                // Copy address data.
                memcpy( (void*)&DataAddr.sin_addr, hp->h_addr_list[0], hp->h_length );
                break;
            case 2: // Manual
                g_Core->Print( LOG_INFO, "Our network type is manual. Name: %s\n", Network_IPAddress.GetString() );

                break;
        }
        
        if(bind(NetworkSocket, (struct sockaddr *)&DataAddr, sizeof(DataAddr)) != 0) {
            g_Core->Print( LOG_WARN, "Bind failed, trying to change the port..\n" );
            
            port++;
            
            Network_Port.Set( NC_TEXT("%i", port) );
            continue;
        }
        
        break;
    }

    Network_IPAddress.Set( NC_TEXT( "%s", inet_ntoa(DataAddr.sin_addr) ) );
    Network_Port.Set( NC_TEXT("%i", port) );

    if( Network_AddressType.GetInteger() == 1 )
        g_Core->Print( LOG_INFO, "Listening on '%s:%i'. \n", inet_ntoa(DataAddr.sin_addr), port );
    else
        g_Core->Print( LOG_INFO, "Listening on '%i' port. \n", port );


    // Turn on the networking.
    Network_Active.Set( "1" );
}


/*
     Parse all received data.
*/

bool ncNetwork::Frame( void ) {
    
    if( !Network_Active.GetInteger() )
        return false;

    if( !NetworkSocket ) {
        g_Core->Error( ERR_NETWORK, "ncNetwork::Frame - missing socket." );
        return false;
    }
    
    int bytes;
    int size;

    byte data[MAX_UDP_PACKET + 4];

    ncNetdata remoteEP;
    socklen_t packet_len;

    packet_len = sizeof(RecvAddr);
    size = sizeof(data);
    bytes = recvfrom( NetworkSocket, &data, size, 0, (struct sockaddr *)&RecvAddr, (socklen_t*)&packet_len );
    
    if ( bytes == -1 ) {
        
        if( errno == EWOULDBLOCK || errno == ECONNREFUSED )
            return false;
   
        g_Core->Print( LOG_WARN, "ncNetwork::recvfrom() - %s\n", strerror( errno ) );
        
        return false;
    }
    
    assert( bytes < size );

    if( bytes <= 4 )
        return false;

    if( bytes >= MAX_UDP_PACKET ) {
        g_Core->Print( LOG_WARN, "Too much data from %s\n", inet_ntoa( RecvAddr.sockaddr_in::sin_addr ) );
        return false;
    }

    // Protocol.
    if ( data[0] != (Byte)( PROTOCOL_ID >> 24 ) ||
        data[1] != (Byte)( ( PROTOCOL_ID >> 16 ) & 0xFF ) ||
        data[2] != (Byte)( ( PROTOCOL_ID >> 8 ) & 0xFF ) ||
        data[3] != (Byte)( PROTOCOL_ID & 0xFF ) ) {
        
        g_Core->Print( LOG_WARN, "User from %s has wrong network protocolId.\n", inet_ntoa( RecvAddr.sockaddr_in::sin_addr) );
        return false;
    }
    
    // Copy network message for further parsing and
    // cut first four bytes.
    memcpy( network_message_buffer, &data[4], bytes - 4 );
    remoteEP.SockAddress = RecvAddr;

    network_message_buffer[bytes] = 0;
    
    // Assign data packet.
    Assign( &remoteEP, network_message_buffer );       // Server/client process.

    network_message_buffer[bytes] = 0;
    bytes = 0;

    return true;
}

/*
    Network channel
*/
void ncNetwork::CreateChannel( ncNetchannel *chan, ncNetdata *adr ) {
    
    chan->address.Port = adr->Port;
    chan->address.Socket = NetworkSocket;
    
    memcpy( &chan->address.SockAddress, &adr->SockAddress, sizeof(adr->SockAddress) );
}

/*
    Send message to chosen channel.
*/
void ncNetwork::SendMessageChannel( ncNetchannel *chan, ncBitMessage *msg ) {
    SendPacket( msg->Size, msg->Data, &chan->address );
    chan->sequenceOut += 1;
}

bool ncNetwork::ProcessChannel( ncNetchannel *chan, byte *packet ) {
    return true;
}

/*
    Shutdown networking.
*/
void ncNetwork::Shutdown( void ) {
    if( !Network_Active.GetInteger() )
        return;

    // 2 - Disable receiving/sending
    shutdown( NetworkSocket, 2 );
    close( NetworkSocket );

#ifdef _WIN32
    // Windows needs sockets to be closed.
    WSACleanup();
#endif

    g_Core->Print( LOG_INFO, "Successfully closed the network socket.\n" );
}


/*
    Network manager.
*/
void ncNetwork::Assign( ncNetdata *from, byte *buffer ) {

    if( Server_Active.GetInteger() )
        n_server->Process( from, buffer );

    if( Client_Running.GetInteger() )
        n_client->Process( from, buffer );
}

/*
    Send packet.
*/
void ncNetwork::SendPacket( unsigned long len, const void *data, ncNetdata *from ) {
    int ret;
    byte packet[len + 4];

    // First four bytes - protocol.
    packet[0] = (byte)( PROTOCOL_ID >> 24 );
    packet[1] = (byte)( ( PROTOCOL_ID >> 16 ) & 0xFF );
    packet[2] = (byte)( ( PROTOCOL_ID >> 8 ) & 0xFF );
    packet[3] = (byte)( ( PROTOCOL_ID ) & 0xFF );
    
    memcpy( &packet[4], data, len );

    ret = sendto( NetworkSocket, packet, len + 4, 0, (struct sockaddr *)&from->SockAddress, sizeof(from->SockAddress) );
    
    if( ret == -1 )
        g_Core->Print( LOG_ERROR, "Could not send packet to %s\n", inet_ntoa(from->SockAddress.sin_addr) );
}

/*
    Print Out Of band message.
*/
void ncNetwork::PrintOutOfBand( ncNetdata *adr, const NString format, ... ) {
	va_list		argptr;
	char		string[1024];

	// Set the header. ( so the client/server sees that ).
	string[0] = -1;
	string[1] = -1;
	string[2] = -1;
	string[3] = -1;

	va_start( argptr, format );
	vsprintf( string+4, format, argptr );
	va_end( argptr );

	// Send the data.
	SendPacket( strlen( string ), string, adr );
}

/*
    Send out of band.
*/
void ncNetwork::PrintOutOfBandData( ncNetdata *adr, Byte *format, int len ) {
    Byte		string[MAX_SERVER_COMMAND * 2];
    int			i;
    ncBitMessage   msg;

    // Set the header.
    string[0] = 0xff;
    string[1] = 0xff;
    string[2] = 0xff;
    string[3] = 0xff;

    for( i = 0; i < len; i++ ) {
        string[i + 4] = format[i];
    }

    msg.Data = string;
    msg.Size = len + 4;

    // Send the data.
    SendPacket( msg.Size, msg.Data, adr );
}

/*
    Compare socket addresses.
*/
bool ncNetwork::CompareAddress( ncNetdata *t1, ncNetdata *t2 ) {
#ifdef _WIN32
    return t1->SockAddress.sin_addr.S_un.S_addr == t2->SockAddress.sin_addr.S_un.S_addr;
#else
    return t1->SockAddress.sin_addr.s_addr == t2->SockAddress.sin_addr.s_addr;
#endif
}

/*
    Check if address is on local machine.
*/
bool ncNetwork::IsLanAddress( ncNetdata *adr ) {
    //
    // This is VERY ugly - FIXME
    //
    if(adr->IPAddress[0] == '1' &&
       adr->IPAddress[1] == '2' &&
       adr->IPAddress[2] == '7' &&
       adr->IPAddress[3] == '.' &&
       adr->IPAddress[4] == '0' &&
       adr->IPAddress[5] == '.' &&
       adr->IPAddress[6] == '0' &&
       adr->IPAddress[7] == '.')
    {
        return true;
    }

    return false;
}

/*
    Resolve address.
*/
bool ncNetwork::Resolve( ncNetdata *address, const NString host ) {
    g_Core->Print( LOG_INFO, "ncNetwork::Resolve - %s\n", host );
    
    struct hostent *hn;
    hn = gethostbyname( host );
    
    if( !hn ) {
        switch( h_errno ) {
            // These are default.
            case HOST_NOT_FOUND:
                g_Core->Print( LOG_INFO, "ncNetwork::Resolve - couldn't find host.\n" );
                return false;
            case NO_ADDRESS:
                g_Core->Print( LOG_INFO, "ncNetwork::Resolve - host '%s' has no addresses.\n", host );
                return false;
            case NO_RECOVERY:
                g_Core->Print( LOG_INFO, "ncNetwork::Resolve - non-recoverable name server error.\n" );
                return false;
            case TRY_AGAIN:
                g_Core->Print( LOG_INFO, "ncNetwork::Resolve - host '%s' is temporarily unavailable.\n", host );
                return false;
        }
    }
    else
    {
        g_Core->Print( LOG_INFO, "ncNetwork::Resolve - Resolved as '%s'\n", inet_ntoa (*((struct in_addr *) hn->h_addr_list[0])) );
        memcpy( (void *)&address->SockAddress.sin_addr, hn->h_addr_list[0], hn->h_length );
        g_stringHelper->Copy( address->IPAddress, inet_ntoa ( *((struct in_addr *) hn->h_addr_list[0]) ) );
        //address->port = -1; // Not assigned yet.
        
        return true;
    }

    // We'll never get here, so
    return true;
}

int ncNetwork::GetSocket() {
    if( !NetworkSocket )
        return -1;
    
    return NetworkSocket;
}

struct sockaddr_in ncNetwork::GetSockAddr() {
    return DataAddr;
}

ncNetdata::ncNetdata( void ) {
    this->Port = -1;
    this->Socket = -1;
}

ncNetdata::ncNetdata( struct sockaddr_in _sockaddress, unsigned int port, unsigned int socket ) {
    this->SockAddress = _sockaddress;
    this->Port = port;
    this->Socket = socket;
}
