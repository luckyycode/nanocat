//
//  Nanocat engine.
//
//  System manager..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef network_h
#define network_h

#include "Core.h"
#include "SystemShared.h"
#include "ncBitMessage.h"

#ifdef _WIN32
#define WINSOCKET_STARTUP_CODE       0x0101
#endif

#define PROTOCOL_ID 0x1337c0de
#define	MAX_UDP_PACKET	8192

// System network host name length.
#define MAX_NET_HOSTNAME_LENGTH                 256             // Network host.

#define MAX_CLIENTS_NUM                         1024
// Maximum bind change tries.
#define MAX_BIND_CHANGE                         16

// In case if something fails.
#define NETWORK_IGNORE_GETHOSTBYNAME

// Last raw received message.
extern byte network_message_buffer[MAX_UDP_PACKET];

const int NETWORK_PROTOCOL_MINOR = 37;
const int NETWORK_PROTOCOL_MAJOR = 13;
const int NETWORK_PROTOCOL	= ( NETWORK_PROTOCOL_MAJOR << 16 ) + NETWORK_PROTOCOL_MINOR;

/*
    Network address data.
*/

#define MAX_IPADDRESS_NETDATALEN 32

class ncNetdata {
public:
    
    ncNetdata();
    ncNetdata( struct sockaddr_in _socketaddr, unsigned int port, unsigned int socket );
    
    char IPAddress[MAX_IPADDRESS_NETDATALEN];
    
    int Socket;     // Socket.
    int Port;
    
    struct sockaddr_in SockAddress;
};

class ncNetchannel {
public:
    int type;
    ncNetdata address;
    
    int sequenceIn;
    int sequenceOut;
};

enum ncNetAddressType {
    // Bot.
    ADDR_BOT,
    // Host user.
    ADDR_LOOPBACK,
    // User from 'outside'.
    ADDR_IP,
    // IPv6 protocol
    ADDR_IPv6
}; // Network channel type.

class ncNetwork {
public:
    void Initialize( void );
    bool Frame( void );
    void Shutdown( void );
    void Assign( ncNetdata *from, byte *buffer );
    void SendPacket( unsigned long len, const void *data, ncNetdata *from );
    void PrintOutOfBand( ncNetdata *adr, const NString format, ... ) ;
    void PrintOutOfBandData( ncNetdata *adr, Byte *format, int len );
    bool CompareAddress( ncNetdata *t1, ncNetdata *t2 );
    bool IsLanAddress( ncNetdata *adr );
    bool Resolve( ncNetdata *address, const NString host );
    
    void CreateChannel( ncNetchannel *chan, ncNetdata *adr );
    void SendMessageChannel( ncNetchannel *chan, ncBitMessage *msg );
    bool ProcessChannel( ncNetchannel *chan, byte *packet );
   
    int GetSocket( void );
    struct sockaddr_in GetSockAddr( void );
    
private:
    int                 NetworkSocket;
    struct sockaddr_in  DataAddr, RecvAddr;
};

extern ncNetwork *g_networkManager;

// NETWORK
extern ncConsoleVariable       Network_Port;                          // Network port.
extern ncConsoleVariable       Network_IPAddress;                            // Network ip address.
extern ncConsoleVariable       Network_Active;                        // Turn on/off networking.
extern ncConsoleVariable       Network_LocalIPAddress;                       // Local Device IP Address.
extern ncConsoleVariable        Network_NotAvailable;                        // Is internet connection available?
#endif
