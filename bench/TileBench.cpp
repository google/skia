/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"

static void create_gradient(SkBitmap* bm) {
    SkASSERT(1 == bm->width());
    const int height = bm->height();

    float deltaB = 255.0f / height;
    float blue = 255.0f;

    for (int y = 0; y < height; y++) {
        *bm->getAddr32(0, y) = SkColorSetRGB(0, 0, (U8CPU) blue);
        blue -= deltaB;
    }
}

// Test out the special case of a tiled 1xN texture. Test out opacity,
// filtering and the different tiling modes
class ConstXTileBench : public Benchmark {
    SkPaint             fPaint;
    SkString            fName;
    bool                fDoFilter;
    bool                fDoTrans;
    bool                fDoScale;
    static const int kWidth = 1;
    static const int kHeight = 300;

public:
    ConstXTileBench(SkTileMode xTile,
                    SkTileMode yTile,
                    bool doFilter,
                    bool doTrans,
                    bool doScale)
        : fDoFilter(doFilter)
        , fDoTrans(doTrans)
        , fDoScale(doScale) {
        SkBitmap bm;

        bm.allocN32Pixels(kWidth, kHeight, true);
        bm.eraseColor(SK_ColorWHITE);

        create_gradient(&bm);

        fPaint.setShader(bm.makeShader(xTile, yTile));

        fName.printf("constXTile_");

        static const char* gTileModeStr[kSkTileModeCount] = { "C", "R", "M", "D" };
        fName.append(gTileModeStr[(unsigned)xTile]);
        fName.append(gTileModeStr[(unsigned)yTile]);

        if (doFilter) {
            fName.append("_filter");
        }

        if (doTrans) {
            fName.append("_trans");
        }

        if (doScale) {
            fName.append("_scale");
        }
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(int loops, SkCanvas* canvas) {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);
        paint.setFilterQuality(fDoFilter ? kLow_SkFilterQuality
                                         : kNone_SkFilterQuality);
        if (fDoTrans) {
            paint.setColor(SkColorSetARGB(0x80, 0xFF, 0xFF, 0xFF));
        }

        SkRect r;

        if (fDoScale) {
            r = SkRect::MakeWH(SkIntToScalar(2 * 640), SkIntToScalar(2 * 480));
            canvas->scale(SK_ScalarHalf, SK_ScalarHalf);
        } else {
            r = SkRect::MakeWH(SkIntToScalar(640), SkIntToScalar(480));
        }

        SkPaint bgPaint;
        bgPaint.setColor(SK_ColorWHITE);

        for (int i = 0; i < loops; i++) {
            if (fDoTrans) {
                canvas->drawRect(r, bgPaint);
            }

            canvas->drawRect(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

// Scaled benches are trending towards free.  Seems like caching.
// TODO(mtklein, reed): fix and reenable

//DEF_BENCH(return new ConstXTileBench(SkTileMode::kRepeat, SkTileMode::kRepeat, false, false, true))
DEF_BENCH(return new ConstXTileBench(SkTileMode::kClamp, SkTileMode::kClamp, false, false, false))
//DEF_BENCH(return new ConstXTileBench(SkTileMode::kMirror, SkTileMode::kMirror, false, false, true))

DEF_BENCH(return new ConstXTileBench(SkTileMode::kRepeat, SkTileMode::kRepeat, true, false, false))
//DEF_BENCH(return new ConstXTileBench(SkTileMode::kClamp, SkTileMode::kClamp, true, false, true))
DEF_BENCH(return new ConstXTileBench(SkTileMode::kMirror, SkTileMode::kMirror, true, false, false))

//DEF_BENCH(return new ConstXTileBench(SkTileMode::kRepeat, SkTileMode::kRepeat, false, true, true))
DEF_BENCH(return new ConstXTileBench(SkTileMode::kClamp, SkTileMode::kClamp, false, true, false))
//DEF_BENCH(return new ConstXTileBench(SkTileMode::kMirror, SkTileMode::kMirror, false, true, true))

DEF_BENCH(return new ConstXTileBench(SkTileMode::kRepeat, SkTileMode::kRepeat, true, true, false))
//DEF_BENCH(return new ConstXTileBench(SkTileMode::kClamp, SkTileMode::kClamp, true, true, true))
DEF_BENCH(return new ConstXTileBench(SkTileMode::kMirror, SkTileMode::kMirror, true, true, false))
