
#import <Cocoa/Cocoa.h>
#include <OpenGL/gl3.h>
#import <objc/objc.h>

#import <QuartzCore/QuartzCore.h>

@interface glView : NSOpenGLView {
    CVDisplayLinkRef m_displayLink;
}

@end