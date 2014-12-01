//
//  Nanocat engine.
//
//  GameMath library header..
//
//  Created by Neko Vision on 10/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#ifndef gamemath_h
#define gamemath_h

#define PI 3.14159265358979323846
#define DEGTORAD(degree)    ((degree) * (PI / 180.0f))
#define RADTODEG(radian)    ((radian) * (180.0f / PI))
#define DEG2RAD(a) ( (a) * PI / 180.0 )
#define DEGINRAD ( PI / 180.0 )
#define ANG2RAD PI / 180.0
#define TWO_PI	2.0f * PI
#define HALF_PI	0.5f * PI
#define EPSILON 2.71828182845904523536f
#define M_INFINITY 999999999.0f
#define RAD2DEG 180.0f / PI
#define SEC2MS 1000.0f
#define MS2SEC 0.001f

#define maxdef(x, y) (((x) > (y)) ? (x) : (y))
#define mindef(x, y) (((x) < (y)) ? (x) : (y))

#define POINT_ON_PLANE 0
#define POINT_IN_FRONT_OF_PLANE 1
#define POINT_BEHIND_PLANE 2

class ncVec2 {
public:
    float x;
    float y;
    
    ncVec2() {
        x = 1.0f;
        y = 1.0f;
    }
    
    ncVec2( float v ) {
        x = v;
        y = v;
    }
    
    ncVec2( float x1, float y1 ) {
        x = x1;
        y = y1;
    }
    
    ncVec2 operator-(const ncVec2& v) const;
    ncVec2 operator+(const ncVec2& v) const;
    ncVec2 operator/(const ncVec2& v) const;
    ncVec2 operator*(const ncVec2& v) const;
    
    ncVec2 operator-(const float& v) const;
    ncVec2 operator+(const float& v) const;
    ncVec2 operator/(const float& v) const;
    ncVec2 operator*(const float& v) const;
};

class ncVec4 {
public:
    float x;
    float y;
    float z;
    float w;
    
    ncVec4() {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
    }
    ncVec4( float x, float y, float z, float w ) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    ncVec4( float v ) {
        this->x = v;
        this->y = v;
        this->z = v;
        this->w = v;
    }
    
    void Inverse( ncVec4& v );
    ncVec4 operator/(const float& v) const;
};

class ncVec3 {
public:
    float x, y, z;
    
    ncVec3() {
        x = 1.0f;
        y = 1.0f;
        z = 1.0f;
    }
    
    ncVec3( float x1, float y1, float z1 ) {
        x = x1;
        y = y1;
        z = z1;
    }
    
    ncVec3( float v ) {
        x = v;
        y = v;
        z = v;
    }
    
    void Normalize( void );
    void Cross( ncVec3 v1, ncVec3 v2 );
    float Length( void );
    float Distance( const ncVec3 &v1 );
    void Inverse( void );
    
    ncVec3 operator+(const ncVec3& v) const;
    ncVec3 operator-(const ncVec3& v) const;
    ncVec3 operator*(const ncVec3& v) const;
    ncVec3 operator*(const float v) const;
    ncVec3 operator/(const float v) const;
    
    // Static functions.
    static float Dot( ncVec3 v1, ncVec3 v2 );
    static ncVec3 CrossTwoVectors( ncVec3 v1, ncVec3 v2 );
};

class ncMatrix4 {
public:
    float m[16];
    
    ncMatrix4() {
        m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
        m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f; m[13] = 0.0f;
        m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
        m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
    }
    
    void Identity( void );
    
    void Translate( ncVec3 v );
    void TranslateX( float v );
    void TranslateY( float v );
    void TranslateZ( float v );
    
    void Rotate( float angle, ncVec3 axis );
    void RotateX( float angle );
    void RotateY( float angle );
    void RotateZ( float angle );
    
    void Scale( ncVec3 scale );
    
    void Inverse( void );
    void Transpose( void);
    
    ncMatrix4 operator+(const ncMatrix4& v) const;
    ncMatrix4 operator-(const ncMatrix4& v) const;
    ncMatrix4 operator*(const ncMatrix4& v) const;
    ncMatrix4 operator*(const float f) const;
    ncVec4 operator*(const ncVec4&v ) const;
    ncVec3 operator*(const ncVec3&v ) const;
    
    // View stuff.
    void CreatePerspective( float fovy, float aspect_ratio, float near_plane, float far_plane );
    void CreateOrtho( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
};

/*
            GameMath Library functions.
*/



void Matrix4_TransformPoint( ncMatrix4 *mat, ncVec3 *vec );
void Matrix4_TransformVector( ncMatrix4 *mat, ncVec3 *vec );

bool Frustum_IsBoxInside( ncVec3 *points );
bool Frustum_IsPointInside( const ncVec3 point );
void Frustum_Update( void );

enum frustumplane_t {
    LEFT_PLANE = 0,
    RIGHT_PLANE,
    TOP_PLANE,
    BOTTOM_PLANE,
    NEAR_PLANE,
    FAR_PLANE
};

enum frustumtype_t {
    OUTSIDE_FRUSTUM = 0,
    IN_FRUSTUM
};

class ncPlane {
public:
    ncVec3 normal = ncVec3( 1.0f );
    ncPlane() { }
    
    float intercept;
    
    int ClassifyPoint( ncVec3 point );
    void Normalize( void );
    
    void SetNormal( const ncVec3 & rhs ) { normal = rhs; }
    void SetIntercept( float newIntercept ) { intercept = newIntercept; }
    void SetFromPoints(const ncVec3 & p0, const ncVec3 & p1, const ncVec3 & p2);
    
    void CalculateIntercept(const ncVec3 & pointOnPlane) {
        float nDotp = ncVec3::Dot( pointOnPlane, normal );
        intercept=-nDotp;
    }
    
    bool Intersect3(const ncPlane & p2, const ncPlane & p3, ncVec3 & result);
    
    
    float GetDistance(const ncVec3 & point) const
    {
        return point.x * normal.x + point.y * normal.y + point.z * normal.z + intercept;
    }
    
};

class ncFrustum {
public:
    ncFrustum() { }
    
    ncPlane planes[6];
};

class ncGameMath {
public:
    void Initialize( void );
};

extern ncFrustum _frustum;

int Math_Sign( int x );
float Math_Cotangent( float angle );
float Math_Vec3Distance( ncVec3 s1, ncVec3 s2 );
int Math_Roundf( float value );
float Math_Lerpf( float a, float b, float t );
float Math_Lerpf2( float a, float b, float t );
bool Math_IsFloat( float value );

int Math_RandInt32( int *seed );
float Math_Randf( int *seed );

template<typename T>
T Math_Clamp(T value, T min, T max) {
    if (value > max)
        return max;
    
    if (value < min)
        return min;
    else
        return value;
}

const ncVec4 COLOR_RED = ncVec4( 1.0f, 0.0f, 0.0f, 1.0f );
const ncVec4 COLOR_WHITE = ncVec4( 1.0f, 1.0f, 1.0f, 1.0f );
const ncVec4 COLOR_BLUE = ncVec4( 0.0f, 0.0f, 1.0f, 1.0f );
const ncVec4 COLOR_GREEN = ncVec4( 0.0f, 1.0f, 0.0f, 1.0f );

const ncVec3 VECTOR_UP = ncVec3( 0.0f, 1.0f, 0.0f );
const ncVec3 VECTOR_ZERO = ncVec3( 0.0f, 0.0f, 0.0f );
const ncVec3 VECTOR_BACK = ncVec3( 0.0f, 0.0f, -1.0f );
const ncVec3 VECTOR_DOWN = ncVec3( 0.0f, -1.0f, 0.0f );
const ncVec3 VECTOR_RIGHT = ncVec3( 1.0f, 0.0f, 0.0f );

#endif
