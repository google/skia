/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "samplecode/Sample.h"
#include <chrono>

struct TimingSample : public Sample {
    static constexpr int W = 24,
                         H = 16;
    sk_sp<SkImage>   fImage;
    sk_sp<SkSurface> fOffscreen;

    SkString name() override { return SkString("Timing"); }

    void onOnceBeforeDraw() override {
        sk_sp<SkSurface> surf = SkSurface::MakeRasterN32Premul(W,H);
        surf->getCanvas()->drawString("abc", 2,H-4, SkFont{}, SkPaint{});
        fImage = surf->makeImageSnapshot();

        fOffscreen = SkSurface::MakeRasterN32Premul(1024,1024);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->scale(8,8);

        // Draw normally.
        canvas->drawImage(fImage, 0,0);

        canvas->translate(0,H);

        // Draw one pixel at a time with drawImageRect(),
        // timing how long each drawImageRect() call takes.
        double cost[H][W];
        double min = +INFINITY,
               max = -INFINITY;
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            auto start = std::chrono::steady_clock::now();
            canvas->drawImageRect(fImage, SkRect::MakeXYWH(x,y,1,1)
                                        , SkRect::MakeXYWH(x,y,1,1)
                                        , /*paint=*/nullptr);
            auto elapsed = std::chrono::steady_clock::now() - start;

            cost[y][x] = elapsed.count();
            min = std::min(min, cost[y][x]);
            max = std::max(max, cost[y][x]);
        }

        canvas->translate(0,H);

        // Draw using those per-pixel timings,
        // with the slowest pixel scaled to alpha=1, the fastest to alpha=0.
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            SkPaint p;
            p.setAlphaf( (cost[y][x] - min) / (max - min) );
            canvas->drawRect(SkRect::MakeXYWH(x,y,1,1), p);
        }

        canvas->translate(0,H);

        // Draw each pixel into offscreen, timing each draw.
        min = +INFINITY;
        max = -INFINITY;
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            auto start = std::chrono::steady_clock::now();
            fOffscreen->getCanvas()->drawImageRect(fImage, SkRect::MakeXYWH(x,y,1,1)
                                                         , SkRect::MakeXYWH(0,0,1024,1024)
                                                         , /*paint=*/nullptr);
            auto elapsed = std::chrono::steady_clock::now() - start;

            cost[y][x] = elapsed.count();
            min = std::min(min, cost[y][x]);
            max = std::max(max, cost[y][x]);
        }
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            SkPaint p;
            p.setAlphaf( (cost[y][x] - min) / (max - min) );
            canvas->drawRect(SkRect::MakeXYWH(x,y,1,1), p);
        }
    }
};
DEF_SAMPLE( return new TimingSample; )
