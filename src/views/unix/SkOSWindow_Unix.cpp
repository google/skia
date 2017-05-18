/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkWindow.h"
#include "XkeysToSkKeys.h"
extern "C" {
    #include "keysym2ucs.h"
}

const int WIDTH = 500;
const int HEIGHT = 500;

// Determine which events to listen for.
const long EVENT_MASK = StructureNotifyMask|ButtonPressMask|ButtonReleaseMask
        |ExposureMask|PointerMotionMask|KeyPressMask|KeyReleaseMask;

SkOSWindow::SkOSWindow(void*)
    : fVi(nullptr)
    , fMSAASampleCount(0) {
    fUnixWindow.fDisplay = nullptr;
    fUnixWindow.fGLContext = nullptr;
    this->initWindow(0, nullptr);
    this->resize(WIDTH, HEIGHT);
}

SkOSWindow::~SkOSWindow() {
    this->internalCloseWindow();
}

void SkOSWindow::internalCloseWindow() {
    if (fUnixWindow.fDisplay) {
        this->release();
        SkASSERT(fUnixWindow.fGc);
        XFreeGC(fUnixWindow.fDisplay, fUnixWindow.fGc);
        fUnixWindow.fGc = nullptr;
        XDestroyWindow(fUnixWindow.fDisplay, fUnixWindow.fWin);
        fVi = nullptr;
        XCloseDisplay(fUnixWindow.fDisplay);
        fUnixWindow.fDisplay = nullptr;
        fMSAASampleCount = 0;
    }
}

void SkOSWindow::initWindow(int requestedMSAASampleCount, AttachmentInfo* info) {
    if (fMSAASampleCount != requestedMSAASampleCount) {
        this->internalCloseWindow();
    }
    // presence of fDisplay means we already have a window
    if (fUnixWindow.fDisplay) {
        if (info) {
            if (fVi) {
                glXGetConfig(fUnixWindow.fDisplay, fVi, GLX_SAMPLES_ARB, &info->fSampleCount);
                glXGetConfig(fUnixWindow.fDisplay, fVi, GLX_STENCIL_SIZE, &info->fStencilBits);
            } else {
                info->fSampleCount = 0;
                info->fStencilBits = 0;
            }
        }
        return;
    }
    fUnixWindow.fDisplay = XOpenDisplay(nullptr);
    Display* dsp = fUnixWindow.fDisplay;
    if (nullptr == dsp) {
        SkDebugf("Could not open an X Display");
        return;
    }
    // Attempt to create a window that supports GL
    GLint att[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        GLX_STENCIL_SIZE, 8,
        None
    };
    SkASSERT(nullptr == fVi);
    if (requestedMSAASampleCount > 0) {
        static const GLint kAttCount = SK_ARRAY_COUNT(att);
        GLint msaaAtt[kAttCount + 4];
        memcpy(msaaAtt, att, sizeof(att));
        SkASSERT(None == msaaAtt[kAttCount - 1]);
        msaaAtt[kAttCount - 1] = GLX_SAMPLE_BUFFERS_ARB;
        msaaAtt[kAttCount + 0] = 1;
        msaaAtt[kAttCount + 1] = GLX_SAMPLES_ARB;
        msaaAtt[kAttCount + 2] = requestedMSAASampleCount;
        msaaAtt[kAttCount + 3] = None;
        fVi = glXChooseVisual(dsp, DefaultScreen(dsp), msaaAtt);
        fMSAASampleCount = requestedMSAASampleCount;
    }
    if (nullptr == fVi) {
        fVi = glXChooseVisual(dsp, DefaultScreen(dsp), att);
        fMSAASampleCount = 0;
    }

    if (fVi) {
        if (info) {
            glXGetConfig(dsp, fVi, GLX_SAMPLES_ARB, &info->fSampleCount);
            glXGetConfig(dsp, fVi, GLX_STENCIL_SIZE, &info->fStencilBits);
        }
        Colormap colorMap = XCreateColormap(dsp,
                                            RootWindow(dsp, fVi->screen),
                                            fVi->visual,
                                             AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = colorMap;
        swa.event_mask = EVENT_MASK;
        fUnixWindow.fWin = XCreateWindow(dsp,
                                         RootWindow(dsp, fVi->screen),
                                         0, 0, // x, y
                                         WIDTH, HEIGHT,
                                         0, // border width
                                         fVi->depth,
                                         InputOutput,
                                         fVi->visual,
                                         CWEventMask | CWColormap,
                                         &swa);
    } else {
        if (info) {
            info->fSampleCount = 0;
            info->fStencilBits = 0;
        }
        // Create a simple window instead.  We will not be able to show GL
        fUnixWindow.fWin = XCreateSimpleWindow(dsp,
                                               DefaultRootWindow(dsp),
                                               0, 0,  // x, y
                                               WIDTH, HEIGHT,
                                               0,     // border width
                                               0,     // border value
                                               0);    // background value
    }
    this->mapWindowAndWait();
    fUnixWindow.fGc = XCreateGC(dsp, fUnixWindow.fWin, 0, nullptr);
}

static unsigned getModi(const XEvent& evt) {
    static const struct {
        unsigned    fXMask;
        unsigned    fSkMask;
    } gModi[] = {
        // X values found by experiment. Is there a better way?
        { 1,    kShift_SkModifierKey },
        { 4,    kControl_SkModifierKey },
        { 8,    kOption_SkModifierKey },
    };

    unsigned modi = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gModi); ++i) {
        if (evt.xkey.state & gModi[i].fXMask) {
            modi |= gModi[i].fSkMask;
        }
    }
    return modi;
}

static SkMSec gTimerDelay;

static bool MyXNextEventWithDelay(Display* dsp, XEvent* evt) {
    // Check for pending events before entering the select loop. There might
    // be events in the in-memory queue but not processed yet.
    if (XPending(dsp)) {
        XNextEvent(dsp, evt);
        return true;
    }

    SkMSec ms = gTimerDelay;
    if (ms > 0) {
        int x11_fd = ConnectionNumber(dsp);
        fd_set input_fds;
        FD_ZERO(&input_fds);
        FD_SET(x11_fd, &input_fds);

        timeval tv;
        tv.tv_sec = ms / 1000;              // seconds
        tv.tv_usec = (ms % 1000) * 1000;    // microseconds

        if (!select(x11_fd + 1, &input_fds, nullptr, nullptr, &tv)) {
            if (!XPending(dsp)) {
                return false;
            }
        }
    }
    XNextEvent(dsp, evt);
    return true;
}

static Atom wm_delete_window_message;

SkOSWindow::NextXEventResult SkOSWindow::nextXEvent() {
    XEvent evt;
    Display* dsp = fUnixWindow.fDisplay;

    if (!MyXNextEventWithDelay(dsp, &evt)) {
        return kContinue_NextXEventResult;
    }

    switch (evt.type) {
        case Expose:
            if (0 == evt.xexpose.count) {
                return kPaintRequest_NextXEventResult;
            }
            break;
        case ConfigureNotify:
            this->resize(evt.xconfigure.width, evt.xconfigure.height);
            break;
        case ButtonPress:
            if (evt.xbutton.button == Button1)
                this->handleClick(evt.xbutton.x, evt.xbutton.y,
                            SkView::Click::kDown_State, nullptr, getModi(evt));
            break;
        case ButtonRelease:
            if (evt.xbutton.button == Button1)
                this->handleClick(evt.xbutton.x, evt.xbutton.y,
                              SkView::Click::kUp_State, nullptr, getModi(evt));
            break;
        case MotionNotify:
            this->handleClick(evt.xmotion.x, evt.xmotion.y,
                           SkView::Click::kMoved_State, nullptr, getModi(evt));
            break;
        case KeyPress: {
            int shiftLevel = (evt.xkey.state & ShiftMask) ? 1 : 0;
            KeySym keysym = XkbKeycodeToKeysym(dsp, evt.xkey.keycode,
                                               0, shiftLevel);
            if (keysym == XK_Escape) {
                return kQuitRequest_NextXEventResult;
            }
            this->handleKey(XKeyToSkKey(keysym));
            long uni = keysym2ucs(keysym);
            if (uni != -1) {
                this->handleChar((SkUnichar) uni);
            }
            break;
        }
        case KeyRelease:
            this->handleKeyUp(XKeyToSkKey(XkbKeycodeToKeysym(dsp, evt.xkey.keycode, 0, 0)));
            break;
        case ClientMessage:
            if ((Atom)evt.xclient.data.l[0] == wm_delete_window_message) {
                return kQuitRequest_NextXEventResult;
            }
            // fallthrough
        default:
            // Do nothing for other events
            break;
    }
    return kContinue_NextXEventResult;
}

void SkOSWindow::loop() {
    Display* dsp = fUnixWindow.fDisplay;
    if (nullptr == dsp) {
        return;
    }
    Window win = fUnixWindow.fWin;

    wm_delete_window_message = XInternAtom(dsp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dsp, win, &wm_delete_window_message, 1);

    XSelectInput(dsp, win, EVENT_MASK);

    bool sentExposeEvent = false;

    for (;;) {
        SkEvent::ServiceQueueTimer();

        bool moreToDo = SkEvent::ProcessEvent();

        if (this->isDirty() && !sentExposeEvent) {
            sentExposeEvent = true;

            XEvent evt;
            sk_bzero(&evt, sizeof(evt));
            evt.type = Expose;
            evt.xexpose.display = dsp;
            XSendEvent(dsp, win, false, ExposureMask, &evt);
        }

        if (XPending(dsp) || !moreToDo) {
            switch (this->nextXEvent()) {
                case kContinue_NextXEventResult:
                    break;
                case kPaintRequest_NextXEventResult:
                    sentExposeEvent = false;
                    if (this->isDirty()) {
                        this->update(nullptr);
                    }
                    this->doPaint();
                    break;
                case kQuitRequest_NextXEventResult:
                    return;
            }
        }
    }
}

void SkOSWindow::mapWindowAndWait() {
    SkASSERT(fUnixWindow.fDisplay);
    Display* dsp = fUnixWindow.fDisplay;
    Window win = fUnixWindow.fWin;
    XMapWindow(dsp, win);

    long eventMask = StructureNotifyMask;
    XSelectInput(dsp, win, eventMask);

    // Wait until screen is ready.
    XEvent evt;
    do {
        XNextEvent(dsp, &evt);
    } while(evt.type != MapNotify);

}

////////////////////////////////////////////////

// Some helper code to load the correct version of glXSwapInterval
#define GLX_GET_PROC_ADDR(name) glXGetProcAddress(reinterpret_cast<const GLubyte*>((name)))
#define EXT_WRANGLE(name, type, ...) \
    if (GLX_GET_PROC_ADDR(#name)) { \
        static type k##name; \
        if (!k##name) { \
            k##name = (type) GLX_GET_PROC_ADDR(#name); \
        } \
        k##name(__VA_ARGS__); \
        /*SkDebugf("using %s\n", #name);*/ \
        return; \
    }

static void glXSwapInterval(Display* dsp, GLXDrawable drawable, int interval) {
    EXT_WRANGLE(glXSwapIntervalEXT, PFNGLXSWAPINTERVALEXTPROC, dsp, drawable, interval);
    EXT_WRANGLE(glXSwapIntervalMESA, PFNGLXSWAPINTERVALMESAPROC, interval);
    EXT_WRANGLE(glXSwapIntervalSGI, PFNGLXSWAPINTERVALSGIPROC, interval);
}

/////////////////////////////////////////////////////////////////////////

bool SkOSWindow::attach(SkBackEndTypes, int msaaSampleCount, bool deepColor,
                        AttachmentInfo* info) {
    this->initWindow(msaaSampleCount, info);

    if (nullptr == fUnixWindow.fDisplay) {
        return false;
    }
    if (nullptr == fUnixWindow.fGLContext) {
        SkASSERT(fVi);

        fUnixWindow.fGLContext = glXCreateContext(fUnixWindow.fDisplay,
                                                  fVi,
                                                  nullptr,
                                                  GL_TRUE);
        if (nullptr == fUnixWindow.fGLContext) {
            return false;
        }
    }
    glXMakeCurrent(fUnixWindow.fDisplay,
                   fUnixWindow.fWin,
                   fUnixWindow.fGLContext);
    glViewport(0, 0,
               SkScalarRoundToInt(this->width()),
               SkScalarRoundToInt(this->height()));
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return true;
}

void SkOSWindow::release() {
    if (nullptr == fUnixWindow.fDisplay || nullptr == fUnixWindow.fGLContext) {
        return;
    }
    glXMakeCurrent(fUnixWindow.fDisplay, None, nullptr);
    glXDestroyContext(fUnixWindow.fDisplay, fUnixWindow.fGLContext);
    fUnixWindow.fGLContext = nullptr;
}

void SkOSWindow::present() {
    if (fUnixWindow.fDisplay && fUnixWindow.fGLContext) {
        glXSwapBuffers(fUnixWindow.fDisplay, fUnixWindow.fWin);
    }
}

void SkOSWindow::onSetTitle(const char title[]) {
    if (nullptr == fUnixWindow.fDisplay) {
        return;
    }
    XTextProperty textProp;
    textProp.value = (unsigned char*)title;
    textProp.format = 8;
    textProp.nitems = strlen((char*)textProp.value);
    textProp.encoding = XA_STRING;
    XSetWMName(fUnixWindow.fDisplay, fUnixWindow.fWin, &textProp);
}

static bool convertBitmapToXImage(XImage& image, const SkBitmap& bitmap) {
    sk_bzero(&image, sizeof(image));

    int bitsPerPixel = bitmap.bytesPerPixel() * 8;
    image.width = bitmap.width();
    image.height = bitmap.height();
    image.format = ZPixmap;
    image.data = (char*) bitmap.getPixels();
    image.byte_order = LSBFirst;
    image.bitmap_unit = bitsPerPixel;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = bitsPerPixel;
    image.depth = 24;
    image.bytes_per_line = bitmap.rowBytes() - bitmap.width() * 4;
    image.bits_per_pixel = bitsPerPixel;
    return XInitImage(&image);
}

void SkOSWindow::doPaint() {
    if (nullptr == fUnixWindow.fDisplay) {
        return;
    }
    // If we are drawing with GL, we don't need XPutImage.
    if (fUnixWindow.fGLContext) {
        return;
    }
    // Draw the bitmap to the screen.
    const SkBitmap& bitmap = getBitmap();
    int width = bitmap.width();
    int height = bitmap.height();

    XImage image;
    if (!convertBitmapToXImage(image, bitmap)) {
        return;
    }

    XPutImage(fUnixWindow.fDisplay,
              fUnixWindow.fWin,
              fUnixWindow.fGc,
              &image,
              0, 0,     // src x,y
              0, 0,     // dst x,y
              width, height);
}

enum {
    _NET_WM_STATE_REMOVE =0,
    _NET_WM_STATE_ADD = 1,
    _NET_WM_STATE_TOGGLE =2
};

bool SkOSWindow::makeFullscreen() {
    Display* dsp = fUnixWindow.fDisplay;
    if (nullptr == dsp) {
        return false;
    }

    // Full screen
    Atom wm_state = XInternAtom(dsp, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(dsp, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent evt;
    sk_bzero(&evt, sizeof(evt));
    evt.type = ClientMessage;
    evt.xclient.window = fUnixWindow.fWin;
    evt.xclient.message_type = wm_state;
    evt.xclient.format = 32;
    evt.xclient.data.l[0] = _NET_WM_STATE_ADD;
    evt.xclient.data.l[1] = fullscreen;
    evt.xclient.data.l[2] = 0;

    XSendEvent(dsp, DefaultRootWindow(dsp), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &evt);
    return true;
}

void SkOSWindow::setVsync(bool vsync) {
    if (fUnixWindow.fDisplay && fUnixWindow.fGLContext && fUnixWindow.fWin) {
        int swapInterval = vsync ? 1 : 0;
        glXSwapInterval(fUnixWindow.fDisplay, fUnixWindow.fWin, swapInterval);
    }
}

void SkOSWindow::closeWindow() {
    Display* dsp = fUnixWindow.fDisplay;
    if (nullptr == dsp) {
        return;
    }

    XEvent evt;
    sk_bzero(&evt, sizeof(evt));
    evt.type = ClientMessage;
    evt.xclient.message_type = XInternAtom(dsp, "WM_PROTOCOLS", true);
    evt.xclient.window = fUnixWindow.fWin;
    evt.xclient.format = 32;
    evt.xclient.data.l[0] = XInternAtom(dsp, "WM_DELETE_WINDOW", false);
    evt.xclient.data.l[1] = CurrentTime;

    XSendEvent(dsp, fUnixWindow.fWin, false, NoEventMask, &evt);
}

///////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue() {
    // nothing to do, since we spin on our event-queue, polling for XPending
}

void SkEvent::SignalQueueTimer(SkMSec delay) {
    // just need to record the delay time. We handle waking up for it in
    // MyXNextEventWithDelay()
    gTimerDelay = delay;
}
