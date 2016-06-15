/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

//#include <tchar.h>

#include "SkUtils.h"
#include "Timer.h"
#include "../GLWindowContext.h"
#ifdef SK_VULKAN
#include "../VulkanWindowContext.h"
#endif
#include "Window_unix.h"

extern "C" {
    #include "keysym2ucs.h"
}
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

namespace sk_app {

SkTDynamicHash<Window_unix, XWindow> Window_unix::gWindowMap;

Window* Window::CreateNativeWindow(void* platformData) {
    Display* display = (Display*)platformData;

    Window_unix* window = new Window_unix();
    if (!window->initWindow(display, nullptr)) {
        delete window;
        return nullptr;
    }

    return window;
}

const long kEventMask = ExposureMask | StructureNotifyMask | 
                        KeyPressMask | KeyReleaseMask | 
                        PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

bool Window_unix::initWindow(Display* display, const DisplayParams* params) {
    if (params && params->fMSAASampleCount != fMSAASampleCount) {
        this->closeWindow();
    }
    // we already have a window
    if (fDisplay) {
        return true;
    } 
    fDisplay = display;

    fWidth = 1280;
    fHeight = 960;

    // Attempt to create a window that supports GL
    GLint att[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        GLX_STENCIL_SIZE, 8,
        None
    };
    SkASSERT(nullptr == fVisualInfo);
    if (params && params->fMSAASampleCount > 0) {
        static const GLint kAttCount = SK_ARRAY_COUNT(att);
        GLint msaaAtt[kAttCount + 4];
        memcpy(msaaAtt, att, sizeof(att));
        SkASSERT(None == msaaAtt[kAttCount - 1]);
        msaaAtt[kAttCount - 1] = GLX_SAMPLE_BUFFERS_ARB;
        msaaAtt[kAttCount + 0] = 1;
        msaaAtt[kAttCount + 1] = GLX_SAMPLES_ARB;
        msaaAtt[kAttCount + 2] = params->fMSAASampleCount;
        msaaAtt[kAttCount + 3] = None;
        fVisualInfo = glXChooseVisual(display, DefaultScreen(display), msaaAtt);
        fMSAASampleCount = params->fMSAASampleCount;
    }
    if (nullptr == fVisualInfo) {
        fVisualInfo = glXChooseVisual(display, DefaultScreen(display), att);
        fMSAASampleCount = 0;
    }

    if (fVisualInfo) {
        Colormap colorMap = XCreateColormap(display,
                                            RootWindow(display, fVisualInfo->screen),
                                            fVisualInfo->visual,
                                            AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = colorMap;
        swa.event_mask = kEventMask;
        fWindow = XCreateWindow(display,
                                RootWindow(display, fVisualInfo->screen),
                                0, 0, // x, y
                                fWidth, fHeight,
                                0, // border width
                                fVisualInfo->depth,
                                InputOutput,
                                fVisualInfo->visual,
                                CWEventMask | CWColormap,
                                &swa);
    } else {
        // Create a simple window instead.  We will not be able to show GL
        fWindow = XCreateSimpleWindow(display,
                                      DefaultRootWindow(display),
                                      0, 0,  // x, y
                                      fWidth, fHeight,
                                      0,     // border width
                                      0,     // border value
                                      0);    // background value
        XSelectInput(display, fWindow, kEventMask);
    }

    if (!fWindow) {
        return false;
    }

    // set up to catch window delete message
    fWmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, fWindow, &fWmDeleteMessage, 1);

    // add to hashtable of windows
    gWindowMap.add(this);

    // init event variables
    fPendingPaint = false;
    fPendingResize = false;

    return true;
}

void Window_unix::closeWindow() {
    if (fDisplay) {
        this->detach();
        SkASSERT(fGC);
        XFreeGC(fDisplay, fGC);
        fGC = nullptr;
        gWindowMap.remove(fWindow);
        XDestroyWindow(fDisplay, fWindow);
        fWindow = 0;
        fVisualInfo = nullptr;
        fDisplay = nullptr;
        fMSAASampleCount = 0;
    }
}

static Window::Key get_key(KeySym keysym) {
    static const struct {
        KeySym      fXK;
        Window::Key fKey;
    } gPair[] = {
        { XK_BackSpace, Window::Key::kBack },
        { XK_Clear, Window::Key::kBack },
        { XK_Return, Window::Key::kOK },
        { XK_Up, Window::Key::kUp },
        { XK_Down, Window::Key::kDown },
        { XK_Left, Window::Key::kLeft },
        { XK_Right, Window::Key::kRight }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fXK == keysym) {
            return gPair[i].fKey;
        }
    }
    return Window::Key::kNONE;
}

static uint32_t get_modifiers(const XEvent& event) {
    static const struct {
        unsigned    fXMask;
        unsigned    fSkMask;
    } gModifiers[] = {
        { ShiftMask,   Window::kShift_ModifierKey },
        { ControlMask, Window::kControl_ModifierKey },
        { Mod1Mask,    Window::kOption_ModifierKey },
    };

    auto modifiers = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gModifiers); ++i) {
        if (event.xkey.state & gModifiers[i].fXMask) {
            modifiers |= gModifiers[i].fSkMask;
        }
    }
    return modifiers;
}

bool Window_unix::handleEvent(const XEvent& event) {
    switch (event.type) {
        case MapNotify:
            if (!fGC) {
                fGC = XCreateGC(fDisplay, fWindow, 0, nullptr);
            }
            break;

        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == fWmDeleteMessage &&
                gWindowMap.count() == 1) {
                return true;
            }
            break;

        case ButtonPress:
            if (event.xbutton.button == Button1) {
                this->onMouse(event.xbutton.x, event.xbutton.y,
                              Window::kDown_InputState, get_modifiers(event));
            }
            break;

        case ButtonRelease:
            if (event.xbutton.button == Button1) {
                this->onMouse(event.xbutton.x, event.xbutton.y,
                              Window::kUp_InputState, get_modifiers(event));
            }
            break;

        case MotionNotify:
            // only track if left button is down
            if (event.xmotion.state & Button1Mask) {
                this->onMouse(event.xmotion.x, event.xmotion.y, 
                              Window::kMove_InputState, get_modifiers(event));
            }
            break;

        case KeyPress: {
            int shiftLevel = (event.xkey.state & ShiftMask) ? 1 : 0;
            KeySym keysym = XkbKeycodeToKeysym(fDisplay, event.xkey.keycode,
                                               0, shiftLevel);
            if (keysym == XK_Escape) {
                return true;
            }
            Window::Key key = get_key(keysym);
            if (key != Window::Key::kNONE) {
                (void) this->onKey(key, Window::kDown_InputState, 
                                   get_modifiers(event));
            } else {
                long uni = keysym2ucs(keysym);
                if (uni != -1) {
                    (void) this->onChar((SkUnichar) uni, 
                                        get_modifiers(event));
                }
            }
        } break;

        case KeyRelease: {
            int shiftLevel = (event.xkey.state & ShiftMask) ? 1 : 0;
            KeySym keysym = XkbKeycodeToKeysym(fDisplay, event.xkey.keycode,
                                               0, shiftLevel);
            Window::Key key = get_key(keysym);
            (void) this->onKey(key, Window::kUp_InputState, 
                               get_modifiers(event));
        } break;
        

        default:
            // these events should be handled in the main event loop
            SkASSERT(event.type != Expose && event.type != ConfigureNotify);
            break;
    }

    return false;
}

void Window_unix::setTitle(const char* title) {
    XTextProperty textproperty;
    XStringListToTextProperty(const_cast<char**>(&title), 1, &textproperty);
    XSetWMName(fDisplay, fWindow, &textproperty);    
}

void Window_unix::show() {
    XMapWindow(fDisplay, fWindow);
}

bool Window_unix::attach(BackendType attachType, const DisplayParams& params) {
    this->initWindow(fDisplay, &params);

    ContextPlatformData_unix platformData;
    platformData.fDisplay = fDisplay;
    platformData.fWindow = fWindow;
    platformData.fVisualInfo = fVisualInfo;
    switch (attachType) {
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = VulkanWindowContext::Create((void*)&platformData, params);
            break;
#endif
        case kNativeGL_BackendType:
        default:
            fWindowContext = GLWindowContext::Create((void*)&platformData, params);
            break;
    }

    return (SkToBool(fWindowContext));
}

void Window_unix::onInval() {
    XEvent event;
    event.type = Expose;
    event.xexpose.send_event = True;
    event.xexpose.display = fDisplay;
    event.xexpose.window = fWindow;
    event.xexpose.x = 0;
    event.xexpose.y = 0;
    event.xexpose.width = fWidth;
    event.xexpose.height = fHeight;
    event.xexpose.count = 0;
    
    XSendEvent(fDisplay, fWindow, False, 0, &event);
}

}   // namespace sk_app
