#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkEvent.h"
#include "SkTypeface.h"

SkOSWindow::SkOSWindow(void* unused)
{
    fUnixWindow.fDisplay = NULL;
    fMouseX = fMouseY = 0;
    fTypeface = SkTypeface::CreateFromTypeface(NULL, SkTypeface::kBold);
    fShowZoomer = false;
    fScale = 4;
}

SkOSWindow::~SkOSWindow()
{
    fTypeface->unref();
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

bool SkOSWindow::zoomIn()
{
    // Arbitrarily decided
    if (fScale == 25) return false;
    fScale++;
    inval(NULL);
    return true;
}

bool SkOSWindow::zoomOut()
{
    if (fScale == 1) return false;
    fScale--;
    inval(NULL);
    return true;
}

void SkOSWindow::toggleZoomer()
{
    fShowZoomer = !fShowZoomer;
    inval(NULL);
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

void SkOSWindow::updatePointer(int x, int y)
{
    fMouseX = x;
    fMouseY = y;
    inval(NULL);
}

static void drawText(SkCanvas& canvas, SkString string, SkScalar left, SkScalar top, SkPaint& paint)
{
    SkColor desiredColor = paint.getColor();
    paint.setColor(SK_ColorWHITE);
    const char* c_str = string.c_str();
    size_t size = string.size();
    SkRect bounds;
    paint.measureText(c_str, size, &bounds);
    bounds.offset(left, top);
    SkScalar inset = SkIntToScalar(-2);
    bounds.inset(inset, inset);
    canvas.drawRect(bounds, paint);
    if (desiredColor != SK_ColorBLACK) {
        paint.setColor(SK_ColorBLACK);
        canvas.drawText(c_str, size, left + SK_Scalar1, top + SK_Scalar1, paint);
    }
    paint.setColor(desiredColor);
    canvas.drawText(c_str, size, left, top, paint);
}

void SkOSWindow::doPaint() {
    if (!fUnixWindow.fDisplay) return;
    // Draw the bitmap to the screen.
    const SkBitmap& bitmap = getBitmap();
    int width = bitmap.width();
    int height = bitmap.height();

    if (!fShowZoomer) {
        XImage image;
        if (!convertBitmapToXImage(image, bitmap)) return;

        XPutImage(fUnixWindow.fDisplay, fUnixWindow.fWin, fUnixWindow.fGc, &image, 0, 0, 0, 0, width, height);
    } else {
        // Ensure the mouse position is on screen.
        if (fMouseX >= width) fMouseX = width - 1;
        else if (fMouseX < 0) fMouseX = 0;
        if (fMouseY >= height) fMouseY = height - 1;
        else if (fMouseY < 0) fMouseY = 0;
        // zoomedBitmap will show the original bitmap, plus a zoomed in view (fat bits).
        SkBitmap zoomedBitmap;
        bitmap.copyTo(&zoomedBitmap, bitmap.getConfig());
        SkCanvas canvas(zoomedBitmap);
        // Find the size of the zoomed in view, forced to be odd, so the examined pixel is in the middle.
        int zoomedWidth = (width >> 2) | 1;
        int zoomedHeight = (height >> 2) | 1;
        SkIRect src;
        src.set(0, 0, zoomedWidth / fScale, zoomedHeight / fScale);
        src.offset(fMouseX - (src.width()>>1), fMouseY - (src.height()>>1));
        SkRect dest;
        dest.set(0, 0, SkIntToScalar(zoomedWidth), SkIntToScalar(zoomedHeight));
        dest.offset(SkIntToScalar(width - zoomedWidth), SkIntToScalar(height - zoomedHeight));
        SkPaint paint;
        // Clear the background behind our zoomed in view
        paint.setColor(SK_ColorWHITE);
        canvas.drawRect(dest, paint);
        canvas.drawBitmapRect(bitmap, &src, dest);
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kStroke_Style);
        // Draw a border around the pixel in the middle
        SkRect originalPixel;
        originalPixel.set(SkIntToScalar(fMouseX), SkIntToScalar(fMouseY), SkIntToScalar(fMouseX + 1), SkIntToScalar(fMouseY + 1));
        SkMatrix matrix;
        SkRect scalarSrc;
        scalarSrc.set(src);
        SkColor color = bitmap.getColor(fMouseX, fMouseY);
        if (matrix.setRectToRect(scalarSrc, dest, SkMatrix::kFill_ScaleToFit)) {
            SkRect pixel;
            matrix.mapRect(&pixel, originalPixel);
            // TODO Perhaps measure the values and make the outline white if it's "dark"
            if (color == SK_ColorBLACK) {
                paint.setColor(SK_ColorWHITE);
            }
            canvas.drawRect(pixel, paint);
        }
        paint.setColor(SK_ColorBLACK);
        // Draw a border around the destination rectangle
        canvas.drawRect(dest, paint);
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        // Identify the pixel and its color on screen
        paint.setTypeface(fTypeface);
        paint.setAntiAlias(true);
        SkScalar lineHeight = paint.getFontMetrics(NULL);
        SkString string;
        string.appendf("(%i, %i)", fMouseX, fMouseY);
        SkScalar left = dest.fLeft + SkIntToScalar(3); 
        SkScalar i = SK_Scalar1;
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Alpha
        i += SK_Scalar1;
        string.reset();
        string.appendf("A: %X", SkColorGetA(color));
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Red
        i += SK_Scalar1;
        string.reset();
        string.appendf("R: %X", SkColorGetR(color));
        paint.setColor(SK_ColorRED);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Green
        i += SK_Scalar1;
        string.reset();
        string.appendf("G: %X", SkColorGetG(color));
        paint.setColor(SK_ColorGREEN);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Blue
        i += SK_Scalar1;
        string.reset();
        string.appendf("B: %X", SkColorGetB(color));
        paint.setColor(SK_ColorBLUE);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Finally, put our bitmap on the screen
        XImage zoomedImage;
        convertBitmapToXImage(zoomedImage, zoomedBitmap);
        XPutImage(fUnixWindow.fDisplay, fUnixWindow.fWin, fUnixWindow.fGc, &zoomedImage, 0, 0, 0, 0, width, height);
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
