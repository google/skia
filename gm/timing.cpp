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
#include "include/effects/SkLumaColorFilter.h"
#include <chrono>

constexpr int W = 24,
              H = 16;
DEF_SIMPLE_GM(timing, canvas, W*8, H*8*3) {
    sk_sp<SkImage> img;
    {
        sk_sp<SkSurface> s = SkSurface::MakeRasterN32Premul(32,H);

        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        s->getCanvas()->drawRect({0.5f,0.5f, W-0.5f,W-0.5f}, stroke);

        s->getCanvas()->drawString("abc", 2,H-4, SkFont{}, SkPaint{});

        img = s->makeImageSnapshot();
    }

    canvas->scale(8,8);
    {
        canvas->drawImage(img, 0,0);

        canvas->translate(0,H);

        double min = +INFINITY,
               max = -INFINITY;
        double cost[H][W];

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
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            SkPaint p;
            p.setAlphaf( (cost[y][x] - min) / (max - min) );
            canvas->drawRect(SkRect::MakeXYWH(x,y,1,1), p);
        }
    }
}
