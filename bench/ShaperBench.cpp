// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "modules/skshaper/include/SkShaper.h"
#include "tools/Resources.h"

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
        SkFont font;
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

DEF_BENCH(return new ShaperBench("text/arabic.txt",          "shaper_arabic");)
DEF_BENCH(return new ShaperBench("text/armenian.txt",        "shaper_armenian");)
DEF_BENCH(return new ShaperBench("text/balinese.txt",        "shaper_balinese");)
DEF_BENCH(return new ShaperBench("text/bengali.txt",         "shaper_bengali");)
DEF_BENCH(return new ShaperBench("text/buginese.txt",        "shaper_buginese");)
DEF_BENCH(return new ShaperBench("text/cherokee.txt",        "shaper_cherokee");)
DEF_BENCH(return new ShaperBench("text/cyrillic.txt",        "shaper_cyrillic");)
DEF_BENCH(return new ShaperBench("text/devanagari.txt",      "shaper_devanagari");)
DEF_BENCH(return new ShaperBench("text/emoji.txt",           "shaper_emoji");)
DEF_BENCH(return new ShaperBench("text/english.txt",         "shaper_english");)
DEF_BENCH(return new ShaperBench("text/ethiopic.txt",        "shaper_ethiopic");)
DEF_BENCH(return new ShaperBench("text/greek.txt",           "shaper_greek");)
DEF_BENCH(return new ShaperBench("text/hangul.txt",          "shaper_hangul");)
DEF_BENCH(return new ShaperBench("text/han-simplified.txt",  "shaper_han_simplified");)
DEF_BENCH(return new ShaperBench("text/han-traditional.txt", "shaper_han_traditional");)
DEF_BENCH(return new ShaperBench("text/hebrew.txt",          "shaper_hebrew");)
DEF_BENCH(return new ShaperBench("text/javanese.txt",        "shaper_javanese");)
DEF_BENCH(return new ShaperBench("text/kana.txt",            "shaper_kana");)
DEF_BENCH(return new ShaperBench("text/khmer.txt",           "shaper_khmer");)
DEF_BENCH(return new ShaperBench("text/lao.txt",             "shaper_lao");)
DEF_BENCH(return new ShaperBench("text/mandaic.txt",         "shaper_mandaic");)
DEF_BENCH(return new ShaperBench("text/myanmar.txt",         "shaper_myanmar");)
DEF_BENCH(return new ShaperBench("text/newtailue.txt",       "shaper_newtailue");)
DEF_BENCH(return new ShaperBench("text/nko.txt",             "shaper_nko");)
DEF_BENCH(return new ShaperBench("text/sinhala.txt",         "shaper_sinhala");)
DEF_BENCH(return new ShaperBench("text/sundanese.txt",       "shaper_sundanese");)
DEF_BENCH(return new ShaperBench("text/syriac.txt",          "shaper_syriac");)
DEF_BENCH(return new ShaperBench("text/taitham.txt",         "shaper_taitham");)
DEF_BENCH(return new ShaperBench("text/tamil.txt",           "shaper_tamil");)
DEF_BENCH(return new ShaperBench("text/thaana.txt",          "shaper_thaana");)
DEF_BENCH(return new ShaperBench("text/thai.txt",            "shaper_thai");)
DEF_BENCH(return new ShaperBench("text/tibetan.txt",         "shaper_tibetan");)
DEF_BENCH(return new ShaperBench("text/tifnagh.txt",         "shaper_tifnagh");)
DEF_BENCH(return new ShaperBench("text/vai.txt",             "shaper_vai");)

#endif  // !SK_BUILD_FOR_ANDROID_FRAMEWORK
