/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkData.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

static SkData* fileToData(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        return SkData::NewEmpty();
    }
    size_t size = stream.getLength();
    void* mem = sk_malloc_throw(size);
    stream.read(mem, size);
    return SkData::NewFromMalloc(mem, size);
}

static void drawJpeg(SkCanvas* canvas, const SkISize& size) {
    // TODO: Make this draw a file that is checked in, so it can
    // be exercised on machines other than mike's. Will require a
    // rebaseline.
    SkAutoDataUnref data(fileToData("/Users/mike/Downloads/skia.google.jpeg"));
    SkImage* image = SkImage::NewEncodedData(data);
    if (image) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->scale(size.width() * 1.0f / image->width(),
                      size.height() * 1.0f / image->height());
        image->draw(canvas, 0, 0, NULL);
        image->unref();
    }
}

static void drawContents(SkSurface* surface, SkColor fillC) {
    SkSize size = SkSize::Make(SkIntToScalar(surface->width()),
                               SkIntToScalar(surface->height()));
    SkCanvas* canvas = surface->getCanvas();

    SkScalar stroke = size.fWidth / 10;
    SkScalar radius = (size.fWidth - stroke) / 2;

    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setColor(fillC);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(stroke);
    paint.setColor(SK_ColorBLACK);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);
}

static void test_surface(SkCanvas* canvas, SkSurface* surf, bool usePaint) {
    drawContents(surf, SK_ColorRED);
    SkImage* imgR = surf->newImageSnapshot();

    if (true) {
        SkImage* imgR2 = surf->newImageSnapshot();
        SkASSERT(imgR == imgR2);
        imgR2->unref();
    }

    drawContents(surf, SK_ColorGREEN);
    SkImage* imgG = surf->newImageSnapshot();

    // since we've drawn after we snapped imgR, imgG will be a different obj
    SkASSERT(imgR != imgG);

    drawContents(surf, SK_ColorBLUE);

    SkPaint paint;
//    paint.setFilterBitmap(true);
//    paint.setAlpha(0x80);

    imgR->draw(canvas, 0, 0, usePaint ? &paint : NULL);
    imgG->draw(canvas, 0, 80, usePaint ? &paint : NULL);
    surf->draw(canvas, 0, 160, usePaint ? &paint : NULL);

    SkRect src1, src2, src3;
    src1.iset(0, 0, surf->width(), surf->height());
    src2.iset(-surf->width() / 2, -surf->height() / 2,
             surf->width(), surf->height());
    src3.iset(0, 0, surf->width() / 2, surf->height() / 2);

    SkRect dst1, dst2, dst3, dst4;
    dst1.set(0, 240, 65, 305);
    dst2.set(0, 320, 65, 385);
    dst3.set(0, 400, 65, 465);
    dst4.set(0, 480, 65, 545);

    imgR->draw(canvas, &src1, dst1, usePaint ? &paint : NULL);
    imgG->draw(canvas, &src2, dst2, usePaint ? &paint : NULL);
    imgR->draw(canvas, &src3, dst3, usePaint ? &paint : NULL);
    imgG->draw(canvas, NULL, dst4, usePaint ? &paint : NULL);

    imgG->unref();
    imgR->unref();
}

class ImageGM : public skiagm::GM {
    void*   fBuffer;
    size_t  fBufferSize;
    SkSize  fSize;
    enum {
        W = 64,
        H = 64,
        RB = W * 4 + 8,
    };
public:
    ImageGM() {
        fBufferSize = RB * H;
        fBuffer = sk_malloc_throw(fBufferSize);
        fSize.set(SkIntToScalar(W), SkIntToScalar(H));
    }

    virtual ~ImageGM() {
        sk_free(fBuffer);
    }


protected:
    virtual SkString onShortName() {
        return SkString("image-surface");
    }

    virtual SkISize onISize() {
        return SkISize::Make(960, 1200);
    }

    virtual void onDraw(SkCanvas* canvas) {
        drawJpeg(canvas, this->getISize());

        canvas->scale(2, 2);

        static const char* kLabel1 = "Original Img";
        static const char* kLabel2 = "Modified Img";
        static const char* kLabel3 = "Cur Surface";
        static const char* kLabel4 = "Full Crop";
        static const char* kLabel5 = "Over-crop";
        static const char* kLabel6 = "Upper-left";
        static const char* kLabel7 = "No Crop";

        static const char* kLabel8 = "Pre-Alloc Img";
        static const char* kLabel9 = "New Alloc Img";
        static const char* kLabel10 = "SkPicture";
        static const char* kLabel11 = "Null Paint";
        static const char* kLabel12 = "GPU";

        SkPaint textPaint;

        canvas->drawText(kLabel1, strlen(kLabel1), 10,  60, textPaint);
        canvas->drawText(kLabel2, strlen(kLabel2), 10, 140, textPaint);
        canvas->drawText(kLabel3, strlen(kLabel3), 10, 220, textPaint);
        canvas->drawText(kLabel4, strlen(kLabel4), 10, 300, textPaint);
        canvas->drawText(kLabel5, strlen(kLabel5), 10, 380, textPaint);
        canvas->drawText(kLabel6, strlen(kLabel6), 10, 460, textPaint);
        canvas->drawText(kLabel7, strlen(kLabel7), 10, 540, textPaint);

        canvas->drawText(kLabel8, strlen(kLabel8),  80, 10, textPaint);
        canvas->drawText(kLabel9, strlen(kLabel9), 160, 10, textPaint);
        canvas->drawText(kLabel10, strlen(kLabel10), 250, 10, textPaint);
        canvas->drawText(kLabel11, strlen(kLabel11), 320, 10, textPaint);
        canvas->drawText(kLabel12, strlen(kLabel12), 410, 10, textPaint);

        canvas->translate(80, 20);

        // since we draw into this directly, we need to start fresh
        sk_bzero(fBuffer, fBufferSize);

        SkImageInfo info = {
            W, H, kPMColor_SkColorType, kPremul_SkAlphaType
        };
        SkAutoTUnref<SkSurface> surf0(SkSurface::NewRasterDirect(info, fBuffer, RB));
        SkAutoTUnref<SkSurface> surf1(SkSurface::NewRaster(info));
        SkAutoTUnref<SkSurface> surf2(SkSurface::NewPicture(info.fWidth, info.fHeight));
        SkAutoTUnref<SkSurface> surf3(SkSurface::NewPicture(info.fWidth, info.fHeight));
#if SK_SUPPORT_GPU
        GrContext* ctx = canvas->getGrContext();

        SkAutoTUnref<SkSurface> surf4(SkSurface::NewRenderTarget(ctx, info, 0));
#endif

        test_surface(canvas, surf0, true);
        canvas->translate(80, 0);
        test_surface(canvas, surf1, true);
        canvas->translate(80, 0);
        test_surface(canvas, surf2, true);
        canvas->translate(80, 0);
        test_surface(canvas, surf3, false);
#if SK_SUPPORT_GPU
        if (NULL != ctx) {
            canvas->translate(80, 0);
            test_surface(canvas, surf4, true);
        }
#endif
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return GM::kSkipPicture_Flag | GM::kSkipPipe_Flag;
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageGM; }
static skiagm::GMRegistry reg(MyFactory);
