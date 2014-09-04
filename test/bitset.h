//
//  Nanocat engine.
//
//  Bitset library header.
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.


#ifndef bitset_included
#define bitset_included

#include "systemshared.h"

class ncBitset {
public:
    int Size;
    Byte *Bits;
    
    bool Initialize( int num );
    
    void ClearAll( void );
    void Clear( int num );
    void Set( int num );
    void SetAll( void );
    
    Byte IsSet( int num );
};

class ncBitMessage {
public:
    ncBitMessage( int size ) {
        byte data[size];
        this->Create( data, sizeof(data) );
    }
    
    ncBitMessage() {
        this->Clear();
    }
    
    ncBitMessage( byte *data, int size ) {
        Clear();

        byte datsa[size];
        memcpy( datsa, data, sizeof(datsa) );
        
        Data = datsa;
        MaxSize = sizeof(datsa);
        Size = sizeof(datsa);
    }
    
    int		MaxSize;
    int		Size;
    int     DataRead;
    
    bool    Corrupted;
    bool    AllowOverflow;
    bool    Overflowed;
    
    Byte	*Data;
    
    void Clear( void );
    void Create( byte data[], int len );
    void *GetSpace( int length );
    
    void WriteChar( int c );
    void WriteByte( int c );
    void WriteInt32( int c );
    void WriteLong( int c );
    void Write( void *data, int length );
    void WriteFloat( float f );
    void WriteString( const char *s );
    void WriteCoord( float f );
    void WriteAngle( float f );
    void WriteData( byte *data, int length );
    void ReadData( byte **data, int length );
    void BeginReading( void );
    
    int ReadChar( void );
    int ReadByte( void );
    int ReadInt32( void );
    int ReadLong( void );
    float ReadFloat( void );
    char *ReadString( void );
    float ReadCoord( void );
    float ReadAngle( void );
};

#endif
