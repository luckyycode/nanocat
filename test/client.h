//
//  Nanocat engine.
//
//  Game client.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef client_h
#define client_h

#include "network.h"

#define RESPONSE_REPEAT 2000

// Maximum player name length.
#define MAX_PLAYER_NAME_LEN  18

enum ClientState_t {
    // Not connected to any server.
    CLIENT_IDLE,
    // Trying to connecting to server
    CLIENT_PRECONNECTING,
    // No information from server for a long time.
    CLIENT_NORESPONSE,
    // Client is connecting.
    CLIENT_CONNECTING,
    // Client is connected.
    CLIENT_CONNECTED,
};    // Local client state.

class Client {
    
public:
    
    Client() { }
    
    void Disconnect( void );
    void Initialize( void );
    void SendAcknowledge( const char *command );
    void CheckTimeout( void );
    void Connect( void );
    void LoopbackConnect( void );
    void ProcessAcknowledgeCommands( ncBitMessage *msg );
    void SendCommandPacket( void );
    void CheckCommands( void );
    void ParseCommands( netdata_t *from, ncBitMessage *buffer );
    void Connectionless( netdata_t *from, byte *data );
    void Process( netdata_t *from, byte *buffer );
    void Reconnect( void );
    void DisconnectForced( const char *msg, bool forced );
    void ChangeName( void );
    void CheckConnect( void );
    void Interpolate( void );
    void Frame( int msec );
    void Say( void );
        
    ClientState_t       State;
    netchannel_t        Channel;
    netdata_t           *CurrentServer;
        
    char                Name[MAX_PLAYER_NAME_LEN];
        
    // .. Uh oh, a lot of integers.
    
    int                 Response;
    
    int                 TimeBase;
    int                 Frametime;
    int                 Time;
        
    int                 LastCommandTime;
    int                 LastServerMessage;
    int                 LastMessageReceivedAt;
    int                 LastExecutedack;
    int                 LastackMessage;
    int                 LastConnectPacketTime;
        
    int                 AckAcknowledged;
    int                 AckSequence;
        
    int                 CommandSequence;
    int                 TimeSinceLastPacket;
        
    const char          *AckCommands[MAX_RELIABLESERVERMESSAGE];
};

extern Client _client;

// CLIENT
extern ConsoleVariable      client_running;                        // Is client running?
extern ConsoleVariable      namevar;                               // Client name.
extern ConsoleVariable      client_servertimeout;                  // In seconds.
extern ConsoleVariable      client_maxpackets;                     // Max client packets per frame to be sended.


#endif
