/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/ios/WindowContextFactory_ios.h"
#include "tools/sk_app/ios/Window_ios.h"

@interface WindowViewController : UIViewController

- (WindowViewController*)initWithWindow:(sk_app::Window_ios*)initWindow;

@end

///////////////////////////////////////////////////////////////////////////////

using sk_app::Window;

namespace sk_app {

Window_ios* Window_ios::gWindow = nullptr;

Window* Window::CreateNativeWindow(void*) {
    // already have a window
    if (Window_ios::MainWindow()) {
        return nullptr;
    }

    Window_ios* window = new Window_ios();
    if (!window->initWindow()) {
        delete window;
        return nullptr;
    }

    return window;
}

bool Window_ios::initWindow() {
    // we already have a window
    if (fWindow) {
        return true;
    }

    // Create a delegate to track certain events
    WindowViewController* viewController = [[WindowViewController alloc] initWithWindow:this];
    if (nil == viewController) {
        return false;
    }

    fWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    if (nil == fWindow) {
        [viewController release];
        return false;
    }
    fWindow.backgroundColor = [UIColor whiteColor];

    // create view
//    if (nil == view) {
//        [fWindow release];
//        [viewController release];
//        return false;
//    }
//
    viewController.view = nil;
    [fWindow setRootViewController:viewController];
    [fWindow makeKeyAndVisible];

    // Should be retained by window now
//    [view release];

    gWindow = this;

    return true;
}

void Window_ios::closeWindow() {
    if (nil != fWindow) {
        gWindow = nullptr;
        //*** release?
        fWindow = nil;
        //*** viewController?
    }
}

void Window_ios::setTitle(const char* title) {
//    SDL_SetWindowTitle(fWindow, title);
}

void Window_ios::show() {
//    SDL_ShowWindow(fWindow);
}

bool Window_ios::attach(BackendType attachType) {
    this->initWindow();

    window_context_factory::IOSWindowInfo info;
    info.fWindow = this;
    info.fViewController = fWindow.rootViewController;
    switch (attachType) {
        case kRaster_BackendType:
            fWindowContext = MakeRasterForIOS(info, fRequestedDisplayParams);
            break;
//#ifdef SK_METAL
//        case kMetal_BackendType:
//            fWindowContext = MakeMetalForMac(info, fRequestedDisplayParams);
//            break;
//#endif
        case kNativeGL_BackendType:
        default:
            fWindowContext = MakeGLForIOS(info, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_ios::PaintWindow() {
    gWindow->onPaint();
}

void Window_ios::onInval() {
//    SDL_Event sdlevent;
//    sdlevent.type = SDL_WINDOWEVENT;
//    sdlevent.window.windowID = fWindowID;
//    sdlevent.window.event = SDL_WINDOWEVENT_EXPOSED;
//    SDL_PushEvent(&sdlevent);
}

}   // namespace sk_app

///////////////////////////////////////////////////////////////////////////////

@implementation WindowViewController {
    sk_app::Window_ios* fWindow;
}

- (WindowViewController*)initWithWindow:(sk_app::Window_ios *)initWindow {
    fWindow = initWindow;

    return self;
}

//- (void)windowDidResize:(NSNotification *)notification {
//    const NSRect mainRect = [fWindow->window().contentView bounds];
//
//    fWindow->onResize(mainRect.size.width, mainRect.size.height);
//    fWindow->inval();
//}
//
//- (BOOL)windowShouldClose:(NSWindow*)sender {
//    fWindow->closeWindow();
//
//    return FALSE;
//}

@end

///////////////////////////////////////////////////////////////////////////////

@implementation MainView {
    sk_app::Window_ios* fWindow;
}

- (MainView*)initWithWindow:(sk_app::Window_ios *)initWindow {
    self = [super init];

    fWindow = initWindow;

    return self;
}

#if 0
static Window::Key get_key(const SDL_Keysym& keysym) {
    static const struct {
        SDL_Keycode fSDLK;
        Window::Key fKey;
    } gPair[] = {
        { SDLK_BACKSPACE, Window::Key::kBack },
        { SDLK_CLEAR, Window::Key::kBack },
        { SDLK_RETURN, Window::Key::kOK },
        { SDLK_UP, Window::Key::kUp },
        { SDLK_DOWN, Window::Key::kDown },
        { SDLK_LEFT, Window::Key::kLeft },
        { SDLK_RIGHT, Window::Key::kRight },
        { SDLK_TAB, Window::Key::kTab },
        { SDLK_PAGEUP, Window::Key::kPageUp },
        { SDLK_PAGEDOWN, Window::Key::kPageDown },
        { SDLK_HOME, Window::Key::kHome },
        { SDLK_END, Window::Key::kEnd },
        { SDLK_DELETE, Window::Key::kDelete },
        { SDLK_ESCAPE, Window::Key::kEscape },
        { SDLK_LSHIFT, Window::Key::kShift },
        { SDLK_RSHIFT, Window::Key::kShift },
        { SDLK_LCTRL, Window::Key::kCtrl },
        { SDLK_RCTRL, Window::Key::kCtrl },
        { SDLK_LALT, Window::Key::kOption },
        { SDLK_LALT, Window::Key::kOption },
        { 'A', Window::Key::kA },
        { 'C', Window::Key::kC },
        { 'V', Window::Key::kV },
        { 'X', Window::Key::kX },
        { 'Y', Window::Key::kY },
        { 'Z', Window::Key::kZ },
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fSDLK == keysym.sym) {
            return gPair[i].fKey;
        }
    }
    return Window::Key::kNONE;
}

static ModifierKey get_modifiers(const SDL_Event& event) {
    static const struct {
        unsigned    fSDLMask;
        ModifierKey fSkMask;
    } gModifiers[] = {
        { KMOD_SHIFT, ModifierKey::kShift },
        { KMOD_CTRL,  ModifierKey::kControl },
        { KMOD_ALT,   ModifierKey::kOption },
    };

    ModifierKey modifiers = ModifierKey::kNone;

    switch (event.type) {
        case SDL_KEYDOWN:
            // fall through
        case SDL_KEYUP: {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModifiers); ++i) {
                if (event.key.keysym.mod & gModifiers[i].fSDLMask) {
                    modifiers |= gModifiers[i].fSkMask;
                }
            }
            if (0 == event.key.repeat) {
                modifiers |= ModifierKey::kFirstPress;
            }
            break;
        }

        default: {
            SDL_Keymod mod = SDL_GetModState();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModifiers); ++i) {
                if (mod & gModifiers[i].fSDLMask) {
                    modifiers |= gModifiers[i].fSkMask;
                }
            }
            break;
        }
    }
    return modifiers;
}

bool Window_ios::HandleWindowEvent(const SDL_Event& event) {
    Window_ios* win = gWindowMap.find(event.window.windowID);
    if (win && win->handleEvent(event)) {
        return true;
    }

    return false;
}

bool Window_ios::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_WINDOWEVENT:
            if (SDL_WINDOWEVENT_EXPOSED == event.window.event) {
                this->onPaint();
            } else if (SDL_WINDOWEVENT_RESIZED == event.window.event) {
                this->onResize(event.window.data1, event.window.data2);
            }
            break;

        case SDL_FINGERDOWN:
            this->onTouch(event.tfinger.fingerId, InputState::kDown,
                          (int)(this->width()*event.tfinger.x),
                          (int)(this->height()*event.tfinger.y));
            break;

        case SDL_FINGERUP:
            this->onTouch(event.tfinger.fingerId, InputState::kUp,
                          (int)(this->width()*event.tfinger.x),
                          (int)(this->height()*event.tfinger.y));
            break;

        case SDL_FINGERMOTION:
            this->onTouch(event.tfinger.fingerId, InputState::kMove,
                          (int)(this->width()*event.tfinger.x),
                          (int)(this->height()*event.tfinger.y));
            break;

        case SDL_KEYDOWN: {
            Window::Key key = get_key(event.key.keysym);
            if (key != Window::Key::kNONE) {
                if (!this->onKey(key, InputState::kDown, get_modifiers(event))) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        return true;
                    }
                }
            }
        } break;

        case SDL_KEYUP: {
            Window::Key key = get_key(event.key.keysym);
            if (key != Window::Key::kNONE) {
                (void) this->onKey(key, InputState::kUp,
                                   get_modifiers(event));
            }
        } break;

        case SDL_TEXTINPUT: {
            const char* textIter = &event.text.text[0];
            while (SkUnichar c = SkUTF8_NextUnichar(&textIter)) {
                (void) this->onChar(c, get_modifiers(event));
            }
        } break;

        default:
            break;
    }

    return false;
}
#endif

@end

