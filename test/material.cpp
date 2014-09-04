//
//  material.c
//  test
//
//  Created by Neko Vision on 18/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "files.h"
#include "ncstring.h"
#include "material.h"
#include "system.h"

ncMaterialManager _materials;
ncMaterial *m_material;

/*
    Initialize material system.
*/
void ncMaterialManager::Initialize( void ) {
    // allocate memory for MAX_MATERIALS slots
    m_material = (ncMaterial*)malloc( sizeof(ncMaterial) * MAX_MATERIAL_CELLS );

    if( !m_material ) {
        _core.Error( ERC_ASSET, "Could not allocate memory for %i material entries\n", MAX_MATERIAL_CELLS );
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


    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glGenTextures( 1, &m_material[MaterialEntryCount].texture.tex_id );
    glBindTexture( GL_TEXTURE_2D, m_material[MaterialEntryCount].texture.tex_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, NULL_TEXTURE_SIZE,
                 NULL_TEXTURE_SIZE, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, ci_data1 );


    MaterialEntryCount++;
    // Everything went ok.
}

/*
    Load requested material.
    Simple file reading method.
*/
void ncMaterialManager::Load( const char *mat_name ) {
    FILE    *mat_file;
    float   t1, t2;

    if( !mat_name )
        return;

    mat_file = _filesystem.OpenRead( _stringhelper.STR("%s/%s/%s.material", filesystem_path.GetString(), MATERIAL_FOLDER, mat_name) );

    if( !mat_file ) {
        // throw error because it's
        _core.Error( ERC_ASSET, "material_load: Could not find '%s' material.\n", mat_name );
        return;
    }
    else {
        t1 = _system.Milliseconds();

        static char material_name[32], material_parameter[16], material_file[32];

        while( 1 ) {
            int res;
            char l_header[128];

            res = fscanf(mat_file, "%s", l_header);

            if (res == EOF)
                break;

            if( !strcmp( l_header, "-tgau") ) {
                fscanf( mat_file, "%s %s %s", &material_name, &material_parameter, &material_file );

                LoadImage( material_name, material_parameter, material_file, TGA_IMAGE_UNCOMPRESSED );
            }
            else if( !strcmp( l_header, "-bmp") ) {
                fscanf( mat_file, "%s %s %s", &material_name, &material_parameter, &material_file );

                LoadImage( material_name, material_parameter, material_file, BMP_IMAGE );
            }
            else {
                _core.Print( LOG_WARN, "unknown image type for \"%s\"\n", mat_name );
                return;
            }
        }

        t2 = _system.Milliseconds();
        _core.Print( LOG_INFO, "Material asset '%s' loaded ( 0.%i )\n", mat_name, t2 - t1 );
    }

    fclose( mat_file );
}

/*
    Load textures for material.
*/
void ncMaterialManager::LoadImage( const char *material_name, const char *material_parameter, const char *material_file,
                      imagetype_t type ) {
    int tex_param;

    if(!strcmp(material_parameter, "-repeat"))
        tex_param   = GL_REPEAT;
    else if(!strcmp(material_parameter, "-clamp_to_edge"))
        tex_param   = GL_CLAMP_TO_EDGE;

    _stringhelper.Copy(m_material[MaterialEntryCount].name,        material_name);
    m_material[MaterialEntryCount].index                =     MaterialEntryCount;

    if( &m_material[MaterialEntryCount].texture )
        _image.Unload(&m_material[MaterialEntryCount].texture);     // Unload previous texture data.

    if( _image.Load(type, material_file, &m_material[MaterialEntryCount].texture) ) {

        //glEnable(GL_TEXTURE_2D);
        glDeleteTextures(1, &m_material[MaterialEntryCount].texture.tex_id);
        glGenTextures(1, &m_material[MaterialEntryCount].texture.tex_id);

        glBindTexture(GL_TEXTURE_2D, m_material[MaterialEntryCount].texture.tex_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_material[MaterialEntryCount].texture.width, m_material[MaterialEntryCount].texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_material[MaterialEntryCount].texture.imageData);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex_param );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex_param );

        //glDisable(GL_TEXTURE_2D);

        MaterialEntryCount++;
    }
    else                                                                                            // img_load returned FALSE
    {
        _core.Print( LOG_WARN, "Couldn't load \"%s\" material, using default instead.\n", material_name );
        m_material[MaterialEntryCount] = m_material[0];
        MaterialEntryCount++;
    }
}

/*
    Find material.
*/
ncMaterial ncMaterialManager::Find( const char *entry ) {
    int i;
    for( i = 0; i < MaterialEntryCount; i++ )
        if( !strcmp(m_material[i].name, entry) )
            return m_material[i];

    // Texture wasn't found, so use checker texture instead.
    return m_material[0];
}
