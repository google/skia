/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU

class GrMipMapBench: public Benchmark {
    sk_sp<SkSurface> fSurface;
    SkString fName;
    const int fW, fH;

public:
    GrMipMapBench(int w, int h) : fW(w), fH(h) {
        fName.printf("gr_mipmap_build_%dx%d", w, h);
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        if (!fSurface) {
            GrContext* context = canvas->getGrContext();
            if (nullptr == context) {
                return;
            }
            auto srgb = SkColorSpace::MakeSRGB();
            SkImageInfo info =
                    SkImageInfo::Make(fW, fH, kRGBA_8888_SkColorType, kPremul_SkAlphaType, srgb);
            fSurface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
        }

        // Clear surface once:
        fSurface->getCanvas()->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);
        paint.setColor(SK_ColorWHITE);
        for (int i = 0; i < loops; i++) {
            // Touch surface so mips are dirtied
            fSurface->getCanvas()->drawPoint(0, 0, paint);

            // Draw reduced version of surface to original canvas, to trigger mip generation
            canvas->save();
            canvas->scale(0.1f, 0.1f);
            canvas->drawImage(fSurface->makeImageSnapshot(), 0, 0, &paint);
            canvas->restore();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        fSurface.reset(nullptr);
    }

private:
    typedef Benchmark INHERITED;
};

// Build variants that exercise the width and heights being even or odd at each level, as the
// impl specializes on each of these.
//
DEF_BENCH( return new GrMipMapBench(511, 511); )
DEF_BENCH( return new GrMipMapBench(512, 511); )
DEF_BENCH( return new GrMipMapBench(511, 512); )
DEF_BENCH( return new GrMipMapBench(512, 512); )

#endif
