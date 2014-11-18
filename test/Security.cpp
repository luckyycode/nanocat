//
//  Security.h
//
//  Security functions and stuff.
//
//  Created by Neko Code on 11/17/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Security.h"

#include "Core.h"
#include "SystemShared.h"

ncSecurity local_securitySector;
ncSecurity *g_Security = &local_securitySector;

/*
    Make XOR encryption/decrypyion by multichar key. 
*/
NString MakeXORMultichar( NString value, NString key )
{
    short uint klen = strlen( key );
    short uint vlen = strlen( value );
    short uint k = 0;
    
    NString retval = value;
    
    for( int v = 0; v < vlen; v++ ) {
        retval[v] = value[v] ^ key[k];
        k = ( ++k < klen ? k : 0 );
    }
    
    return retval;
}

/*
    Make XOR encryption/decryption.
*/
NString MakeXOR( NString value, NChar key  ) {
    
    short uint vlen = strlen( value );
    NString output = value;
    
    for (int i = 0; i < vlen; i++)
        output[i] = value[i] ^ key;
    
    return output;
}

