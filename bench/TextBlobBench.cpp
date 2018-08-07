/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

#include "Benchmark.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

#include "sk_tool_utils.h"

/*
 * A trivial test which benchmarks the performance of a textblob with a single run.
 */
class SkTextBlobBench : public Benchmark {
public:
    SkTextBlobBench() {}

protected:
    void onDelayedSetup() override {
        fTypeface = sk_tool_utils::create_portable_typeface("serif", SkFontStyle());
        // make textblob
        SkPaint paint;
        paint.setTypeface(fTypeface);
        const char* text = "Keep your sentences short, but not overly so.";
        SkTDArray<uint16_t> glyphs;
        size_t len = strlen(text);

        glyphs.append(paint.textToGlyphs(text, len, nullptr));
        paint.textToGlyphs(text, len, glyphs.begin());

        SkTextBlobBuilder builder;

        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        const SkTextBlobBuilder::RunBuffer& run =
                builder.allocRunPosH(paint, glyphs.count(), 10, nullptr);
        memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));

        for (size_t i = 0; i < len; i++) {
            run.pos[i] = i * 10 + 10;
        }

        fBlob = builder.make();
    }

    void refreshUnique() {
        fBlob->testRefreshUnique();
    }

    sk_sp<SkTextBlob>    fBlob;

private:


    SkTDArray<uint16_t>  fGlyphs;
    std::vector<SkPoint> fPos;
    sk_sp<SkTypeface>    fTypeface;

    typedef Benchmark INHERITED;
};

class TextBlobCachedBench : public SkTextBlobBench {
    const char* onGetName() override {
        return "TextBlobCachedBench";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;

        // To ensure maximum caching, we just redraw the blob at the same place everytime
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(fBlob, 0, 0, paint);
        }
    }
};

class TextBlobFirstTimeBench : public SkTextBlobBench {
    const char* onGetName() override {
        return "TextBlobFirstTimeBench";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;

        // To ensure maximum caching, we just redraw the blob at the same place everytime
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(fBlob, 0, 0, paint);
            this->refreshUnique();
        }
    }
};

DEF_BENCH( return new TextBlobCachedBench(); )

DEF_BENCH( return new TextBlobFirstTimeBench(); )