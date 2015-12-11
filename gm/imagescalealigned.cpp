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
           set.fImages.push_back().reset(MakeImage(vectors[i], SK_ColorGREEN));
           set.fScales.push_back() = 1;
           set.fImages.push_back().reset(MakeImage(vectors[i], SK_ColorRED));
           set.fScales.push_back() = kStretchFactor;
           set.fImages.push_back().reset(MakeImage(vectors[i], SK_ColorGREEN));
           set.fScales.push_back() = 1;
        }
    }

    SkString onShortName() override {
        return SkString("image_scale_aligned");
    }

    SkISize onISize() override {
        return SkISize::Make(550, 300);
    }

    void onDraw(SkCanvas* canvas) override {
        struct {
            SkPoint offset;
            SkScalar scale;
        } cfgs[] = {
            { {  10,    10    }, 1 },
            { { 140.5f, 10    }, 1 },
            { {  10,    80.5f }, 1 },
            { { 140.5f, 80.5f }, 1 },

            { { 280.49f, 10.49f }, 1 },
            { { 410.51f, 10.49f }, 1 },
            { { 280.49f, 80.51f }, 1 },
            { { 410.51f, 80.51f }, 1 },

            { {  10,    170    }, 1.1f },
            { { 140.5f, 170    }, 1.1f },
            { {  10,    240.5f }, 1.1f },
            { { 140.5f, 240.5f }, 1.1f },

            { { 280.49f, 170.49f }, 1.1f },
            { { 410.51f, 170.49f }, 1.1f },
            { { 280.49f, 240.51f }, 1.1f },
            { { 410.51f, 240.51f }, 1.1f },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(cfgs); ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(cfgs[i].offset.x(), cfgs[i].offset.y());
            canvas->scale(cfgs[i].scale, cfgs[i].scale);
            drawSets(canvas);
        }
    }

private:
    struct ImageSet {
        SkSTArray<3, SkAutoTUnref<SkImage>, true> fImages;
        SkSTArray<3, SkScalar>                    fScales;
        SkVector                                  fVector;
    };

    static SkImage* MakeImage(const SkVector& vec, SkColor color) {
        const SkPoint start = SkPoint::Make(vec.y() * kSegLen / 2, vec.x() * kSegLen / 2);
        const SkPoint end   = SkPoint::Make(start.x() + vec.x() * (kSegLen - 1),
                                            start.y() + vec.y() * (kSegLen - 1));

        SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(kSegLen, kSegLen));
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        surface->getCanvas()->drawLine(start.x(), start.y(), end.x(), end.y(), paint);
        surface->getCanvas()->drawPoint(start.x(), start.y(), color);
        surface->getCanvas()->drawPoint(end.x(), end.y(), color);

        return surface->newImageSnapshot();
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
                    canvas->translate((kSegLen / 2) * set.fVector.y(),
                                      (kSegLen / 2) * set.fVector.x());
                }
            }
            canvas->translate(lastPt.x() + kSegLen,
                - SkIntToScalar(kSegLen) / 2 * SK_ARRAY_COUNT(filters) * SK_ARRAY_COUNT(AAs));
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

            canvas->drawImageRect(img, dst, &paint);
            pt.offset(dst.width() * set.fVector.x(), dst.height() * set.fVector.y());
        }

        return pt;
    }

    static const unsigned  kSegLen = 9;
    static const unsigned  kStretchFactor = 2;
    SkSTArray<2, ImageSet> fSets;

    typedef GM INHERITED;
};

DEF_GM(return new ImageScaleAlignedGM();)
