/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#import <Cocoa/Cocoa.h>

#include "Window_mac.h"
#include "../Application.h"

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

using sk_app::Application;

int main(int argc, char * argv[]) {
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
    // we only run on systems that support at least Core Profile 3.2
    return EXIT_FAILURE;
#endif

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    AppDelegate * appDelegate = [[[AppDelegate alloc] init] autorelease];
    NSApp.delegate = appDelegate;
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

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
    [NSApp setMenu:menu];

    Application* app = Application::Create(argc, argv, nullptr);

    [NSApp run];

    delete app;

    [menu release];
    [appDelegate release];
    [pool release];

    return EXIT_SUCCESS;
}
