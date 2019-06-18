// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "tools/Resources.h"

#include <cfloat>

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
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    void onDelayedSetup() override { fData = GetResourceAsData(fResource); }
    void onDraw(int loops, SkCanvas*) override {
        if (!fData) {
            return;
        }

        const char* text = (const char*)fData->data();

        sk_sp<FontCollection> fontCollection = sk_make_sp<FontCollection>();
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.addText(text);
        auto paragraph = builder.Build();

        while (loops-- > 0) {
            paragraph->layout(fWidth);
            auto impl = static_cast<ParagraphImpl*>(paragraph.get());
            impl->formatLines(fWidth);
        }
    }
};
}  // namespace

#define PARAGRAPH_BENCH(X) DEF_BENCH(return new ParagraphBench(500, "text/" #X ".txt", "paragraph_" #X);)
//PARAGRAPH_BENCH(arabic)
//PARAGRAPH_BENCH(emoji)
PARAGRAPH_BENCH(english)
#undef PARAGRAPH_BENCH

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
