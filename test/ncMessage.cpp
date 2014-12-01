//
//  Nanocat engine.
//
//  Bitset library..
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

#include "Core.h"
#include "ncMessage.h"

bool ncBitset::Initialize( int num ) {
	if( Bits )
        delete [] Bits;

	Bits = NULL;
	Size = ( num >> 3 ) + 1;
    Bits = new Byte[Size];

	if( !Bits ) {
        g_Core->Error( ERR_FATAL, "ncBitset failed to allocate memory for %i bytes.\n", Size );
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

void ncBitset::Delete( void ) {
    ClearAll();
    Size = 0;
    
    delete [] Bits;
}

void ncBitMessage::Clear() {
    Size = 0;
}

void ncBitMessage::Create( Byte data[], int len ) {
    MaxSize = len;
    Size = 0;
    Data = data;
}

void *ncBitMessage::GetSpace( int length ) {
	void    *g_data;

	if( Size + length > MaxSize ) {

		if( !AllowOverflow )
			g_Core->Error( ERR_FATAL, "No overflow allowed for ncBitMessage.\n" );

		if( length > MaxSize )
			g_Core->Error( ERR_FATAL, "Given length to ncBitMessage is more than its max allowed size.", length );

		Overflowed = true;

		g_Core->Print( LOG_WARN, "ncBitMessage overflowed ( Now: %i MaxSize: %i ) \n", Size + length, MaxSize );

        Clear();
	}

    g_data = Data + Size;
	Size += length;

	return g_data;
}

void ncBitMessage::WriteChar( int c ) {
	Byte    *g_buf;
    g_buf       = (Byte*)GetSpace( 1 );
	g_buf[0]    = c;
}

void ncBitMessage::WriteByte( int c )  {
	Byte    *g_buf;

    g_buf       = (Byte*)GetSpace( 1 );
	g_buf[0]    = c;
}

void ncBitMessage::WriteInt32( int c ) {
	Byte    *g_buf;

    g_buf       = (Byte*)GetSpace( 2 );
	g_buf[0]    = c & 0xff;
	g_buf[1]    = c >> 8;
}

void ncBitMessage::WriteLong( int c ) {
	Byte    *g_buf;
    
	g_buf       = (Byte*)GetSpace( 4 );
    g_buf[0]    = c & 0xff;
	g_buf[1]    = (c >> 8) & 0xff;
	g_buf[2]    = (c >> 16) & 0xff;
	g_buf[3]    = c >> 24;
}

// Main write function for Bitstream.
#pragma mark - TODO: fail checks.
void ncBitMessage::Write( void *data, int length ) {
    if( !memcpy( GetSpace( length ), data, length ) ) {
        
        g_Core->Print( LOG_ERROR, "ncBitMessage::Write failed to copy data with %s!\n", data );
        g_Core->Error( ERR_FATAL, "ncBitMessage::Write failed to copy data." );
    }
    
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

void ncBitMessage::WriteString( const NString s ) {
    if ( !s ) {
        // Empty string, skip it
        // and write empty space to keep something.
		Write( (Byte*)"", 1 );
    }
    else {
        // Ignore this damned warning.
#pragma mark - Ignore this damned warning.
		Write( (Byte*)s, strlen(s) + 1 );
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

	c = (Byte)Data[DataRead];
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

	c = Data[DataRead] + (Data[DataRead + 1] << 8) + (Data[DataRead + 2] << 16) + (Data[DataRead + 3] << 24);

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

NString ncBitMessage::ReadString() {
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


