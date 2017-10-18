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
    MultitextureImages(int imageW, int imageH, int dstRectW, int dstRectH,
                       bool disableMultitexturing)
            : fImageW(imageW)
            , fImageH(imageH)
            , fDstRectW(dstRectW)
            , fDstRectH(dstRectH)
            , fDisableMultitexturing(disableMultitexturing) {
        fName.appendf("multitexture_images_%dx%d_image_%dx%d_rect", imageW, imageH, dstRectW,
                      dstRectH);
        if (disableMultitexturing) {
            fName.append("_disable_multitexturing");
        }
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto ii = SkImageInfo::Make(fImageW, fImageH, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                    nullptr);
        SkRandom random;
        for (int i = 0; i < kNumImages; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            surf->getCanvas()->drawRect(SkRect::MakeLTRB(3, 3, fImageW - 3, fImageH - 3),
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
        SkPaint paint;
        paint.setAlpha(0x40);
        paint.setFilterQuality(kLow_SkFilterQuality);
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < kNumImages; ++j) {
                SkVector translate = this->translation(i * kNumImages + j);
                SkRect rect = SkRect::MakeXYWH(translate.fX, translate.fY, fDstRectW, fDstRectH);
                canvas->drawImageRect(fImages[j].get(), rect, &paint);
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
        return SkIPoint::Make(kTranslate * (kNumColumns - 1) + fDstRectW,
                              kTranslate * (kNumRows - 1) + fDstRectH);
    }

    SkVector translation(int i) const {
        SkVector offset;
        offset.fX = i % kNumColumns * kTranslate;
        offset.fY = (i / kNumColumns) % kNumRows * kTranslate;
        return offset;
    }

    static const int kTranslate = 200;
    static const int kNumColumns = 5;
    static const int kNumRows = 5;
    static const int kNumImages = 8;

    sk_sp<SkImage> fImages[kNumImages];
    SkString fName;
    int fImageW;
    int fImageH;
    int fDstRectW;
    int fDstRectH;
    bool fDisableMultitexturing;

    typedef Benchmark INHERITED;
};
static const int kIS = 256;
DEF_BENCH(return new MultitextureImages(kIS, kIS, 100, 100, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 100, 100, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 150, 150, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 150, 150, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 200, 200, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 200, 200, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 210, 210, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 210, 210, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 220, 220, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 220, 220, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 230, 230, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 230, 230, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 240, 240, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 240, 240, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 250, 250, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 250, 250, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 260, 260, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 260, 260, true));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 270, 270, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 270, 270, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 300, 300, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 300, 300, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 350, 350, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 350, 350, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 400, 400, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 400, 400, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 500, 500, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 500, 500, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 700, 700, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 700, 700, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 1024, 1024, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 1024, 1024, true));

DEF_BENCH(return new MultitextureImages(kIS, kIS, 2048, 2048, false));
DEF_BENCH(return new MultitextureImages(kIS, kIS, 2048, 2048, true));


#endif
