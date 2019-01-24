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

@interface WindowDelegate : NSObject

- (WindowDelegate*)initWithWindow:(sk_app::Window*)initWindow;

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
    void* delegate = [[WindowDelegate alloc] initWithWindow:this];
    [fWindow setDelegate:delegate];
    //? [delegate release];

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
@implementation WindowDelegate {
    sk_app::Window* fWindow;
}

- (WindowDelegate*)initWithWindow:(sk_app::Window *)initWindow {
    fWindow = initWindow;

    return self;
}

- (void)windowDidResize:(NSNotification *)notification
{
    sk_app::Window_mac* macWindow = reinterpret_cast<sk_app::Window_mac*>(fWindow);
    const NSRect mainRect = [macWindow->fMainView frame];
    const NSRect backingRect = [macWindow->fMainView convertRectToBacking:mainRect];

    fWindow->onResize(backingRect.size.width, backingRect.size.height);
}

@end

///////////////////////////////////////////////////////////////////////////////
@implementation MainView {
    sk_app::Window* fWindow;
}

- (MainView*)initWithWindow:(sk_app::Window *)initWindow {
    fWindow = initWindow;

    return self;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    fWindow->onPaint();
}

- (void)mouseDown:(NSEvent *)event
{
//    _glfwInputMouseClick(window,
//                         GLFW_MOUSE_BUTTON_LEFT,
//                         GLFW_PRESS,
//                         translateFlags([event modifierFlags]));
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
//    _glfwInputMouseClick(window,
//                         GLFW_MOUSE_BUTTON_LEFT,
//                         GLFW_RELEASE,
//                         translateFlags([event modifierFlags]));
}

- (void)mouseMoved:(NSEvent *)event
{
//    if (window->cursorMode == GLFW_CURSOR_DISABLED)
//    {
//        _glfwInputCursorMotion(window,
//                               [event deltaX] - window->ns.warpDeltaX,
//                               [event deltaY] - window->ns.warpDeltaY);
//    }
//    else
//    {
//        const NSRect contentRect = [window->ns.view frame];
//        const NSPoint pos = [event locationInWindow];
//
//        _glfwInputCursorMotion(window, pos.x, contentRect.size.height - pos.y);
//    }
//
//    window->ns.warpDeltaX = 0;
//    window->ns.warpDeltaY = 0;
}

- (void)rightMouseDown:(NSEvent *)event {
//    _glfwInputMouseClick(window,
//                         GLFW_MOUSE_BUTTON_RIGHT,
//                         GLFW_PRESS,
//                         translateFlags([event modifierFlags]));
}

- (void)rightMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
//    _glfwInputMouseClick(window,
//                         GLFW_MOUSE_BUTTON_RIGHT,
//                         GLFW_RELEASE,
//                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDown:(NSEvent *)event
{
//    _glfwInputMouseClick(window,
//                         (int) [event buttonNumber],
//                         GLFW_PRESS,
//                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
//    _glfwInputMouseClick(window,
//                         (int) [event buttonNumber],
//                         GLFW_RELEASE,
//                         translateFlags([event modifierFlags]));
}

- (void)mouseExited:(NSEvent *)event
{
//    _glfwInputCursorEnter(window, GL_FALSE);
}

- (void)mouseEntered:(NSEvent *)event
{
//    _glfwInputCursorEnter(window, GL_TRUE);
}

- (void)keyDown:(NSEvent *)event
{
//    const int key = translateKey([event keyCode]);
//    const int mods = translateFlags([event modifierFlags]);
//
//    _glfwInputKey(window, key, [event keyCode], GLFW_PRESS, mods);
//
//    NSString* characters = [event characters];
//    NSUInteger i, length = [characters length];
//    const int plain = !(mods & GLFW_MOD_SUPER);
//
//    for (i = 0;  i < length;  i++)
//    {
//        const unichar codepoint = [characters characterAtIndex:i];
//        if ((codepoint & 0xff00) == 0xf700)
//            continue;
//
//        _glfwInputChar(window, codepoint, mods, plain);
//    }
}

- (void)flagsChanged:(NSEvent *)event
{
//    int action;
//    const unsigned int modifierFlags =
//    [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
//    const int key = translateKey([event keyCode]);
//    const int mods = translateFlags(modifierFlags);
//
//    if (modifierFlags == window->ns.modifierFlags)
//    {
//        if (window->keys[key] == GLFW_PRESS)
//            action = GLFW_RELEASE;
//        else
//            action = GLFW_PRESS;
//    }
//    else if (modifierFlags > window->ns.modifierFlags)
//        action = GLFW_PRESS;
//    else
//        action = GLFW_RELEASE;
//
//    window->ns.modifierFlags = modifierFlags;
//
//    _glfwInputKey(window, key, [event keyCode], action, mods);
}

- (void)keyUp:(NSEvent *)event
{
//    const int key = translateKey([event keyCode]);
//    const int mods = translateFlags([event modifierFlags]);
//    _glfwInputKey(window, key, [event keyCode], GLFW_RELEASE, mods);
}


@end

