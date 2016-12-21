/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

static sk_sp<SkShader> make_grad(SkScalar w, SkScalar h) {
    SkColor colors[] = { 0xFF000000, 0xFF333333 };
    SkPoint pts[] = { { 0, 0 }, { w, h } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}

class BigGradientView : public SampleView {
public:
    BigGradientView() {}

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BigGradient");
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
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BigGradientView; }
static SkViewRegister reg(MyFactory);

///////////////////////////////////////////////////////////////////////////////

#include "SkCGUtils.h"

class GraphicsPort {
protected:
    SkCanvas* fCanvas;

public:
    GraphicsPort(SkCanvas* canvas) : fCanvas(canvas) {}
    virtual ~GraphicsPort() {}

    void save() { fCanvas->save(); }
    void saveLayer(SkAlpha alpha) {
        fCanvas->saveLayerAlpha(nullptr, alpha);
    }
    void restore() { fCanvas->restore(); }

    void translate(float x, float y) { fCanvas->translate(x, y); }
    void scale(float s) { fCanvas->scale(s, s); }

    virtual void drawRect(const SkRect& r, SkColor c) {
        SkPaint p;
        p.setColor(c);
        fCanvas->drawRect(r, p);
    }
};

class CGGraphicsPort : public GraphicsPort {
public:
    CGGraphicsPort(SkCanvas* canvas) : GraphicsPort(canvas) {}

    void drawRect(const SkRect& r, SkColor c) override {
        CGContextRef cg = (CGContextRef)fCanvas->accessTopHandle();
        
        CGColorRef color = CGColorCreateGenericRGB(SkColorGetR(c)/255.f,
                                                   SkColorGetG(c)/255.f,
                                                   SkColorGetB(c)/255.f,
                                                   SkColorGetA(c)/255.f);

        CGContextSetFillColorWithColor(cg, color);
        CGContextFillRect(cg, CGRectMake(r.x(), r.y(), r.width(), r.height()));
    }
};

static CGAffineTransform matrix_to_transform(const SkMatrix& matrix) {
    return CGAffineTransformMake(matrix[SkMatrix::kMScaleX],
                                 matrix[SkMatrix::kMSkewY],
                                 matrix[SkMatrix::kMSkewX],
                                 matrix[SkMatrix::kMScaleY],
                                 matrix[SkMatrix::kMTransX],
                                 matrix[SkMatrix::kMTransY]);
}

static void release_cgcontext(void* pixels, void* ctx) {
    CGContextRelease((CGContextRef)ctx);
}

class Allocator_CG : public SkCanvasExternalAllocator {
public:
    Allocator_CG() {}
    
    bool allocHandle(const SkImageInfo& info, Rec* rec) override {
        // let CG allocate the pixels
        CGContextRef cg = SkCreateCGContext(SkPixmap(info, nullptr, 0));
        if (!cg) {
            return false;
        }
        rec->fReleaseProc = release_cgcontext;
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
        CGContextClearRect(cg, CGRectMake(clip.x(), clip.y(), clip.width(), clip.height()));
        CGContextConcatCTM(cg, matrix_to_transform(ctm));
    }
};

class ExtAllocSample : public SampleView {
public:
    ExtAllocSample() {}
        
protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ext-alloc");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void doDraw(GraphicsPort* port) {
        port->drawRect({0, 0, 256, 256}, SK_ColorRED);
        port->save();
        port->translate(30, 30);
        port->drawRect({0, 0, 20, 20}, SK_ColorBLUE);
        port->restore();
    }
    
    void onDrawContent(SkCanvas* canvas) override {
        GraphicsPort skp(canvas);
        doDraw(&skp);

        const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
        Allocator_CG alloc;
        std::unique_ptr<SkCanvas> c2 = alloc.makeCanvas(info, nullptr);
        CGGraphicsPort cgp(c2.get());
        doDraw(&cgp);

        SkPixmap pm;
        c2->peekPixels(&pm);
        SkBitmap bm;
        bm.installPixels(pm);
        canvas->drawBitmap(bm, 280, 0, nullptr);
    }
    
private:
    typedef SampleView INHERITED;
};
DEF_SAMPLE( return new ExtAllocSample; )
