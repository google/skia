/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/core/SkUtils.h"
#include "tools/ModifierKey.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"
#include "tools/sk_app/mac/Window_mac.h"

@interface WindowDelegate : NSObject<NSWindowDelegate>

- (WindowDelegate*)initWithWindow:(sk_app::Window_mac*)initWindow;

@end

@interface MainView : NSView

- (MainView*)initWithWindow:(sk_app::Window_mac*)initWindow;

@end

///////////////////////////////////////////////////////////////////////////////

using sk_app::Window;

namespace sk_app {

SkTDynamicHash<Window_mac, NSInteger> Window_mac::gWindowMap;

Window* Window::CreateNativeWindow(void*) {
    Window_mac* window = new Window_mac();
    if (!window->initWindow()) {
        delete window;
        return nullptr;
    }

    return window;
}

bool Window_mac::initWindow() {
    // we already have a window
    if (fWindow) {
        return true;
    }

    // Create a delegate to track certain events
    WindowDelegate* delegate = [[WindowDelegate alloc] initWithWindow:this];
    if (nil == delegate) {
        return false;
    }

    // Create Cocoa window
    constexpr int initialWidth = 1280;
    constexpr int initialHeight = 960;
    NSRect windowRect = NSMakeRect(100, 100, initialWidth, initialHeight);

    NSUInteger windowStyle = (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask |
                              NSMiniaturizableWindowMask);

    fWindow = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle
                                backing:NSBackingStoreBuffered defer:NO];
    if (nil == fWindow) {
        [delegate release];
        return false;
    }

    // create view
    MainView* view = [[MainView alloc] initWithWindow:this] ;
    if (nil == view) {
        [fWindow release];
        [delegate release];
        return false;
    }

    [fWindow setContentView:view];
    [fWindow makeFirstResponder:view];
    [fWindow setDelegate:delegate];
    [fWindow setAcceptsMouseMovedEvents:YES];
    [fWindow setRestorable:NO];

    // Should be retained by window now
    [view release];

    fWindowNumber = fWindow.windowNumber;
    gWindowMap.add(this);

    return true;
}

void Window_mac::closeWindow() {
    if (nil != fWindow) {
        gWindowMap.remove(fWindowNumber);
        if (sk_app::Window_mac::gWindowMap.count() < 1) {
            [NSApp terminate:fWindow];
        }
        [fWindow close];
        fWindow = nil;
    }
}

void Window_mac::setTitle(const char* title) {
    NSString *titleString = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
    [fWindow setTitle:titleString];
}

void Window_mac::show() {
    [fWindow orderFront:nil];

    [NSApp activateIgnoringOtherApps:YES];
    [fWindow makeKeyAndOrderFront:NSApp];
}

bool Window_mac::attach(BackendType attachType) {
    this->initWindow();

    window_context_factory::MacWindowInfo info;
    info.fMainView = [fWindow contentView];
    switch (attachType) {
        case kRaster_BackendType:
            fWindowContext = NewRasterForMac(info, fRequestedDisplayParams);
            break;
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = NewVulkanForMac(info, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_METAL
        case kMetal_BackendType:
            fWindowContext = NewMetalForMac(info, fRequestedDisplayParams);
            break;
#endif
        case kNativeGL_BackendType:
        default:
            fWindowContext = NewGLForMac(info, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_mac::PaintWindows() {
    SkTDynamicHash<Window_mac, NSInteger>::Iter iter(&gWindowMap);
    while (!iter.done()) {
        if ((*iter).fIsContentInvalidated) {
            (*iter).onPaint();
        }
        ++iter;
    }
}

}   // namespace sk_app

///////////////////////////////////////////////////////////////////////////////

@implementation WindowDelegate {
    sk_app::Window_mac* fWindow;
}

- (WindowDelegate*)initWithWindow:(sk_app::Window_mac *)initWindow {
    fWindow = initWindow;

    return self;
}

- (void)windowDidResize:(NSNotification *)notification {
    const NSRect mainRect = [fWindow->window().contentView bounds];

    fWindow->onResize(mainRect.size.width, mainRect.size.height);
    fWindow->inval();
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    fWindow->closeWindow();

    return FALSE;
}

@end

///////////////////////////////////////////////////////////////////////////////

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

static ModifierKey get_modifiers(const NSEvent* event) {
    NSUInteger modifierFlags = [event modifierFlags];
    ModifierKey modifiers = ModifierKey::kNone;

    if (modifierFlags & NSEventModifierFlagShift) {
        modifiers |= ModifierKey::kShift;
    }
    if (modifierFlags & NSEventModifierFlagControl) {
        modifiers |= ModifierKey::kControl;
    }
    if (modifierFlags & NSEventModifierFlagOption) {
        modifiers |= ModifierKey::kOption;
    }

    if ((NSKeyDown == [event type] || NSKeyUp == [event type]) &&
        NO == [event isARepeat]) {
        modifiers |= ModifierKey::kFirstPress;
    }

    return modifiers;
}

@implementation MainView {
    sk_app::Window_mac* fWindow;
    // A TrackingArea prevents us from capturing events outside the view
    NSTrackingArea* fTrackingArea;
}

- (MainView*)initWithWindow:(sk_app::Window_mac *)initWindow {
    self = [super init];

    fWindow = initWindow;
    fTrackingArea = nil;

    [self updateTrackingAreas];

    return self;
}

- (void)dealloc
{
    [fTrackingArea release];
    [super dealloc];
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

- (void)updateTrackingAreas {
    if (fTrackingArea != nil) {
        [self removeTrackingArea:fTrackingArea];
        [fTrackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    fTrackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:fTrackingArea];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event {
    Window::Key key = get_key([event keyCode]);
    if (key != Window::Key::kNONE) {
        if (!fWindow->onKey(key, InputState::kDown, get_modifiers(event))) {
            if (Window::Key::kEscape == key) {
                [NSApp terminate:fWindow->window()];
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
        (void) fWindow->onKey(key, InputState::kUp, get_modifiers(event));
    }
}

- (void)mouseDown:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [fWindow->window().contentView frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, InputState::kDown,
                    get_modifiers(event));
}

- (void)mouseUp:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [fWindow->window().contentView frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, InputState::kUp,
                     get_modifiers(event));
}

- (void)mouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)mouseMoved:(NSEvent *)event {
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [fWindow->window().contentView frame];
    fWindow->onMouse(pos.x, rect.size.height - pos.y, InputState::kMove,
                     get_modifiers(event));
}

- (void)scrollWheel:(NSEvent *)event {
    // TODO: support hasPreciseScrollingDeltas?
    fWindow->onMouseWheel([event scrollingDeltaY], get_modifiers(event));
}

- (void)drawRect:(NSRect)rect {
    fWindow->onPaint();
}

@end
