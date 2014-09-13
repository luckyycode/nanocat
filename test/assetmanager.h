//
//  Nanocat engine.
//
//  Asset manager..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef assetmanager_h
#define assetmanager_h

#include "ShaderLoader.h"

enum AssetType {
    ASSET_MODEL,
    ASSET_SOUND,
    ASSET_SHADER,
    ASSET_MATERIAL,
    ASSET_IMAGE
};

class ncAssetManager {
public:
    
    void Initialize( void );
    void Load( AssetType type, const char *name );
    void FindShader( const char *name, ncGLShader *shader );
    bool Exists( AssetType type, char *name );
};

extern ncAssetManager _assetmanager;

#endif
