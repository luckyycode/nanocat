//
//  Nanocat engine.
//
//  Asset manager.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef assetmanager_h
#define assetmanager_h

#include "shader.h"

enum assettype_t {
    ASSET_MODEL,
    ASSET_SOUND,
    ASSET_SHADER,
    ASSET_MATERIAL,
    ASSET_IMAGE
};

class ncAssetManager {
public:
    
    void Initialize( void );
    void Load( assettype_t type, const char *name );
    void FindShader( const char *name, ncGLShader *shader );
    bool Exists( assettype_t type, char *name );
};

extern ncAssetManager _assetmanager;

#endif
