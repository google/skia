/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkTypeface.h"

enum {
    LOOP = SkBENCHLOOP(1000),
    NGLYPHS = 100
};

static SkTypeface::Encoding paint2Encoding(const SkPaint& paint) {
    SkPaint::TextEncoding enc = paint.getTextEncoding();
    SkASSERT(SkPaint::kGlyphID_TextEncoding != enc);
    return (SkTypeface::Encoding)enc;
}

typedef void (*TypefaceProc)(const SkPaint&, const void* text, size_t len,
                             int glyphCount);

static void containsText_proc(const SkPaint& paint, const void* text, size_t len,
                              int glyphCount) {
    for (int i = 0; i < LOOP; ++i) {
        paint.containsText(text, len);
    }
}

static void textToGlyphs_proc(const SkPaint& paint, const void* text, size_t len,
                              int glyphCount) {
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    for (int i = 0; i < LOOP; ++i) {
        paint.textToGlyphs(text, len, glyphs);
    }
}

static void charsToGlyphs_proc(const SkPaint& paint, const void* text,
                               size_t len, int glyphCount) {
    SkTypeface::Encoding encoding = paint2Encoding(paint);
    uint16_t glyphs[NGLYPHS];
    SkASSERT(glyphCount <= NGLYPHS);

    SkTypeface* face = paint.getTypeface();
    for (int i = 0; i < LOOP; ++i) {
        face->charsToGlyphs(text, encoding, glyphs, glyphCount);
    }
}

static void charsToGlyphsNull_proc(const SkPaint& paint, const void* text,
                                   size_t len, int glyphCount) {
    SkTypeface::Encoding encoding = paint2Encoding(paint);

    SkTypeface* face = paint.getTypeface();
    for (int i = 0; i < LOOP; ++i) {
        face->charsToGlyphs(text, encoding, NULL, glyphCount);
    }
}

class CMAPBench : public SkBenchmark {
    TypefaceProc fProc;
    SkString     fName;
    char         fText[NGLYPHS];
    SkPaint      fPaint;

public:
    CMAPBench(void* param, TypefaceProc proc, const char name[]) : SkBenchmark(param) {
        fProc = proc;
        fName.printf("cmap_%s", name);

        for (int i = 0; i < NGLYPHS; ++i) {
            // we're jamming values into utf8, so we must keep it legal utf8
            fText[i] = 'A' + (i & 31);
        }
        fPaint.setTypeface(SkTypeface::RefDefault())->unref();
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        fProc(fPaint, fText, sizeof(fText), NGLYPHS);
    }

private:

    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new CMAPBench(p, containsText_proc, "paint_containsText"); )
DEF_BENCH( return new CMAPBench(p, textToGlyphs_proc, "paint_textToGlyphs"); )
DEF_BENCH( return new CMAPBench(p, charsToGlyphs_proc, "face_charsToGlyphs"); )
DEF_BENCH( return new CMAPBench(p, charsToGlyphsNull_proc, "face_charsToGlyphs_null"); )
