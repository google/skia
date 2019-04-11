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
        face->charsToGlyphs(r.fText, SkTypeface::kUTF32_Encoding, glyphs, r.fCount);
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

namespace {
class UnicharGen {
    SkUnichar fU;
    const int fStep;
public:
    UnicharGen(int step) : fU(0), fStep(step) {}

    SkUnichar next() {
        fU += fStep;
        return fU;
    }
};
}

class CMAPBench : public Benchmark {
    TypefaceProc fProc;
    SkString     fName;
    SkUnichar    fText[NGLYPHS];
    SkFont       fFont;
    SkCharToGlyphCache fCache;

public:
    CMAPBench(TypefaceProc proc, const char name[]) {
        fProc = proc;
        fName.printf("cmap_%s", name);

        UnicharGen gen(3);
        for (int i = 0; i < NGLYPHS; ++i) {
            fText[i] = gen.next();
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
        fProc({fCache, loops, fFont, fText, NGLYPHS});
    }

private:

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new CMAPBench(textToGlyphs_proc, "font_charToGlyph"); )
DEF_BENCH( return new CMAPBench(charsToGlyphs_proc, "face_charToGlyph"); )
DEF_BENCH( return new CMAPBench(addcache_proc, "addcache_charToGlyph"); )
DEF_BENCH( return new CMAPBench(findcache_proc, "findcache_charToGlyph"); )
