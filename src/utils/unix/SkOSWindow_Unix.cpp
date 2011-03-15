#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkEvent.h"

SkOSWindow::SkOSWindow(void* unused)
{
    fUnixWindow.fDisplay = NULL;
}

SkOSWindow::~SkOSWindow()
{
}

void SkOSWindow::setUnixWindow(Display* dsp, Window win, size_t screenNumber, GC gc)
{
    fUnixWindow.fDisplay = dsp;
    fUnixWindow.fWin = win;
    fUnixWindow.fOSWin = screenNumber;
    fUnixWindow.fGc = gc;
}

bool SkOSWindow::attachGL(const SkBitmap* offscreen)
{
    return false;
}

void SkOSWindow::detachGL()
{

}

void SkOSWindow::presentGL()
{

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

void SkOSWindow::onHandleInval(const SkIRect&)
{
    SkEvent* evt = new SkEvent("inval-imageview");
    evt->post(getSinkID());
}

bool SkOSWindow::onEvent(const SkEvent& evt)
{
    if (evt.isType("inval-imageview")) {
        update(NULL);
        doPaint();
        return true;
    }
    return INHERITED::onEvent(evt);
}

void SkOSWindow::doPaint() {
    if (!fUnixWindow.fDisplay) return;
    // Draw the bitmap to the screen.
    const SkBitmap& bitmap = getBitmap();
    for (int i = 0; i < bitmap.width(); i++) {
        for (int j = 0; j < bitmap.height(); j++) {
            // Get the pixel, put it on the screen.
            SkColor color = bitmap.getColor(i, j);
            XSetForeground(fUnixWindow.fDisplay, fUnixWindow.fGc, color);
            XDrawPoint(fUnixWindow.fDisplay, fUnixWindow.fWin, fUnixWindow.fGc, i, j);
        }
    }
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
