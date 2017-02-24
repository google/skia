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

namespace sk_app {

SkTDynamicHash<Window_mac, Uint32> Window_mac::gWindowMap;

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (fRequestedDisplayParams.fMSAASampleCount > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fRequestedDisplayParams.fMSAASampleCount);
    } else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    }
    // TODO: handle other display params

    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    fWindow = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               initialWidth, initialHeight, windowFlags);

    if (!fWindow) {
        return false;
    }

    fMSAASampleCount = fRequestedDisplayParams.fMSAASampleCount;

    // add to hashtable of windows
    fWindowID = SDL_GetWindowID(fWindow);
    gWindowMap.add(this);

    return true;
}

void Window_mac::closeWindow() {
    if (fWindow) {
        gWindowMap.remove(fWindowID);
        SDL_DestroyWindow(fWindow);
        fWindowID = 0;
        fWindow = nullptr;
    }
}

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

static uint32_t get_modifiers(const SDL_Event& event) {
    static const struct {
        unsigned    fSDLMask;
        unsigned    fSkMask;
    } gModifiers[] = {
        { KMOD_SHIFT, Window::kShift_ModifierKey },
        { KMOD_CTRL,  Window::kControl_ModifierKey },
        { KMOD_ALT,   Window::kOption_ModifierKey },
    };

    auto modifiers = 0;

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
                modifiers |= Window::kFirstPress_ModifierKey;
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

bool Window_mac::HandleWindowEvent(const SDL_Event& event) {
    Window_mac* win = gWindowMap.find(event.window.windowID);
    if (win && win->handleEvent(event)) {
        return true;
    }

    return false;
}

bool Window_mac::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_WINDOWEVENT:
            if (SDL_WINDOWEVENT_EXPOSED == event.window.event) {
                this->onPaint();
            } else if (SDL_WINDOWEVENT_RESIZED == event.window.event) {
                this->onResize(event.window.data1, event.window.data2);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                this->onMouse(event.button.x, event.button.y,
                              Window::kDown_InputState, get_modifiers(event));
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                this->onMouse(event.button.x, event.button.y,
                              Window::kUp_InputState, get_modifiers(event));
            }
            break;

        case SDL_MOUSEMOTION:
            this->onMouse(event.motion.x, event.motion.y,
                          Window::kMove_InputState, get_modifiers(event));
            break;

        case SDL_MOUSEWHEEL:
            this->onMouseWheel(event.wheel.y, get_modifiers(event));
            break;

        case SDL_KEYDOWN: {
            Window::Key key = get_key(event.key.keysym);
            if (key != Window::Key::kNONE) {
                if (!this->onKey(key, Window::kDown_InputState, get_modifiers(event))) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        return true;
                    }
                }
            }
        } break;

        case SDL_KEYUP: {
            Window::Key key = get_key(event.key.keysym);
            if (key != Window::Key::kNONE) {
                (void) this->onKey(key, Window::kUp_InputState,
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

void Window_mac::setTitle(const char* title) {
    SDL_SetWindowTitle(fWindow, title);
}

void Window_mac::show() {
    SDL_ShowWindow(fWindow);
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
    SDL_Event sdlevent;
    sdlevent.type = SDL_WINDOWEVENT;
    sdlevent.window.windowID = fWindowID;
    sdlevent.window.event = SDL_WINDOWEVENT_EXPOSED;
    SDL_PushEvent(&sdlevent);
}

}   // namespace sk_app
