/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkSurface.h"

class GraphicsPort {
protected:
    SkCanvas* fCanvas;

public:
    GraphicsPort(SkCanvas* canvas) : fCanvas(canvas) {}
    virtual ~GraphicsPort() {}

    void save() { fCanvas->save(); }
    void saveLayer(const SkRect& bounds, SkAlpha alpha) {
        fCanvas->saveLayerAlpha(&bounds, alpha);
    }
    void restore() { fCanvas->restore(); }

    void translate(float x, float y) { fCanvas->translate(x, y); }
    void scale(float s) { fCanvas->scale(s, s); }
    void clip(const SkRect& r) { fCanvas->clipRect(r); }

    void drawOval(const SkRect& r, SkColor c) {
        SkPaint p;
        p.setColor(c);
        fCanvas->drawOval(r, p);
    }

    virtual void drawRect(const SkRect& r, SkColor c) {
        SkPaint p;
        p.setColor(c);
        fCanvas->drawRect(r, p);
    }

    SkCanvas* peekCanvas() const { return fCanvas; }
};

class SkiaGraphicsPort : public GraphicsPort {
public:
    SkiaGraphicsPort(SkCanvas* canvas) : GraphicsPort(canvas) {}

    void drawRect(const SkRect& r, SkColor c) override {
        SkCanvas* canvas = (SkCanvas*)fCanvas->accessTopRasterHandle();
        canvas->drawRect(r, SkPaint(SkColor4f::FromColor(c)));
    }
};

class SkiaAllocator : public SkRasterHandleAllocator {
public:
    SkiaAllocator() {}

    bool allocHandle(const SkImageInfo& info, Rec* rec) override {
        sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
        if (!surface) {
            return false;
        }
        SkCanvas* canvas = surface->getCanvas();
        SkPixmap pixmap;
        canvas->peekPixels(&pixmap);

        rec->fReleaseProc = [](void* pixels, void* ctx){ SkSafeUnref((SkSurface*)ctx); };
        rec->fReleaseCtx = surface.release();
        rec->fPixels = pixmap.writable_addr();
        rec->fRowBytes = pixmap.rowBytes();
        rec->fHandle = canvas;
        canvas->save();    // balanced each time updateHandle is called
        return true;
    }

    void updateHandle(Handle hndl, const SkMatrix& ctm, const SkIRect& clip) override {
        SkCanvas* canvas = (SkCanvas*)hndl;
        canvas->restore();
        canvas->save();
        canvas->clipRect(SkRect::Make(clip));
        canvas->concat(ctm);
    }
};

#ifdef SK_BUILD_FOR_MAC

#include "include/utils/mac/SkCGUtils.h"
class CGGraphicsPort : public GraphicsPort {
public:
    CGGraphicsPort(SkCanvas* canvas) : GraphicsPort(canvas) {}

    void drawRect(const SkRect& r, SkColor c) override {
        CGContextRef cg = (CGContextRef)fCanvas->accessTopRasterHandle();

        CGColorRef color = CGColorCreateGenericRGB(SkColorGetR(c)/255.f,
                                                   SkColorGetG(c)/255.f,
                                                   SkColorGetB(c)/255.f,
                                                   SkColorGetA(c)/255.f);

        CGContextSetFillColorWithColor(cg, color);
        CGContextFillRect(cg, CGRectMake(r.x(), r.y(), r.width(), r.height()));
    }
};

static CGAffineTransform matrix_to_transform(CGContextRef cg, const SkMatrix& ctm) {
    SkMatrix matrix;
    matrix.setScale(1, -1);
    matrix.postTranslate(0, SkIntToScalar(CGBitmapContextGetHeight(cg)));
    matrix.preConcat(ctm);

    return CGAffineTransformMake(matrix[SkMatrix::kMScaleX],
                                 matrix[SkMatrix::kMSkewY],
                                 matrix[SkMatrix::kMSkewX],
                                 matrix[SkMatrix::kMScaleY],
                                 matrix[SkMatrix::kMTransX],
                                 matrix[SkMatrix::kMTransY]);
}

class CGAllocator : public SkRasterHandleAllocator {
public:
    CGAllocator() {}

    bool allocHandle(const SkImageInfo& info, Rec* rec) override {
        // let CG allocate the pixels
        CGContextRef cg = SkCreateCGContext(SkPixmap(info, nullptr, 0));
        if (!cg) {
            return false;
        }
        rec->fReleaseProc = [](void* pixels, void* ctx){ CGContextRelease((CGContextRef)ctx); };
        rec->fReleaseCtx = cg;
        rec->fPixels = CGBitmapContextGetData(cg);
        rec->fRowBytes = CGBitmapContextGetBytesPerRow(cg);
        rec->fHandle = cg;
        CGContextSaveGState(cg);    // balanced each time updateHandle is called
        return true;
    }

    void updateHandle(Handle hndl, const SkMatrix& ctm, const SkIRect& clip) override {
        CGContextRef cg = (CGContextRef)hndl;

        CGContextRestoreGState(cg);
        CGContextSaveGState(cg);
        CGContextClipToRect(cg, CGRectMake(clip.x(), clip.y(), clip.width(), clip.height()));
        CGContextConcatCTM(cg, matrix_to_transform(cg, ctm));
    }
};

using MyPort = CGGraphicsPort;
using MyAllocator = CGAllocator;

#elif defined(SK_BUILD_FOR_WIN)

#include "src/core/SkLeanWindows.h"

static RECT toRECT(const SkIRect& r) {
    return { r.left(), r.top(), r.right(), r.bottom() };
}

class GDIGraphicsPort : public GraphicsPort {
public:
    GDIGraphicsPort(SkCanvas* canvas) : GraphicsPort(canvas) {}

    void drawRect(const SkRect& r, SkColor c) override {
        HDC hdc = (HDC)fCanvas->accessTopRasterHandle();

        COLORREF cr = RGB(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));// SkEndian_Swap32(c) >> 8;
        RECT rounded = toRECT(r.round());
        FillRect(hdc, &rounded, CreateSolidBrush(cr));

        // Assuming GDI wrote zeros for alpha, this will or-in 0xFF for alpha
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstATop);
        fCanvas->drawRect(r, paint);
    }
};

// We use this static factory function instead of the regular constructor so
// that we can create the pixel data before calling the constructor. This is
// required so that we can call the base class' constructor with the pixel
// data.
static bool Create(int width, int height, bool is_opaque, SkRasterHandleAllocator::Rec* rec) {
    BITMAPINFOHEADER hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.biSize = sizeof(BITMAPINFOHEADER);
    hdr.biWidth = width;
    hdr.biHeight = -height;  // Minus means top-down bitmap.
    hdr.biPlanes = 1;
    hdr.biBitCount = 32;
    hdr.biCompression = BI_RGB;  // No compression.
    hdr.biSizeImage = 0;
    hdr.biXPelsPerMeter = 1;
    hdr.biYPelsPerMeter = 1;
    void* pixels;
    HBITMAP hbitmap = CreateDIBSection(nullptr, (const BITMAPINFO*)&hdr, 0, &pixels, 0, 0);
    if (!hbitmap) {
        return false;
    }

    size_t row_bytes = width * sizeof(SkPMColor);
    sk_bzero(pixels, row_bytes * height);

    HDC hdc = CreateCompatibleDC(nullptr);
    if (!hdc) {
        DeleteObject(hbitmap);
        return false;
    }
    SetGraphicsMode(hdc, GM_ADVANCED);
    HGDIOBJ origBitmap = SelectObject(hdc, hbitmap);

    struct ReleaseContext {
        HDC hdc;
        HGDIOBJ hbitmap;
    };
    rec->fReleaseProc = [](void*, void* context) {
        ReleaseContext* ctx = static_cast<ReleaseContext*>(context);
        HBITMAP hbitmap = static_cast<HBITMAP>(SelectObject(ctx->hdc, ctx->hbitmap));
        DeleteObject(hbitmap);
        DeleteDC(ctx->hdc);
        delete ctx;
    };
    rec->fReleaseCtx = new ReleaseContext{hdc, origBitmap};
    rec->fPixels = pixels;
    rec->fRowBytes = row_bytes;
    rec->fHandle = hdc;
    return true;
}

/**
*  Subclass of SkRasterHandleAllocator that returns an HDC as its "handle".
*/
class GDIAllocator : public SkRasterHandleAllocator {
public:
    GDIAllocator() {}

    bool allocHandle(const SkImageInfo& info, Rec* rec) override {
        SkASSERT(info.colorType() == kN32_SkColorType);
        return Create(info.width(), info.height(), info.isOpaque(), rec);
    }

    void updateHandle(Handle handle, const SkMatrix& ctm, const SkIRect& clip_bounds) override {
        HDC hdc = static_cast<HDC>(handle);

        XFORM xf;
        xf.eM11 = ctm[SkMatrix::kMScaleX];
        xf.eM21 = ctm[SkMatrix::kMSkewX];
        xf.eDx = ctm[SkMatrix::kMTransX];
        xf.eM12 = ctm[SkMatrix::kMSkewY];
        xf.eM22 = ctm[SkMatrix::kMScaleY];
        xf.eDy = ctm[SkMatrix::kMTransY];
        SetWorldTransform(hdc, &xf);

        RECT clip_bounds_RECT = toRECT(clip_bounds);
        HRGN hrgn = CreateRectRgnIndirect(&clip_bounds_RECT);
        int result = SelectClipRgn(hdc, hrgn);
        SkASSERT(result != ERROR);
        result = DeleteObject(hrgn);
        SkASSERT(result != 0);
    }
};

using MyPort = GDIGraphicsPort;
using MyAllocator = GDIAllocator;

#else

using MyPort = SkiaGraphicsPort;
using MyAllocator = SkiaAllocator;

#endif

DEF_SIMPLE_GM(rasterallocator, canvas, 600, 300) {
    auto doDraw = [](GraphicsPort* port) {
        SkAutoCanvasRestore acr(port->peekCanvas(), true);

        port->drawRect({0, 0, 256, 256}, SK_ColorRED);
        port->save();
        port->translate(30, 30);
        port->drawRect({0, 0, 30, 30}, SK_ColorBLUE);
        port->drawOval({10, 10, 20, 20}, SK_ColorWHITE);
        port->restore();

        port->saveLayer({50, 50, 100, 100}, 0x80);
        port->drawRect({55, 55, 95, 95}, SK_ColorGREEN);
        port->restore();

        port->clip({150, 50, 200, 200});
        port->drawRect({0, 0, 256, 256}, 0xFFCCCCCC);
    };

    // TODO: this common code fails pic-8888 and serialize-8888
    //GraphicsPort skiaPort(canvas);
    //doDraw(&skiaPort);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    std::unique_ptr<SkCanvas> nativeCanvas =
        SkRasterHandleAllocator::MakeCanvas(std::make_unique<MyAllocator>(), info);
    MyPort nativePort(nativeCanvas.get());
    doDraw(&nativePort);

    SkPixmap pm;
    nativeCanvas->peekPixels(&pm);
    SkBitmap bm;
    bm.installPixels(pm);
    canvas->drawBitmap(bm, 280, 0, nullptr);
}
