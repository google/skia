/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkRandom.h"

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
        static constexpr SkScalar kPad = 2;
        // To avoid tripping up bounds tracking we position the draws such that all the
        // draws of image 0 are above those of image 1, etc.
        static const int imagesPerRow =
                SkScalarFloorToInt(kDeviceSize.fWidth / (kImageSize.fWidth + kPad));
        int rowsPerImage = SkScalarCeilToInt((SkScalar)fRepeatCnt / imagesPerRow);
        for (int l = 0; l < loops; ++l) {
            for (int r = 0; r < fRepeatCnt; ++r) {
                for (int i = 0; i < fImageCnt; ++i) {
                    SkScalar imageYOffset = i * rowsPerImage * (kImageSize.fHeight + kPad);
                    SkScalar rowYOffset = (r / imagesPerRow) * (kImageSize.fHeight + kPad);
                    SkScalar x = (r % imagesPerRow) * (kImageSize.fWidth + kPad);
                    canvas->drawImage(fImages[i].get(), x, imageYOffset + rowYOffset, &paint);
                }
            }
            // Prevent any batching between "frames".
            if (auto surf = canvas->getSurface()) {
                surf->flush();
            }
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
