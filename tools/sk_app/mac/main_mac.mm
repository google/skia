/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#import <Cocoa/Cocoa.h>

#include "tools/sk_app/Application.h"
#include "tools/sk_app/mac/Window_mac.h"

@interface AppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>

@property (nonatomic, assign) BOOL done;

@end

@implementation AppDelegate : NSObject

@synthesize done = _done;

- (id)init {
    self = [super init];
    _done = FALSE;
    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    _done = TRUE;
    return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp stop:nil];
}

@end

///////////////////////////////////////////////////////////////////////////////////////////

using sk_app::Application;
using sk_app::Window_mac;

int main(int argc, char * argv[]) {
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
    // we only run on systems that support at least Core Profile 3.2
    return EXIT_FAILURE;
#endif

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    //Create the application menu.
    NSMenu* menuBar=[[NSMenu alloc] initWithTitle:@"AMainMenu"];
    [NSApp setMenu:menuBar];

    NSMenuItem* item;
    NSMenu* subMenu;

    item=[[NSMenuItem alloc] initWithTitle:@"Apple" action:NULL keyEquivalent:@""];
    [menuBar addItem:item];
    subMenu=[[NSMenu alloc] initWithTitle:@"Apple"];
    [menuBar setSubmenu:subMenu forItem:item];
    [item release];
    item=[[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
    [subMenu addItem:item];
    [item release];
    [subMenu release];

    // Set AppDelegate to catch certain global events
    AppDelegate* appDelegate = [[[AppDelegate alloc] init] autorelease];
    [NSApp setDelegate:appDelegate];

    Application* app = Application::Create(argc, argv, nullptr);

    // This will run until the application finishes launching, then lets us take over
    [NSApp run];

    // Now we process the events
    while (![appDelegate done]) {
        // Rather than depending on a Mac event to drive this, we treat our window
        // invalidation flag as a separate event stream. Window::onPaint() will clear
        // the invalidation flag, effectively removing it from the stream.
        Window_mac::PaintWindows();

        NSEvent* event;
        do {
            event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES];
            NSEventType type = event.type;
            switch (type) {
                case NSEventTypeKeyDown:
                case NSEventTypeKeyUp:
                case NSEventTypeLeftMouseDown:
                case NSEventTypeLeftMouseUp:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeMouseMoved:
                case NSEventTypeScrollWheel:
                    Window_mac::HandleWindowEvent(event);
                    break;
                default:
                    break;
            }
            // We send all events through the system to catch window close events, drags, etc.
            [NSApp sendEvent:event];
        } while (event != nil);

        app->onIdle();
    }

    delete app;

    [menuBar release];
    [pool release];

    return EXIT_SUCCESS;
}
