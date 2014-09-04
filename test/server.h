//
//  Nanocat engine.
//
//  Game server.
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef server_h
#define server_h

#include "network.h"
#include "gmath.h"

#define MAX_RELIABLESERVERMESSAGE 16384
#define MAX_PLAYER_NAME_LEN 18

#define MAX_SERVERNAMELEN 18
#define MAX_SNAPSHOT_SIZE 13894
#define SERVER_CLIENTAUTHORIZE_TIME 3000

#define DEFAULT_SERVER_NAME "Meow server"

enum serverclientstate_t {
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

enum serverstate_t {
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

// Quake-like system.
typedef struct {
    int			response;
    int			pingTime;
    int			firstTime;
    int			time;
    
    bool	    connected;
    netdata_t   adr;
} response_t;

/*
    Player entity.
*/
typedef struct _playerentity {
    ncVec3 g_vLook;
    ncVec3 g_vRight;
    ncVec3 g_vEye;
    ncVec3 g_vUp;
} playerentity_t;

class ncServerClient {
public:
    char                    name[MAX_PLAYER_NAME_LEN];
    char                    version[2];
    
    netdata_t               address;
    serverclientstate_t             state;
    netaddress_t            type;
    netchannel_t            channel;
    playerentity_t          player;
    
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
    void Initialize( void );
    void CreateSession( void );
    void SetupClients( int maxclients );
    void Maprestart( void );
    void ProcessClientMessage( ncServerClient *client, ncBitMessage *msg );
    void CreateClient( netdata_t *from, int response, const char *name, const char *version );
    void Connectionless( netdata_t *from, byte *data );
    void ParseClients( netdata_t *from, ncBitMessage *packet );
    void SendFrame( ncServerClient *cl );
    void SendFrames( void );
    void Process( netdata_t *from, byte *buffer );
    void CheckParams( void );
    void ClearWorld( const char *msg );
    void PrintInfo( void );
    void CheckTimeouts( void );
    void Frame( int msec );
    void Loadmap( void );
    void Launchmap( void );
    void DisconnectClient( ncServerClient *client, const char *message );
    void GetResponse( netdata_t *from );
    void CheckZombies( void );
    void PrintStatus( void );
    ncServerClient *GetClientByNum( int num );
    ncServerClient *GetClientByAddress( struct sockaddr_in data );
    void SendFinalMessage( const char *msg );
    void KickClient( void );
    void AddAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, char *message );
    void SendAcknowledgeCommand( ncServerClient *cl, bool isDisconnect, const char *cmd, ... );
    void RemoveBots( void );
    void Disconnect( void );
    void Shutdown( const char *finalmsg );
    void AddBot( void );
    
    bool            Initialized;
    
    char            Name[MAX_SERVERNAMELEN];
    
    int             MaxClients;
    int             ClientNum;
    int             Port;
    int             serverId;
    
    int             TimeResidual;
    int             Time;
    int             LastInfoPrintTime;
    
    serverstate_t   State;
    ncServerClient  *Clients;
    response_t	    Response[MAX_RESPONSES];
};

extern ncServer _server;


// SERVER
extern ConsoleVariable       server_hostname;                       // Server name.
extern ConsoleVariable       server_running;                        // Is server running?
extern ConsoleVariable       server_maxclients;                     // Maximum server clients.
extern ConsoleVariable       server_cheats;                         // Can server clients use cheat commands?
extern ConsoleVariable       server_fps;                            // Server fps.
extern ConsoleVariable       server_spawnonlaunch;                  // Create server on its launch?
extern ConsoleVariable       server_sayname;                        // What name server should have in the chat?
extern ConsoleVariable       server_dedi;                           // Is server dedicated?


#endif
