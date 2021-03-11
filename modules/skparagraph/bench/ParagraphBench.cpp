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
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    void onDelayedSetup() override { fData = GetResourceAsData(fResource); }
    void onDraw(int loops, SkCanvas*) override {
        if (!fData) {
            return;
        }

        const char* text = (const char*)fData->data();

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.addText(text);
        auto paragraph = builder.Build();

        SkPictureRecorder rec;
        SkCanvas* canvas = rec.beginRecording({0,0, 2000,3000});
        while (loops-- > 0) {
            paragraph->layout(fWidth);
            paragraph->paint(canvas, 0, 0);
            paragraph->markDirty();
            fontCollection->clearCaches();
        }
    }
};

struct MimicingLibTxtBench : public Benchmark {
    MimicingLibTxtBench(SkScalar width, size_t repeat, const char* t, const char* n)
            : fText(t), fName(n), fWidth(width), fRepeat(repeat) {}
    const char* fText;
    const char* fName;
    SkScalar fWidth;
    size_t fRepeat;
    const char* onGetName() override { return fName; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    void onDraw(int loops, SkCanvas*) override {

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

        const char* text = "vry shrt ";
        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setColor(SK_ColorBLACK);
        auto builder = ParagraphBuilder::make(paragraph_style, fontCollection);
        for (size_t i = 0; i < fRepeat; ++i) {
            builder->pushStyle(text_style);
            builder->addText(text);
        }
        auto paragraph = builder->Build();

        while (loops-- > 0) {
            paragraph->markDirty();
            paragraph->layout(fWidth);
        }
    }
};
}  // namespace

#define PARAGRAPH_BENCH(X) DEF_BENCH(return new ParagraphBench(50000, "text/" #X ".txt", "paragraph_" #X);)
#define MIMICLIBTXT_BENCH(X, Y) DEF_BENCH(return new MimicingLibTxtBench(X , Y, "vry shrt ", "styles_" #X "_" #Y);)

//PARAGRAPH_BENCH(arabic)
//PARAGRAPH_BENCH(emoji)
PARAGRAPH_BENCH(english)
MIMICLIBTXT_BENCH(300, 1000)
#undef PARAGRAPH_BENCH

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
