/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <Carbon/Carbon.h>

#include "include/core/SkTypes.h"
#include "tools/sk_app/mac/Window_mac.h"
#include "tools/skui/ModifierKey.h"
#include "tools/window/mac/WindowContextFactory_mac.h"

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
    MainView* view = [[MainView alloc] initWithWindow:this];
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
    if (NSString* titleStr = [NSString stringWithUTF8String:title]) {
        [fWindow setTitle:titleStr];
    }
}

void Window_mac::show() {
    [fWindow orderFront:nil];

    [NSApp activateIgnoringOtherApps:YES];
    [fWindow makeKeyAndOrderFront:NSApp];
}

bool Window_mac::attach(BackendType attachType) {
    this->initWindow();

    skwindow::MacWindowInfo info;
    info.fMainView = [fWindow contentView];
    switch (attachType) {
#if SK_ANGLE
        case kANGLE_BackendType:
            fWindowContext = skwindow::MakeANGLEForMac(info, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
        case kGraphiteDawn_BackendType:
            fWindowContext = MakeGraphiteDawnMetalForMac(info, fRequestedDisplayParams);
            break;
#endif
#endif
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = MakeVulkanForMac(info, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_METAL
        case kMetal_BackendType:
            fWindowContext = MakeMetalForMac(info, fRequestedDisplayParams);
            break;
#if defined(SK_GRAPHITE)
        case kGraphiteMetal_BackendType:
            fWindowContext = MakeGraphiteMetalForMac(info, fRequestedDisplayParams);
            break;
#endif
#endif
#ifdef SK_GL
        case kNativeGL_BackendType:
            fWindowContext = MakeGLForMac(info, fRequestedDisplayParams);
            break;
        case kRaster_BackendType:
            fWindowContext = MakeRasterForMac(info, fRequestedDisplayParams);
            break;
#endif
        default:
            SkASSERT_RELEASE(false);
    }
    this->onBackendCreated();

    return SkToBool(fWindowContext);
}

float Window_mac::scaleFactor() const {
    return skwindow::GetBackingScaleFactor(fWindow.contentView);
}

void Window_mac::PaintWindows() {
    gWindowMap.foreach([&](Window_mac* window) {
        if (window->fIsContentInvalidated) {
            window->onPaint();
        }
    });
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
    NSView* view = fWindow->window().contentView;
    CGFloat scale = skwindow::GetBackingScaleFactor(view);
    fWindow->onResize(view.bounds.size.width * scale, view.bounds.size.height * scale);
    fWindow->inval();
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    fWindow->closeWindow();

    return FALSE;
}

@end

///////////////////////////////////////////////////////////////////////////////

static skui::Key get_key(unsigned short vk) {
    // This will work with an ANSI QWERTY keyboard.
    // Something more robust would be needed to support alternate keyboards.
    static const struct {
        unsigned short fVK;
        skui::Key      fKey;
    } gPair[] = {
        { kVK_Delete,        skui::Key::kBack },
        { kVK_Return,        skui::Key::kOK },
        { kVK_UpArrow,       skui::Key::kUp },
        { kVK_DownArrow,     skui::Key::kDown },
        { kVK_LeftArrow,     skui::Key::kLeft },
        { kVK_RightArrow,    skui::Key::kRight },
        { kVK_Tab,           skui::Key::kTab },
        { kVK_PageUp,        skui::Key::kPageUp },
        { kVK_PageDown,      skui::Key::kPageDown },
        { kVK_Home,          skui::Key::kHome },
        { kVK_End,           skui::Key::kEnd },
        { kVK_ForwardDelete, skui::Key::kDelete },
        { kVK_Escape,        skui::Key::kEscape },
        { kVK_Shift,         skui::Key::kShift },
        { kVK_RightShift,    skui::Key::kShift },
        { kVK_Control,       skui::Key::kCtrl },
        { kVK_RightControl,  skui::Key::kCtrl },
        { kVK_Option,        skui::Key::kOption },
        { kVK_RightOption,   skui::Key::kOption },
        { kVK_Command,       skui::Key::kSuper },
        { kVK_RightCommand,  skui::Key::kSuper },
        { kVK_ANSI_A,        skui::Key::kA },
        { kVK_ANSI_C,        skui::Key::kC },
        { kVK_ANSI_V,        skui::Key::kV },
        { kVK_ANSI_X,        skui::Key::kX },
        { kVK_ANSI_Y,        skui::Key::kY },
        { kVK_ANSI_Z,        skui::Key::kZ },
    };

    for (size_t i = 0; i < std::size(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }

    return skui::Key::kNONE;
}

static skui::ModifierKey get_modifiers(const NSEvent* event) {
    NSUInteger modifierFlags = [event modifierFlags];
    skui::ModifierKey modifiers = skui::ModifierKey::kNone;

    if (modifierFlags & NSEventModifierFlagCommand) {
        modifiers |= skui::ModifierKey::kCommand;
    }
    if (modifierFlags & NSEventModifierFlagShift) {
        modifiers |= skui::ModifierKey::kShift;
    }
    if (modifierFlags & NSEventModifierFlagControl) {
        modifiers |= skui::ModifierKey::kControl;
    }
    if (modifierFlags & NSEventModifierFlagOption) {
        modifiers |= skui::ModifierKey::kOption;
    }

    if ((NSKeyDown == [event type] || NSKeyUp == [event type]) && ![event isARepeat]) {
        modifiers |= skui::ModifierKey::kFirstPress;
    }

    return modifiers;
}

@implementation MainView {
    sk_app::Window_mac* fWindow;
    // A TrackingArea prevents us from capturing events outside the view
    NSTrackingArea* fTrackingArea;
    // We keep track of the state of the modifier keys on each event in order to synthesize
    // key-up/down events for each modifier.
    skui::ModifierKey fLastModifiers;
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

- (skui::ModifierKey) updateModifierKeys:(NSEvent*) event {
    using sknonstd::Any;

    skui::ModifierKey modifiers = get_modifiers(event);
    skui::ModifierKey changed = modifiers ^ fLastModifiers;
    fLastModifiers = modifiers;

    struct ModMap {
        skui::ModifierKey modifier;
        skui::Key         key;
    };

    // Map each modifier bit to the equivalent skui Key and send key-up/down events.
    for (const ModMap& cur : {ModMap{skui::ModifierKey::kCommand, skui::Key::kSuper},
                              ModMap{skui::ModifierKey::kShift,   skui::Key::kShift},
                              ModMap{skui::ModifierKey::kControl, skui::Key::kCtrl},
                              ModMap{skui::ModifierKey::kOption,  skui::Key::kOption}}) {
        if (Any(changed & cur.modifier)) {
            const skui::InputState state = Any(modifiers & cur.modifier) ? skui::InputState::kDown
                                                                         : skui::InputState::kUp;
            (void) fWindow->onKey(cur.key, state, modifiers);
        }
    }

    return modifiers;
}

- (BOOL)performKeyEquivalent:(NSEvent *)event {
    [self updateModifierKeys:event];

    // By default, unhandled key equivalents send -keyDown events; unfortunately, they do not send
    // a matching -keyUp. In other words, we can claim that we didn't handle the event and OS X will
    // turn this event into a -keyDown automatically, but we need to synthesize a matching -keyUp on
    // a later frame. Since we only read the modifiers and key code from the event, we can reuse
    // this "key-equivalent" event as a "key up".
    [self performSelector:@selector(keyUp:) withObject:event afterDelay:0.1];
    return NO;
}

- (void)keyDown:(NSEvent *)event {
    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    skui::Key key = get_key([event keyCode]);
    if (key != skui::Key::kNONE) {
        if (!fWindow->onKey(key, skui::InputState::kDown, modifiers)) {
            if (skui::Key::kEscape == key) {
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
            (void) fWindow->onChar((SkUnichar) charBuffer[i], modifiers);
        }
        delete [] charBuffer;
    }
}

- (void)keyUp:(NSEvent *)event {
    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    skui::Key key = get_key([event keyCode]);
    if (key != skui::Key::kNONE) {
        (void) fWindow->onKey(key, skui::InputState::kUp, modifiers);
    }
}

-(void)flagsChanged:(NSEvent *)event {
    [self updateModifierKeys:event];
}

- (void)mouseDown:(NSEvent *)event {
    NSView* view = fWindow->window().contentView;
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(view);

    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [view frame];
    fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
                     skui::InputState::kDown, modifiers);
}

- (void)mouseUp:(NSEvent *)event {
    NSView* view = fWindow->window().contentView;
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(view);

    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [view frame];
    fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
                     skui::InputState::kUp, modifiers);
}

- (void)mouseDragged:(NSEvent *)event {
    [self updateModifierKeys:event];
    [self mouseMoved:event];
}

- (void)mouseMoved:(NSEvent *)event {
    NSView* view = fWindow->window().contentView;
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(view);

    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [view frame];
    fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
                     skui::InputState::kMove, modifiers);
}

- (void)scrollWheel:(NSEvent *)event {
    NSView* view = fWindow->window().contentView;
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(view);

    skui::ModifierKey modifiers = [self updateModifierKeys:event];

    // TODO: support hasPreciseScrollingDeltas?
    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [view frame];
    fWindow->onMouseWheel([event scrollingDeltaY],
                          pos.x * backingScaleFactor,
                          (rect.size.height - pos.y) * backingScaleFactor,
                          modifiers);
}

- (void)drawRect:(NSRect)rect {
    fWindow->onPaint();
}

@end
