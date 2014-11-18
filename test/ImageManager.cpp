//
//  Nanocat engine.
//
//  Image loader..
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


#include "Core.h"
#include "FileSystem.h"
#include "NCString.h"
#include "ImageManager.h"
#include "Renderer.h"

ncImageLoader local_imageManager;
ncImageLoader *g_imageManager = &local_imageManager;

/*
bool LoadUncompressedTGA(ncImage* texture, NString  filename, FILE * fTGA)
{
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)
    {
        if(fTGA != NULL)
        {
            fclose(fTGA);
        }
        return false;
    }
    
    texture->Width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
    texture->Heigth = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
    texture->BitsPerPixel	= tga.header[4];										// Determine the bits per pixel
    tga.Width		= texture->Width;										// Copy width into local structure
    tga.Height		= texture->Heigth;										// Copy height into local structure
    tga.bytesPerPixel			= texture->BitsPerPixel;											// Copy BPP into local structure
    
    if((texture->Width <= 0) || (texture->Heigth <= 0) || ((texture->BitsPerPixel != 24) && (texture->BitsPerPixel !=32)))	// Make sure all information is valid
    {
   
        if(fTGA != NULL)													// Check if file is still open
        {
            fclose(fTGA);													// If so, close it
        }
        return false;														// Return failed
    }
    
    if(texture->BitsPerPixel == 24)													// If the BPP of the image is 24...
        texture->Type	= GL_RGB;											// Set Image type to GL_RGB
    else																	// Else if its 32 BPP
        texture->Type	= GL_RGBA;											// Set image type to GL_RGBA
    
    tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute the number of BYTES per pixel
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute the total amout ofmemory needed to store data
    texture->ImageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory
    
    if(texture->ImageData == NULL)											// If no space was allocated
    {

        fclose(fTGA);														// Close the file
        return false;														// Return failed
    }
    
    if(fread(texture->ImageData, 1, tga.imageSize, fTGA) != tga.imageSize)	// Attempt to read image data
    {

        if(texture->ImageData != NULL)										// If imagedata has data in it
        {
            free(texture->ImageData);										// Delete data from memory
        }
        fclose(fTGA);														// Close file
        return false;														// Return failed
    }
    
    // Byte Swapping Optimized By Steve Thomas
    for(GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
    {
        texture->ImageData[cswap] ^= texture->ImageData[cswap+2] ^=
        texture->ImageData[cswap] ^= texture->ImageData[cswap+2];
    }
    
    fclose(fTGA);															// Close file
    return true;															// Return success
}


bool LoadCompressedTGA(ncImage * texture, NString  filename, FILE * fTGA)		// Load COMPRESSED TGAs
{
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Attempt to read header
    {
        
        if(fTGA != NULL)													// If file is open
        {
            fclose(fTGA);													// Close it
        }
        return false;														// Return failed
    }
    
    texture->Width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
    texture->Heigth = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
    texture->BitsPerPixel	= tga.header[4];										// Determine Bits Per Pixel
    tga.Width		= texture->Width;										// Copy width to local structure
    tga.Height		= texture->Heigth;										// Copy width to local structure
    tga.Bpp			= texture->BitsPerPixel;											// Copy width to local structure
    
    if((texture->Width <= 0) || (texture->Heigth <= 0) || ((texture->BitsPerPixel != 24) && (texture->BitsPerPixel !=32)))	//Make sure all texture info is ok
    {
        if(fTGA != NULL)													// Check if file is open
        {
            fclose(fTGA);													// Ifit is, close it
        }
        return false;														// Return failed
    }
    
    if(texture->BitsPerPixel == 24)													// If the BPP of the image is 24...
        texture->Type	= GL_RGB;											// Set Image type to GL_RGB
    else																	// Else if its 32 BPP
        texture->Type	= GL_RGBA;											// Set image type to GL_RGBA
    
    tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute BYTES per pixel
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute amout of memory needed to store image
    texture->ImageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory
    
    if(texture->ImageData == NULL)											// If it wasnt allocated correctly..
    {
        fclose(fTGA);														// Close file
        return false;														// Return failed
    }
    
    GLuint pixelcount	= tga.Height * tga.Width;							// Nuber of pixels in the image
    GLuint currentpixel	= 0;												// Current pixel being read
    GLuint currentbyte	= 0;												// Current byte
    GLubyte * colorbuffer = (GLubyte *)malloc(tga.bytesPerPixel);			// Storage for 1 pixel
    
    do
    {
        GLubyte chunkheader = 0;											// Storage for "chunk" header
        
        if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)				// Read in the 1 byte header
        {
            if(fTGA != NULL)												// If file is open
            {
                fclose(fTGA);												// Close file
            }
            if(texture->ImageData != NULL)									// If there is stored image data
            {
                free(texture->ImageData);									// Delete image data
            }
            return false;													// Return failed
        }
        
        if(chunkheader < 128)												// If the ehader is < 128, it means the that is the number of RAW color packets minus 1
        {																	// that follow the header
            chunkheader++;													// add 1 to get number of following color values
            for(short counter = 0; counter < chunkheader; counter++)		// Read RAW color values
            {
                if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
                {

                    
                    if(fTGA != NULL)													// See if file is open
                    {
                        fclose(fTGA);													// If so, close file
                    }
                    
                    if(colorbuffer != NULL)												// See if colorbuffer has data in it
                    {
                        free(colorbuffer);												// If so, delete it
                    }
                    
                    if(texture->ImageData != NULL)										// See if there is stored Image data
                    {
                        free(texture->ImageData);										// If so, delete it too
                    }
                    
                    return false;														// Return failed
                }
                // write to memory
                texture->ImageData[currentbyte		] = colorbuffer[2];				    // Flip R and B vcolor values around in the process
                texture->ImageData[currentbyte + 1	] = colorbuffer[1];
                texture->ImageData[currentbyte + 2	] = colorbuffer[0];
                
                if(tga.bytesPerPixel == 4)												// if its a 32 bpp image
                {
                    texture->ImageData[currentbyte + 3] = colorbuffer[3];				// copy the 4th byte
                }
                
                currentbyte += tga.bytesPerPixel;										// Increase thecurrent byte by the number of bytes per pixel
                currentpixel++;															// Increase current pixel by 1
                
                if(currentpixel > pixelcount)											// Make sure we havent read too many pixels
                {
                    
                    
                    if(fTGA != NULL)													// If there is a file open
                    {
                        fclose(fTGA);													// Close file
                    }
                    
                    if(colorbuffer != NULL)												// If there is data in colorbuffer
                    {
                        free(colorbuffer);												// Delete it
                    }
                    
                    if(texture->ImageData != NULL)										// If there is Image data
                    {
                        free(texture->ImageData);										// delete it
                    }
                    
                    return false;														// Return failed
                }
            }
        }
        else																			// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
        {
            chunkheader -= 127;															// Subteact 127 to get rid of the ID bit
            if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)		// Attempt to read following color values
            {
                if(fTGA != NULL)														// If thereis a file open
                {
                    fclose(fTGA);														// Close it
                }
                
                if(colorbuffer != NULL)													// If there is data in the colorbuffer
                {
                    free(colorbuffer);													// delete it
                }
                
                if(texture->ImageData != NULL)											// If thereis image data
                {
                    free(texture->ImageData);											// delete it
                }
                
                return false;															// return failed
            }
            
            for(short counter = 0; counter < chunkheader; counter++)					// copy the color into the image data as many times as dictated
            {																			// by the header
                texture->ImageData[currentbyte		] = colorbuffer[2];					// switch R and B bytes areound while copying
                texture->ImageData[currentbyte + 1	] = colorbuffer[1];
                texture->ImageData[currentbyte + 2	] = colorbuffer[0];
                
                if(tga.bytesPerPixel == 4)												// If TGA images is 32 bpp
                {
                    texture->ImageData[currentbyte + 3] = colorbuffer[3];				// Copy 4th byte
                }
                
                currentbyte += tga.bytesPerPixel;										// Increase current byte by the number of bytes per pixel
                currentpixel++;															// Increase pixel count by 1
                
                if(currentpixel > pixelcount)											// Make sure we havent written too many pixels
                {
                    if(fTGA != NULL)													// If there is a file open
                    {
                        fclose(fTGA);													// Close file
                    }	
                    
                    if(colorbuffer != NULL)												// If there is data in colorbuffer
                    {
                        free(colorbuffer);												// Delete it
                    }
                    
                    if(texture->ImageData != NULL)										// If there is Image data
                    {
                        free(texture->ImageData);										// delete it
                    }
                    
                    return false;														// Return failed
                }
            }
        }
    }
    
    while(currentpixel < pixelcount);													// Loop while there are still pixels left
    fclose(fTGA);																		// Close the file
    return true;																		// return success
}

bool ncImageLoader::LoadTGA( const NString name, ncImage *image ) {
    
    if( !name ) {
        g_Core->Print( LOG_WARN, "Empty image name given\n" );
        return false;
    }
    
    g_Core->Print( LOG_INFO, "%s/%s/%s\n", Filesystem_Path.GetString(), TEXTURE_FOLDER, name  );
    FILE *img = c_FileSystem->OpenRead(  _stringhelper.STR( "%s/%s/%s", Filesystem_Path.GetString(), TEXTURE_FOLDER, name ) );
    c_FileSystem->Read( img, (void**)&tgaheader, sizeof(tgaheader) );

    if( memcmp( uTGAcompare, &tgaheader, sizeof(tgaheader) ) ) {
        // Uncompressed.
        g_Core->Print( LOG_NONE, "ncImageLoader::LoadTGA - %s is uncompressed.\n", name );
        
    } else if( memcmp( cTGAcompare, &tgaheader, sizeof(tgaheader) ) ) {
        // Compressed.
        g_Core->Print( LOG_NONE, "ncImageLoader::LoadTGA - %s is compressed.\n", name );
 
    } else {
        // Not TGA.
        g_Core->Error( ERC_ASSET, "ncImageLoader::LoadTGA - %s is not an targa image\n" );
    }
    
    
    return true;
}
*/
 
 
/*
    Write image.
*/
bool ncImageLoader::CreateImage( int width, int height, byte *data, ncImageType type, const NString filename ) {
    
    switch( type ){
        case NCBMP_IMAGE: {
                FILE *f;
            
                f = fopen( _stringhelper.STR( "%s/%s.bmp", Filesystem_Path.GetString(), filename), "wb" );
                if( !f ) {
                    g_Core->Error( ERR_ASSET, "ncImageLoader::CreateImage - Couldn't create %s image.\n", filename );
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
            
                g_Core->Print( LOG_INFO, "%s.bmp successfully created.\n", filename );
            }
            break;
        case NCTGA_IMAGE:
            g_Core->Print( LOG_WARN, "ncImageLoader::CreateImage(TGA_IMAGE_UNCOMPRESSED) - Implement me!\n" );
            break;
        default:
            g_Core->Error( ERR_ASSET, "ncImageLoader::CreateImage - Undefined image type given while loading %s.\n", filename );
            break;
            
    }
    
    return true;
}


bool ncImageLoader::LoadBMP( const NString filename, ncImage *img ) {
    FILE    *imgfile;
    
    Byte            header[54];
    unsigned int    dataPos;
    unsigned int    imageSize;
    
    // BMP image.
    imgfile = c_FileSystem->OpenRead( _stringhelper.STR( "%s/%s/%s.bmp", Filesystem_Path.GetString(), TEXTURE_FOLDER, filename ) );
    
    if( !imgfile ) {
        g_Core->Print( LOG_ERROR, "ncImageLoader::Load - Couldn't find %s image\n", filename);
        return false;
    }
    
    if( fread( header, 1, 54, imgfile ) != 54 ) {
        g_Core->Print( LOG_ERROR, "ncImageLoader::Load - %s.bmp is corrupted\n", filename);
        
        fclose( imgfile );
        return false;
    }
    
    if( header[0] != 'B' || header[1] != 'M' ) {
        g_Core->Print( LOG_ERROR, "ncImageLoader::Load - Image %s is not a BMP.\n", filename);
        
        fclose( imgfile );
        return false;
    }
    
    // Read data from header.
    dataPos = *(int*) & (header[0x0A]);
    imageSize = *(int*) & (header[0x22]);
    img->Width = *(int*) & (header[0x12]);
    img->Heigth = *(int*) & (header[0x16]);
    
    // Reset to default.
    if( imageSize == 0 )
        imageSize = img->Width * img->Heigth * 3;
    
    if( dataPos == 0 )
        dataPos = 54;
    
    img->ImageData = new Byte[imageSize];
    
    fread( img->ImageData, 1, imageSize, imgfile );
    
    fclose( imgfile );
    
    return true;

}

/*
    Load image.
*/

bool ncImageLoader::Load( ncImageType type, const NString filename, ncImage *img ) {
    
    FILE    *imgfile;

    Byte            header[54];
    unsigned int    dataPos;
    unsigned int    imageSize;

    Byte            ucharBad;
    Byte            colorSwap;
    
    short int       sintBad;
    int             colorMode;
    int             imageIdx;

    switch( type ) {
            
        // Load uncompressed targa image.
        case NCTGA_IMAGE:

            imgfile = c_FileSystem->OpenRead( _stringhelper.STR( "%s/%s/%s.tga", Filesystem_Path.GetString(), TEXTURE_FOLDER, filename ) );

            if( !imgfile ) {
                g_Core->Print( LOG_ERROR, "ncImageLoader::Load - Couldn't find %s image.\n", filename );
                return false;
            }

            fread( &ucharBad, sizeof(Byte), 1, imgfile );
            fread( &ucharBad, sizeof(Byte), 1, imgfile );
            fread( &img->Type, sizeof(Byte), 1, imgfile );

            if( img->Type != 2 && img->Type != 3 ) {
                g_Core->Print( LOG_ERROR, "ncImageLoader::Load - %s.tga is corrupted\n", filename );
                
                // Close file now.
                fclose(imgfile);
                return false;
            }

            // Ignore useless bytes.
            fread( &sintBad, sizeof(short int), 1, imgfile );
            fread( &sintBad, sizeof(short int), 1, imgfile );
            fread( &ucharBad, sizeof(Byte), 1, imgfile );
            fread( &sintBad, sizeof(short int), 1, imgfile );
            fread( &sintBad, sizeof(short int), 1, imgfile );

            // Get width and height.
            fread( &img->Width, sizeof(short int), 1, imgfile );
            fread( &img->Heigth, sizeof(short int), 1, imgfile );

            // Depth.
            fread( &img->BitsPerPixel, sizeof(Byte), 1, imgfile );

            // Skip one byte.
            fread( &ucharBad, sizeof(Byte), 1, imgfile );

            // Color mode.
            colorMode = img->BitsPerPixel / 8;
            imageSize = img->Width * img->Heigth * colorMode;

            // IMPORTANT - remove previous data to avoid crashes
            memset( &img->ImageData, 0, sizeof(Byte) );

            img->ImageData = new Byte[imageSize];

            // Read the image data.
            fread( img->ImageData, sizeof(Byte), imageSize, imgfile );

            for( imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode ) {
                colorSwap = img->ImageData[imageIdx];
                
                img->ImageData[imageIdx] =  img->ImageData[imageIdx + 2];
                img->ImageData[imageIdx + 2] = colorSwap;
            }

            // Don't free imageData here.
            // Will be freed in material manager.
            
            fclose( imgfile );

            return true;

        case NCBMP_IMAGE:
            
            // BMP image.
            imgfile = c_FileSystem->OpenRead( _stringhelper.STR( "%s/%s/%s.bmp", Filesystem_Path.GetString(), TEXTURE_FOLDER, filename ) );

            if( !imgfile ) {
                g_Core->Print( LOG_ERROR, "ncImageLoader::Load - Couldn't find %s image\n", filename);
                return false;
            }

            if( fread( header, 1, 54, imgfile ) != 54 ) {
                g_Core->Print( LOG_ERROR, "ncImageLoader::Load - %s.bmp is corrupted\n", filename);
                
                fclose( imgfile );
                return false;
            }

            if( header[0] != 'B' || header[1] != 'M' ) {
                g_Core->Print( LOG_ERROR, "ncImageLoader::Load - Image %s is not a BMP.\n", filename);
                
                fclose( imgfile );
                return false;
            }

            // Read data from header.
            dataPos = *(int*) & (header[0x0A]);
            imageSize = *(int*) & (header[0x22]);
            img->Width = *(int*) & (header[0x12]);
            img->Heigth = *(int*) & (header[0x16]);

            // Reset to default.
            if( imageSize == 0 )
                imageSize = img->Width * img->Heigth * 3;
            
            if( dataPos == 0 )
                dataPos = 54;

            img->ImageData = new Byte[imageSize];
	
            fread( img->ImageData, 1, imageSize, imgfile );

            fclose( imgfile );

            return true;
        default:
            g_Core->Print( LOG_DEVELOPER, "ncImageLoader::Load - Undefined image type given while loading %s.\n", filename );
            return false;
    }

    return true;
}

/*
    Unload texture from a memory for memory safe.
*/
void ncImageLoader::Unload( ncImage *tex ) {
    if( tex->ImageData ) {
        glDeleteTextures( 1, &tex->TextureID );

        delete [] tex->ImageData;
        
        tex->ImageData = NULL;
        tex = NULL;
    }
}
