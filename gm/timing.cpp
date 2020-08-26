/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include <chrono>

constexpr int W = 24,
              H = 16;
DEF_SIMPLE_GM(timing, canvas, W*8, H*8*3) {
    sk_sp<SkImage> img;
    {
        sk_sp<SkSurface> s = SkSurface::MakeRasterN32Premul(32,H);
        s->getCanvas()->drawString("abc", 2,H-4, SkFont{}, SkPaint{});
        img = s->makeImageSnapshot();
    }

    canvas->scale(8,8);
    {
        // Draw normally.
        canvas->drawImage(img, 0,0);

        canvas->translate(0,H);

        // Draw one pixel at a time with drawImageRect(),
        // timing how long each drawImageRect() call takes.
        double cost[H][W];
        double min = +INFINITY,
               max = -INFINITY;
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            auto start = std::chrono::steady_clock::now();
            canvas->drawImageRect(img, SkRect::MakeXYWH(x,y,1,1)
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
    }
}
