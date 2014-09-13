//
//  Nanocat engine.
//
//  GameMath library..
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

#include "Core.h"
#include "Camera.h" // For Matrices and another view stuff.
#include "SystemShared.h"
/* Took me two nights to write this... Aaaaand... */
/* Oh my gosh, I can't believe.. I finally have something like this... */

/*
    Initialize game math.
*/
void gamemath_init( void ) {
    _core.Print( LOG_INFO, "GameMath initializing..\n" );
    // Nothing to initialize for now, lol.
}

/*
    Matrix4x4 stuff.
*/
void ncMatrix4::Identity( void ) {
    m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
    m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f; m[13] = 0.0f;
    m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

void ncMatrix4::Translate( ncVec3 v ) {
    this->Identity();
    
    m[12] = v.x;
    m[13] = v.y;
    m[14] = v.z;
}

void ncMatrix4::TranslateX( float v ) {
    this->Identity();
    m[12] = v;
}

void ncMatrix4::TranslateY( float v ) {
    this->Identity();
    m[13] = v;
}

void ncMatrix4::TranslateZ( float v ) {
    this->Identity();
    m[14] = v;
}

void ncMatrix4::Rotate( float angle, ncVec3 axis ) {
    float s = sin( DEGTORAD(angle) );
    float c = cos( DEGTORAD(angle) );
    
    axis.Normalize();
    
    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
    
    m[0]  = c + (1-c) * ux;
    m[1]  = (1-c) * ux*uy + s*uz;
    m[2]  = (1-c) * ux*uz - s*uy;
    m[3]  = 0;
    
    m[4]  = (1-c) * uy*ux - s*uz;
    m[5]  = c + (1-c) * pow(uy,2);
    m[6]  = (1-c) * uy*uz + s*ux;
    m[7]  = 0;
    
    m[8]  = (1-c) * uz*ux + s*uy;
    m[9]  = (1-c) * uz*uz - s*ux;
    m[10] = c + (1-c) * pow(uz,2);
    m[11] = 0;
    
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

void ncMatrix4::RotateX( float angle ) {
    float s = sin( DEGTORAD(angle) );
    float c = cos( DEGTORAD(angle) );
    
    this->Identity();
    
    m[5]  =  c;
    m[6]  =  s;
    m[9]  = -s;
    m[10] =  c;
}

void ncMatrix4::RotateY( float angle ) {
    float s = sin( DEGTORAD(angle) );
    float c = cos( DEGTORAD(angle) );
    
    this->Identity();
    
    m[0]  =  c;
    m[2]  = -s;
    m[8]  =  s;
    m[10] =  c;
}

void ncMatrix4::RotateZ( float angle ) {
    float s = sin( DEGTORAD(angle) );
    float c = cos( DEGTORAD(angle) );
    
    this->Identity();
    
    m[0] =  c;
    m[1] =  s;
    m[4] = -s;
    m[5] =  c;
}

void ncMatrix4::Scale( ncVec3 v ) {
    m[0] = v.x;
    m[5] = v.y;
    m[10] = v.z;
}

void Matrix4_TransformPoint( ncMatrix4 *mat,
                         ncVec3 *vec ) {
    float x = vec->x;
    float y = vec->y;
    float z = vec->z;
    
    vec->x = x * mat->m[0] + y * mat->m[4] +
    z * mat->m[8] + mat->m[12];
    
    vec->y = x * mat->m[1] + y * mat->m[5] +
    z * mat->m[9] + mat->m[13];
    
    vec->z = x * mat->m[2] +
    y * mat->m[6] +
    z * mat->m[10]+ mat->m[14];
}

void Matrix4_TransformVector( ncMatrix4 *mat,
                          ncVec3 *vec ) {
    float x = vec->x;
    float y = vec->y;
    float z = vec->z;
    
    vec->x = x * mat->m[0] + y * mat->m[4] +
    z * mat->m[8];
    
    vec->y = x * mat->m[1] + y * mat->m[5] +
    z * mat->m[9];
    
    vec->z = x * mat->m[2] + y * mat->m[6] +
    z * mat->m[10];
}

ncMatrix4 ncMatrix4::operator-(const ncMatrix4 &v) const {
    ncMatrix4 result;
    
    result.m[0]  = m[0]  - v.m[0];
    result.m[1]  = m[1]  - v.m[1];
    result.m[2]  = m[2]  - v.m[2];
    result.m[3]  = m[3]  - v.m[3];
    
    result.m[4]  = m[4]  - v.m[4];
    result.m[5]  = m[5]  - v.m[5];
    result.m[6]  = m[6]  - v.m[6];
    result.m[7]  = m[7]  - v.m[7];
    
    result.m[8]  = m[8]  - v.m[8];
    result.m[9]  = m[9]  - v.m[9];
    result.m[10] = m[10] - v.m[10];
    result.m[11] = m[11] - v.m[11];
    
    result.m[12] = m[12] - v.m[12];
    result.m[13] = m[13] - v.m[13];
    result.m[14] = m[14] - v.m[14];
    result.m[15] = m[15] - v.m[15];
    
    return result;
}

ncMatrix4 ncMatrix4::operator+(const ncMatrix4 &v) const {
    ncMatrix4 result;
    
    result.m[0]  = m[0]  + v.m[0];
    result.m[1]  = m[1]  + v.m[1];
    result.m[2]  = m[2]  + v.m[2];
    result.m[3]  = m[3]  + v.m[3];
    
    result.m[4]  = m[4]  + v.m[4];
    result.m[5]  = m[5]  + v.m[5];
    result.m[6]  = m[6]  + v.m[6];
    result.m[7]  = m[7]  + v.m[7];
    
    result.m[8]  = m[8]  + v.m[8];
    result.m[9]  = m[9]  + v.m[9];
    result.m[10] = m[10] + v.m[10];
    result.m[11] = m[11] + v.m[11];
    
    result.m[12] = m[12] + v.m[12];
    result.m[13] = m[13] + v.m[13];
    result.m[14] = m[14] + v.m[14];
    result.m[15] = m[15] + v.m[15];
    
    return result;
}


/*
    Multiply two matrices.
*/
ncMatrix4 ncMatrix4::operator*( const ncMatrix4 &v ) const {
    ncMatrix4 result;
    
    result.m[0]  = (m[0] * v.m[0]) + (m[4] * v.m[1]) + (m[8] * v.m[2]) + (m[12] * v.m[3]);
    result.m[1]  = (m[1] * v.m[0]) + (m[5] * v.m[1]) + (m[9] * v.m[2]) + (m[13] * v.m[3]);
    result.m[2]  = (m[2] * v.m[0]) + (m[6] * v.m[1]) + (m[10] * v.m[2]) + (m[14] * v.m[3]);
    result.m[3]  = (m[3] * v.m[0]) + (m[7] * v.m[1]) + (m[11] * v.m[2]) + (m[15] * v.m[3]);
    
    result.m[4]  = (m[0] * v.m[4]) + (m[4] * v.m[5]) + (m[8] * v.m[6]) + (m[12] * v.m[7]);
    result.m[5]  = (m[1] * v.m[4]) + (m[5] * v.m[5]) + (m[9] * v.m[6]) + (m[13] * v.m[7]);
    result.m[6]  = (m[2] * v.m[4]) + (m[6] * v.m[5]) + (m[10] * v.m[6]) + (m[14] * v.m[7]);
    result.m[7]  = (m[3] * v.m[4]) + (m[7] * v.m[5]) + (m[11] * v.m[6]) + (m[15] * v.m[7]);
    
    result.m[8]  = (m[0] * v.m[8]) + (m[4] * v.m[9]) + (m[8] * v.m[10]) + (m[12] * v.m[11]);
    result.m[9]  = (m[1] * v.m[8]) + (m[5] * v.m[9]) + (m[9] * v.m[10]) + (m[13] * v.m[11]);
    result.m[10] = (m[2] * v.m[8]) + (m[6] * v.m[9]) + (m[10] * v.m[10]) + (m[14] * v.m[11]);
    result.m[11] = (m[3] * v.m[8]) + (m[7] * v.m[9]) + (m[11] * v.m[10]) + (m[15] * v.m[11]);
    
    result.m[12] = (m[0] * v.m[12]) + (m[4] * v.m[13]) + (m[8] * v.m[14]) + (m[12] * v.m[15]);
    result.m[13] = (m[1] * v.m[12]) + (m[5] * v.m[13]) + (m[9] * v.m[14]) + (m[13] * v.m[15]);
    result.m[14] = (m[2] * v.m[12]) + (m[6] * v.m[13]) + (m[10] * v.m[14]) + (m[14] * v.m[15]);
    result.m[15] = (m[3] * v.m[12]) + (m[7] * v.m[13]) + (m[11] * v.m[14]) + (m[15] * v.m[15]);
    
    return result;
}

ncMatrix4 ncMatrix4::operator*( const float f ) const {
    ncMatrix4 result;
    
    result.m[0]  = m[0]  * f;
    result.m[1]  = m[1]  * f;
    result.m[2]  = m[2]  * f;
    result.m[3]  = m[3]  * f;
    
    result.m[4]  = m[4]  * f;
    result.m[5]  = m[5]  * f;
    result.m[6]  = m[6]  * f;
    result.m[7]  = m[7]  * f;
    
    result.m[8]  = m[8]  * f;
    result.m[9]  = m[9]  * f;
    result.m[10] = m[10] * f;
    result.m[11] = m[11] * f;
    
    result.m[12] = m[12] * f;
    result.m[13] = m[13] * f;
    result.m[14] = m[14] * f;
    result.m[15] = m[15] * f;
    
    return result;
}


/*
    Inverse matrix.
*/
void ncMatrix4::Inverse( void ) {

#define MAT(this, r, c) this->m[c * 4 + r]
    
#define m11 MAT(this, 0, 0)
#define m12 MAT(this, 0, 1)
#define m13 MAT(this, 0, 2)
#define m14 MAT(this, 0, 3)
#define m21 MAT(this, 1, 0)
#define m22 MAT(this, 1, 1)
#define m23 MAT(this, 1, 2)
#define m24 MAT(this, 1, 3)
#define m31 MAT(this, 2, 0)
#define m32 MAT(this, 2, 1)
#define m33 MAT(this, 2, 2)
#define m34 MAT(this, 2, 3)
#define m41 MAT(this, 3, 0)
#define m42 MAT(this, 3, 1)
#define m43 MAT(this, 3, 2)
#define m44 MAT(this, 3, 3)
    
    float d12 = (m31 * m42 - m41 * m32);
    float d13 = (m31 * m43 - m41 * m33);
    float d23 = (m32 * m43 - m42 * m33);
    float d24 = (m32 * m44 - m42 * m34);
    float d34 = (m33 * m44 - m43 * m34);
    float d41 = (m34 * m41 - m44 * m31);
    
    float tmp[16];
    
    tmp[0] =  (m22 * d34 - m23 * d24 + m24 * d23);
    tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
    tmp[2] =  (m21 * d24 + m22 * d41 + m24 * d12);
    tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);
    
    // Compute determinant.
    float det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];
    
    // Singularity test.
    if( det == 0.0 ) {
        _core.Print( LOG_WARN, "GameMath: Call to InverseMatrix produced a singular matrix.\n" );
        
        float identity[16] = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
        
        memcpy( this, identity, 16 * sizeof(float) );
    }
    else {
        float invDet = 1.0f / det;
        
        tmp[0] *= invDet;
        tmp[1] *= invDet;
        tmp[2] *= invDet;
        tmp[3] *= invDet;
        
        tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
        tmp[5] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
        tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
        tmp[7] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;
        
        d12 = m11 * m22 - m21 * m12;
        d13 = m11 * m23 - m21 * m13;
        d23 = m12 * m23 - m22 * m13;
        d24 = m12 * m24 - m22 * m14;
        d34 = m13 * m24 - m23 * m14;
        d41 = m14 * m21 - m24 * m11;
        
        tmp[8]  =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
        tmp[9]  = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
        tmp[10] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
        tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
        tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
        tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
        tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
        tmp[15] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
        
        memcpy( this, tmp, 16 * sizeof(float) );
    }
    
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}

void ncMatrix4::Transpose( void ) {
    m[0]  = m[0];
    m[1]  = m[4];
    m[2]  = m[8];
    m[3]  = m[12];
    
    m[4]  = m[1];
    m[5]  = m[5];
    m[6]  = m[9];
    m[7]  = m[13];
    
    m[8]  = m[2];
    m[9]  = m[6];
    m[10] = m[10];
    m[11] = m[14];
    
    m[12] = m[3];
    m[13] = m[7];
    m[14] = m[11];
    m[15] = m[15];
}

/*
 
    Vector stuff.
 
*/

void ncVec4::Inverse( ncVec4& v ) {
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    v.w = -v.w;
}

void ncVec3::Inverse( void ) {
    x = -x;
    y = -y;
    z = -z;
}

ncVec3 ncVec3::operator*( const ncVec3 &v ) const {
    ncVec3 vResult;

    vResult.x = x * v.x;
    vResult.y = y * v.y;
    vResult.z = z * v.z;
    
    return vResult;
}

ncVec3 ncVec3::operator*( const float v ) const {
    ncVec3 vResult;
    
    vResult.x = x * v;
    vResult.y = y * v;
    vResult.z = z * v;
    
    return vResult;
}

float ncVec3_Dot( ncVec3 v1, ncVec3 v2 ) {
    return( v1.x * v2.x + v1.y * v2.y + v1.z * v2.z  );
}

ncVec3 ncVec3::operator-(const ncVec3& v) const {
    ncVec3 vResult;
    
    vResult.x = x - v.x;
    vResult.y = y - v.y;
    vResult.z = z - v.z;
    
    return vResult;
}

ncVec3 ncVec3::operator+(const ncVec3& v) const {
    ncVec3 vResult;
    
    vResult.x = x + v.x;
    vResult.y = y + v.y;
    vResult.z = z + v.z;
    
    return vResult;
}

void ncVec3::Cross( ncVec3 v1, ncVec3 v2 ) {
    x = v1.y * v2.z - v1.z * v2.y;
    y = v1.z * v2.x - v1.x * v2.z;
    z = v1.x * v2.y - v1.y * v2.x;
}

void ncVec3::Normalize( void ) {
    float fLength = Length();
    
     if( fLength == 1 || fLength == 0 )
         return;
    
    x = x / fLength;
    y = y / fLength;
    z = z / fLength;
}

ncVec3 ncVec3::operator/( const float v ) const {
    ncVec3 vResult;
    vResult.x = x / v;
    vResult.y = y / v;
    vResult.z = z / v;
    
    return vResult;
}

float ncVec3::Length( void ) {
    return( (float)sqrt( x * x + y * y + z * z ) );
}


/*
        View stuff.
*/

void ncMatrix4::CreatePerspective( float fovy, float aspect_ratio, float near_plane, float far_plane ) {

    this->Identity();
    
    m[15] = 0;
    
    float y_scale = Math_Cotangent( (fovy * ANG2RAD) / 2 );
    float x_scale = y_scale / aspect_ratio;
    float frustum_length = far_plane - near_plane;
    
    m[ 0] = x_scale;
    m[ 5] = y_scale;
    m[10] = -( (far_plane + near_plane) / frustum_length);
    m[11] = -1;
    m[14] = -( (2 * near_plane * far_plane) / frustum_length );
}

void ncMatrix4::CreateOrtho( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax ) {
    this->Identity();
    
    m[0] = 2.0 / (xMax - xMin);
    m[5] = 2.0 / (yMax - yMin);
    m[10] = -2.0 / (zMax - zMin);
    m[12] = -((xMax + xMin)/(xMax - xMin));
    m[13] = -((yMax + yMin)/(yMax - yMin));
    m[14] = -((zMax + zMin)/(zMax - zMin));
    m[15] = 1.0;
}

/*
        Frustum functions.
*/
ncFrustum _frustum;

void Frustum_Update( void ) {
    ncMatrix4 mvp;

    int i;
    
    // Calculate clip matrix.
    mvp = _camera.ProjectionMatrix * _camera.ViewMatrix;
    
    _frustum.planes[RIGHT_PLANE].normal.x = mvp.m[3] - mvp.m[0];
    _frustum.planes[RIGHT_PLANE].normal.y = mvp.m[7] - mvp.m[4];
    _frustum.planes[RIGHT_PLANE].normal.z = mvp.m[11] - mvp.m[8];
    _frustum.planes[RIGHT_PLANE].intercept = mvp.m[15] - mvp.m[12];
    
    _frustum.planes[LEFT_PLANE].normal.x = mvp.m[3] + mvp.m[0];
    _frustum.planes[LEFT_PLANE].normal.y = mvp.m[7] + mvp.m[4];
    _frustum.planes[LEFT_PLANE].normal.z = mvp.m[11] + mvp.m[8];
    _frustum.planes[LEFT_PLANE].intercept = mvp.m[15] + mvp.m[12];
    
    _frustum.planes[BOTTOM_PLANE].normal.x = mvp.m[3] + mvp.m[1];
    _frustum.planes[BOTTOM_PLANE].normal.y = mvp.m[7] + mvp.m[5];
    _frustum.planes[BOTTOM_PLANE].normal.z = mvp.m[11] + mvp.m[9];
    _frustum.planes[BOTTOM_PLANE].intercept = mvp.m[15] + mvp.m[13];
    
    _frustum.planes[TOP_PLANE].normal.x = mvp.m[3] - mvp.m[1];
    _frustum.planes[TOP_PLANE].normal.y = mvp.m[7] - mvp.m[5];
    _frustum.planes[TOP_PLANE].normal.z = mvp.m[11] - mvp.m[9];
    _frustum.planes[TOP_PLANE].intercept = mvp.m[15] - mvp.m[13];
    
    _frustum.planes[FAR_PLANE].normal.x = mvp.m[3] - mvp.m[2];
    _frustum.planes[FAR_PLANE].normal.y = mvp.m[7] - mvp.m[6];
    _frustum.planes[FAR_PLANE].normal.z = mvp.m[11] - mvp.m[10];
    _frustum.planes[FAR_PLANE].intercept=mvp.m[15] - mvp.m[14];
    
    _frustum.planes[NEAR_PLANE].normal.x = mvp.m[3] + mvp.m[2];
    _frustum.planes[NEAR_PLANE].normal.y = mvp.m[7] + mvp.m[6];
    _frustum.planes[NEAR_PLANE].normal.z = mvp.m[11] + mvp.m[10];
    _frustum.planes[NEAR_PLANE].intercept = mvp.m[15] + mvp.m[14];
    
    for( i = 0; i < 6; ++i )
        _frustum.planes[i].Normalize();
}

/*
    Check if point inside.
*/
bool Frustum_IsPointInside( const ncVec3 point ) {
    int i;
    for( i = 0; i < 6; ++i ) {
        if( _frustum.planes[i].ClassifyPoint( point ) == POINT_BEHIND_PLANE )
            return false;
    }
    
    return true;
}

/*
    Check if bounding box inside.
*/
bool Frustum_IsBoxInside( ncVec3 *points ) {
    
    int i;
    for( i = 0; i < 6; ++i ) {
        if( _frustum.planes[i].ClassifyPoint( points[0] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[1] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[2] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[3] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[4] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[5] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[6] ) != POINT_BEHIND_PLANE )
            continue;
        if( _frustum.planes[i].ClassifyPoint( points[7] ) != POINT_BEHIND_PLANE )
            continue;
        
        // None inside.
        return false;
    }
    
    return true;
}

/*
    Plane helpers.
*/

/*
    Check point plane.
*/
int ncPlane::ClassifyPoint( ncVec3 point ) {
    
    float distance = point.x * normal.x + point.y * normal.y + point.z * normal.z + intercept;
    
    if( distance > EPSILON )
        return POINT_IN_FRONT_OF_PLANE;
    
    if( distance <- EPSILON )
        return POINT_BEHIND_PLANE;
    
    return POINT_ON_PLANE;
}

/*
    Normalize plane.
*/
void ncPlane::Normalize() {
    float normalLength = normal.Length();
    
    normal.x = normal.x / normalLength;
    normal.y = normal.y / normalLength;
    normal.z = normal.z / normalLength;
    
    intercept /= normalLength;
}

/*
    Tools and utilities.
*/

int Math_Sign( int x ) {
    return ( x > 0 ? +1 : ( x < 0 ? -1 : 0 ) );
}

float Math_Cotangent( float angle ) {
    return (float)(1.0 / tan(angle));
}

/*
    Find distance between two vectors.
*/
float Math_Vec3Distance( ncVec3 s1, ncVec3 s2 ) {
    float x = s2.x - s1.x;
    float y = s2.y - s1.y;
    float z = s2.z - s1.z;
    return sqrtf(x * x + y * y + z * z);
}

/*
    Round the float value.
*/
int Math_Roundf( float value ) {
    return (int)(value + 0.5);
}

/*
    Lerp.
*/
float Math_Lerpf( float a, float b, float t ) {
    return a + (b - a) * t;
}

float Math_Lerpf2( float a, float b, float t ) {
    return ( 1 - t ) * a + t * b;
}

/*
    Check if given value is float.
*/
bool Math_IsFloat( float value ) {
    if (value-(int)value == 0)
        return true;
    else return false;
}

/*
 Get random integer.
*/
int Math_RandInt32( int *seed ) {
    *seed = (69069 * *seed + 1);
    return *seed;
}

/*
 Get random float value.
*/
float Math_Randf( int *seed ) {
    return ( Math_RandInt32( seed ) & 0xffff ) / (float)0x10000;
}

