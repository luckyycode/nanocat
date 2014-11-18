//
//  Security.h
//
//  Security functions and stuff.
//
//  Created by Neko Code on 11/17/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef _Security__
#define _Security__

#include "NCString.h"

class ncSecurity {
    
public:
    // Super simple XOR methods.
    NString MakeXORMultichar( NString value, NString key );
    NString MakeXOR( NString value, NChar key  );
};

extern ncSecurity *g_Security;

#endif /* defined(__Nanocat__Security__) */
