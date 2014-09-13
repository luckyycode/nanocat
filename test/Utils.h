//
//  Utils.h
//  Nanocat.
//
//  Created by Neko Code on 9/12/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef Utils_h
#define Utils_h


class ncUtils {
public:
    static void OBJtoSM( const char *file );
    
    static char *HexEncode( const char *data, unsigned int data_size );
    static bool HexCharDecode( char hexdigit, unsigned char &value );
};

#endif
