/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"

static constexpr int kScreenWidth = 1500;
static constexpr int kScreenHeight = 1500;

static constexpr int kNumDraws = 2000;

// I and l are rects on OS X.
static constexpr char kGlyphs[] =  "ABCDEFGH7JKLMNOPQRSTUVWXYZabcdefghijk1mnopqrstuvwxyz";
static constexpr int kNumGlyphs = sizeof(kGlyphs) - 1;
static_assert(52 == kNumGlyphs, "expected 52 glyphs");

/*
 * This class benchmarks drawing many glyphs at random scales and rotations.
 */
class PathTextBench : public Benchmark {
public:
    bool isVisual() override { return true; }

private:
    const char* onGetName() override {
        return "path_text";
    }
    SkIPoint onGetSize() override { return SkIPoint::Make(kScreenWidth, kScreenHeight); }

    void onDelayedSetup() override {
        SkPaint defaultPaint;
        SkAutoGlyphCache agc(defaultPaint, nullptr, &SkMatrix::I());
        SkGlyphCache* cache = agc.getCache();
        for (int i = 0; i < kNumGlyphs; ++i) {
            SkGlyphID id = cache->unicharToGlyph(kGlyphs[i]);
            cache->getScalerContext()->getPath(SkPackedGlyphID(id), &fGlyphs[i]);
        }

        SkRandom rand;
        for (int i = 0; i < kNumDraws; ++i) {
            const SkPath& glyph = fGlyphs[i % kNumGlyphs];
            const SkRect& bounds = glyph.getBounds();
            float glyphSize = SkTMax(bounds.width(), bounds.height());

            float t0 = pow(rand.nextF(), 100);
            float size = (1 - t0) * SkTMin(kScreenWidth, kScreenHeight) / 50 +
                         t0 * SkTMin(kScreenWidth, kScreenHeight) / 3;
            float scale = size / glyphSize;
            float t1 = rand.nextF(), t2 = rand.nextF();
            fXforms[i].setTranslate((1 - t1) * sqrt(2) * scale/2 * glyphSize +
                                     t1 * (kScreenWidth - sqrt(2) * scale/2 * glyphSize),
                                     (1 - t2) * sqrt(2) * scale/2 * glyphSize +
                                     t2 * (kScreenHeight - sqrt(2) * scale/2 * glyphSize));
            fXforms[i].preRotate(rand.nextF() * 360);
            fXforms[i].preTranslate(-scale/2 * bounds.width(), -scale/2 * bounds.height());
            fXforms[i].preScale(scale, scale);
            fPaints[i].setAntiAlias(true);
            fPaints[i].setColor(rand.nextU() | 0x80808080);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);
        for (int i = 0; i < kNumDraws; ++i) {
            const SkPath& glyph = fGlyphs[i % kNumGlyphs];
            canvas->setMatrix(fXforms[i]);
            canvas->drawPath(glyph, fPaints[i]);
        }
    }

    SkPath fGlyphs[kNumGlyphs];
    SkPaint fPaints[kNumDraws];
    SkMatrix fXforms[kNumDraws];

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new PathTextBench;)
