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

//////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkAutoPixmapStorage.h"
#include "SkCoreBlitters.h"
#include "SkRasterPipeline.h"
#include "Resources.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "sk_tool_utils.h"

extern void SkImageShader_ApendStagesRaw(SkRasterPipeline* p, SkColorSpace* dstCS,
                                         SkArenaAlloc* alloc, const SkPixmap& pm,
                                         const SkMatrix& invCTM,
                                         SkShader::TileMode tx, SkShader::TileMode ty,
                                         SkFilterQuality quality, SkColor color);

static void draw(const SkPixmap& src, const SkPixmap& dst) {
    SkMatrix inv = SkMatrix::MakeScale(SkIntToScalar(src.width()) / dst.width(),
                                       SkIntToScalar(src.height()) / dst.height());

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);

    SkImageShader_ApendStagesRaw(&p, dst.colorSpace(), &alloc, src, inv,
                                 SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                 kHigh_SkFilterQuality, SK_ColorBLACK);

    auto blitter = SkCreateRasterPipelineBlitter(dst, SkPaint(), p, src.isOpaque(), &alloc);

    dst.erase(0);
    blitter->blitRect(0, 0, dst.width(), dst.height());
}

static sk_sp<SkImage> make_checker(int size) {
    auto surf = SkSurface::MakeRasterN32Premul(size, size);
    sk_tool_utils::draw_checkerboard(surf->getCanvas(), SK_ColorBLACK, SK_ColorWHITE, 1);
    return surf->makeImageSnapshot();
}

DEF_SIMPLE_GM(bicubic_scale, canvas, 1024, 1024) {
    canvas->translate(10, 10);

    int src_size = 256;
    auto src_img = make_checker(src_size);
    auto src_info = SkImageInfo::MakeN32Premul(src_img->width(), src_img->height());
    SkAutoPixmapStorage src_pm;
    src_pm.alloc(src_info);
    src_img->readPixels(src_pm, 0, 0);

    if (false) {
        auto data = src_img->encode();
        SkString name;
        name.printf("checker%d.png", src_size);
        SkFILEWStream stream(name.c_str());
        stream.write(data->data(), data->size());
    }

    for (int dst_size = 129; dst_size >= 126; dst_size--) {
        const SkRect dstR = {0, 0, SkIntToScalar(dst_size), SkIntToScalar(dst_size)};
        auto info = SkImageInfo::MakeN32Premul(dst_size, dst_size);
//                                               sk_ref_sp(canvas->imageInfo().colorSpace()));
        auto surf = SkSurface::MakeRaster(info);
        SkPixmap dst_pm;
        if (!surf->peekPixels(&dst_pm)) {
            continue;
        }
        auto surfCanvas = surf->getCanvas();

        canvas->save();
        for (int fq = kNone_SkFilterQuality; fq <= kHigh_SkFilterQuality; ++fq) {
            if (fq < kHigh_SkFilterQuality) {
                SkPaint paint;
                paint.setFilterQuality((SkFilterQuality)fq);
                surfCanvas->clear(0);
                surfCanvas->drawImageRect(src_img, dstR, &paint);
            } else {
                draw(src_pm, dst_pm);
            }

            surf->draw(canvas, 0, 0, nullptr);

            canvas->translate(dst_size + 10.f, 0);
        }
        canvas->restore();

        canvas->translate(0, dst_size + 20.f);
    }
}


