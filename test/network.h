//
//  Nanocat engine.
//
//  System manager.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef network_h
#define network_h

#include "core.h"
#include "systemshared.h"
#include "bitset.h"

#ifdef _WIN32
#define WINSOCKET_STARTUP_CODE       0x0101
#endif

#define PROTOCOL_ID 0x1337c0de
#define	MAX_UDP_PACKET	8192

// System network host name length.
#define MAX_NET_HOSTNAME_LENGTH                 256             // Network host.

// Network "bind" change limit.
#define MAX_CLIENTS_NUM                         1024
// Maximum bind change tries.
#define MAX_BIND_CHANGE                         16

// Max network packet length.
#define MAX_PACKET_LEN                          1400

// Last raw received message.
extern byte network_message_buffer[MAX_UDP_PACKET];

#define NS_INADDRSZ  4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ   2


const int ASYNC_PROTOCOL_MINOR		= 41;
const int ASYNC_PROTOCOL_MAJOR      = 2;
const int ASYNC_PROTOCOL_VERSION	= ( ASYNC_PROTOCOL_MAJOR << 16 ) + ASYNC_PROTOCOL_MINOR;


/*
    Network address data.
*/
struct netdata_t {
    char                ip[32];
    
    int sc;     // Socket.
    int port;
    
    struct sockaddr_in  sockaddress;
};

struct netchannel_t {
    int type;
    netdata_t address;
    
    int sequenceIn;
    int sequenceOut;
};

enum netaddress_t {
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
    void Assign( netdata_t *from, byte *buffer );
    void SendPacket( unsigned long len, const void *data, netdata_t *from );
    void PrintOutOfBand( netdata_t *adr, const char *format, ... ) ;
    void PrintOutOfBandData( netdata_t *adr, Byte *format, int len );
    bool CompareAddress( netdata_t *t1, netdata_t *t2 );
    bool IsLanAddress( netdata_t *adr );
    bool Resolve( netdata_t *address, const char *host );
    
    void CreateChannel( netchannel_t *chan, netdata_t *adr );
    void SendMessageChannel( netchannel_t *chan, ncBitMessage *msg );
    bool ProcessChannel( netchannel_t *chan, byte *packet );
   
    int GetSocket( void );
    struct sockaddr_in GetSockAddr( void );
    
private:
    int                 n_socket;
    struct sockaddr_in  n_sv, n_cl;
};

extern ncNetwork _netmanager;

// NETWORK
extern ConsoleVariable       network_port;                          // Network port.
extern ConsoleVariable       network_ip;                            // Network ip address.
extern ConsoleVariable       network_active;                        // Turn on/off networking.
extern ConsoleVariable       network_localip;                       // Local Device IP Address.

#endif
