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

@interface WindowDelegate : NSObject<NSWindowDelegate>

- (WindowDelegate*)initWithWindow:(sk_app::Window*)initWindow;

@end

using sk_app::Window;

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
    WindowDelegate* delegate = [[WindowDelegate alloc] initWithWindow:this];
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

- (void)windowDidResize:(NSNotification *)notification {
    sk_app::Window_mac* macWindow = reinterpret_cast<sk_app::Window_mac*>(fWindow);
    const NSRect mainRect = [macWindow->view() frame];
    const NSRect backingRect = [macWindow->view() convertRectToBacking:mainRect];

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

static Window::Key get_key(unsigned short vk) {
    // This will work with an ANSI QWERTY keyboard.
    // Something more robust would be needed to support alternate keyboards.
    static const struct {
        unsigned short fVK;
        Window::Key    fKey;
    } gPair[] = {
        { 0x33, Window::Key::kBack },
        { 0x24, Window::Key::kOK },
        { 0x7E, Window::Key::kUp },
        { 0x7D, Window::Key::kDown },
        { 0x7B, Window::Key::kLeft },
        { 0x7C, Window::Key::kRight },
        { 0x30, Window::Key::kTab },
        { 0x74, Window::Key::kPageUp },
        { 0x79, Window::Key::kPageDown },
        { 0x73, Window::Key::kHome },
        { 0x77, Window::Key::kEnd },
        { 0x75, Window::Key::kDelete },
        { 0x35, Window::Key::kEscape },
        { 0x38, Window::Key::kShift },
        { 0x3C, Window::Key::kShift },
        { 0x3B, Window::Key::kCtrl },
        { 0x3E, Window::Key::kCtrl },
        { 0x3A, Window::Key::kOption },
        { 0x3D, Window::Key::kOption },
        { 0x00, Window::Key::kA },
        { 0x08, Window::Key::kC },
        { 0x09, Window::Key::kV },
        { 0x07, Window::Key::kX },
        { 0x10, Window::Key::kY },
        { 0x06, Window::Key::kZ },
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }

    return Window::Key::kNONE;
}

static uint32_t get_modifiers(const NSEvent* event) {
    NSUInteger modifierFlags = [event modifierFlags];
    auto modifiers = 0;

    if (modifierFlags & NSEventModifierFlagShift) {
        modifiers |= Window::kShift_ModifierKey;
    }
    if (modifierFlags & NSEventModifierFlagControl) {
        modifiers |= Window::kControl_ModifierKey;
    }
    if (modifierFlags & NSEventModifierFlagOption) {
        modifiers |= Window::kOption_ModifierKey;
    }

    if (NO == [event isARepeat] &&
        (NSKeyDown == [event type] || NSKeyUp == [event type])) {
        modifiers |= Window::kFirstPress_ModifierKey;
    }

    return modifiers;
}

- (void)keyDown:(NSEvent *)event {
    Window::Key key = get_key([event keyCode]);
    if (key != Window::Key::kNONE) {
        if (!fWindow->onKey(key, Window::kDown_InputState, get_modifiers(event))) {
//*** need to catch ESC somewhere?
//            if (event.key.keysym.sym != SDLK_ESCAPE) {
                [[self superview] keyDown:event];
//            }
        }
    } else {
        NSString* characters = [event charactersIgnoringModifiers];
        if ([characters length] > 0) {
            unichar firstChar = [characters characterAtIndex:0];
            (void) fWindow->onChar((SkUnichar) firstChar, get_modifiers(event));
        }
    }
}

- (void)keyUp:(NSEvent *)event {
    Window::Key key = get_key([event keyCode]);
    if (key != Window::Key::kNONE) {
        (void) fWindow->onKey(key, Window::kUp_InputState, get_modifiers(event));
    }
}

@end

