//
//  AppDelegate.m
//  test
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#import "AppDelegate.h"
#import "Core.h"
#import "OpenGL.h"
#import "Renderer.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    gl_Core->Initialized = true;
    gl_Core->OnResize( Render_Width.GetInteger(), Render_Height.GetInteger() );
    
    g_Core->Loaded();
}

@end


