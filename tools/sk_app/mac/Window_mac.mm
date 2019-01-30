/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkUtils.h"
#include "WindowContextFactory_mac.h"
#include "Window_mac.h"

@interface MainView : NSView

- (MainView*)initWithWindow:(sk_app::Window*)initWindow;

@end

@interface WindowDelegate : NSObject<NSWindowDelegate>

- (WindowDelegate*)initWithWindow:(sk_app::Window*)initWindow;

@end

///////////////////////////////////////////////////////////////////////////////

using sk_app::Window;

namespace sk_app {

static int gWindowCount = 0;

Window* Window::CreateNativeWindow(void*) {
    Window_mac* window = new Window_mac();
    if (!window->initWindow()) {
        delete window;
        return nullptr;
    }

    ++gWindowCount;
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
    [delegate release];

    // create view
    MainView* view = [[[MainView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)] initWithWindow:this];
    if (nil == view) {
        [fWindow release];
        fWindow = nil;
        return false;
    }

    // attach view to window
    [fWindow setContentView:view];

    return true;
}

void Window_mac::closeWindow() {
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
    info.fMainView = this->view();
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
    // MacOS already queues a single drawRect event for multiple invalidations
    // so we don't need to use our invalidation method (and it can mess things up
    // if for some reason MacOS skips a drawRect when we need one).
    this->markInvalProcessed();
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
    const NSRect mainRect = [macWindow->view() bounds];

    fWindow->onResize(mainRect.size.width, mainRect.size.height);
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    --sk_app::gWindowCount;
    if (sk_app::gWindowCount < 1) {
        [NSApp terminate:self];
    }

    reinterpret_cast<sk_app::Window_mac*>(fWindow)->closeWindow();

    return FALSE;
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

    if ((NSKeyDown == [event type] || NSKeyUp == [event type]) &&
        NO == [event isARepeat]) {
        modifiers |= Window::kFirstPress_ModifierKey;
    }

    return modifiers;
}

- (void)keyDown:(NSEvent *)event {
    Window::Key key = get_key([event keyCode]);
    if (key != Window::Key::kNONE) {
        if (!fWindow->onKey(key, Window::kDown_InputState, get_modifiers(event))) {
            if (Window::Key::kEscape == key) {
                [NSApp terminate:self];
            }
        }
    }

    NSString* characters = [event charactersIgnoringModifiers];
    NSUInteger len = [characters length];
    if (len > 0) {
        unichar* charBuffer = new unichar[len+1];
        [characters getCharacters:charBuffer range:NSMakeRange(0, len)];
        for (NSUInteger i = 0; i < len; ++i) {
            (void) fWindow->onChar((SkUnichar) charBuffer[i], get_modifiers(event));
        }
        delete [] charBuffer;
    }
}

- (void)keyUp:(NSEvent *)event {
    Window::Key key = get_key([event keyCode]);
    if (key != Window::Key::kNONE) {
        (void) fWindow->onKey(key, Window::kUp_InputState, get_modifiers(event));
    }
}

- (void)mouseDown:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [self frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, Window::kDown_InputState,
                     get_modifiers(event));
}

- (void)mouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [self frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, Window::kUp_InputState,
                     get_modifiers(event));
}

- (void)mouseMoved:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [self frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, Window::kMove_InputState,
                     get_modifiers(event));
}

- (void)scrollWheel:(NSEvent *)event {
    // TODO: support hasPreciseScrollingDeltas?
    fWindow->onMouseWheel([event scrollingDeltaY], get_modifiers(event));
}


@end

