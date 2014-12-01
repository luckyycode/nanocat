//
//  Nanocat engine.
//
//  Bitset library header..
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.


#ifndef bitset_included
#define bitset_included

#include "SystemShared.h"

// Memory stream.
class ncDataStream {
private:
    
    size_t  d_size;
    size_t  d_offset;
    NString d_location;
    NString d_origin;
    size_t  d_maxsize;
    bool    d_freeOnClose;
    
public:
    ncDataStream();
    ncDataStream( NString buf, size_t size );
    ncDataStream( size_t size );
    ~ncDataStream();
    
    void Open( NString buf, size_t size );
    void Resize( size_t newsize );
    void WriteToFile( FILE * file );
    
    size_t GetSize();
    size_t Read( void *dest, size_t size, size_t count );
    size_t Write( const void * d_str, size_t size, size_t count );
    size_t Write( int value, size_t count );
    size_t Seek( size_t offset, size_t type );
    size_t Tell();
    size_t ReadString( register NString str, size_t max_size );
    
    NString     Current();
    NString     GetData();

    bool Find( NString d_str, size_t len );
    ncDataStream *CompressZlib();
    
    void operator++() {
        d_offset++;
        d_location++;
    }
    void operator--() {
        d_offset--;
        d_location--;
    }
    void operator +=(int b) {
        d_offset += b;
        d_location += b;
    }
    void operator -= (int b) {
        d_offset -= b;
        d_location -= b;
    }
    operator NString() const {
        return d_location;
    }
    
};


// Bitset.
class ncBitset {
public:
    int Size;
    Byte *Bits;
    
    bool Initialize( int num );
    
    void ClearAll( void );
    void Clear( int num );
    void Set( int num );
    void SetAll( void );
    void Delete( void );
    
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
    void WriteString( const NString s );
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
    NString ReadString( void );
    float ReadCoord( void );
    float ReadAngle( void );
};

#endif
