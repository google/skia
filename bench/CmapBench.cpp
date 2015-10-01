/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkTypeface.h"

enum {
    NGLYPHS = 100
};

static SkTypeface::Encoding paint2Encoding(const SkPaint& paint) {
    SkPaint::TextEncoding enc = paint.getTextEncoding();
    SkASSERT(SkPaint::kGlyphID_TextEncoding != enc);
    return (SkTypeface::Encoding)enc;
}

typedef void (*TypefaceProc)(int loops, const SkPaint&, const void* text, size_t len,
                             int glyphCount);

static void containsText_proc(int loops, const SkPaint& paint, const void* text, size_t len,
                              int glyphCount) {
    for (int i = 0; i < loops; ++i) {
        paint.containsText(text, len);
    }
}

static void textToGlyphs_proc(int loops, const SkPaint& paint, const void* text, size_t len,
                              int glyphCount) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    for (int i = 0; i < loops; ++i) {
        paint.textToGlyphs(text, len, glyphs);
    }
}

static void charsToGlyphs_proc(int loops, const SkPaint& paint, const void* text,
                               size_t len, int glyphCount) {
    SkTypeface::Encoding encoding = paint2Encoding(paint);
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    SkTypeface* face = paint.getTypeface();
    for (int i = 0; i < loops; ++i) {
        face->charsToGlyphs(text, encoding, glyphs, glyphCount);
    }
}

static void charsToGlyphsNull_proc(int loops, const SkPaint& paint, const void* text,
                                   size_t len, int glyphCount) {
    SkTypeface::Encoding encoding = paint2Encoding(paint);

    SkTypeface* face = paint.getTypeface();
    for (int i = 0; i < loops; ++i) {
        face->charsToGlyphs(text, encoding, nullptr, glyphCount);
    }
}

class CMAPBench : public Benchmark {
    TypefaceProc fProc;
    SkString     fName;
    char         fText[NGLYPHS];
    SkPaint      fPaint;

public:
    CMAPBench(TypefaceProc proc, const char name[]) {
        fProc = proc;
        fName.printf("cmap_%s", name);

        for (int i = 0; i < NGLYPHS; ++i) {
            // we're jamming values into utf8, so we must keep it legal utf8
            fText[i] = 'A' + (i & 31);
        }
        fPaint.setTypeface(SkTypeface::RefDefault())->unref();
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        fProc(loops, fPaint, fText, sizeof(fText), NGLYPHS);
    }

private:

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new CMAPBench(containsText_proc, "paint_containsText"); )
DEF_BENCH( return new CMAPBench(textToGlyphs_proc, "paint_textToGlyphs"); )
DEF_BENCH( return new CMAPBench(charsToGlyphs_proc, "face_charsToGlyphs"); )
DEF_BENCH( return new CMAPBench(charsToGlyphsNull_proc, "face_charsToGlyphs_null"); )
