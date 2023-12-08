// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "modules/skshaper/include/SkShaper.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <cfloat>

namespace {
struct ShaperBench : public Benchmark {
    ShaperBench(const char* r, const char* n) : fResource(r), fName(n) {}
    std::unique_ptr<SkShaper> fShaper;
    sk_sp<SkData> fData;
    const char* fResource;
    const char* fName;
    const char* onGetName() override { return fName; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    void onDelayedSetup() override {
        fShaper = SkShaper::Make();
        fData = GetResourceAsData(fResource);
    }
    void onDraw(int loops, SkCanvas*) override {
        if (!fData || !fShaper) { return; }
        SkFont font = ToolUtils::DefaultFont();
        const char* text = (const char*)fData->data();
        size_t len = fData->size();
        while (loops-- > 0) {
            SkTextBlobBuilderRunHandler rh(text, {0, 0});
            fShaper->shape(text, len, font, true, FLT_MAX, &rh);
            (void)rh.makeBlob();
        }
    }
};
}  // namespace

#define SHAPER_BENCH(X) DEF_BENCH(return new ShaperBench("text/" #X ".txt", "shaper_" #X);)
SHAPER_BENCH(arabic)
SHAPER_BENCH(armenian)
SHAPER_BENCH(balinese)
SHAPER_BENCH(bengali)
SHAPER_BENCH(buginese)
SHAPER_BENCH(cherokee)
SHAPER_BENCH(cyrillic)
SHAPER_BENCH(devanagari)
SHAPER_BENCH(emoji)
SHAPER_BENCH(english)
SHAPER_BENCH(ethiopic)
SHAPER_BENCH(greek)
SHAPER_BENCH(hangul)
SHAPER_BENCH(han_simplified)
SHAPER_BENCH(han_traditional)
SHAPER_BENCH(hebrew)
SHAPER_BENCH(javanese)
SHAPER_BENCH(kana)
SHAPER_BENCH(khmer)
SHAPER_BENCH(lao)
SHAPER_BENCH(mandaic)
SHAPER_BENCH(myanmar)
SHAPER_BENCH(newtailue)
SHAPER_BENCH(nko)
SHAPER_BENCH(sinhala)
SHAPER_BENCH(sundanese)
SHAPER_BENCH(syriac)
SHAPER_BENCH(taitham)
SHAPER_BENCH(tamil)
SHAPER_BENCH(thaana)
SHAPER_BENCH(thai)
SHAPER_BENCH(tibetan)
SHAPER_BENCH(tifnagh)
SHAPER_BENCH(vai)
#undef SHAPER_BENCH

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
