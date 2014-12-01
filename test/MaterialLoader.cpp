//
//  Nanocat engine.
//
//  Material manager..
//
//  Created by Neko Vision on 18/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "FileSystem.h"
#include "NCString.h"
#include "MaterialLoader.h"
#include "System.h"

ncMaterialManager local_materialManager;
ncMaterialManager *g_materialManager = &local_materialManager;

/*
    Initialize material system.
*/
void ncMaterialManager::Initialize( void ) {
    // allocate memory for MAX_MATERIALS slots
    m_Materials = new ncMaterial[MAX_MATERIAL_CELLS];
    
    if( !m_Materials ) {
        g_Core->Error( ERR_ASSET, "Could not allocate memory for %i material entries\n", MAX_MATERIAL_CELLS );
        return;
    }

    MaterialEntryCount = 0;
    MaterialCount = 0;

    // Thanks to kimes for checker texture generate method.
    static Byte ci_data1[NULL_TEXTURE_SIZE][NULL_TEXTURE_SIZE][4];

    int i, j, c;

    for( i = 0; i < NULL_TEXTURE_SIZE; i++ ) {
        for( j = 0; j < NULL_TEXTURE_SIZE; j++ ) {
            c = ((( ( i & 0x8 ) == 0 ) ^ (( ( j & 0x8 )) == 0 ))) * 255;
            ci_data1[i][j][0] = 0;
            ci_data1[i][j][1] = c;
            ci_data1[i][j][2] = 0;
            ci_data1[i][j][3] = 128;
        }
    }

    // Set first material as default fill texture.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glGenTextures( 1, &m_Materials[MaterialEntryCount].Image.TextureID );
    glBindTexture( GL_TEXTURE_2D, m_Materials[MaterialEntryCount].Image.TextureID );
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_TEXTUREFILTERING );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTUREFILTERING );
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, NULL_TEXTURE_SIZE, NULL_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, ci_data1 );

    MaterialEntryCount++;
    // Everything went ok.
}

/*
    Load requested material.
    Simple file reading method.
*/
void ncMaterialManager::Load( const NString mat_name ) {
    FILE    *mat_file;
    float   t1, t2;

    if( !mat_name ) {
        g_Core->Error( ERR_ASSET, "Could not load material with empty name." );
        return;
    }

    mat_file = c_FileSystem->OpenRead( NC_TEXT("%s/%s/%s.material", Filesystem_Path.GetString(), MATERIAL_FOLDER, mat_name) );

    if( !mat_file ) {
        // Throw error if file wasn't found.
        g_Core->Error( ERR_ASSET, "ncMaterialManager::Load - Could not find '%s' material.\n", mat_name );
        return;
    }
    else {
        t1 = c_coreSystem->Milliseconds();
    
        NString material_name = new char[32];
        NString material_parameter = new char[16];
        NString material_file = new char[32];
        
        while( 1 ) {
            int res;
            char l_header[128];

            res = fscanf( mat_file, "%s", l_header );

            if ( res == EOF )
                break;

            if( !strcmp( l_header, "-tgau" ) ) {
                fscanf( mat_file, "%s %s %s", material_name, material_parameter, material_file );

                LoadImage( material_name, material_parameter, material_file, NCTGA_IMAGE );
            }
            else if( !strcmp( l_header, "-bmp" ) ) {
                fscanf( mat_file, "%s %s %s", material_name, material_parameter, material_file );

                LoadImage( material_name, material_parameter, material_file, NCBMP_IMAGE );
            }
            else {
                g_Core->Print( LOG_WARN, "Unknown image type for \"%s\"\n", mat_name );
                return;
            }
        }

        delete [] material_name;
        delete [] material_parameter;
        delete [] material_file;
        
        t2 = c_coreSystem->Milliseconds();
        g_Core->Print( LOG_INFO, "Asset with type material \"%s\" took %4.2f msec to load.\n", mat_name, t2 - t1 );
    }

    fclose( mat_file );
}

/*
    Load textures for material.
*/
void ncMaterialManager::LoadImage( const NString material_name, const NString material_parameter, const NString material_file, ncImageType type ) {
    
    int tex_param;

    // Tiling texture.
    if( !strcmp( material_parameter, "-repeat") ) {
        tex_param   = GL_REPEAT;
    }
    // Mirrored repeat.
    else if( !strcmp( material_parameter, "-mrepeat") ) {
        tex_param   = GL_MIRRORED_REPEAT;
    }
    // Clamp to edge.
    else if( !strcmp( material_parameter, "-ctd" ) ) {
        tex_param   = GL_CLAMP_TO_EDGE;
    }
    // Clamp to border.
    else if( !strcmp( material_parameter, "-ctb" ) ) {
        tex_param   = GL_CLAMP_TO_BORDER;
    }
    // By default.
    else {
        tex_param = GL_REPEAT;
    }

    g_stringHelper->Copy( m_Materials[MaterialEntryCount].Name, material_name );
    m_Materials[MaterialEntryCount].Index = MaterialEntryCount;

    if( &m_Materials[MaterialEntryCount].Image ) {
         // I don't know why it's here, just for safe.
        g_imageManager->Unload( &m_Materials[MaterialEntryCount].Image );
    }
    
    // Load an Image.
    if( g_imageManager->Load( type, material_file, &m_Materials[MaterialEntryCount].Image ) ) {

        // Gene
        glDeleteTextures( 1, &m_Materials[MaterialEntryCount].Image.TextureID );
        glGenTextures( 1, &m_Materials[MaterialEntryCount].Image.TextureID );

        glBindTexture( GL_TEXTURE_2D, m_Materials[MaterialEntryCount].Image.TextureID );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Materials[MaterialEntryCount].Image.Width, m_Materials[MaterialEntryCount].Image.Heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, m_Materials[MaterialEntryCount].Image.ImageData);

        // Remove all loaded images here.
        delete [] m_Materials[MaterialEntryCount].Image.ImageData;

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 1 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 1 );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_TEXTUREFILTERING );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTUREFILTERING );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex_param );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex_param );

        glGenerateMipmap( GL_TEXTURE_2D );
        
        MaterialEntryCount++;
    }
    else { // Error.
        g_Core->Print( LOG_ERROR, "ncMaterialManager::LoadImage - Couldn't load \"%s\" material, using default instead.\n", material_name );
        m_Materials[MaterialEntryCount] = m_Materials[0];
        MaterialEntryCount++;
    }
}

/*
    Find material.
*/
ncMaterial *ncMaterialManager::Find( const NString entry ) {
    int i;
    for( i = 0; i < MaterialEntryCount; i++ ) {
        if( !strcmp(m_Materials[i].Name, entry) ) {
            return &m_Materials[i];
        }
    }

    // Texture wasn't found, so use checker texture instead.
    return &m_Materials[0];
}
