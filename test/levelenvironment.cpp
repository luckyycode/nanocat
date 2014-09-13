//
//  Nanocat engine.
//
//  Game world environment..
//
//  Created by Neko Vision on 3/5/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ncBSP.h"
#include "NCString.h"
#include "LevelEnvironment.h"

ncLevelEnvironment _levelenvironment;

struct ncBaseLight
{
    ncVec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
    
    ncBaseLight()
    {
        Color = ncVec3(0.0f, 0.0f, 0.0f);
        AmbientIntensity = 0.0f;
        DiffuseIntensity = 0.0f;
    }
};

struct ncDirectionalLight : public ncBaseLight
{
    ncVec3 Direction;
    
    ncDirectionalLight()
    {
        Direction = ncVec3(0.0f, 0.0f, 0.0f);
    }
};

struct ncPointLight : public ncBaseLight
{
    ncVec3 Position;
    
    struct
    {
        float Constant;
        float Linear;
        float Exp;
    } Attenuation;
    
    ncPointLight()
    {
        Position = ncVec3(0.0f, 0.0f, 0.0f);
        Attenuation.Constant = 1.0f;
        Attenuation.Linear = 0.0f;
        Attenuation.Exp = 0.0f;
    }
};



void ncLevelEnvironment::Prepare( void ) {
    ncPointLight pl[2];
    pl[0].DiffuseIntensity = 2.5f;
    pl[0].Color = ncVec3(1.0f, 1.0f, 1.0f);
    pl[0].Position = ncVec3(-5.0, 27.0, -5.0);
    pl[0].Attenuation.Linear = 0.1f;
    pl[1].DiffuseIntensity = 0.5f;
    pl[1].Color = ncVec3(0.0f, 0.0f, 0.5f);
    pl[1].Position = ncVec3(0.0f, -5.0f, 20.0f);
    pl[1].Attenuation.Linear = 0.1f;
    
    glUseProgram( _bspmngr.bspShader.shader_id );
    for( int i = 0; i < 2; i++ ) {
        glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Base.Color", i)), pl[i].Color.x, pl[i].Color.y, pl[i].Color.z );
        
        glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Base.AmbientIntensity", i)), pl[i].AmbientIntensity );

        glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Position", i)), pl[i].Position.x, pl[i].Position.y, pl[i].Position.z );
        
        glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Base.DiffuseIntensity", i)), pl[i].DiffuseIntensity );

        glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Atten.Constant", i)), pl[i].Attenuation.Constant );
        
        glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Atten.Linear", i)), pl[i].Attenuation.Linear );
        
        glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("gPointLights[%d].Atten.Exp", i)), pl[i].Attenuation.Exp );
    }
    
    ncVec3 LightDir( 1.0f, -1.0f, 1.0f );
    LightDir.Normalize();
    
    glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Direction"),  LightDir.x, LightDir.y, LightDir.z );
    glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.Color"), 0.9f, 1.0f, 1.0f );
    glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.AmbientIntensity"), 0.7f );
    glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.DiffuseIntensity"), 0.001f );
    
    glUseProgram(0);
}

void ncLevelEnvironment::PassShader( uint *id ) {
    ncPointLight pl[2];
    pl[0].DiffuseIntensity = 2.5f;
    pl[0].Color = ncVec3(1.0f, 1.0f, 1.0f);
    pl[0].Position = ncVec3(-5.0, 27.0, -5.0);
    pl[0].Attenuation.Linear = 0.1f;
    pl[1].DiffuseIntensity = 0.5f;
    pl[1].Color = ncVec3(0.0f, 0.0f, 0.5f);
    pl[1].Position = ncVec3(0.0f, -5.0f, 20.0f);
    pl[1].Attenuation.Linear = 0.1f;
    
    //glUseProgram( *id );
    for( int i = 0; i < 2; i++ ) {
        glUniform3f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Base.Color", i)), pl[i].Color.x, pl[i].Color.y, pl[i].Color.z );
        
        glUniform1f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Base.AmbientIntensity", i)), pl[i].AmbientIntensity );
        
        glUniform3f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Position", i)), pl[i].Position.x, pl[i].Position.y, pl[i].Position.z );
        
        glUniform1f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Base.DiffuseIntensity", i)), pl[i].DiffuseIntensity );
        
        glUniform1f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Atten.Constant", i)), pl[i].Attenuation.Constant );
        
        glUniform1f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Atten.Linear", i)), pl[i].Attenuation.Linear );
        
        glUniform1f( glGetUniformLocation( *id, _stringhelper.STR("gPointLights[%d].Atten.Exp", i)), pl[i].Attenuation.Exp );
    }
    
    ncVec3 LightDir( 1.0f, -1.0f, 1.0f );
    LightDir.Normalize();
    
    glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Direction"),  LightDir.x, LightDir.y, LightDir.z );
    glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.Color"), 0.9f, 1.0f, 1.0f );
    glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.AmbientIntensity"), 0.7f );
    glUniform1f( glGetUniformLocation( _bspmngr.bspShader.shader_id, "gDirectionalLight.Base.DiffuseIntensity"), 0.001f );
    
    //glUseProgram(0);
}
