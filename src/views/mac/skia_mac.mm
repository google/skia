
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <crt_externs.h>
#import <Cocoa/Cocoa.h>
#include "SkApplication.h"
#include "SkNSView.h"

@interface MainView : SkNSView {
}
- (id)initWithFrame: (NSRect)frame ;
- (void)dealloc;
- (void)begin;
@end

@implementation MainView : SkNSView

- (id)initWithFrame: (NSRect)frame {
    self = [super initWithFrame:frame];
    return self;
}

- (void)dealloc {
    delete fWind;
    [super dealloc];
}

- (void)begin {
    fWind = create_sk_window(self, *_NSGetArgc(), *_NSGetArgv());
    [self setUpWindow];
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate> {
}
- (id)init;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
@end

#
@implementation AppDelegate : NSObject
- (id)init {
    self = [super init];
    return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return TRUE;
}
@end

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    NSApplication* app = [NSApplication sharedApplication];

    NSUInteger windowStyle = (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask);
    
    NSRect windowRect = NSMakeRect(100, 100, 1000, 1000);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
   
    NSRect rect = [NSWindow contentRectForFrameRect:windowRect styleMask:windowStyle];
    MainView* customView = [[MainView alloc] initWithFrame:rect];
    [customView setTranslatesAutoresizingMaskIntoConstraints:NO];
    NSView* contentView = window.contentView;
    [contentView addSubview:customView];
    NSDictionary *views = NSDictionaryOfVariableBindings(customView);
    
    [contentView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[customView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];
    
    [contentView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[customView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];
    
    [customView begin];
    [customView release];
    
    [window makeKeyAndOrderFront:NSApp];

    AppDelegate * appDelegate = [[[AppDelegate alloc] init] autorelease];
    
    app.delegate = appDelegate;
    
    NSMenu* menu=[[NSMenu alloc] initWithTitle:@"AMainMenu"];
    NSMenuItem* item;
    NSMenu* subMenu;
    
    //Create the application menu.
    item=[[NSMenuItem alloc] initWithTitle:@"Apple" action:NULL keyEquivalent:@""];
    [menu addItem:item];
    subMenu=[[NSMenu alloc] initWithTitle:@"Apple"];
    [menu setSubmenu:subMenu forItem:item];
    [item release];
    item=[[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
    [subMenu addItem:item];
    [item release];
    [subMenu release];
    
    //Add the menu to the app.
    [app setMenu:menu];
    
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    [app run];

    [menu release];
    [appDelegate release];
    [window release];
    [pool release];
    
    return EXIT_SUCCESS;
}
