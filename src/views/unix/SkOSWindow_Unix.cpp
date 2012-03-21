
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
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

SkOSWindow::SkOSWindow(void* unused) : INHERITED(), fGLAttached(false), fVi(0)
{
    fUnixWindow.fDisplay = XOpenDisplay(NULL);
    Display* dsp = fUnixWindow.fDisplay;
    if (dsp) {
        // Attempt to create a window that supports GL
        GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER,
                GLX_STENCIL_SIZE, 8, None };
        fVi = glXChooseVisual(dsp, DefaultScreen(dsp), att);
        if (fVi) {
            Colormap colorMap = XCreateColormap(dsp, RootWindow(dsp, fVi->screen),
                fVi->visual, AllocNone);
            XSetWindowAttributes swa;
            swa.colormap = colorMap;
            swa.event_mask = EVENT_MASK;
            fUnixWindow.fWin = XCreateWindow(dsp, RootWindow(dsp, fVi->screen),
                    0, 0, WIDTH, HEIGHT, 0, fVi->depth,
                    InputOutput, fVi->visual, CWEventMask | CWColormap, &swa);

        } else {
            // Create a simple window instead.  We will not be able to
            // show GL
            fUnixWindow.fWin = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
                    0, 0, WIDTH, HEIGHT, 0, 0, 0);
        }
        mapWindowAndWait();
        fUnixWindow.fGc = XCreateGC(dsp, fUnixWindow.fWin, 0, NULL);
    }
    this->resize(WIDTH, HEIGHT);
    fUnixWindow.fGLCreated = false;
}

SkOSWindow::~SkOSWindow()
{
    if (fUnixWindow.fDisplay) {
        if (fGLAttached)
            glXMakeCurrent(fUnixWindow.fDisplay, None, NULL);
        XFreeGC(fUnixWindow.fDisplay, fUnixWindow.fGc);
        if (fUnixWindow.fGLCreated)
            glXDestroyContext(fUnixWindow.fDisplay, fUnixWindow.fGLContext);
        XDestroyWindow(fUnixWindow.fDisplay, fUnixWindow.fWin);
        XCloseDisplay(fUnixWindow.fDisplay);
        fUnixWindow.fDisplay = 0;
    }
}

void SkOSWindow::post_linuxevent()
{
    // Put an event in the X queue to fire an SkEvent.
    if (!fUnixWindow.fDisplay) return;
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

void SkOSWindow::loop()
{
    Display* dsp = fUnixWindow.fDisplay;
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
            case KeyPress:
            {
                KeySym keysym = XKeycodeToKeysym(dsp, evt.xkey.keycode, 0);
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
                this->handleKeyUp(XKeyToSkKey(XKeycodeToKeysym(dsp, evt.xkey.keycode, 0)));
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

void SkOSWindow::mapWindowAndWait()
{
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

bool SkOSWindow::attachGL()
{
    if (fGLAttached) return true;
    Display* dsp = fUnixWindow.fDisplay;
    if (!dsp || !fVi) return false;

    if (!fUnixWindow.fGLCreated) {
        fUnixWindow.fGLContext = glXCreateContext(dsp, fVi, NULL, GL_TRUE);
        fUnixWindow.fGLCreated = true;
        glXMakeCurrent(dsp, fUnixWindow.fWin, fUnixWindow.fGLContext);
        glViewport(0, 0, SkScalarRound(this->width()), SkScalarRound(this->height()));
        glClearColor(0, 0, 0, 0);
        glClearStencil(0);
        glStencilMask(0xffffffff);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else
        glXMakeCurrent(dsp, fUnixWindow.fWin, fUnixWindow.fGLContext);
    fGLAttached = true;

    return true;
}

void SkOSWindow::detachGL()
{
    if (!fUnixWindow.fDisplay || !fGLAttached) return;
    fGLAttached = false;
    // Returns back to normal drawing.
    glXMakeCurrent(fUnixWindow.fDisplay, None, NULL);
    // Ensure that we redraw when switching back to raster.
    this->inval(NULL);
}

void SkOSWindow::presentGL()
{
    if (fUnixWindow.fDisplay && fGLAttached) {
        glXSwapBuffers(fUnixWindow.fDisplay, fUnixWindow.fWin);
    }
}

void SkOSWindow::onSetTitle(const char title[])
{
    if (!fUnixWindow.fDisplay) return;
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

bool SkOSWindow::onEvent(const SkEvent& evt)
{
    if (evt.isType("inval-imageview")) {
        update(NULL);
        if (!fGLAttached)
            doPaint();
        return true;
    }
    return INHERITED::onEvent(evt);
}

static bool convertBitmapToXImage(XImage& image, const SkBitmap& bitmap)
{
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
    if (!fUnixWindow.fDisplay) return;
    // Draw the bitmap to the screen.
    const SkBitmap& bitmap = getBitmap();
    int width = bitmap.width();
    int height = bitmap.height();

    XImage image;
    if (!convertBitmapToXImage(image, bitmap)) return;

    XPutImage(fUnixWindow.fDisplay, fUnixWindow.fWin, fUnixWindow.fGc, &image, 0, 0, 0, 0, width, height);
}

bool SkOSWindow::onHandleChar(SkUnichar)
{
    return false;
}

bool SkOSWindow::onHandleKey(SkKey key)
{
    return false;
}

bool SkOSWindow::onHandleKeyUp(SkKey key)
{
    return false;
}
