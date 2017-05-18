/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGlyphCache.h"

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkGlyphCache_Globals.h"
#include "SkGraphics.h"
#include "SkTaskGroup.h"
#include "SkTypeface.h"
#include "sk_tool_utils.h"


static void do_font_stuff(SkPaint* paint) {
    for (SkScalar i = 8; i < 64; i++) {
        paint->setTextSize(i);
        SkAutoGlyphCacheNoGamma autoCache(*paint, nullptr, nullptr);
        SkGlyphCache* cache = autoCache.getCache();
        uint16_t glyphs['z'];
        for (int c = ' '; c < 'z'; c++) {
            glyphs[c] = cache->unicharToGlyph(c);
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
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setTypeface(sk_tool_utils::create_portable_typeface(
                              "serif", SkFontStyle::FromOldStyle(SkTypeface::kItalic)));

        for (int work = 0; work < loops; work++) {
            do_font_stuff(&paint);
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
        sk_sp<SkTypeface> typefaces[] =
            {sk_tool_utils::create_portable_typeface("serif",
                  SkFontStyle::FromOldStyle(SkTypeface::kItalic)),
             sk_tool_utils::create_portable_typeface("sans-serif",
                  SkFontStyle::FromOldStyle(SkTypeface::kItalic))};

        for (int work = 0; work < loops; work++) {
            SkTaskGroup().batch(16, [&](int threadIndex) {
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setSubpixelText(true);
                paint.setTypeface(typefaces[threadIndex % 2]);
                do_font_stuff(&paint);
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
