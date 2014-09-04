//
//  Nanocat engine.
//
//  Bitset library.
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

#include "core.h"
#include "bitset.h"

bool ncBitset::Initialize( int num ) {
	if( Bits )
		free( Bits );

	Bits = NULL;
	Size = ( num >> 3 ) + 1;
	Bits = (byte*)malloc( sizeof(byte) * Size );

	if( !Bits ) {
        _core.Error( ERC_FATAL, "Bitset failed to allocate memory for %i bytes.\n", Size );
		return false;
	}

	memset( Bits, 0, Size );

	return true;
}

void ncBitset::ClearAll( void ) {
	memset( Bits, 0, Size );
}

void ncBitset::SetAll( void ) {
	memset( Bits, 0xFF, Size );
}

void ncBitset::Clear( int num ) {
	Bits[num >> 3] &= ~(1 << (num & 7));
}

void ncBitset::Set( int num ) {
	Bits[num >> 3] |= 1 << (num & 7);
}

byte ncBitset::IsSet( int num ) {
	return Bits[num >> 3] & 1 << (num & 7);
}

void ncBitMessage::Clear() {
    Size = 0;
}

void ncBitMessage::Create( Byte data[], int len ) {
    MaxSize = len;
    Size = 0;
    Data = data;
    
    //_core.Print( LOG_NONE, "size: %i maxsize: %i data: %s\n", Size, MaxSize, Data );
    //memcpy( Data, data, sizeof(Byte[13894]));
}

void *ncBitMessage::GetSpace( int length ) {
	void    *g_data;

	if( Size + length > MaxSize ) {

		if( !AllowOverflow )
			_core.Error( ERC_FATAL, "No overflow allowed for ncBitsetMessage.\n" );

		if( length > MaxSize )
			_core.Error( ERC_FATAL, "Given length to ncBitsetMessage is more than its max allowed size.", length );

		Overflowed = true;

		_core.Print( LOG_WARN, "ncBitsetMessage overflowed.\n" );

        Clear();
	}

    g_data = Data + Size;
	Size += length;

	return g_data;
}

void ncBitMessage::WriteChar( int c ) {
	byte    *g_buf;
    g_buf       = (byte*)GetSpace( 1 );
	g_buf[0]    = c;
}

void ncBitMessage::WriteByte( int c )  {
	byte    *g_buf;

    g_buf       = (byte*)GetSpace( 1 );
	g_buf[0]    = c;
}

void ncBitMessage::WriteInt32( int c ) {
	byte    *g_buf;

    g_buf       = (byte*)GetSpace( 2 );
	g_buf[0]    = c & 0xff;
	g_buf[1]    = c >> 8;
}

void ncBitMessage::WriteLong( int c ) {
	byte    *g_buf;
	g_buf       = (byte*)GetSpace( 4 );
    g_buf[0]    = c & 0xff;
	g_buf[1]    = (c >> 8) & 0xff;
	g_buf[2]    = (c >> 16) & 0xff;
	g_buf[3]    = c >> 24;
}

void ncBitMessage::Write( void *data, int length ) {
	memcpy( GetSpace( length ), data, length );
}

void ncBitMessage::WriteFloat( float f ) {
    union {
		float   f;
		int     l;
	} rev;

	rev.f = f;
	rev.l = rev.l;

	Write( &rev.l, 4 );
}

void ncBitMessage::WriteString( const char *s ) {
    if ( !s ) {
        // Empty string, skip it
        // and write empty space to keep something.
		Write( (byte*)"", 1 );
    }
    else {
        // Ignore this damned warning.
		Write( (byte*)s, strlen(s) + 1 );
    }
}

void ncBitMessage::WriteCoord( float f ) {
	WriteInt32( (int)( f * 8 ) );
}

void ncBitMessage::WriteAngle( float f ) {
	WriteByte( ((int)f * 256 / 360) & 255 );
}

void ncBitMessage::BeginReading() {
	DataRead = 0;
	Corrupted = false;
}

int ncBitMessage::ReadChar() {
	int c;

	if( DataRead + 1 > Size ) {
		Corrupted = true;
		return -1;
	}

	c = (signed char)Data[DataRead];
	DataRead++;

	return c;
}

int ncBitMessage::ReadByte() {
	int c;

	if( DataRead + 1 > Size ) {
		Corrupted = true;
		return -1;
	}

	c = (byte)Data[DataRead];
	DataRead++;

	return c;
}

int ncBitMessage::ReadInt32() {
	int c;

	if( DataRead + 2 > Size ) {
		Corrupted = true;
		return -1;
	}

	c = (short)( Data[DataRead] + (Data[DataRead + 1] << 8) );

	DataRead += 2;

	return c;
}

int ncBitMessage::ReadLong() {
	int c;

	if( DataRead + 4 > Size ) {
		Corrupted = true;
		return -1;
	}

	c = Data[DataRead]
	+ (Data[DataRead + 1] << 8)
	+ (Data[DataRead + 2] << 16)
	+ (Data[DataRead + 3] << 24);

	DataRead += 4;

	return c;
}

float ncBitMessage::ReadFloat() {
	union {
		byte    b[4];
		float   f;
		int     l;
	} rev;

	rev.b[0] = Data[DataRead];
	rev.b[1] = Data[DataRead + 1];
	rev.b[2] = Data[DataRead + 2];
	rev.b[3] = Data[DataRead + 3];
	DataRead += 4;

	rev.l = rev.l;

	return rev.f;
}

char *ncBitMessage::ReadString() {
	static char g_string[2048];
	int l, c;

	l = 0;
	do {
        c = ReadChar();
		if( c == -1 || c == 0 )
			break;

		g_string[l] = c;
		l++;
	} while( l < sizeof(g_string) - 1 );

	g_string[l] = 0;

	return g_string;
}

float ncBitMessage::ReadCoord() {
	return ReadInt32() * (1.0 / 8);
}

float ncBitMessage::ReadAngle() {
	return ReadChar() * ( 360.0 / 256 );
}


