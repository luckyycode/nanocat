
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
    g_Core->Preload( GetBundlePath() );
    g_mainRenderer->Preload();
    
    const int swapInt = Render_VSync.GetInteger();
    
    g_Console->Execute( "readconfig config.nconf" );     // Load some settings
    
    g_Core->UseGraphics = Server_Dedicated.GetInteger() ? false : true;
    
    g_Core->Print( LOG_INFO, "Initializing OpenGL context\n" );
    
    NSOpenGLPixelFormat      *w_pf;
    uint attributeCount = 0;
    uint version = 0;
    

    version = NSOpenGLProfileVersion4_1Core;

    
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
    
    g_Core->Print( LOG_INFO, "Initializing pixel format...\n" );
    
    bzero(&w_pf, sizeof(w_pf));
    
    w_pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    
    if( !w_pf )
        g_Core->Error( ERR_OPENGL, "Failed to initialize pixel format..\n" );
    
    g_Core->Print( LOG_INFO, "Initializing OpenGL view..\n" );
    
    self = [super initWithFrame:frameRect pixelFormat:w_pf];
    
    if ( !self )
        g_Core->Error( ERR_OPENGL, "Failed to create OpenGL view.\n" );
    
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
        g_Core->Error( ERR_OPENGL, "Could not create cglContext." );
        return nil;
    }
    
    // Initialize pixel format.
    cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    if( !cglPixelFormat ) {
        g_Core->Error( ERR_OPENGL, "Could not create cglPixelFormat." );
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
        g_Core->Error( ERR_OPENGL, "Fragment or vertex shaders aren't supported." );
        return nil;
    }
    
    CGLSetCurrentContext(cglContext);
    
    /*
        Initialize core.
    */
    g_Core->Initialize();
    gl_Core->Initialize();
    
    [self setNeedsDisplay:YES];
    
    return self;
}

/*
 
 Control handlers.
 
*/

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];
    
    g_Input->OnMouseMove( location.x, location.y );
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];
    
    g_Input->OnMouseDown( location.x, location.y );

    c_Mouse->Holding = true;
}

- (void) mouseDragged:(NSEvent *)theEvent {
    
}

- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint location;
    
    location = [NSEvent mouseLocation];

    g_Input->OnMouseUp( location.x, location.y );
    
    //_mouse.holding = false;
}

-(void)keyUp:(NSEvent*)event {
    g_Input->OnKeyUp( [[event characters] UTF8String][0] );
}

-(void)keyDown:(NSEvent*)event {
    g_Input->OnKeyPress( [[event characters] UTF8String][0] );
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
    
    if( !g_mainRenderer->Initialized )
        return 0.0;
    
    if( !g_Core->Initialized )
        return 0.0;
    
    NSOpenGLContext *currentContext;
    
    currentContext = [self openGLContext];
    
    if(!currentContext)
        return 0.0;
    
    [currentContext makeCurrentContext];
    
    /*
        We need to lock context because it is
        threaded.
    */
    CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
    
    g_Core->Frame();
    
    [currentContext flushBuffer];
    
    CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
    
    return kCVReturnSuccess;
}

- (void)dealloc {
    
    [[self window] release];
    [[self window] dealloc];
    
    CVDisplayLinkRelease( m_displayLink );
    
    [super dealloc];
}

- (void)awakeFromNib {
    [[self window] makeFirstResponder:self];
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    //[self reshape];
    
    CVDisplayLinkStart( m_displayLink );
}

- (void)reshape {
    if( !gl_Core->Initialized )
        return;
    
    NSOpenGLContext *currentContext = [self openGLContext];
    [currentContext makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);

    gl_Core->OnResize( Render_Width.GetInteger(), Render_Height.GetInteger() );
    
    // Update.
    [[self openGLContext] update];
    [[self window] setTitle: [NSString stringWithFormat:@"Nanocat ( OSX, OpenGL %i.%i )", gl_Core->GetMajorVersion(), gl_Core->GetMinorVersion()]];
    
    // Change the window resolution.
    NSRect frame = [[self window] frame];
    
    frame.size.width                    = Render_Water.GetInteger();
    frame.size.height                   = Render_Height.GetInteger();
    [[self window] setFrame:frame display:YES];
    
    NSRect frame2 = [self frame];
    frame2.origin.x = 0;
    frame2.origin.y = 0;
    frame2.size.width                    = Render_Width.GetInteger();
    frame2.size.height                   = Render_Height.GetInteger();
    self.frame = frame2;
    
    CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
}



@end