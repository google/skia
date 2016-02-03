/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkTileImageFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkString.h"

#define WIDTH 512
#define HEIGHT 512

// This bench exercises SkTileImageFilter drawn from a 50x50 source to
// a 512x512 destination. It is drawn using a single rect, or "tiled"
// rendering (using 32x32, 64x64 tiles). Tiled rendering is currently an order
// of magnitude slower, since SkTileImageFilter does not clip the
// source or destination rects.

class TileImageFilterBench : public Benchmark {
public:
    TileImageFilterBench(int tileSize) : fTileSize(tileSize) {
        if (tileSize > 0) {
            fName.printf("tile_image_filter_tiled_%d", tileSize);
        } else {
            fName.printf("tile_image_filter");
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> tile(SkTileImageFilter::Create(
            SkRect::MakeWH(50, 50),
            SkRect::MakeWH(WIDTH, HEIGHT),
            nullptr));
        paint.setImageFilter(tile);

        for (int i = 0; i < loops; i++) {
            if (fTileSize > 0) {
                for (int y = 0; y < HEIGHT; y += fTileSize) {
                    for (int x = 0; x < WIDTH; x += fTileSize) {
                        canvas->save();
                        SkIRect clipIRect = SkIRect::MakeXYWH(x, y, fTileSize, fTileSize);
                        canvas->clipRect(SkRect::Make(clipIRect));
                        canvas->drawRect(SkRect::MakeWH(WIDTH, HEIGHT), paint);
                        canvas->restore();
                    }
                }
            } else {
                canvas->drawRect(SkRect::MakeWH(WIDTH, HEIGHT), paint);
            }
        }
    }

private:
    SkString fName;
    // Note: this is the tile size used for tiled rendering, not for the size
    // of the SkTileImageFilter source rect.
    int fTileSize;
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new TileImageFilterBench(0);)
DEF_BENCH(return new TileImageFilterBench(32);)
DEF_BENCH(return new TileImageFilterBench(64);)
