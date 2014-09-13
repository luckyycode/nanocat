//
//  Nanocat engine.
//
//  Game server..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef server_h
#define server_h

#include "Network.h"
#include "GameMath.h"

#define MAX_RELIABLESERVERMESSAGE 16384
#define MAX_PLAYER_NAME_LEN 18

#define MAX_SERVERNAMELEN 18
#define MAX_SNAPSHOT_SIZE 13894
#define SERVER_CLIENTAUTHORIZE_TIME 5000


#define SERVER_DEFAULT_CLIENTNUM 24
#define DEFAULT_SERVER_NAME "Meow server"

enum ServerClientState {
    // Free client slot.
    SVCL_FREE,
    // Client is connecting..
    SVCL_CONNECTING,
    // Client is connected.
    SVCL_CONNECTED,
    // Client is verifying..
    SVCL_CHALLENGING,
    // Stuck ( inactive ) client.
    SVCL_ZOMBIE
}; // Server client state.

enum ServerState {
    // Server is not created.
    SERVER_IDLE,
    // Server is loading.
    SERVER_LOADING,
    // Only authorized persons or connect by a password.
    SERVER_AUTHORIZE,
    // Error thrown.
    SERVER_ERROR,
    // Active.
    SERVER_GAME,
    // Locked, clients are not able to connect.
    SERVER_LOCKED
};    // Server state.

#define	MAX_RESPONSES	1024

class ncServerResponseData {
public:
    int			Response;
    int			PingAt;
    int			FirstAt;
    int			Time;
    
    bool	    Connected;
    ncNetdata   Address;
};

/*
    Player entity.
*/
class ncServerPlayerEntity {
public:
    ncVec3 g_vLook;
    ncVec3 g_vRight;
    ncVec3 g_vEye;
    ncVec3 g_vUp;
};

class ncServerClient {
public:
    
    ncServerClient();
    
    char                    name[MAX_PLAYER_NAME_LEN];
    char                    version[2];
    
    ncNetdata               address;
    ServerClientState             state;
    ncNetAddressType            type;
    ncNetchannel            channel;
    ncServerPlayerEntity          player;
    
    int                     clientnum;
    int                     response;
    
    int                     lastMessageTime;
    int                     lastCommandTime;
    int                     lastConnectTime;
    int                     lastReceivedackCommand;
    int                     lastExecutedackCommand;
    int                     lastMessageReceivedAt;
    int                     lastAcknowledgedMessage;
    
    char                    *ackCommands[MAX_RELIABLESERVERMESSAGE];
    uint                    ackSequence;
    uint                    ackAcknowledged;
    
    int                     zombifiedAt;
};


class ncServer {
public:
    
    ncServer() {
        Initialized = false;
        Port = 0;
        MaxClients = 0;
        ServerIdentificator = 0;
        ClientNum = 0;
        
        LastInfoPrintTime = 0;
        Time = 0;
        LastTime = 0;
    }
    
    void Initialize( void );
    void CreateSession( void );
    void SetupClients( int maxclients );
    void Maprestart( void );
    void ProcessClientMessage( ncServerClient *client, ncBitMessage *msg );
    void CreateClient( ncNetdata *from, int response, const char *name, const char *version );
    void Connectionless( ncNetdata *from, byte *data );
    void ParseClients( ncNetdata *from, ncBitMessage *packet );
    void SendFrame( ncServerClient *cl );
    void SendFrames( void );
    void Process( ncNetdata *from, byte *buffer );
    void CheckParams( void );
    void ClearWorld( const char *msg );
    void PrintInfo( void );
    void CheckTimeouts( void );
    void Frame( int msec );
    void Loadmap( void );
    void Launchmap( void );
    void DisconnectClient( ncServerClient *client, const char *message );
    void GetResponse( ncNetdata *from );
    void CheckZombies( void );
    void PrintStatus( void );
    void SendByeMessage( const char *msg );
    void KickClient( void );
    void AddAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, char *message );
    void SendAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, const char *cmd, ... );
    void RemoveBots( void );
    void Disconnect( void );
    void Shutdown( const char *finalmsg );
    void AddBot( void );
    
    ncServerClient *GetClientByNum( int num );
    ncServerClient *GetClientByAddress( struct sockaddr_in data );
    
    bool            Initialized;
    
    char            Name[MAX_SERVERNAMELEN];
    
    int             MaxClients;
    int             ClientNum;
    int             Port;
    int             ServerIdentificator;
    
    int             LastTime;
    int             Time;
    int             LastInfoPrintTime;
    
    ServerState                 State;
    ncServerClient              *Clients;
    ncServerResponseData	    Response[MAX_RESPONSES];
};

extern ncServer _server;


// SERVER
extern ncConsoleVariable       Server_Name;                       // Server name.
extern ncConsoleVariable       Server_Active;                        // Is server running?
extern ncConsoleVariable       Server_Maxclients;                     // Maximum server clients.
extern ncConsoleVariable       Server_Fun;                         // Can server clients use cheat commands?
extern ncConsoleVariable       Server_Maxfps;                            // Server fps.
extern ncConsoleVariable       Server_CreateAtStart;                  // Create server on its launch?
extern ncConsoleVariable       Server_Sayname;                        // What name server should have in the chat?
extern ncConsoleVariable       Server_Dedicated;                           // Is server dedicated?


#endif
