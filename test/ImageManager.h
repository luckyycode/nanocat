//
//  Nanocat engine.
//
//  Image loader..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef image_h
#define image_h

#include "SystemShared.h"

enum ncImageType {
    NCTGA_IMAGE,                      // TGA
    NCBMP_IMAGE                       // BMP
};


typedef struct {
    GLubyte Header[12];									// TGA File Header
} ncTGAHeader;

typedef struct {
    GLubyte		header[6];								// First 6 Useful Bytes From The Header
    GLuint		bytesPerPixel;							// Holds Number Of Bytes Per Pixel Used In The TGA File
    GLuint		imageSize;								// Used To Store The Image Size When Setting Aside Ram
    GLuint		temp;									// Temporary Variable
    GLuint		type;
    GLuint		Height;									//Height of Image
    GLuint		Width;									//Width ofImage
    GLuint		Bpp;									// Bits Per Pixel
} ncTGAImage;

typedef	struct {
    GLubyte	*ImageData;									// Image Data (Up To 32 Bits)
    GLuint	BitsPerPixel;											// Image Color Depth In Bits Per Pixel
    GLuint	Width;											// Image Width
    GLuint	Heigth;											// Image Height
    GLuint	TextureID;											// Texture ID Used To Select A Texture
    GLuint	Type;											// Image Type (GL_RGB, GL_RGBA)
} ncImage;


class ncImageLoader {
public:
    bool CreateImage( int width, int height, byte *data, ncImageType type, const NString filename );
    bool Load( ncImageType type, const NString filename, ncImage *img );
    void Unload( ncImage *tex );
    
    bool LoadBMP( const NString filename, ncImage *img );
    bool LoadTGA( const NString name, ncImage *image );
    
    bool LoadUncompressedTGA(ncImage* texture, const NString  filename, FILE * fTGA);
    bool LoadCompressedTGA(ncImage * texture, const NString  filename, FILE * fTGA);
    
    void MakeSeamlessTGA( ncImage *img );
};

extern ncImageLoader *g_imageManager;

#endif
