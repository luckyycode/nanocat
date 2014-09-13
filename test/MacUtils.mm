//
//  MacUtils.m
//  SkyCatCPP
//
//  Created by Neko Code on 9/2/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MacUtils.h"
#import <Cocoa/Cocoa.h>

@implementation ncMacUtils : NSObject

/*
    Another system stuff.
*/
void SetCursorPos( int x, int y ) {
    CGPoint point;
    point.x = x;
    point.y = y;
    
    //CGWarpMouseCursorPosition( point );
}

/*
    Popup window.
*/
void MassageBox( const char *msg ) {
    NSString    *message;
    
    message = [NSString stringWithFormat:@"%s", msg];
    
    // Ignore this damned warning.

    //NSAlert *alert = [[NSAlert alloc] init];
    //[alert setMessageText:[NSString stringWithFormat:@"%@", message]];
    //[alert setDelegate:self];
    //[alert runModal];
}

/*
    Since it won't work properly,
    we need to write custom path finder.
*/
const char *GetBundlePath() {
    const char        *fs_path;
    
        NSString *bundlePath = [[NSBundle mainBundle] resourcePath];
        NSString *secondParentPath = [[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
        NSString *thirdParentPath = [[secondParentPath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];

        fs_path = [thirdParentPath UTF8String];
    
        return fs_path;
}

/*
    Current Mac version running.
*/
const char *GetSystemVersion() {
        NSString *operatingSystemVersionString = [[NSProcessInfo processInfo] operatingSystemVersionString];
        return [operatingSystemVersionString UTF8String];
}

@end