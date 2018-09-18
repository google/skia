/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#if SK_SUPPORT_GPU

#include "GrContextOptions.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSurface.h"

class MultitextureImages : public Benchmark {
public:
    MultitextureImages(int imageSize, int dstRectSize, bool disableMultitexturing, bool aa)
            : fImageSize(imageSize)
            , fDstRectSize(dstRectSize)
            , fDisableMultitexturing(disableMultitexturing)
            , fAA(aa) {
        fName.appendf("multitexture_images_%dx%d_image_%dx%d_rect", imageSize, imageSize,
                      dstRectSize, dstRectSize);
        if (aa) {
            fName.append("_aa");
        }
        if (disableMultitexturing) {
            fName.append("_disable_multitexturing");
        }
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto ii = SkImageInfo::Make(fImageSize, fImageSize, kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType, nullptr);
        SkRandom random;
        for (int i = 0; i < kNumImages; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            surf->getCanvas()->drawRect(SkRect::MakeLTRB(3, 3, fImageSize - 3, fImageSize - 3),
                                        paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        for (int i = 0; i < kNumImages; ++i) {
            fImages[i].reset();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect rect = SkRect::MakeWH(fDstRectSize, fDstRectSize);
        SkPaint paint;
        paint.setAlpha(0x40);
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setAntiAlias(fAA);
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < kNumImages; ++j) {
                SkVector translate = this->translation(i * kNumImages + j);
                canvas->drawImageRect(fImages[j].get(), rect.makeOffset(translate.fX, translate.fY),
                                      &paint);
            }
            // Prevent any batching except without multitexturing since we're trying to measure
            // drawing distinct images and just repeating images here to increase the workload for
            // timing reasons.
            canvas->flush();
        }
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fDisableImageMultitexturing = fDisableMultitexturing;
    }

private:
    SkIPoint onGetSize() override {
        // The rows and columns are spaced by kTranslate, but the images may overlap if they are
        // larger than kTranslate and extend beyond the last row/column.
        return SkIPoint::Make(kTranslate * (kNumColumns - 1) + fDstRectSize,
                              kTranslate * (kNumRows - 1) + fDstRectSize);
    }

    SkVector translation(int i) const {
        SkVector offset;
        // Fractional offsets to ensure we can't ignore antialiasing.
        offset.fX = i % kNumColumns * kTranslate + 0.1f;
        offset.fY = (i / kNumColumns) % kNumRows * kTranslate + 0.1f;
        return offset;
    }

    static const int kTranslate = 200;
    static const int kNumColumns = 5;
    static const int kNumRows = 5;
    static const int kNumImages = 8;

    sk_sp<SkImage> fImages[kNumImages];
    SkString fName;
    int fImageSize;
    int fDstRectSize;
    bool fDisableMultitexturing;
    bool fAA;

    typedef Benchmark INHERITED;
};

// Non-AA
DEF_BENCH(return new MultitextureImages(128, 32, false, false));
DEF_BENCH(return new MultitextureImages(128, 32, true, false));
DEF_BENCH(return new MultitextureImages(128, 128, false, false));
DEF_BENCH(return new MultitextureImages(128, 128, true, false));
DEF_BENCH(return new MultitextureImages(128, 256, false, false));
DEF_BENCH(return new MultitextureImages(128, 256, true, false));

DEF_BENCH(return new MultitextureImages(512, 32, false, false));
DEF_BENCH(return new MultitextureImages(512, 32, true, false));
DEF_BENCH(return new MultitextureImages(512, 128, false, false));
DEF_BENCH(return new MultitextureImages(512, 128, true, false));
DEF_BENCH(return new MultitextureImages(512, 256, false, false));
DEF_BENCH(return new MultitextureImages(512, 256, true, false));
DEF_BENCH(return new MultitextureImages(512, 512, false, false));
DEF_BENCH(return new MultitextureImages(512, 512, true, false));

// AA
DEF_BENCH(return new MultitextureImages(512, 512, true, true));
DEF_BENCH(return new MultitextureImages(512, 512, false, true));
DEF_BENCH(return new MultitextureImages(128, 32, true, true));
DEF_BENCH(return new MultitextureImages(128, 32, false, true));

#endif
