/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkRandom.h"
#include "tools/fonts/FontToolUtils.h"

// Creates an RSX form blob from the the randomized fXforms
sk_sp<SkTextBlob> makeBlob() {
    SkFont font;
    SkTDArray<SkGlyphID> glyphs;
    SkTDArray<SkRSXform> xForms;
    font = ToolUtils::DefaultFont();
    font.setSubpixel(true);
    font.setSize(20);

    const char* text = "Keep your sentences short, but not overly so.";
    int glyphCount = font.countText(text, strlen(text), SkTextEncoding::kUTF8);
    glyphs.append(glyphCount);
    font.textToGlyphs(
            text, strlen(text), SkTextEncoding::kUTF8, {glyphs.data(), (size_t)glyphCount});

    xForms.append(glyphCount);
    SkRandom rand;
    SkScalar x = 0;
    for (int i = 0; i < glyphCount; ++i) {
        SkScalar s = rand.nextF() * 0.5f + 0.5f;
        SkScalar a = rand.nextF() * SK_ScalarPI * 0.25f;
        xForms[i] = SkRSXform::Make(s * SkScalarCos(a), s * SkScalarSin(a), x, rand.nextF() * 20);
        x += 20;
    }

    return SkTextBlob::MakeFromRSXform(glyphs.data(),
                                       glyphs.size() * sizeof(SkGlyphID),
                                       {xForms.data(), (size_t)xForms.size()},
                                       font,
                                       SkTextEncoding::kGlyphID);
}

class GlyphRunRSXformCachedBench : public Benchmark {
public:
    GlyphRunRSXformCachedBench() {}

protected:
    const char* onGetName() override { return "GlyphRunRSXform_cached"; }

    void onDelayedSetup() override { fBlob = makeBlob(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(fBlob, 0, 0, paint);
        }
    }

private:
    sk_sp<SkTextBlob> fBlob;
};

DEF_BENCH(return new GlyphRunRSXformCachedBench();)
