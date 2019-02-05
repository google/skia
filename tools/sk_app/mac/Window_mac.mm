/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkUtils.h"
#include "WindowContextFactory_mac.h"
#include "Window_mac.h"



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
*/


