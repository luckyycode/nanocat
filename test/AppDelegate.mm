//
//  AppDelegate.m
//  test
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#import "init.h"
#import "core.h"
#import "gl.h"
#import "renderer.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    _opengl.Initialized = true;
    _opengl.OnResize( render_modeWidth.GetInteger(), render_modeHeight.GetInteger() );
    _core.Loaded();
}

@end


