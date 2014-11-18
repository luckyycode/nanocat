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

// Asset types.
enum ncAssetType {
    ASSET_MODEL = 0,
    ASSET_SOUND = 1,
    ASSET_SHADER = 2,
    ASSET_MATERIAL = 3 // I.e image/texture.
};

class ncAssetManager {
public:
    
    void Initialize( void );
    void Load( ncAssetType type, const NString name );
    
    void FindShaderByName( const NString name, ncGLShader *shader );
    ncGLShader *FindShaderByName( const NString name );
    
    bool Exists( ncAssetType type, NString name );
};

extern ncAssetManager *f_AssetManager;

#endif
