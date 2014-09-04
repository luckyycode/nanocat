//
//  Nanocat engine.
//
//  Game network manager.
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//


#include "network.h"
#include "server.h"
#include "client.h"
#include "ncstring.h"

ConsoleVariable    network_port("net", "port", "Network port", "4004", CVAR_NEEDSREFRESH);
ConsoleVariable    network_ip("net", "ip", "Network IP address.", "0.0.0.0", CVAR_NEEDSREFRESH);
ConsoleVariable    network_active("net", "active", "Is network active?", "0", CVAR_NEEDSREFRESH);
ConsoleVariable    network_addrtype("net", "addrtype", "Address type.", "0", CVAR_NEEDSREFRESH);
ConsoleVariable    network_localip("net", "localip", "Local IP address.", "0.0.0.0", CVAR_NEEDSREFRESH);

ncNetwork _netmanager;

byte network_message_buffer[MAX_UDP_PACKET];

/*
    Initialize network.
*/

void ncNetwork::Initialize( void ) {

    int                 port, i;
    int                 flags;
    int                 err;
    bool                bound, g_allow;
    struct hostent      *hp;
    static char         n_hostname[MAX_NET_HOSTNAME_LENGTH];

    _core.Print( LOG_INFO, "Initializing network..\n" );

    i       = 0;
    bound   = false;

#ifdef _WIN32
    // We have to initialize winsock on Windows.
    int  w;
    if( WSAStartup( WINSOCKET_STARTUP_CODE, &w ) != 0 ) {
        _core.Error( ERC_NETWORK, "Failed to initialize Windows socket. Initialization code: %s.\n", WINSOCKET_STARTUP_CODE );
        return;
    }
#endif

    _core.Print( LOG_INFO, "Creating network socket using udp protocol..\n" );
    if( (n_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) == - 1 ) {
        err = errno;
        _core.Error( ERC_NETWORK, "Could not initialize network socket.", err );
        return;
    }

    // Get local data.
    if( gethostname( n_hostname, sizeof(n_hostname) ) == -1 ) {
        _core.Error( ERC_NETWORK, "ncNetwork::Init - Could not get host name.\n" );
        return;
    }
    
    struct hostent *host = gethostbyname( n_hostname );

    char *n_localIP;
    n_localIP = inet_ntoa( *(struct in_addr *)*host->h_addr_list );
    
    _core.Print( LOG_INFO, "Our local host name is '%s'\n", n_hostname );
    _core.Print( LOG_INFO, "Our local address is %s\n", n_localIP );
    
    network_localip.Set( n_localIP );
    
    memset( &n_sv, 0, sizeof(struct sockaddr_in) );

    port = network_port.GetInteger();

    // Prevent the network socket blocking.
#ifdef _WIN32
    
    long blockmode;
    blockmode = 1;
    if( ioctlsocket( _network.socket, FIONBIO, &blockmode ) == -1 ) {
        _core.Error( ERC_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
        return;
    }
    
#else
    
    flags = fcntl(n_socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(n_socket, F_SETFL, flags);

    if( ioctl (n_socket, FIONBIO, &g_allow) == -1 ) {
        _core.Error( ERC_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
		return;
	}
    
#endif

	if( setsockopt( n_socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i) ) == -1 ) {
        _core.Error( ERC_NETWORK, "Failed to setup socket broadcasting.\n" );
		return;
	}

    // Scan for available ports.
    _core.Print( LOG_INFO, "Binding socket with parameters..\n" );
    while( true ) {
        memset( &n_sv, 0, sizeof(struct sockaddr_in) );
        
        n_sv.sin_family = AF_INET;
        n_sv.sin_port = htons(port);
        
        switch( network_addrtype.GetInteger() ) {
            case 0: // Any.
                _core.Print( LOG_INFO, "Our network type is any.\n" );
                n_sv.sin_addr.s_addr = INADDR_ANY;
                break;
            case 1: // Set automatically.
                _core.Print( LOG_INFO, "Our network type is automatic.\n" );
                
                hp = gethostbyname(n_hostname);
                if( !hp ) {
                    _core.Error( ERC_NETWORK, "Couldn't resolve %s", n_hostname );
                }
                
                // Copy address data.
                memcpy( (void *)&n_sv.sin_addr, hp->h_addr_list[0], hp->h_length );
                break;
            case 2: // Manual
                _core.Print( LOG_INFO, "Our network type is manual. Name: %s\n", network_ip.GetString() );
                //inet_pton4_alt(network_ip->string, &(_network.sv.sin_addr));
                break;
        }
        
        if(bind(n_socket, (struct sockaddr *)&n_sv, sizeof(n_sv)) != 0) {
            _core.Print( LOG_WARN, "Bind failed, trying to change the port..\n" );
            
            port++;
            
            network_port.Set( _stringhelper.STR("%i", port) );
            continue;
        }
        
        break;
    }

    network_ip.Set( _stringhelper.STR( "%s", inet_ntoa(n_sv.sin_addr) ) );
    network_port.Set( _stringhelper.STR("%i", port) );

    if( network_addrtype.GetInteger() == 1 )
        _core.Print( LOG_INFO, "Listening on '%s:%i'. \n", inet_ntoa(n_sv.sin_addr), port );
    else
        _core.Print( LOG_INFO, "Listening on '%i' port. \n", port );


    // Turn on the networking.
    network_active.Set( "1" );
}


/*
     Parse all received data.
*/

bool ncNetwork::Frame( void ) {
    
    if( !network_active.GetInteger() )
        return false;

    if( !n_socket ) {
        _core.Error( ERC_NETWORK, "Function network_getpackets could not find active socket." );
        return false;
    }
    
    int bytes;

    byte data[MAX_UDP_PACKET + 4]; // Four for protocol identificator.

    netdata_t remoteEP;
    socklen_t packet_len;

    packet_len = sizeof(n_cl);
    bytes = recvfrom( n_socket, &data, sizeof(data), 0, (struct sockaddr *)&n_cl, &packet_len );

    if( bytes <= 0 )
        return false;

    if ( bytes <= 4 )
        return false;

    if( bytes >= MAX_UDP_PACKET ) {
        _core.Print( LOG_WARN, "Too much data from %s\n", inet_ntoa( n_cl.sockaddr_in::sin_addr ) );
        return false;
    }

    if ( data[0] != (byte) ( PROTOCOL_ID >> 24 ) ||
        data[1] != (byte) ( ( PROTOCOL_ID >> 16 ) & 0xFF ) ||
        data[2] != (byte) ( ( PROTOCOL_ID >> 8 ) & 0xFF ) ||
        data[3] != (byte) ( PROTOCOL_ID & 0xFF ) ) {
        
        _core.Print( LOG_WARN, "User from %s has wrong network protocolId and tried to connect.\n", inet_ntoa( n_cl.sockaddr_in::sin_addr) );
        return false;
    }
    
    // Copy network message for further parsing and
    // cut first four bytes.
    memcpy( network_message_buffer, &data[4], bytes - 4 );
    remoteEP.sockaddress = n_cl;

    network_message_buffer[bytes] = 0;
    
    // Assign data packet.
    Assign( &remoteEP, network_message_buffer );       // server/client process

    network_message_buffer[bytes] = 0;
    bytes = 0;

    return true;
}

/*
    Network channel
*/
void ncNetwork::CreateChannel( netchannel_t *chan, netdata_t *adr ) {
    chan->address.port = adr->port;
    chan->address.sc = n_socket;
    
    memcpy(&chan->address.sockaddress, &adr->sockaddress, sizeof(adr->sockaddress));
}

/*
    Send message to chosen channel.
*/
void ncNetwork::SendMessageChannel( netchannel_t *chan, ncBitMessage *msg ) {
    SendPacket( msg->Size, msg->Data, &chan->address );
    chan->sequenceOut += 1;
}

bool ncNetwork::ProcessChannel( netchannel_t *chan, byte *packet ) {
    return true;
}

/*
    Shutdown networking.
*/
void ncNetwork::Shutdown( void ) {
    if( !network_active.GetInteger() )
        return;

    // 2 - Disable receiving/sending
    shutdown( n_socket, 2 );
    close( n_socket );

#ifdef _WIN32
    WSACleanup();
#endif

    _core.Print( LOG_INFO, "Successfully closed the network socket.\n" );
}


/*
    Network manager.
*/
void ncNetwork::Assign( netdata_t *from, byte *buffer ) {
    
    if( !network_active.GetInteger() )
        return;

    if( server_running.GetInteger() )
        _server.Process( from, buffer );

    if( client_running.GetInteger() )
        _client.Process( from, buffer );
}

/*
    Send packet.
*/
void ncNetwork::SendPacket( unsigned long len, const void *data, netdata_t *from ) {
    int ret;
    byte packet[len+4];

    packet[0] = (byte) ( PROTOCOL_ID >> 24 );
    packet[1] = (byte) ( ( PROTOCOL_ID >> 16 ) & 0xFF );
    packet[2] = (byte) ( ( PROTOCOL_ID >> 8 ) & 0xFF );
    packet[3] = (byte) ( ( PROTOCOL_ID ) & 0xFF );
    
    memcpy( &packet[4], data, len );

    ret = sendto( n_socket, packet, len + 4, 0, (struct sockaddr *)&from->sockaddress, sizeof(from->sockaddress) );
    
    if( ret == -1 )
        _core.Print( LOG_ERROR, "Could not send packet to %s\n", inet_ntoa(from->sockaddress.sin_addr) );
}

/*
    Print Out Of band message.
*/
void ncNetwork::PrintOutOfBand( netdata_t *adr, const char *format, ... ) {
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
void ncNetwork::PrintOutOfBandData( netdata_t *adr, Byte *format, int len ) {
    Byte		string[MAX_SERVER_COMMAND*2];
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
bool ncNetwork::CompareAddress( netdata_t *t1, netdata_t *t2 ) {
#ifdef _WIN32
    return t1->sockaddress.sin_addr.S_un.S_addr == t2->sockaddress.sin_addr.S_un.S_addr;
#else
    return t1->sockaddress.sin_addr.s_addr == t2->sockaddress.sin_addr.s_addr;
#endif
}

/*
    Check if address is on local machine.
*/
bool ncNetwork::IsLanAddress( netdata_t *adr ) {
    //
    // This is VERY ugly - FIXME
    //
    if(adr->ip[0] == '1' &&
       adr->ip[1] == '2' &&
       adr->ip[2] == '7' &&
       adr->ip[3] == '.' &&
       adr->ip[4] == '0' &&
       adr->ip[5] == '.' &&
       adr->ip[6] == '0' &&
       adr->ip[7] == '.')
    {
        return true;
    }

    return false;
}

/*
    Resolve address.
*/
bool ncNetwork::Resolve( netdata_t *address, const char *host ) {
    _core.Print( LOG_INFO, "ncNetwork::Resolve - %s\n", host );
    
    struct hostent *hn;
    hn = gethostbyname( host );
    
    if( !hn ) {
        switch( h_errno ) {
            // These are default.
            case HOST_NOT_FOUND:
                _core.Print( LOG_INFO, "ncNetwork::Resolve - couldn't find host.\n" );
                return false;
            case NO_ADDRESS:
                _core.Print( LOG_INFO, "ncNetwork::Resolve - host '%s' has no addresses.\n", host );
                return false;
            case NO_RECOVERY:
                _core.Print( LOG_INFO, "ncNetwork::Resolve - non-recoverable name server error.\n" );
                return false;
            case TRY_AGAIN:
                _core.Print( LOG_INFO, "ncNetwork::Resolve - host '%s' is temporarily unavailable.\n", host );
                return false;
        }
    }
    else
    {
        _core.Print( LOG_INFO, "ncNetwork::Resolve - resolved as '%s'\n", inet_ntoa (*((struct in_addr *) hn->h_addr_list[0])) );
        memcpy( (void *)&address->sockaddress.sin_addr, hn->h_addr_list[0], hn->h_length );
        return true;
    }

    // We'll never get here, so
    return true;
}

int ncNetwork::GetSocket() {
    if( !n_socket )
        return 0;
    
    return n_socket;
}

struct sockaddr_in ncNetwork::GetSockAddr() {
    return n_sv;
}


/*
    Helper function.
*/
int inet_pton4_alt( const char *src, char *dst ) {
    uint8_t tmp[NS_INADDRSZ], *tp;
    
    int saw_digit = 0;
    int octets = 0;
    *(tp = tmp) = 0;
    
    int ch;
    while ((ch = *src++) != '\0')
    {
        if (ch >= '0' && ch <= '9')
        {
            uint32_t n = *tp * 10 + (ch - '0');
            
            if (saw_digit && *tp == 0)
                return 0;
            
            if (n > 255)
                return 0;
            
            *tp = n;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;
    
    memcpy(dst, tmp, NS_INADDRSZ);
    
    return 1;
}