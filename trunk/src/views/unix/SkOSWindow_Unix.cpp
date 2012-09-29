
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

SkOSWindow::SkOSWindow(void* unused)
    : fVi(NULL)
    , fMSAASampleCount(0) {
    fUnixWindow.fDisplay = NULL;
    fUnixWindow.fGLContext = NULL;
    this->initWindow(0);
    this->resize(WIDTH, HEIGHT);
}

SkOSWindow::~SkOSWindow() {
    this->closeWindow();
}

void SkOSWindow::closeWindow() {
    if (NULL != fUnixWindow.fDisplay) {
        this->detach();
        SkASSERT(NULL != fUnixWindow.fGc);
        XFreeGC(fUnixWindow.fDisplay, fUnixWindow.fGc);
        fUnixWindow.fGc = NULL;
        XDestroyWindow(fUnixWindow.fDisplay, fUnixWindow.fWin);
        fVi = NULL;
        XCloseDisplay(fUnixWindow.fDisplay);
        fUnixWindow.fDisplay = NULL;
        fMSAASampleCount = 0;
    }
}

void SkOSWindow::initWindow(int requestedMSAASampleCount) {
    if (fMSAASampleCount != requestedMSAASampleCount) {
        this->closeWindow();
    }
    // presence of fDisplay means we already have a window
    if (NULL != fUnixWindow.fDisplay) {
        return;
    }
    fUnixWindow.fDisplay = XOpenDisplay(NULL);
    Display* dsp = fUnixWindow.fDisplay;
    if (NULL == dsp) {
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
    SkASSERT(NULL == fVi);
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
    if (NULL == fVi) {
        fVi = glXChooseVisual(dsp, DefaultScreen(dsp), att);
        fMSAASampleCount = 0;
    }

    if (fVi) {
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
    fUnixWindow.fGc = XCreateGC(dsp, fUnixWindow.fWin, 0, NULL);
}


void SkOSWindow::post_linuxevent() {
    // Put an event in the X queue to fire an SkEvent.
    if (NULL == fUnixWindow.fDisplay) {
        return;
    }
    long event_mask = NoEventMask;
    XClientMessageEvent event;
    event.type = ClientMessage;
    Atom myAtom(0);
    event.message_type = myAtom;
    event.format = 32;
    event.data.l[0] = 0;
    XSendEvent(fUnixWindow.fDisplay, fUnixWindow.fWin, false, 0,
               (XEvent*) &event);
    XFlush(fUnixWindow.fDisplay);
}

void SkOSWindow::loop() {
    Display* dsp = fUnixWindow.fDisplay;
    if (NULL == dsp) {
        return;
    }
    XSelectInput(dsp, fUnixWindow.fWin, EVENT_MASK);

    bool loop = true;
    XEvent evt;
    while (loop) {
        XNextEvent(dsp, &evt);
        switch (evt.type) {
            case Expose:
                if (evt.xexpose.count == 0)
                    this->inval(NULL);
                break;
            case ConfigureNotify:
                this->resize(evt.xconfigure.width, evt.xconfigure.height);
                break;
            case ButtonPress:
                if (evt.xbutton.button == Button1)
                    this->handleClick(evt.xbutton.x, evt.xbutton.y, SkView::Click::kDown_State);
                break;
            case ButtonRelease:
                if (evt.xbutton.button == Button1)
                    this->handleClick(evt.xbutton.x, evt.xbutton.y, SkView::Click::kUp_State);
                break;
            case MotionNotify:
                this->handleClick(evt.xmotion.x, evt.xmotion.y, SkView::Click::kMoved_State);
                break;
            case KeyPress: {
                KeySym keysym = XkbKeycodeToKeysym(dsp, evt.xkey.keycode, 0, 0);
                //SkDebugf("pressed key %i!\n\tKeySym:%i\n", evt.xkey.keycode, XKeycodeToKeysym(dsp, evt.xkey.keycode, 0));
                if (keysym == XK_Escape) {
                    loop = false;
                    break;
                }
                this->handleKey(XKeyToSkKey(keysym));
                long uni = keysym2ucs(keysym);
                if (uni != -1) {
                    this->handleChar((SkUnichar) uni);
                }
                break;
            }
            case KeyRelease:
                //SkDebugf("released key %i\n", evt.xkey.keycode);
                this->handleKeyUp(XKeyToSkKey(XkbKeycodeToKeysym(dsp, evt.xkey.keycode, 0, 0)));
                break;
            case ClientMessage:
                if (SkEvent::ProcessEvent()) {
                    this->post_linuxevent();
                }
                break;
            default:
                // Do nothing for other events
                break;
        }
    }
}

void SkOSWindow::mapWindowAndWait() {
    SkASSERT(NULL != fUnixWindow.fDisplay);
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

bool SkOSWindow::attach(SkBackEndTypes /* attachType */, int msaaSampleCount) {
    this->initWindow(msaaSampleCount);
    if (NULL == fUnixWindow.fDisplay) {
        return false;
    }
    if (NULL == fUnixWindow.fGLContext) {
        SkASSERT(NULL != fVi);

        fUnixWindow.fGLContext = glXCreateContext(fUnixWindow.fDisplay,
                                                  fVi,
                                                  NULL,
                                                  GL_TRUE);
        if (NULL == fUnixWindow.fGLContext) {
            return false;
        }
    }
    glXMakeCurrent(fUnixWindow.fDisplay,
                   fUnixWindow.fWin,
                   fUnixWindow.fGLContext);
    glViewport(0, 0,
               SkScalarRound(this->width()), SkScalarRound(this->height()));
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return true;
}

void SkOSWindow::detach() {
    if (NULL == fUnixWindow.fDisplay || NULL == fUnixWindow.fGLContext) {
        return;
    }
    glXMakeCurrent(fUnixWindow.fDisplay, None, NULL);
    glXDestroyContext(fUnixWindow.fDisplay, fUnixWindow.fGLContext);
    fUnixWindow.fGLContext = NULL;
}

void SkOSWindow::present() {
    if (NULL != fUnixWindow.fDisplay && NULL != fUnixWindow.fGLContext) {
        glXSwapBuffers(fUnixWindow.fDisplay, fUnixWindow.fWin);
    }
}

void SkOSWindow::onSetTitle(const char title[]) {
    if (NULL == fUnixWindow.fDisplay) {
        return;
    }
    XTextProperty textProp;
    textProp.value = (unsigned char*)title;
    textProp.format = 8;
    textProp.nitems = strlen((char*)textProp.value);
    textProp.encoding = XA_STRING;
    XSetWMName(fUnixWindow.fDisplay, fUnixWindow.fWin, &textProp);
}

void SkOSWindow::onHandleInval(const SkIRect&) {
    (new SkEvent("inval-imageview", this->getSinkID()))->post();
}

bool SkOSWindow::onEvent(const SkEvent& evt) {
    if (evt.isType("inval-imageview")) {
        update(NULL);
        if (NULL == fUnixWindow.fGLContext)
            this->doPaint();
        return true;
    }
    return INHERITED::onEvent(evt);
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
    image.bytes_per_line = bitmap.rowBytes() - bitmap.width() * bitmap.bytesPerPixel();
    image.bits_per_pixel = bitsPerPixel;
    return XInitImage(&image);
}

void SkOSWindow::doPaint() {
    if (NULL == fUnixWindow.fDisplay) {
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

bool SkOSWindow::onHandleChar(SkUnichar) {
    return false;
}

bool SkOSWindow::onHandleKey(SkKey key) {
    return false;
}

bool SkOSWindow::onHandleKeyUp(SkKey key) {
    return false;
}
