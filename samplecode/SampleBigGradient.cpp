/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "samplecode/Sample.h"
#include "src/core/SkMakeUnique.h"

static sk_sp<SkShader> make_grad(SkScalar w, SkScalar h) {
    SkColor colors[] = { 0xFF000000, 0xFF333333 };
    SkPoint pts[] = { { 0, 0 }, { w, h } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

class BigGradientView : public Sample {
public:
    BigGradientView() {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "BigGradient");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkRect r;
        r.set(0, 0, this->width(), this->height());
        SkPaint p;
        p.setShader(make_grad(this->width(), this->height()));
        canvas->drawRect(r, p);
    }

private:
    typedef Sample INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new BigGradientView(); )

///////////////////////////////////////////////////////////////////////////////

#include "include/core/SkRasterHandleAllocator.h"

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

class Allocator_CG : public SkRasterHandleAllocator {
public:
    Allocator_CG() {}

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
        CGContextSaveGState(cg);    // balanced each time updateContext is called
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

#define MyPort CGGraphicsPort
#define MyAllocator Allocator_CG

#elif defined(WIN32)

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

static void DeleteHDCCallback(void*, void* context) {
    HDC hdc = static_cast<HDC>(context);
    HBITMAP hbitmap = static_cast<HBITMAP>(SelectObject(hdc, nullptr));
    DeleteObject(hbitmap);
    DeleteDC(hdc);
}

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
    SelectObject(hdc, hbitmap);

    rec->fReleaseProc = DeleteHDCCallback;
    rec->fReleaseCtx = hdc;
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

#define MyPort GDIGraphicsPort
#define MyAllocator GDIAllocator

#endif

#ifdef MyAllocator
class RasterAllocatorSample : public Sample {
public:
    RasterAllocatorSample() {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "raster-allocator");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void doDraw(GraphicsPort* port) {
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
    }

    void onDrawContent(SkCanvas* canvas) override {
        GraphicsPort skp(canvas);
        doDraw(&skp);

        const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
        std::unique_ptr<SkCanvas> c2 =
            SkRasterHandleAllocator::MakeCanvas(skstd::make_unique<MyAllocator>(), info);
        MyPort cgp(c2.get());
        doDraw(&cgp);

        SkPixmap pm;
        c2->peekPixels(&pm);
        SkBitmap bm;
        bm.installPixels(pm);
        canvas->drawBitmap(bm, 280, 0, nullptr);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new RasterAllocatorSample; )
#endif
