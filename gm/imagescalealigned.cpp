/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkImage.h"
#include "SkSurface.h"
#include "SkTArray.h"

class ImageScaleAlignedGM : public skiagm::GM {
protected:
    void onOnceBeforeDraw() override {
        const SkVector vectors[] = { { 1, 0 }, { 0, 1 } };

        for (size_t i = 0; i < SK_ARRAY_COUNT(vectors); ++i) {
           auto& set = fSets.push_back();

           set.fVector = vectors[i];
           set.fImages.push_back() = MakeImage(vectors[i], SK_ColorGREEN);
           set.fScales.push_back() = 1;
           set.fImages.push_back() = MakeImage(vectors[i], SK_ColorRED);
           set.fScales.push_back() = kStretchFactor;
           set.fImages.push_back() = MakeImage(vectors[i], SK_ColorGREEN);
           set.fScales.push_back() = 1;
        }
    }

    SkString onShortName() override {
        return SkString("image_scale_aligned");
    }

    SkISize onISize() override {
        return SkISize::Make(580, 780);
    }

    void onDraw(SkCanvas* canvas) override {
        struct {
            SkPoint offset;
            SkVector scale;
        } cfgs[] = {
            {{  10,    10    }, { 1, 1 }},
            {{ 300.5f, 10    }, { 1, 1 }},
            {{  10,    200.5f }, { 1, 1 }},
            {{ 300.5f, 200.5f }, { 1, 1 }},

            {{  10.5f, 400.5f }, {  1,  1 }},
            {{ 550.5f, 400.5f }, { -1,  1 }},
            {{  10.5f, 750.5f }, {  1, -1 }},
            {{ 550.5f, 750.5f }, { -1, -1 }},
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(cfgs); ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(cfgs[i].offset.x(), cfgs[i].offset.y());
            canvas->scale(cfgs[i].scale.x(), cfgs[i].scale.y());
            drawSets(canvas);
        }
    }

private:
    struct ImageSet {
        SkSTArray<3, sk_sp<SkImage>, true>  fImages;
        SkSTArray<3, SkScalar>              fScales;
        SkVector                            fVector;
    };

    static sk_sp<SkImage> MakeImage(const SkVector& vec, SkColor color) {
        const SkPoint start = SkPoint::Make(vec.y() * kSegLen / 2, vec.x() * kSegLen / 2);
        const SkPoint end   = SkPoint::Make(start.x() + vec.x() * (kSegLen - 1),
                                            start.y() + vec.y() * (kSegLen - 1));

        auto surface(SkSurface::MakeRasterN32Premul(kSegLen, kSegLen));
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);

        SkPaint paint;
        paint.setAntiAlias(true);
        const SkRect border = SkRect::MakeIWH(kSegLen, kSegLen).makeInset(.5f, .5f);
        paint.setColor(SK_ColorBLUE);
        paint.setStyle(SkPaint::kStroke_Style);
        surface->getCanvas()->drawRect(border, paint);

        paint.setColor(SK_ColorBLACK);
        surface->getCanvas()->drawLine(start, end, paint);

        paint.reset();
        paint.setColor(color);
        const SkPoint pts[] = { start, end };
        surface->getCanvas()->drawPoints(SkCanvas::kPoints_PointMode, 2, pts, paint);

        return surface->makeImageSnapshot();
    }

    void drawSets(SkCanvas* canvas) const {
        SkAutoCanvasRestore acr(canvas, true);

        const SkFilterQuality filters[] = {
            kNone_SkFilterQuality,
            kLow_SkFilterQuality,
            kMedium_SkFilterQuality,
            kHigh_SkFilterQuality
        };
        const bool AAs[] = { false, true };

        SkPaint paint;
        for (int i = 0; i < fSets.count(); ++i) {
            auto& set = fSets[i];
            SkPoint lastPt;
            for (size_t j = 0; j < SK_ARRAY_COUNT(AAs); ++j) {
                paint.setAntiAlias(AAs[j]);
                for (size_t k = 0; k < SK_ARRAY_COUNT(filters); ++k) {
                    paint.setFilterQuality(filters[k]);
                    lastPt = drawSet(canvas, set, paint);
                    canvas->translate((kSegLen + 4) * set.fVector.y(),
                                      (kSegLen + 4) * set.fVector.x());
                }
            }
            canvas->translate(lastPt.x() + kSegLen,
                - SkIntToScalar(kSegLen + 4) * SK_ARRAY_COUNT(filters) * SK_ARRAY_COUNT(AAs));
        }
    }

    SkPoint drawSet(SkCanvas* canvas, const ImageSet& set, const SkPaint& paint) const {
        SkASSERT(set.fImages.count() == set.fScales.count());

        SkPoint pt = SkPoint::Make(0, 0);
        for (int i = 0; i < set.fImages.count(); ++i) {
            auto& img = set.fImages[i];
            const SkRect dst =
                SkRect::MakeXYWH(pt.x(), pt.y(),
                    img->width() * (1 + (set.fScales[i] - 1) * set.fVector.x()),
                    img->height() * (1 + (set.fScales[i] - 1) * set.fVector.y()));

            canvas->drawImageRect(img.get(), dst, &paint);
            pt.offset(dst.width() * set.fVector.x(), dst.height() * set.fVector.y());
        }

        return pt;
    }

    static constexpr unsigned  kSegLen = 15;
    static constexpr unsigned  kStretchFactor = 4;
    SkSTArray<2, ImageSet> fSets;

    typedef GM INHERITED;
};

DEF_GM(return new ImageScaleAlignedGM();)

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkAutoPixmapStorage.h"
#include "SkImage.h"
#include "SkSurface.h"

sk_sp<SkImage> make_img() {
    auto surf = SkSurface::MakeRasterN32Premul(64, 64);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    surf->getCanvas()->drawCircle(32, 32, 30, paint);
    return surf->makeImageSnapshot();
}

static void draw_pixmap(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPixmap& pm) {
    SkBitmap bm;
    bm.installPixels(pm);
    canvas->drawBitmap(bm, x, y, nullptr);
}

DEF_SIMPLE_GM(scale_pixels_unpremul, canvas, 1024, 1024) {
    SkPixmap pm;
    auto img = make_img();
    img->peekPixels(&pm);

    SkAutoPixmapStorage upm;
    upm.alloc(pm.info().makeAlphaType(kUnpremul_SkAlphaType));
    pm.readPixels(upm);

 //   draw_pixmap(canvas, 10, 10, pm);
    draw_pixmap(canvas, 80, 10, upm);

    canvas->translate(0, 70);

    const int newW = 100, newH = 100;
    for (SkFilterQuality fq : { kNone_SkFilterQuality, kLow_SkFilterQuality, kMedium_SkFilterQuality, kHigh_SkFilterQuality }) {
        SkAutoPixmapStorage tmp;
        tmp.alloc(pm.info().makeWH(newW, newH));
        pm.scalePixels(tmp, fq);
        draw_pixmap(canvas, 10, 10, tmp);

        tmp.alloc(upm.info().makeWH(newW, newH));
        upm.scalePixels(tmp, fq);
        draw_pixmap(canvas, 120, 10, tmp);

        canvas->translate(0, 120);
    }
}
