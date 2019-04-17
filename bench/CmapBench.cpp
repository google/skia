/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkCharToGlyphCache.h"
#include "SkFont.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkUTF.h"

enum {
    NGLYPHS = 100
};

namespace {
struct Rec {
    const SkCharToGlyphCache&   fCache;
    int                         fLoops;
    const SkFont&               fFont;
    const SkUnichar*            fText;
    int                         fCount;
};
}

typedef void (*TypefaceProc)(const Rec& r);

static void textToGlyphs_proc(const Rec& r) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(r.fCount <= NGLYPHS);

    for (int i = 0; i < r.fLoops; ++i) {
        r.fFont.textToGlyphs(r.fText, r.fCount*4, kUTF32_SkTextEncoding, glyphs, NGLYPHS);
    }
}

static void charsToGlyphs_proc(const Rec& r) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(r.fCount <= NGLYPHS);

    SkTypeface* face = r.fFont.getTypefaceOrDefault();
    for (int i = 0; i < r.fLoops; ++i) {
        face->unicharsToGlyphs(r.fText, r.fCount, glyphs);
    }
}

static void addcache_proc(const Rec& r) {
    for (int i = 0; i < r.fLoops; ++i) {
        SkCharToGlyphCache cache;
        for (int i = 0; i < r.fCount; ++i) {
            cache.addCharAndGlyph(r.fText[i], i);
        }
    }
}

static void findcache_proc(const Rec& r) {
    for (int i = 0; i < r.fLoops; ++i) {
        for (int i = 0; i < r.fCount; ++i) {
            r.fCache.findGlyphIndex(r.fText[i]);
        }
    }
}

class CMAPBench : public Benchmark {
    TypefaceProc fProc;
    SkString     fName;
    SkUnichar    fText[NGLYPHS];
    SkFont       fFont;
    SkCharToGlyphCache fCache;
    int          fCount;

public:
    CMAPBench(TypefaceProc proc, const char name[], int count) {
        SkASSERT(count <= NGLYPHS);

        fProc = proc;
        fName.printf("%s_%d", name, count);
        fCount = count;

        SkRandom rand;
        for (int i = 0; i < count; ++i) {
            fText[i] = rand.nextU() & 0xFFFF;
            fCache.addCharAndGlyph(fText[i], i);
        }
        fFont.setTypeface(SkTypeface::MakeDefault());
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        fProc({fCache, loops, fFont, fText, fCount});
    }

private:

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

constexpr int SMALL = 10;

DEF_BENCH( return new CMAPBench(textToGlyphs_proc, "font_charToGlyph", SMALL); )
DEF_BENCH( return new CMAPBench(charsToGlyphs_proc, "face_charToGlyph", SMALL); )
DEF_BENCH( return new CMAPBench(addcache_proc, "addcache_charToGlyph", SMALL); )
DEF_BENCH( return new CMAPBench(findcache_proc, "findcache_charToGlyph", SMALL); )

constexpr int BIG = 100;

DEF_BENCH( return new CMAPBench(textToGlyphs_proc, "font_charToGlyph", BIG); )
DEF_BENCH( return new CMAPBench(charsToGlyphs_proc, "face_charToGlyph", BIG); )
DEF_BENCH( return new CMAPBench(addcache_proc, "addcache_charToGlyph", BIG); )
DEF_BENCH( return new CMAPBench(findcache_proc, "findcache_charToGlyph", BIG); )
