/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/base/SkRandom.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

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
    PathTextBench(bool clipped, bool uncached) : fClipped(clipped), fUncached(uncached) {}

private:
    const char* onGetName() override {
        fName = "path_text";
        if (fClipped) {
            fName.append("_clipped");
        }
        if (fUncached) {
            fName.append("_uncached");
        }
        return fName.c_str();
    }
    SkISize onGetSize() override { return SkISize::Make(kScreenWidth, kScreenHeight); }

    void onDelayedSetup() override {
        SkFont defaultFont = ToolUtils::DefaultFont();
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
        SkBulkGlyphMetricsAndPaths pathMaker{strikeSpec};
        for (int i = 0; i < kNumGlyphs; ++i) {
            SkGlyphID id(defaultFont.unicharToGlyph(kGlyphs[i]));
            const SkGlyph* glyph = pathMaker.glyph(id);
            if (glyph->path()) {
                fGlyphs[i] = *glyph->path();
            }
            fGlyphs[i].setIsVolatile(fUncached);
        }

        SkRandom rand;
        for (int i = 0; i < kNumDraws; ++i) {
            const SkPath& glyph = fGlyphs[i % kNumGlyphs];
            const SkRect& bounds = glyph.getBounds();
            float glyphSize = std::max(bounds.width(), bounds.height());
            SkASSERT(glyphSize > 0);

            float t0 = pow(rand.nextF(), 100);
            float size = (1 - t0) * std::min(kScreenWidth, kScreenHeight) / 50 +
                         t0 * std::min(kScreenWidth, kScreenHeight) / 3;
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

        if (fClipped) {
            fClipPath = ToolUtils::make_star(SkRect::MakeIWH(kScreenWidth, kScreenHeight), 11, 3);
            fClipPath.setIsVolatile(fUncached);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);
        if (fClipped) {
            canvas->clipPath(fClipPath, SkClipOp::kIntersect, true);
        }
        for (int i = 0; i < kNumDraws; ++i) {
            const SkPath& glyph = fGlyphs[i % kNumGlyphs];
            canvas->setMatrix(fXforms[i]);
            canvas->drawPath(glyph, fPaints[i]);
        }
    }

    const bool fClipped;
    const bool fUncached;
    SkString fName;
    SkPath fGlyphs[kNumGlyphs];
    SkPaint fPaints[kNumDraws];
    SkMatrix fXforms[kNumDraws];
    SkPath fClipPath;

    using INHERITED = Benchmark;
};

DEF_BENCH(return new PathTextBench(false, false);)
DEF_BENCH(return new PathTextBench(false, true);)
DEF_BENCH(return new PathTextBench(true, true);)
