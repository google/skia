/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowContextFactory_unix.h"
#include "../RasterWindowContext.h"
#include "SkSurface.h"

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;

namespace {

class RasterWindowContext_xlib : public RasterWindowContext {
public:
    RasterWindowContext_xlib(Display*, XWindow, int width, int height, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fWindow); }
    void resize(int  w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

protected:
    sk_sp<SkSurface> fBackbufferSurface;
    Display* fDisplay;
    XWindow  fWindow;
    GC       fGC;

    typedef RasterWindowContext INHERITED;
};

RasterWindowContext_xlib::RasterWindowContext_xlib(Display* display, XWindow window, int width,
                                                   int height, const DisplayParams& params)
        : INHERITED(params)
        , fDisplay(display)
        , fWindow(window) {
    fGC = XCreateGC(fDisplay, fWindow, 0, nullptr);
    this->resize(width, height);
    fWidth = width;
    fHeight = height;
}

void RasterWindowContext_xlib::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    XWindowAttributes attrs;
    XGetWindowAttributes(fDisplay, fWindow, &attrs);
    this->resize(attrs.width, attrs.height);
}

void RasterWindowContext_xlib::resize(int  w, int h) {
    SkImageInfo info = SkImageInfo::Make(w, h, fDisplayParams.fColorType, kPremul_SkAlphaType,
                                         fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);

}

sk_sp<SkSurface> RasterWindowContext_xlib::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContext_xlib::swapBuffers() {
    SkPixmap pm;
    if (!fBackbufferSurface->peekPixels(&pm)) {
        return;
    }
    int bitsPerPixel = pm.info().bytesPerPixel() * 8;
    XImage image;
    memset(&image, 0, sizeof(image));
    image.width = pm.width();
    image.height = pm.height();
    image.format = ZPixmap;
    image.data = (char*) pm.addr();
    image.byte_order = LSBFirst;
    image.bitmap_unit = bitsPerPixel;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = bitsPerPixel;
    image.depth = 24;
    image.bytes_per_line = pm.rowBytes() - pm.width() * pm.info().bytesPerPixel();
    image.bits_per_pixel = bitsPerPixel;
    if (!XInitImage(&image)) {
        return;
    }
    XPutImage(fDisplay, fWindow, fGC, &image, 0, 0, 0, 0, pm.width(), pm.height());
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewRasterForXlib(const XlibWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new RasterWindowContext_xlib(info.fDisplay, info.fWindow, info.fWidth,
                                                      info.fHeight, params);
    if (!ctx->isValid()) {
        delete ctx;
        ctx = nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
