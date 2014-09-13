//
//  Nanocat engine.
//
//  Game client..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef client_h
#define client_h

#include "Network.h"
#include "GameMath.h"

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

class ncClient {
    
public:
    
    ncClient() { }
    
    void Disconnect( void );
    void Initialize( void );
    void SendAcknowledge( const char *command );
    void CheckTimeout( void );
    void Connect( void );
    void LoopbackConnect( void );
    void ProcessAcknowledgeCommands( ncBitMessage *msg );
    void SendCommandPacket( void );
    void CheckCommands( void );
    void ParseCommands( ncNetdata *from, ncBitMessage *buffer );
    void Connectionless( ncNetdata *from, byte *data );
    void Process( ncNetdata *from, byte *buffer );
    void Reconnect( void );
    void DisconnectForced( const char *msg, bool forced );
    void ChangeName( void );
    void CheckConnect( void );
    void Interpolate( void );
    void Frame( int msec );
    void Say( void );
        
    ClientState_t       State;
    ncNetchannel        Channel;
    ncNetdata           *CurrentServer;
        
    char                Name[MAX_PLAYER_NAME_LEN];
        
    // .. Uh oh, a lot of integers.
    
    uint                ClientID;
    
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
    
    ncVec3 latestPosition;
    ncVec3 latestRotation;
    float latestPositionTime = 0.0f;
};

extern ncClient _client;

// CLIENT
extern ncConsoleVariable      Client_Running;                        // Is client running?
extern ncConsoleVariable      NameVar;                               // Client name.
extern ncConsoleVariable      Client_ServerTimeout;                  // In seconds.
extern ncConsoleVariable      Client_MaximumPackets;                     // Max client packets per frame to be sended.


#endif
