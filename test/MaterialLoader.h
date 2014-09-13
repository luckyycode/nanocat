//
//  Nanocat engine.
//
//  Material manager..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef material_h
#define material_h

#include "ImageManager.h"

// Checker texture size. ( Used when no texture found.. whatever )
#define NULL_TEXTURE_SIZE                       128             // Checker texture ( no texture ) size.

// Material files.
#define MAX_MATERIALS                           1024

// Material entries ( or just call them "textures" or "images" ).
#define MAX_MATERIAL_CELLS                      1024

/*
    Material.
*/
class ncMaterial {
public:
    char        name[64];
    
    ncImage     texture;
    int         index;
};

class ncMaterialManager {
public:
    void Initialize( void );
    void Load( const char *mat_name );
    void LoadImage( const char *material_name, const char *material_parameter, const char *material_file,
                          ncImageType type );
    ncMaterial Find( const char *entry );
    
    int MaterialCount;
    int MaterialEntryCount;
};

extern ncMaterialManager _materials;

#endif
