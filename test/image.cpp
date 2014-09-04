//
//  Nanocat engine.
//
//  Image loader.
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "files.h"
#include "ncstring.h"
#include "image.h"
#include "renderer.h"

ncImageLoader _image;

/*
    Write image.
*/
bool ncImageLoader::CreateImage( int width, int height, byte *data, imagetype_t type, const char *filename ) {
    
    switch( type ){
        case BMP_IMAGE: {
                FILE *f;
            
                f = fopen( _stringhelper.STR("%s.bmp", filename), "wb" );
                if( !f ) {
                    _core.Error( ERC_ASSET, "Image write failed: couldn't create file.\n" );
                    return false;
                }
            
                int w = width;
                int h = height;
                int i;
            
                int filesize = 54 + 3 * w * h;

                byte bmpfileheader[14] = {
                    'B', 'M',
                    0, 0, 0, 0,
                    0, 0,
                    0, 0,
                    54, 0, 0, 0 };
            
                byte bmpinfoheader[40] = { 40, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    1, 0,
                    24, 0};
            
                byte bmppad[3] = { 0, 0, 0 };
            
                bmpfileheader[2] = (byte)( filesize );
                bmpfileheader[3] = (byte)( filesize >> 8 );
                bmpfileheader[4] = (byte)( filesize >> 16 );
                bmpfileheader[5] = (byte)( filesize >> 24 );
            
                bmpinfoheader[4] = (byte)( w );
                bmpinfoheader[5] = (byte)( w >> 8);
                bmpinfoheader[6] = (byte)( w >> 16  );
                bmpinfoheader[7] = (byte)( w >> 24);
                bmpinfoheader[8] = (byte)( h );
                bmpinfoheader[9] = (byte)( h >> 8 );
                bmpinfoheader[10] = (byte)( h >> 16 );
                bmpinfoheader[11] = (byte)( h >> 24 );
            
                fwrite( bmpfileheader, 1, 14, f );
                fwrite( bmpinfoheader, 1, 40, f );
            
                for( i = 0; i < h; i++ ) {
                    fwrite( data + ( w * (h - i - 1) * 3 ), 3, w, f );
                    fwrite( bmppad, 1, ( 4 - ( w * 3 ) % 4 ) % 4, f );
                }
            
                fclose(f);
            }
            break;
        case TGA_IMAGE_UNCOMPRESSED:
            
            break;
        default:
            _core.Error( ERC_ASSET, "Image write failed: undefined image type.\n" );
            break;
            
    }
    
    return true;
}

/*
    Load image.
*/
bool ncImageLoader::Load( imagetype_t type, const char *filename, ncImage *img ) {
    
    FILE    *imgfile;

    byte            header[54];
    unsigned int    dataPos;
    unsigned int    imageSize;

    byte            ucharBad;
    byte            colorSwap;
    
    short int       sintBad;
    int             colorMode;
    int             imageIdx;

    switch (type) {
            
        /* Load uncompressed tga. */
        case TGA_IMAGE_UNCOMPRESSED:

            imgfile = _filesystem.OpenRead( _stringhelper.STR( "%s/%s/%s.tga", filesystem_path.GetString(), TEXTURE_FOLDER, filename ) );

            if (!imgfile) {
                _core.Print( LOG_ERROR, "image_load: Couldn't find %s image\n", filename );
                return false;
            }

            fread(&ucharBad, sizeof(unsigned char), 1, imgfile);
            fread(&ucharBad, sizeof(unsigned char), 1, imgfile);
            fread(&img->type, sizeof(unsigned char), 1, imgfile);

            if (img->type != 2 && img->type != 3) {
                _core.Print( LOG_ERROR, "Image %s.tga is corrupted\n", filename );
                fclose(imgfile);
                return false;
            }

            // Ignore useless bytes.
            fread(&sintBad, sizeof(short int), 1, imgfile);
            fread(&sintBad, sizeof(short int), 1, imgfile);
            fread(&ucharBad, sizeof(unsigned char), 1, imgfile);
            fread(&sintBad, sizeof(short int), 1, imgfile);
            fread(&sintBad, sizeof(short int), 1, imgfile);

            // Get width and height.
            fread(&img->width, sizeof(short int), 1, imgfile);
            fread(&img->height, sizeof(short int), 1, imgfile);

            // Depth.
            fread(&img->bitCount, sizeof(unsigned char), 1, imgfile);

            // Skip one byte.
            fread(&ucharBad, sizeof(unsigned char), 1, imgfile);

            // Color mode.
            colorMode = img->bitCount / 8;
            imageSize = img->width * img->height * colorMode;

            // IMPORTANT - remove previous data to avoid crashes
            memset(&img->imageData, 0, sizeof(unsigned char));

            img->imageData = (unsigned char*)malloc(sizeof(unsigned char) * imageSize);

            // read the image data
            fread( img->imageData, sizeof(unsigned char), imageSize, imgfile);

            for ( imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode ) {
                colorSwap = img->imageData[imageIdx];
                img->imageData[imageIdx] =  img->imageData[imageIdx + 2];
                img->imageData[imageIdx + 2] = colorSwap;
            }

            fclose(imgfile);

            // race condition??
            // app crashes on its launch sometimes
            //free( img->imageData );

            return true;
            break;

        case BMP_IMAGE:
            
            /* BMP image loader. */
            imgfile = _filesystem.OpenRead( _stringhelper.STR( "%s/%s/%s.bmp", filesystem_path.GetString(), TEXTURE_FOLDER, filename ) );

            if ( !imgfile ) {
                _core.Print( LOG_DEVELOPER, "img_load: Couldn't find %s image\n", filename);
                return false;
            }

            if ( fread(header, 1, 54, imgfile) != 54 ) {
                _core.Print( LOG_DEVELOPER, "img_load: Image %s.bmp is corrupted\n", filename);
                return false;
            }

            if ( header[0] != 'B' || header[1] != 'M' ) {
                _core.Print( LOG_DEVELOPER, "img_load: Image %s is not a BMP.\n", filename);
                return false;
            }

            // Read data from header.
            dataPos    = *(int*)&(header[0x0A]);
            imageSize  = *(int*)&(header[0x22]);
            img->width      = *(int*)&(header[0x12]);
            img->height     = *(int*)&(header[0x16]);

            if ( imageSize == 0 )
                imageSize = img->width * img->height * 3;
            if ( dataPos == 0 )
                dataPos = 54;

            img->imageData = (byte*)malloc(sizeof(byte) * imageSize);
	
            fread( img->imageData, 1, imageSize, imgfile );

            fclose( imgfile );

            return true;
            break;

        default:
            _core.Print( LOG_DEVELOPER, "img_load: Undefined image type\n");
            return false;
            break;
    }

    return true;
}

/*
    Unload texture from a memory for memory safe.
*/
void ncImageLoader::Unload( ncImage *tex ) {
    if( tex->imageData ) {
        glDeleteTextures(1, &tex->tex_id);

        if( tex->imageData )
            free( tex->imageData );
        
        tex->imageData = NULL;

        //memset(&tex, 0, sizeof(image_t));
        tex = NULL;
    }
}
