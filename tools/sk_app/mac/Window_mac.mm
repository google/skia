/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/core/SkUtils.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"
#include "tools/sk_app/mac/Window_mac.h"

///////////////////////////////////////////////////////////////////////////////

using sk_app::Window;
int gWindowCount = 0;

namespace sk_app {

void closeWindowCallback(GLFWwindow* window) {
    --gWindowCount;
}
void resizeWindowCallback(GLFWwindow* window, int w, int h) {
    sk_app::Window_mac* macWindow =
        reinterpret_cast<sk_app::Window_mac*>(glfwGetWindowUserPointer(window));
    macWindow->onResize(w, h);
}

void refreshWindowCallback(GLFWwindow* window) {
    sk_app::Window_mac* macWindow =
        reinterpret_cast<sk_app::Window_mac*>(glfwGetWindowUserPointer(window));
    macWindow->onPaint();
}

//void resizeFramebufferWindowCallback(GLFWwindow* window, int w, int h) {
//    sk_app::Window_mac* macWindow = reinterpret_cast<sk_app::Window_mac*>(glfwGetWindowUserPointer(fWindow));
//    macWindow->onResize(w, h);
//}

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

    fWindow = glfwCreateWindow(initialWidth, initialHeight, "App", nullptr, nullptr);
    if (!fWindow) {
        return false;
    }

    glfwSetWindowUserPointer(fWindow, this);

    glfwSetWindowSizeCallback(fWindow, resizeWindowCallback);
//        glfwSetWindowFramebufferSizeCallback(fWindow, resizeFramebufferWindowCallback);
    glfwSetWindowCloseCallback(fWindow, closeWindowCallback);
    glfwSetWindowRefreshCallback(fWindow, refreshWindowCallback);

    return true;
}

void Window_mac::closeWindow() {
    glfwDestroyWindow(fWindow);
    fWindow = nullptr;
}

void Window_mac::setTitle(const char* title) {
    glfwSetWindowTitle(fWindow, title);
}

void Window_mac::show() {
    glfwShowWindow(fWindow);
}

bool Window_mac::attach(BackendType attachType) {
    this->initWindow();

    window_context_factory::MacWindowInfo info;
    info.fWindow = fWindow;
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

void Window_mac::onInval() {
//    [[fWindow contentView] setNeedsDisplay:YES];
    // MacOS already queues a single drawRect event for multiple invalidations
    // so we don't need to use our invalidation method (and it can mess things up
    // if for some reason MacOS skips a drawRect when we need one).
//    this->markInvalProcessed();
}

}   // namespace sk_app

///////////////////////////////////////////////////////////////////////////////

/*
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

void Window_mac::PaintWindows() {
    SkTDynamicHash<Window_mac, NSInteger>::Iter iter(&gWindowMap);
    while (!iter.done()) {
        if ((*iter).fIsContentInvalidated) {
            (*iter).onPaint();
        }
        ++iter;
    }
}

void Window_mac::HandleWindowEvent(const NSEvent* event) {
    Window_mac* win = gWindowMap.find(event.window.windowNumber);
    if (win) {
        win->handleEvent(event);
    }
}

void Window_mac::handleEvent(const NSEvent* event) {
    switch (event.type) {
        case NSEventTypeKeyDown: {
            Window::Key key = get_key([event keyCode]);
            if (key != Window::Key::kNONE) {
                if (!this->onKey(key, Window::kDown_InputState, get_modifiers(event))) {
                    if (Window::Key::kEscape == key) {
                        [NSApp terminate:fWindow];
                    }
                }
            }

            NSString* characters = [event charactersIgnoringModifiers];
            NSUInteger len = [characters length];
            if (len > 0) {
                unichar* charBuffer = new unichar[len+1];
                [characters getCharacters:charBuffer range:NSMakeRange(0, len)];
                for (NSUInteger i = 0; i < len; ++i) {
                    (void) this->onChar((SkUnichar) charBuffer[i], get_modifiers(event));
                }
                delete [] charBuffer;
            }
            break;
        }
        case NSEventTypeKeyUp: {
            Window::Key key = get_key([event keyCode]);
            if (key != Window::Key::kNONE) {
                (void) this->onKey(key, Window::kUp_InputState, get_modifiers(event));
            }
            break;
        }
        case NSEventTypeLeftMouseDown: {
            const NSPoint pos = [event locationInWindow];
            const NSRect rect = [fWindow.contentView frame];
            if (NSPointInRect(pos, rect)) {
                // This might be a resize event -- until we know we'll store the event
                // and reset later if need be
                fIsMouseDown = true;
                fMouseDownPos = pos;
                fMouseModifiers = get_modifiers(event);
                this->onMouse(pos.x, rect.size.height - pos.y, Window::kDown_InputState,
                              fMouseModifiers);
            }
            break;
        }
        case NSEventTypeLeftMouseUp: {
            const NSPoint pos = [event locationInWindow];
            const NSRect rect = [fWindow.contentView frame];
            this->onMouse(pos.x, rect.size.height - pos.y, Window::kUp_InputState,
                          get_modifiers(event));
            break;
        }
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged: {
            const NSPoint pos = [event locationInWindow];
            const NSRect rect = [fWindow.contentView frame];
            this->onMouse(pos.x, rect.size.height - pos.y, Window::kMove_InputState,
                          get_modifiers(event));
            break;
        }
        case NSEventTypeScrollWheel:
            // TODO: support hasPreciseScrollingDeltas?
            this->onMouseWheel([event scrollingDeltaY], get_modifiers(event));
            break;

        default:
            break;
    }
}

void Window_mac::resetMouse() {
    if (fIsMouseDown) {
        // We're resizing so just send a mouse up event in the same place
        const NSRect rect = [fWindow.contentView frame];
        this->onMouse(fMouseDownPos.x, rect.size.height - fMouseDownPos.y, Window::kUp_InputState,
                      fMouseModifiers);
        fIsMouseDown = false;
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
    fWindow->resetMouse();
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    fWindow->closeWindow();

    return FALSE;
}

@end

///////////////////////////////////////////////////////////////////////////////

@implementation MainView
- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}


// We keep these around to prevent beeping when the system can't determine
// where the focus for key events is.
- (void)keyDown:(NSEvent *)event {
}
*/
