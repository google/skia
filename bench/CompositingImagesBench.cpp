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
 * Simulates drawing layers images in a grid a la a tile based compositor. The layers are all
 * untransformed.
 */
class CompositingImages : public Benchmark {
public:
    CompositingImages(SkISize tileSize, SkISize tileGridSize, int layerCnt, bool subpixelOffset)
            : fTileSize(tileSize)
            , fTileGridSize(tileGridSize)
            , fLayerCnt(layerCnt)
            , fSubpixelOffset(subpixelOffset) {
        fName.appendf("compositing_images_tile_size_%dx%d_tile_cnt_%dx%d_layers_%d",
                      fTileSize.fWidth, fTileSize.fHeight, fTileGridSize.fWidth,
                      fTileGridSize.fHeight, fLayerCnt);
        if (subpixelOffset) {
            fName.append("_subpixel");
        }
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto ii = SkImageInfo::Make(fTileSize.fWidth, fTileSize.fHeight, kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType, nullptr);
        SkRandom random;
        int numImages = fLayerCnt * fTileGridSize.fWidth * fTileGridSize.fHeight;
        fImages.reset(new sk_sp<SkImage>[numImages]);
        for (int i = 0; i < numImages; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            surf->getCanvas()->drawRect(
                    SkRect::MakeLTRB(3, 3, fTileSize.fWidth - 3, fTileSize.fHeight - 3), paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override { fImages.reset(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkScalar o = this->offset();

        SkPaint paint;
        paint.setFilterQuality(kNone_SkFilterQuality);
        // TODO: Use per-edge AA flags for tiles when API available.
        paint.setAntiAlias(true);
        for (int i = 0; i < loops; ++i) {
            int imgIdx = 0;
            for (int l = 0; l < fLayerCnt; ++l) {
                for (int y = 0; y < fTileGridSize.fHeight; ++y) {
                    for (int x = 0; x < fTileGridSize.fWidth; ++x) {
                        canvas->drawImage(fImages[imgIdx++].get(), x * fTileSize.fWidth + o,
                                          y * fTileSize.fHeight + o, &paint);
                    }
                }
            }
            // Prevent any batching between composited "frames".
            canvas->flush();
        }
    }

private:
    SkScalar offset() const {
        return fSubpixelOffset ? SK_ScalarHalf : 0.f;
    }

    SkIPoint onGetSize() override {
        SkScalar o = this->offset();
        return SkIPoint::Make(SkScalarCeilToInt(fTileSize.fWidth * fTileGridSize.fWidth + o),
                              SkScalarCeilToInt(fTileSize.fHeight * fTileGridSize.fHeight + o));
    }

    std::unique_ptr<sk_sp<SkImage>[]> fImages;
    SkString fName;
    SkISize fTileSize;
    SkISize fTileGridSize;
    int fLayerCnt;
    bool fSubpixelOffset;

    typedef Benchmark INHERITED;
};

// Subpixel = false; all of the draw commands align with integer pixels so AA will be automatically
// turned off within the operation
DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 1, false));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 1, false));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 1, false));

DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 4, false));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 4, false));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 4, false));

DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 16, false));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 16, false));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 16, false));

// Subpixel = true; force the draw commands to not align with pixels exactly so AA remains on
DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 1, true));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 1, true));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 1, true));

DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 4, true));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 4, true));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 4, true));

DEF_BENCH(return new CompositingImages({256, 256}, {8, 8}, 16, true));
DEF_BENCH(return new CompositingImages({512, 512}, {4, 4}, 16, true));
DEF_BENCH(return new CompositingImages({1024, 512}, {2, 4}, 16, true));
