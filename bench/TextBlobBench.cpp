/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"

#include "tools/ToolUtils.h"

/*
 * A trivial test which benchmarks the performance of a textblob with a single run.
 */
class SkTextBlobBench : public Benchmark {
public:
    SkTextBlobBench() {}

    void onDelayedSetup() override {
        fFont.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle()));
        fFont.setSubpixel(true);

        // This text seems representative in both length and letter frequency.
        const char* text = "Keep your sentences short, but not overly so.";

        fGlyphs.setCount(fFont.countText(text, strlen(text), SkTextEncoding::kUTF8));
        fXPos.setCount(fGlyphs.count());

        fFont.textToGlyphs(text, strlen(text), SkTextEncoding::kUTF8, fGlyphs.begin(), fGlyphs.count());
        fFont.getXPos(&fGlyphs[0], fGlyphs.count(), fXPos.begin());
    }

    sk_sp<SkTextBlob> makeBlob() {
        const SkTextBlobBuilder::RunBuffer& run =
            fBuilder.allocRunPosH(fFont, fGlyphs.count(), 10, nullptr);
        memcpy(run.glyphs, &fGlyphs[0], fGlyphs.count() * sizeof(uint16_t));
        memcpy(run.pos, &fXPos[0], fXPos.count() * sizeof(SkScalar));
        return fBuilder.make();
    }

private:
    SkTextBlobBuilder   fBuilder;
    SkFont              fFont;
    SkTDArray<uint16_t> fGlyphs;
    SkTDArray<SkScalar> fXPos;

    typedef Benchmark INHERITED;
};

class TextBlobCachedBench : public SkTextBlobBench {
    const char* onGetName() override {
        return "TextBlobCachedBench";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;

        auto blob = this->makeBlob();
        auto bigLoops = loops * 100;
        for (int i = 0; i < bigLoops; i++) {
            // To ensure maximum caching, we just redraw the blob at the same place everytime
            canvas->drawTextBlob(blob, 0, 0, paint);
        }
    }
};
DEF_BENCH( return new TextBlobCachedBench(); )

class TextBlobFirstTimeBench : public SkTextBlobBench {
    const char* onGetName() override {
        return "TextBlobFirstTimeBench";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;

        auto bigLoops = loops * 100;
        for (int i = 0; i < bigLoops; i++) {
            canvas->drawTextBlob(this->makeBlob(), 0, 0, paint);
        }
    }
};
DEF_BENCH( return new TextBlobFirstTimeBench(); )

class TextBlobMakeBench : public SkTextBlobBench {
    const char* onGetName() override {
        return "TextBlobMakeBench";
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            for (int inner = 0; inner < 1000; ++inner) {
                this->makeBlob();
            }
        }
    }
};
DEF_BENCH( return new TextBlobMakeBench(); )
