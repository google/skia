/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkString.h"
#include <stdio.h>
#include <stdlib.h>
#include <functional>


constexpr int w = 1500;
constexpr int h = 1500;


/*
 * This class is used for several benchmarks that draw different primitive Skia shapes at various
 * sizes. It is used to test both CPU-bound and GPU-bound rendering situations. It draws large
 * amounts of shapes internally (rather than relying on nanobench selecting lots of loops) in order
 * to take advantage of instanced rendering approaches.
 */
class PathTextBench : public Benchmark {
public:
    bool isVisual() override { return true; }

private:
    const char* onGetName() override {
        return "ccpr";
    }
    SkIPoint onGetSize() override { return SkIPoint::Make(w, h); }

    void onDelayedSetup() override {
        SkPaint defaultPaint;
        SkAutoGlyphCache agc(defaultPaint, nullptr, &SkMatrix::I());
        SkGlyphCache* cache = agc.getCache();
        for (int i = 0; i < 52; ++i) {
            // I and l are rects on OS X.....
            char c = "ABCDEFGH7JKLMNOPQRSTUVWXYZabcdefghijk1mnopqrstuvwxyz"[i];
            SkGlyphID id = cache->unicharToGlyph(c);
            cache->getScalerContext()->getPath(SkPackedGlyphID(id), &fGlyphs[i]);
        }

        SkRandom rand;
        for (int i = 0; i < 2000; ++i) {
            const SkPath& glyph = fGlyphs[i % 52];
            const SkRect& bounds = glyph.getBounds();
            float maxdim = SkTMax(bounds.width(), bounds.height());

            float t0 = pow(rand.nextF(), 100);
            float size = (1 - t0) * SkTMin(w, h) / 50 + t0 * SkTMin(w, h) / 3;
            float scale = size / maxdim;
            float t1 = rand.nextF(), t2 = rand.nextF();
            fXforms[i].setTranslate((1 - t1) * sqrt(2) * scale/2 * maxdim +
                                     t1 * (w - sqrt(2) * scale/2 * maxdim),
                                     (1 - t2) * sqrt(2) * scale/2 * maxdim +
                                     t2 * (h - sqrt(2) * scale/2 * maxdim));
            fXforms[i].preRotate(rand.nextF() * 360);
            fXforms[i].preTranslate(-scale/2 * bounds.width(), -scale/2 * bounds.height());
            fXforms[i].preScale(scale, scale);
            fPaints[i].setAntiAlias(true);
            fPaints[i].setColor(rand.nextU() | 0x80808080);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);
        for (int i = 0; i < 2000; ++i) {
            const SkPath& glyph = fGlyphs[i % 52];
            canvas->setMatrix(fXforms[i]);
            canvas->drawPath(glyph, fPaints[i]);
        }
    }

    SkPath fGlyphs[52];
    SkPaint fPaints[2000];
    SkMatrix fXforms[2000];

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new PathTextBench;)
