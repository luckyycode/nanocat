//
//  Nanocat engine.
//
//  Image loader.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef image_h
#define image_h

#include "systemshared.h"

enum imagetype_t {
    TGA_IMAGE_UNCOMPRESSED,                    // Uncompressed TGA.
    BMP_IMAGE                                  // BMP
};

class ncImage {
public:
    Byte                type;
    
    short int           width;
    short int           height;
    
    Byte                bitCount;
    Byte               *imageData;
    
    GLuint              tex_id;
};

class ncImageLoader {
public:
    bool CreateImage( int width, int height, byte *data, imagetype_t type, const char *filename );
    bool Load( imagetype_t type, const char *filename, ncImage *img );
    void Unload( ncImage *tex );
    
};

extern ncImageLoader _image;

#endif
