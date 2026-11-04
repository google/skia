// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <cfloat>
#include <string>
#include "include/core/SkPictureRecorder.h"
#include "modules/skparagraph/utils/TestFontCollection.h"

using namespace skia::textlayout;
namespace {
struct ParagraphBench : public Benchmark {
    ParagraphBench(SkScalar width, const char* r, const char* n)
            : fResource(r), fName(n), fWidth(width) {}
    sk_sp<SkData> fData;
    const char* fResource;
    const char* fName;
    SkScalar fWidth;
    const char* onGetName() override { return fName; }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    void onDelayedSetup() override { fData = GetResourceAsData(fResource); }
    void onDraw(int loops, SkCanvas*) override {
        if (!fData) {
            return;
        }

        const char* text = (const char*)fData->data();

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(
                paragraph_style, fontCollection, SkShapers::BestAvailableUnicode());
        builder.addText(text);
        auto paragraph = builder.Build();

        SkPictureRecorder rec;
        SkCanvas* canvas = rec.beginRecording({0,0, 2000,3000});
        while (loops-- > 0) {
            paragraph->layout(fWidth);
            paragraph->paint(canvas, 0, 0);
            paragraph->markDirty();
            fontCollection->getParagraphCache()->reset();
        }
    }
};

// Every layout is a ParagraphCache hit for a text that no font covers, so the cost of restoring
// the unresolved codepoints from the cache shows up here
struct ParagraphCacheHitUnresolvedBench : public Benchmark {
    std::unique_ptr<Paragraph> fParagraph;
    const char* onGetName() override { return "paragraph_cache_hit_unresolved"; }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    void onDelayedSetup() override {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
        // Without fallback the Private Use Area codepoints stay unresolved on every platform
        fontCollection->disableFontFallback();
        std::u16string text;
        for (char16_t i = 0; i < 1024; ++i) {
            text.push_back(i % 5 == 0 ? u' ' : static_cast<char16_t>(0xE000 + i));
        }
        ParagraphBuilderImpl builder(ParagraphStyle(), fontCollection,
                                     SkShapers::BestAvailableUnicode());
        builder.addText(text);
        fParagraph = builder.Build();
        // Puts the shaped text into the cache
        fParagraph->layout(300);
    }
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            fParagraph->markDirty();
            fParagraph->layout(300);
        }
    }
};
}  // namespace

DEF_BENCH(return new ParagraphCacheHitUnresolvedBench;)

#define PARAGRAPH_BENCH(X) DEF_BENCH(return new ParagraphBench(50000, "text/" #X ".txt", "paragraph_" #X);)
//PARAGRAPH_BENCH(arabic)
//PARAGRAPH_BENCH(emoji)
PARAGRAPH_BENCH(english)
#undef PARAGRAPH_BENCH

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
