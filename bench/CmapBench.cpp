/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkTypeface.h"

enum {
    NGLYPHS = 100
};

typedef void (*TypefaceProc)(int loops, const SkFont&, const SkUnichar*, size_t len,
                             int glyphCount);

static void textToGlyphs_proc(int loops, const SkFont& font, const SkUnichar* text, size_t len,
                              int glyphCount) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    for (int i = 0; i < loops; ++i) {
        font.textToGlyphs(text, len, kUTF32_SkTextEncoding, glyphs, NGLYPHS);
    }
}

static void charsToGlyphs_proc(int loops, const SkFont& font, const SkUnichar* text,
                               size_t len, int glyphCount) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    SkTypeface* face = font.getTypefaceOrDefault();
    for (int i = 0; i < loops; ++i) {
        face->unicharsToGlyphs(text, glyphCount, glyphs);
    }
}

class CMAPBench : public Benchmark {
    TypefaceProc fProc;
    SkString     fName;
    SkUnichar    fText[NGLYPHS];
    SkFont       fFont;

public:
    CMAPBench(TypefaceProc proc, const char name[]) {
        fProc = proc;
        fName.printf("cmap_%s", name);

        for (int i = 0; i < NGLYPHS; ++i) {
            // we're jamming values into utf8, so we must keep it legal utf8
            fText[i] = 'A' + (i & 31);
        }
        fFont.setTypeface(SkTypeface::MakeDefault());
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        fProc(loops, fFont, fText, sizeof(fText), NGLYPHS);
    }

private:

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new CMAPBench(textToGlyphs_proc, "paint_textToGlyphs"); )
DEF_BENCH( return new CMAPBench(charsToGlyphs_proc, "face_charsToGlyphs"); )
