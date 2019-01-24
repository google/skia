/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkUtils.h"
#include "Timer.h"
#include "WindowContextFactory_mac.h"
#include "Window_mac.h"

@interface MainView : NSView

- (MainView*)initWithWindow:(sk_app::Window*)initWindow;

@end

namespace sk_app {

Window* Window::CreateNativeWindow(void*) {
    Window_mac* window = new Window_mac();
    if (!window->initWindow()) {
        delete window;
        return nullptr;
    }

    return window;
}

bool Window_mac::initWindow() {
    if (fRequestedDisplayParams.fMSAASampleCount != fMSAASampleCount) {
        this->closeWindow();
    }

    // we already have a window
    if (fWindow) {
        return true;
    }

    constexpr int initialWidth = 1280;
    constexpr int initialHeight = 960;

    NSUInteger windowStyle = (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask |
                              NSMiniaturizableWindowMask);

    NSRect windowRect = NSMakeRect(100, 100, initialWidth, initialHeight);
    fWindow = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle
                                backing:NSBackingStoreBuffered defer:NO];
    if (nil == fWindow) {
        return false;
    }

    // create view
    NSView* windowView = fWindow.contentView;
    NSRect rect = [NSWindow contentRectForFrameRect:windowRect styleMask:windowStyle];
    fMainView = [[[MainView alloc] initWithFrame:rect] initWithWindow:this];
    if (nil == fMainView) {
        [fWindow release];
        fWindow = nil;
        return false;
    }
    [fMainView setTranslatesAutoresizingMaskIntoConstraints:NO];

    // attach view to window
    [windowView addSubview:fMainView];
    NSDictionary *views = NSDictionaryOfVariableBindings(fMainView);

    [windowView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[fMainView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [windowView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[fMainView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    //*** ?   [fMainView begin];
    //***?    [fMainView release];

    return true;
}

void Window_mac::closeWindow() {
    [fMainView release];
    fMainView = nil;
    [fWindow release];
    fWindow = nil;
}

void Window_mac::setTitle(const char* title) {
    NSString *titleString = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
    [fWindow setTitle:titleString];
}

void Window_mac::show() {
    [NSApp activateIgnoringOtherApps:YES];

    [fWindow makeKeyAndOrderFront:NSApp];
}

bool Window_mac::attach(BackendType attachType) {
    this->initWindow();

    window_context_factory::MacWindowInfo info;
    info.fWindow = fWindow;
    info.fMainView = fMainView;
    switch (attachType) {
        case kRaster_BackendType:
            fWindowContext = NewRasterForMac(info, fRequestedDisplayParams);
            break;

        case kNativeGL_BackendType:
        default:
            fWindowContext = NewGLForMac(info, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_mac::onInval() {
    [[fWindow contentView] setNeedsDisplay:YES];
}

}   // namespace sk_app

///////////////////////////////////////////////////////////////////////////////

@implementation MainView {
    sk_app::Window* fWindow;
}

- (MainView*)initWithWindow:(sk_app::Window *)initWindow {
    fWindow = initWindow;

    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    fWindow->onPaint();
}

@end

