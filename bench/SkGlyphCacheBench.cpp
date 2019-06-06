/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/core/SkStrike.h"

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tools/ToolUtils.h"

static void do_font_stuff(SkFont* font) {
    SkPaint defaultPaint;
    for (SkScalar i = 8; i < 64; i++) {
        font->setSize(i);
        auto strikeSpec = SkStrikeSpec::MakeMask(
                *font,  defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());
        auto cache = strikeSpec.findOrCreateExclusiveStrike();
        uint16_t glyphs['z'];
        for (int c = ' '; c < 'z'; c++) {
            glyphs[c] = font->unicharToGlyph(c);
        }
        for (int lookups = 0; lookups < 10; lookups++) {
            for (int c = ' '; c < 'z'; c++) {
                const SkGlyph& g = cache->getGlyphIDMetrics(glyphs[c]);
                cache->findImage(g);
            }
        }

    }
}

class SkGlyphCacheBasic : public Benchmark {
public:
    explicit SkGlyphCacheBasic(size_t cacheSize) : fCacheSize(cacheSize) { }

protected:
    const char* onGetName() override {
        fName.printf("SkGlyphCacheBasic%dK", (int)(fCacheSize >> 10));
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        size_t oldCacheLimitSize = SkGraphics::GetFontCacheLimit();
        SkGraphics::SetFontCacheLimit(fCacheSize);
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
        font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()));

        for (int work = 0; work < loops; work++) {
            do_font_stuff(&font);
        }
        SkGraphics::SetFontCacheLimit(oldCacheLimitSize);
    }

private:
    typedef Benchmark INHERITED;
    const size_t fCacheSize;
    SkString fName;
};

class SkGlyphCacheStressTest : public Benchmark {
public:
    explicit SkGlyphCacheStressTest(int cacheSize) : fCacheSize(cacheSize) { }

protected:
    const char* onGetName() override {
        fName.printf("SkGlyphCacheStressTest%dK", (int)(fCacheSize >> 10));
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        size_t oldCacheLimitSize = SkGraphics::GetFontCacheLimit();
        SkGraphics::SetFontCacheLimit(fCacheSize);
        sk_sp<SkTypeface> typefaces[] = {
                ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()),
                ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic())};

        for (int work = 0; work < loops; work++) {
            SkTaskGroup().batch(16, [&](int threadIndex) {
                SkFont font;
                font.setEdging(SkFont::Edging::kAntiAlias);
                font.setSubpixel(true);
                font.setTypeface(typefaces[threadIndex % 2]);
                do_font_stuff(&font);
            });
        }
        SkGraphics::SetFontCacheLimit(oldCacheLimitSize);
    }

private:
    typedef Benchmark INHERITED;
    const size_t fCacheSize;
    SkString fName;
};

DEF_BENCH( return new SkGlyphCacheBasic(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheBasic(32 * 1024 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(32 * 1024 * 1024); )
