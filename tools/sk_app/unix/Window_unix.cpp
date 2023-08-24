/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/window/unix/WindowContextFactory_unix.h"

#include "src/base/SkUTF.h"
#include "tools/sk_app/unix/Window_unix.h"
#include "tools/skui/ModifierKey.h"
#include "tools/timer/Timer.h"
#include "tools/window/WindowContext.h"

extern "C" {
    #include "tools/sk_app/unix/keysym2ucs.h"
}
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

namespace sk_app {

SkTDynamicHash<Window_unix, XWindow> Window_unix::gWindowMap;

Window* Window::CreateNativeWindow(void* platformData) {
    Display* display = (Display*)platformData;
    SkASSERT(display);

    Window_unix* window = new Window_unix();
    if (!window->initWindow(display)) {
        delete window;
        return nullptr;
    }

    return window;
}

const long kEventMask = ExposureMask | StructureNotifyMask |
                        KeyPressMask | KeyReleaseMask |
                        PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

bool Window_unix::initWindow(Display* display) {
    if (fRequestedDisplayParams.fMSAASampleCount != fMSAASampleCount) {
        this->closeWindow();
    }
    // we already have a window
    if (fDisplay) {
        return true;
    }
    fDisplay = display;

    constexpr int initialWidth = 1280;
    constexpr int initialHeight = 960;

#ifdef SK_GL
    // Attempt to create a window that supports GL

    // We prefer the more recent glXChooseFBConfig but fall back to glXChooseVisual. They have
    // slight differences in how attributes are specified.
    static int constexpr kChooseFBConfigAtt[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_STENCIL_SIZE, 8,
        None
    };
    // For some reason glXChooseVisual takes a non-const pointer to the attributes.
    int chooseVisualAtt[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_STENCIL_SIZE, 8,
        None
    };
    SkASSERT(nullptr == fVisualInfo);
    if (fRequestedDisplayParams.fMSAASampleCount > 1) {
        static const GLint kChooseFBConifgAttCnt = std::size(kChooseFBConfigAtt);
        GLint msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 4];
        memcpy(msaaChooseFBConfigAtt, kChooseFBConfigAtt, sizeof(kChooseFBConfigAtt));
        SkASSERT(None == msaaChooseFBConfigAtt[kChooseFBConifgAttCnt - 1]);
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt - 1] = GLX_SAMPLE_BUFFERS_ARB;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 0] = 1;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 1] = GLX_SAMPLES_ARB;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 2] = fRequestedDisplayParams.fMSAASampleCount;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 3] = None;
        int n;
        fFBConfig = glXChooseFBConfig(fDisplay, DefaultScreen(fDisplay), msaaChooseFBConfigAtt, &n);
        if (n > 0) {
            fVisualInfo = glXGetVisualFromFBConfig(fDisplay, *fFBConfig);
        } else {
            static const GLint kChooseVisualAttCnt = std::size(chooseVisualAtt);
            GLint msaaChooseVisualAtt[kChooseVisualAttCnt + 4];
            memcpy(msaaChooseVisualAtt, chooseVisualAtt, sizeof(chooseVisualAtt));
            SkASSERT(None == msaaChooseVisualAtt[kChooseVisualAttCnt - 1]);
            msaaChooseFBConfigAtt[kChooseVisualAttCnt - 1] = GLX_SAMPLE_BUFFERS_ARB;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 0] = 1;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 1] = GLX_SAMPLES_ARB;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 2] =
                    fRequestedDisplayParams.fMSAASampleCount;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 3] = None;
            fVisualInfo = glXChooseVisual(display, DefaultScreen(display), msaaChooseVisualAtt);
            fFBConfig = nullptr;
        }
    }
    if (nullptr == fVisualInfo) {
        int n;
        fFBConfig = glXChooseFBConfig(fDisplay, DefaultScreen(fDisplay), kChooseFBConfigAtt, &n);
        if (n > 0) {
            fVisualInfo = glXGetVisualFromFBConfig(fDisplay, *fFBConfig);
        } else {
            fVisualInfo = glXChooseVisual(display, DefaultScreen(display), chooseVisualAtt);
            fFBConfig = nullptr;
        }
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
                                initialWidth, initialHeight,
                                0, // border width
                                fVisualInfo->depth,
                                InputOutput,
                                fVisualInfo->visual,
                                CWEventMask | CWColormap,
                                &swa);
    }
#endif
    if (!fWindow) {
        // Create a simple window instead.  We will not be able to show GL
        fWindow = XCreateSimpleWindow(display,
                                      DefaultRootWindow(display),
                                      0, 0,  // x, y
                                      initialWidth, initialHeight,
                                      0,     // border width
                                      0,     // border value
                                      0);    // background value
        XSelectInput(display, fWindow, kEventMask);
    }

    if (!fWindow) {
        return false;
    }

    fMSAASampleCount = fRequestedDisplayParams.fMSAASampleCount;

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
        if (fGC) {
            XFreeGC(fDisplay, fGC);
            fGC = nullptr;
        }
        gWindowMap.remove(fWindow);
        XDestroyWindow(fDisplay, fWindow);
        fWindow = 0;
        if (fFBConfig) {
            XFree(fFBConfig);
            fFBConfig = nullptr;
        }
        if (fVisualInfo) {
            XFree(fVisualInfo);
            fVisualInfo = nullptr;
        }
        fDisplay = nullptr;
    }
}

static skui::Key get_key(KeySym keysym) {
    static const struct {
        KeySym      fXK;
        skui::Key fKey;
    } gPair[] = {
        { XK_BackSpace, skui::Key::kBack     },
        { XK_Clear,     skui::Key::kBack     },
        { XK_Return,    skui::Key::kOK       },
        { XK_Up,        skui::Key::kUp       },
        { XK_Down,      skui::Key::kDown     },
        { XK_Left,      skui::Key::kLeft     },
        { XK_Right,     skui::Key::kRight    },
        { XK_Tab,       skui::Key::kTab      },
        { XK_Page_Up,   skui::Key::kPageUp   },
        { XK_Page_Down, skui::Key::kPageDown },
        { XK_Home,      skui::Key::kHome     },
        { XK_End,       skui::Key::kEnd      },
        { XK_Delete,    skui::Key::kDelete   },
        { XK_Escape,    skui::Key::kEscape   },
        { XK_Shift_L,   skui::Key::kShift    },
        { XK_Shift_R,   skui::Key::kShift    },
        { XK_Control_L, skui::Key::kCtrl     },
        { XK_Control_R, skui::Key::kCtrl     },
        { XK_Alt_L,     skui::Key::kOption   },
        { XK_Alt_R,     skui::Key::kOption   },
        { 'a',          skui::Key::kA        },
        { 'c',          skui::Key::kC        },
        { 'v',          skui::Key::kV        },
        { 'x',          skui::Key::kX        },
        { 'y',          skui::Key::kY        },
        { 'z',          skui::Key::kZ        },
    };
    for (size_t i = 0; i < std::size(gPair); i++) {
        if (gPair[i].fXK == keysym) {
            return gPair[i].fKey;
        }
    }
    return skui::Key::kNONE;
}

static skui::ModifierKey get_modifiers(const XEvent& event) {
    static const struct {
        unsigned    fXMask;
        skui::ModifierKey  fSkMask;
    } gModifiers[] = {
        { ShiftMask,   skui::ModifierKey::kShift },
        { ControlMask, skui::ModifierKey::kControl },
        { Mod1Mask,    skui::ModifierKey::kOption },
    };

    skui::ModifierKey modifiers = skui::ModifierKey::kNone;
    for (size_t i = 0; i < std::size(gModifiers); ++i) {
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
            switch (event.xbutton.button) {
                case Button1:
                    this->onMouse(event.xbutton.x, event.xbutton.y,
                                  skui::InputState::kDown, get_modifiers(event));
                    break;
                case Button4:
                    this->onMouseWheel(1.0f, 0, 0, get_modifiers(event));
                    break;
                case Button5:
                    this->onMouseWheel(-1.0f, 0, 0, get_modifiers(event));
                    break;
            }
            break;

        case ButtonRelease:
            if (event.xbutton.button == Button1) {
                this->onMouse(event.xbutton.x, event.xbutton.y,
                              skui::InputState::kUp, get_modifiers(event));
            }
            break;

        case MotionNotify:
            this->onMouse(event.xmotion.x, event.xmotion.y,
                          skui::InputState::kMove, get_modifiers(event));
            break;

        case KeyPress: {
            int shiftLevel = (event.xkey.state & ShiftMask) ? 1 : 0;
            KeySym keysym = XkbKeycodeToKeysym(fDisplay, event.xkey.keycode, 0, shiftLevel);
            skui::Key key = get_key(keysym);
            if (key != skui::Key::kNONE) {
                if (!this->onKey(key, skui::InputState::kDown, get_modifiers(event))) {
                    if (keysym == XK_Escape) {
                        return true;
                    }
                }
            }

            long uni = keysym2ucs(keysym);
            if (uni != -1) {
                (void) this->onChar((SkUnichar) uni, get_modifiers(event));
            }
        } break;

        case KeyRelease: {
            int shiftLevel = (event.xkey.state & ShiftMask) ? 1 : 0;
            KeySym keysym = XkbKeycodeToKeysym(fDisplay, event.xkey.keycode,
                                               0, shiftLevel);
            skui::Key key = get_key(keysym);
            (void) this->onKey(key, skui::InputState::kUp,
                               get_modifiers(event));
        } break;

        case SelectionClear: {
            // Lost selection ownership
            fClipboardText.clear();
        } break;

        case SelectionRequest: {
            Atom UTF8      = XInternAtom(fDisplay, "UTF8_STRING", 0),
                 CLIPBOARD = XInternAtom(fDisplay, "CLIPBOARD", 0);

            const XSelectionRequestEvent* xsr = &event.xselectionrequest;

            XSelectionEvent xsel = {};
            xsel.type      = SelectionNotify;
            xsel.requestor = xsr->requestor;
            xsel.selection = xsr->selection;
            xsel.target    = xsr->target;
            xsel.property  = xsr->property;
            xsel.time      = xsr->time;

            if (xsr->selection != CLIPBOARD) {
                // A request for a different kind of selection. This shouldn't happen.
                break;
            }

            if (fClipboardText.empty() || xsr->target != UTF8 || xsr->property == None) {
                // We can't fulfill this request. Deny it.
                xsel.property = None;
                XSendEvent(fDisplay, xsr->requestor, True, NoEventMask, (XEvent*)&xsel);
            } else {
                // We can fulfill this request! Update the contents of the CLIPBOARD property,
                // and let the requestor know.
                XChangeProperty(fDisplay, xsr->requestor, xsr->property, UTF8, /*format=*/8,
                                PropModeReplace, (unsigned char*)fClipboardText.data(),
                                fClipboardText.length());
                XSendEvent(fDisplay, xsr->requestor, True, NoEventMask, (XEvent*)&xsel);
            }
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
    if (!XStringListToTextProperty(const_cast<char**>(&title), 1, &textproperty)) {
        return;
    }
    XSetWMName(fDisplay, fWindow, &textproperty);
    XFree(textproperty.value);
}

void Window_unix::show() {
    XMapWindow(fDisplay, fWindow);
}

bool Window_unix::attach(BackendType attachType) {
    fBackend = attachType;

    this->initWindow(fDisplay);

    skwindow::XlibWindowInfo winInfo;
    winInfo.fDisplay = fDisplay;
    winInfo.fWindow = fWindow;
    winInfo.fFBConfig = fFBConfig;
    winInfo.fVisualInfo = fVisualInfo;

    XWindowAttributes attrs;
    if (XGetWindowAttributes(fDisplay, fWindow, &attrs)) {
        winInfo.fWidth = attrs.width;
        winInfo.fHeight = attrs.height;
    } else {
        winInfo.fWidth = winInfo.fHeight = 0;
    }

    switch (attachType) {
#if defined(SK_DAWN) && defined(SK_GRAPHITE)
        case kGraphiteDawn_BackendType:
            fWindowContext = skwindow::MakeGraphiteDawnVulkanForXlib(winInfo,
                                                                     fRequestedDisplayParams);
            break;
#endif
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = skwindow::MakeVulkanForXlib(winInfo, fRequestedDisplayParams);
            break;
#endif
#if defined(SK_VULKAN) && defined(SK_GRAPHITE)
        case kGraphiteVulkan_BackendType:
            fWindowContext = skwindow::MakeGraphiteVulkanForXlib(winInfo, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_GL
        case kNativeGL_BackendType:
            fWindowContext = skwindow::MakeGLForXlib(winInfo, fRequestedDisplayParams);
            break;
#endif
        case kRaster_BackendType:
            fWindowContext = skwindow::MakeRasterForXlib(winInfo, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

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
    event.xexpose.width = this->width();
    event.xexpose.height = this->height();
    event.xexpose.count = 0;

    XSendEvent(fDisplay, fWindow, False, 0, &event);
}

void Window_unix::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
#if defined(SK_VULKAN)
    // Vulkan on unix crashes if we try to reinitialize the vulkan context without remaking the
    // window.
    if (fBackend == kVulkan_BackendType && allowReattach) {
        // Need to change these early, so attach() creates the window context correctly
        fRequestedDisplayParams = params;

        this->detach();
        this->attach(fBackend);
        return;
    }
#endif

    Window::setRequestedDisplayParams(params, allowReattach);
}

const char* Window_unix::getClipboardText() {
    Atom UTF8      = XInternAtom(fDisplay, "UTF8_STRING", 0),
         CLIPBOARD = XInternAtom(fDisplay, "CLIPBOARD", 0),
         XSEL_DATA = XInternAtom(fDisplay, "XSEL_DATA", 0);

    // Ask for a UTF8 copy of the CLIPBOARD...
    XEvent event;
    XConvertSelection(fDisplay, CLIPBOARD, UTF8, XSEL_DATA, fWindow, CurrentTime);
    XSync(fDisplay, 0);
    XNextEvent(fDisplay, &event);
    if (event.type == SelectionNotify &&
            event.xselection.selection == CLIPBOARD &&
            event.xselection.property != None) {

        // We got a response
        Atom type;
        int format;
        unsigned long nitems, bytes_after;
        char* data;

        // Fetch the CLIPBOARD property
        XSelectionEvent xsel = event.xselection;
        XGetWindowProperty(xsel.display, xsel.requestor, xsel.property, /*offset=*/0,
                           /*length=*/~0L, /*delete=*/False, AnyPropertyType, &type, &format,
                           &nitems, &bytes_after, (unsigned char**)&data);
        SkASSERT(bytes_after == 0);
        if (type == UTF8) {
            fClipboardText.assign(data, nitems);
        }
        XFree(data);
        XDeleteProperty(xsel.display, xsel.requestor, xsel.property);
    }
    return fClipboardText.c_str();
}

void Window_unix::setClipboardText(const char* text) {
    fClipboardText.assign(text);

    // Take ownership of the CLIPBOARD
    Atom CLIPBOARD = XInternAtom(fDisplay, "CLIPBOARD", 0);
    XSetSelectionOwner(fDisplay, CLIPBOARD, fWindow, CurrentTime);
}

}   // namespace sk_app
