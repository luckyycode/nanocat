//
//  Utils.h
//  Nanocat.
//
//  Created by Neko Code on 9/12/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef Utils_h
#define Utils_h


class ncUtils {
public:
    static void OBJtoSM( const NString file );
    
    static NString HexEncode( const NString data, unsigned int data_size );
    static bool HexCharDecode( char hexdigit, Byte &value );
    
    static ncVec3 RayFromMousePos( ncMatrix4 m_modelView, int x, int y );
    
    // GLU library functions.
    static GLint gluUnProject( GLfloat winx, GLfloat winy, GLfloat winz, const GLfloat modelMatrix[16], const GLfloat projMatrix[16], const GLint viewport[4], GLfloat *objx, GLfloat *objy, GLfloat *objz);
    static GLint gluProject( GLfloat objx, GLfloat objy, GLfloat objz, const GLfloat modelMatrix[16], const GLfloat projMatrix[16], const GLint viewport[4], GLfloat *winx, GLfloat *winy, GLfloat *winz);
};

#endif
