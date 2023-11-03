/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_ENABLE_PARAGRAPH)

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"

class ParagraphBench final : public Benchmark {
    SkString fName;
    sk_sp<skia::textlayout::FontCollection> fFontCollection;
    skia::textlayout::TextStyle fTStyle;
    std::unique_ptr<skia::textlayout::Paragraph> fParagraph;

public:
    ParagraphBench() {
        fName.printf("skparagraph");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        // fParagraph might have failed to be created in onDelayedSetup()
        return backend == kNonRendering_Backend && !!fParagraph;
    }

    void onDelayedSetup() override {
        fFontCollection = sk_make_sp<skia::textlayout::FontCollection>();
        fFontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

        fTStyle.setFontFamilies({SkString("Roboto")});
        fTStyle.setColor(SK_ColorBLACK);

        const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are necessary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";
        skia::textlayout::ParagraphStyle paragraph_style;
        auto builder =
            skia::textlayout::ParagraphBuilder::make(paragraph_style, fFontCollection);
        if (!builder) {
            return;
        }

        builder->pushStyle(fTStyle);
        builder->addText(text);
        builder->pop();
        fParagraph = builder->Build();

        // Call onDraw once to warm up the glyph cache otherwise nanobench will mis-calculate the
        // loop count.
        SkCanvas canvas;
        this->onDraw(1, &canvas);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            fParagraph->markDirty();
            fParagraph->layout(300);
        }
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH( return new ParagraphBench; )

#endif // SK_ENABLE_PARAGRAPH
