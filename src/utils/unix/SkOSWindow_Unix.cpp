#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
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

bool SkOSWindow::attachGL()
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
