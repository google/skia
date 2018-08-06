/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSurface.h"

/**
 * Draws a small set of small images multiple times each with no overlaps so that each image could
 * be batched. This was originally added to detect regressions as GrTextureOp is refactored to
 * use "dynamic state" for texture bindings. Everything is kept small as we're mostly interested in
 * CPU overhead.
 */
class ImageCycle : public Benchmark {
public:
    /**
     * imageCnt is the number of images and repeat cnt is how many times each image is drawn per
     * logical "frame."
     */
    ImageCycle(int imageCnt, int repeatCnt) : fImageCnt(imageCnt), fRepeatCnt(repeatCnt) {
        fName.appendf("image_cycle_image_cnt_%d_repeat_cnt_%d", fImageCnt, fRepeatCnt);
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto ii = SkImageInfo::Make(kImageSize.fWidth, kImageSize.fHeight, kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType, nullptr);
        SkRandom random;
        fImages.reset(new sk_sp<SkImage>[fImageCnt]);
        for (int i = 0; i < fImageCnt; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            surf->getCanvas()->drawRect(
                    SkRect::MakeLTRB(1, 1, kImageSize.fWidth - 1, kImageSize.fHeight - 1), paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override { fImages.reset(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setFilterQuality(kNone_SkFilterQuality);
        paint.setAntiAlias(true);
        for (int l = 0; l < loops; ++l) {
            int tx = 0, ty = 0;
            for (int r = 0; r < fRepeatCnt; ++r) {
                for (int i = 0; i < fImageCnt; ++i) {
                    canvas->drawImage(fImages[i].get(), tx, ty, &paint);
                    tx += kImageSize.fWidth + 2;
                    if (tx + kImageSize.fWidth > kDeviceSize.fWidth) {
                        tx = 0;
                        ty += kImageSize.fHeight + 2;
                        // We intend all the image repeats to fit in the device with no
                        // overlaps.
                        SkASSERT(ty + kImageSize.fHeight <= kDeviceSize.fHeight);
                    }
                }
            }
            // Prevent any batching between "frames".
            canvas->flush();
        }
    }

private:
    SkIPoint onGetSize() override { return {kDeviceSize.fWidth, kDeviceSize.fHeight}; }

    static constexpr SkISize kImageSize{4, 4};
    static constexpr SkISize kDeviceSize{64, 64};

    std::unique_ptr<sk_sp<SkImage>[]> fImages;
    SkString fName;
    int fImageCnt;
    int fRepeatCnt;

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ImageCycle(5, 10));
