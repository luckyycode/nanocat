
#import     "MacUtils.h"
#import     "MacOpenGL.h"

#include    "Core.h"
#include    "Console.h"
#include    "Input.h"
#include    "Renderer.h"
#include    "OpenGL.h"
#include    "MultiplayerServer.h"

@interface glView (InternalMethods)

@end

@implementation glView

- (BOOL)acceptsMouseMovedEvents { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)canBecomeKeyWindow { return YES; }

- (id)initWithFrame:(NSRect)frameRect {

    // Stuff which must be loaded first.
    _core.Preload( GetBundlePath() );
    _renderer.Preload();
    
    const int swapInt = render_vsync.GetInteger();
    
    _gconsole.Execute( "readconfig config.nconf" );     // Load some settings
    
    _core.UseGraphics = Server_Dedicated.GetInteger() ? false : true;
    
    _core.Print( LOG_INFO, "Initializing OpenGL context\n" );
    
    NSOpenGLPixelFormat      *w_pf;
    uint attributeCount = 0;
    uint version = 0;
    
    _core.Print( LOG_INFO, "Chosen OpenGL version is %s\n", render_openglversion.GetString() );
    
    // The lowest version is 3.0.
    // Uh, there are only core versions available.
    if( !strcmp(render_openglversion.GetString(), "3.2" ) ) {
        version = NSOpenGLProfileVersion3_2Core;
    } else if( !strcmp(render_openglversion.GetString(), "3.2c" ) ) {
        version = NSOpenGLProfileVersion3_2Core;
    } else if( !strcmp(render_openglversion.GetString(), "4.1" ) ) {
        version = NSOpenGLProfileVersion4_1Core;
    } else if( !strcmp(render_openglversion.GetString(), "4.1c" ) ) {
        version = NSOpenGLProfileVersion4_1Core;
    } else {
        version = NSOpenGLProfileVersion3_2Core;
        _core.Print( LOG_WARN, "Unknown OpenGL version set, using 3.2 Core.\n" );
        return nil;
    }
    
#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }
    
    NSOpenGLPixelFormatAttribute attributes[40];
    
    ADD_ATTR(NSOpenGLPFADoubleBuffer);
    ADD_ATTR(NSOpenGLPFAClosestPolicy);
    ADD_ATTR2(NSOpenGLPFAOpenGLProfile, version);
    //ADD_ATTR(NSOpenGLPFAFullScreen);
    ADD_ATTR2(NSOpenGLPFAColorSize, 24);
    ADD_ATTR2(NSOpenGLPFAAlphaSize,24);
    ADD_ATTR2(NSOpenGLPFADepthSize, 24);
    ADD_ATTR2(NSOpenGLPFAStencilSize, 8);
    //ADD_ATTR(NSOpenGLPFAStereo);
    ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
    ADD_ATTR2(NSOpenGLPFASamples, 1);
    
    ADD_ATTR(0);
    
#undef ADD_ATTR
#undef ADD_ATTR2
    
    _core.Print( LOG_INFO, "Initializing pixel format...\n" );
    
    bzero(&w_pf, sizeof(w_pf));
    
    w_pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    
    if( !w_pf )
        _core.Error( ERC_GL, "Failed to initialize pixel format..\n" );
    
    _core.Print( LOG_INFO, "Initializing OpenGL view..\n" );
    
    self = [super initWithFrame:frameRect pixelFormat:w_pf];
    
    if ( !self )
        _core.Error( ERC_GL, "Failed to create OpenGL view.\n" );
    
    [w_pf release];
    
    //[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapRectangleEnable];
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    /*
        We use display link to avoid timers and
        another timed stuff. It's the best way to do now.
    */
    CVDisplayLinkCreateWithActiveCGDisplays( &m_displayLink );
    CVDisplayLinkSetOutputCallback( m_displayLink, m_displaycallback, self );
    
    CGLContextObj cglContext;
    CGLPixelFormatObj cglPixelFormat;
    
    // Initialize context.
    cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
    if( !cglContext ) {
        _core.Error( ERC_GL, "Could not create cglContext." );
        return nil;
    }
    
    // Initialize pixel format.
    cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    if( !cglPixelFormat ) {
        _core.Error( ERC_GL, "Could not create cglPixelFormat." );
        return nil;
    }
    
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext( m_displayLink, cglContext, cglPixelFormat );
    
    /*
        Check some stuff.
    */
    int fs_supported, vs_supported;
    bool passedthru;
    
    CGLGetParameter( cglContext, kCGLCPGPUFragmentProcessing, &fs_supported );
    CGLGetParameter( cglContext, kCGLCPGPUVertexProcessing, &vs_supported );

    // Wtf..?
    passedthru = ( fs_supported != 0 ) && (vs_supported != 0 );
    
    if( !passedthru ) {
        _core.Error( ERC_GL, "Fragment or vertex shaders aren't supported." );
        return nil;
    }
    
    CGLSetCurrentContext(cglContext);
    
    /*
        Initialize core.
    */
    _core.Initialize();
    _opengl.Initialize();
    
    [self setNeedsDisplay:YES];
    
    return self;
}

/*
 
 Control handlers.
 
*/

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];
    
    _input.OnMouseMove( location.x, location.y );
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];
    
    _input.OnMouseDown( location.x, location.y );

    _imouse.Holding = true;
}

- (void) mouseDragged:(NSEvent *)theEvent {
    
}

- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];

    _input.OnMouseUp( location.x, location.y );
    
    //_mouse.holding = false;
}

-(void)keyUp:(NSEvent*)event {
    _input.OnKeyUp( [[event characters] UTF8String][0] );
}

-(void)keyDown:(NSEvent*)event {
    _input.OnKeyPress( [[event characters] UTF8String][0] );
}

/*
 
        OpenGL
 
*/
static CVReturn m_displaycallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now,
                                  const CVTimeStamp *outputTime, CVOptionFlags flagsIn,
                                  CVOptionFlags *flagsOut, void *displayLinkContext)
{
    CVReturn result = [(glView *)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime {
    
    if( !_renderer.Initialized )
        return 0.0;
    
    if( !_core.Initialized )
        return 0.0;
    
    NSOpenGLContext    *currentContext;
    
    currentContext = [self openGLContext];
    
    if(!currentContext)
        return 0.0;
    
    [currentContext makeCurrentContext];
    
    /*
        We need to lock context because it is
        threaded.
    */
    CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
    
    _core.Frame();
    
    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    
    [currentContext flushBuffer];
    
    CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
    
    return kCVReturnSuccess;
}

- (void)dealloc {
    [[self window] release];
    [[self window] dealloc];
    CVDisplayLinkRelease(m_displayLink);
    [super dealloc];
}

- (void)awakeFromNib {
    [[self window] makeFirstResponder:self];
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    //[self reshape];
    
    CVDisplayLinkStart(m_displayLink);
}

- (void)reshape {
    if( !_opengl.Initialized )
        return;
    
    NSOpenGLContext    *currentContext = [self openGLContext];
    [currentContext makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);

    _opengl.OnResize( render_modeWidth.GetInteger(), render_modeHeight.GetInteger() );
    
    // Update.
    [[self openGLContext] update];
    [[self window] setTitle: [NSString stringWithFormat:@"Nanocat ( OSX, OpenGL %i.%i )", _opengl.GetMajorVersion(), _opengl.GetMinorVersion()]];
    
    // Change the window resolution.
    NSRect frame = [[self window] frame];
    
    frame.size.width                    = render_modeWidth.GetInteger();
    frame.size.height                   = render_modeHeight.GetInteger();
    [[self window] setFrame:frame display:YES];
    
    NSRect frame2 = [self frame];
    frame2.origin.x = 0;
    frame2.origin.y = 0;
    frame2.size.width                    = render_modeWidth.GetInteger();
    frame2.size.height                   = render_modeHeight.GetInteger();
    self.frame = frame2;
    
    CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
}



@end