// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tests/Test.h"

#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "modules/skshaper/include/SkShaper.h"
#include "tools/Resources.h"

namespace {
struct RunHandler final : public SkShaper::RunHandler {
    const char* fResource;
    skiatest::Reporter* fReporter;
    std::unique_ptr<SkGlyphID[]> fGlyphs;
    std::unique_ptr<SkPoint[]> fPositions;
    std::unique_ptr<uint32_t[]> fClusters;
    SkShaper::RunHandler::Range fRange;
    unsigned fGlyphCount = 0;

    RunHandler(const char* resource, skiatest::Reporter* reporter)
        : fResource(resource), fReporter(reporter) {}

    void beginLine() override {}
    void runInfo(const SkShaper::RunHandler::RunInfo& info) override {}
    void commitRunInfo() override {}
    SkShaper::RunHandler::Buffer runBuffer(const SkShaper::RunHandler::RunInfo& info) override {
        fGlyphCount = SkToUInt(info.glyphCount);
        fRange = info.utf8Range;
        fGlyphs.reset(new SkGlyphID[info.glyphCount]);
        fPositions.reset(new SkPoint[info.glyphCount]);
        fClusters.reset(new uint32_t[info.glyphCount]);
        return SkShaper::RunHandler::Buffer{fGlyphs.get(),
                                            fPositions.get(),
                                            nullptr,
                                            fClusters.get(),
                                            {0, 0}};
    }
    void commitRunBuffer(const RunInfo& info) override {
        REPORTER_ASSERT(fReporter, fGlyphCount == info.glyphCount, "%s", fResource);
        REPORTER_ASSERT(fReporter, fRange.begin() == info.utf8Range.begin(), "%s", fResource);
        REPORTER_ASSERT(fReporter, fRange.size() == info.utf8Range.size(), "%s", fResource);
        for (unsigned i = 0; i < fGlyphCount; ++i) {
            REPORTER_ASSERT(fReporter, fClusters[i] >= fRange.begin(),
                            "%s %u %u", fResource, i, fGlyphCount);
            REPORTER_ASSERT(fReporter, fClusters[i] <  fRange.end(),
                            "%s %u %u", fResource, i, fGlyphCount);
        }
    }
    void commitLine() override {}
};
}  // namespace

static void shaper_cluster_test(skiatest::Reporter* reporter, const char* resource) {
    constexpr float kWidth = 400;
    if (auto shaper = SkShaper::Make()) {
        if (auto data = GetResourceAsData(resource)) {
            SkFont font;
            RunHandler rh(resource, reporter);
            shaper->shape((const char*)data->data(), data->size(), font, true, kWidth, &rh);
        }
    }
}

DEF_TEST(Shaper_cluster_arabic,     r) { shaper_cluster_test(r, "text/arabic.txt"); }
DEF_TEST(Shaper_cluster_armenian,   r) { shaper_cluster_test(r, "text/armenian.txt"); }
DEF_TEST(Shaper_cluster_balinese,   r) { shaper_cluster_test(r, "text/balinese.txt"); }
DEF_TEST(Shaper_cluster_buginese,   r) { shaper_cluster_test(r, "text/buginese.txt"); }
DEF_TEST(Shaper_cluster_cherokee,   r) { shaper_cluster_test(r, "text/cherokee.txt"); }
DEF_TEST(Shaper_cluster_cyrillic,   r) { shaper_cluster_test(r, "text/cyrillic.txt"); }
DEF_TEST(Shaper_cluster_emoji,      r) { shaper_cluster_test(r, "text/emoji.txt"); }
DEF_TEST(Shaper_cluster_english,    r) { shaper_cluster_test(r, "text/english.txt"); }
DEF_TEST(Shaper_cluster_ethiopic,   r) { shaper_cluster_test(r, "text/ethiopic.txt"); }
DEF_TEST(Shaper_cluster_greek,      r) { shaper_cluster_test(r, "text/greek.txt"); }
DEF_TEST(Shaper_cluster_hangul,     r) { shaper_cluster_test(r, "text/hangul.txt"); }
DEF_TEST(Shaper_cluster_han_s,      r) { shaper_cluster_test(r, "text/han-simplified.txt"); }
DEF_TEST(Shaper_cluster_han_t,      r) { shaper_cluster_test(r, "text/han-traditional.txt"); }
DEF_TEST(Shaper_cluster_hebrew,     r) { shaper_cluster_test(r, "text/hebrew.txt"); }
DEF_TEST(Shaper_cluster_javanese,   r) { shaper_cluster_test(r, "text/javanese.txt"); }
DEF_TEST(Shaper_cluster_kana,       r) { shaper_cluster_test(r, "text/kana.txt"); }
DEF_TEST(Shaper_cluster_lao,        r) { shaper_cluster_test(r, "text/lao.txt"); }
DEF_TEST(Shaper_cluster_mandaic,    r) { shaper_cluster_test(r, "text/mandaic.txt"); }
DEF_TEST(Shaper_cluster_newtailue,  r) { shaper_cluster_test(r, "text/newtailue.txt"); }
DEF_TEST(Shaper_cluster_nko,        r) { shaper_cluster_test(r, "text/nko.txt"); }
DEF_TEST(Shaper_cluster_sinhala,    r) { shaper_cluster_test(r, "text/sinhala.txt"); }
DEF_TEST(Shaper_cluster_sundanese,  r) { shaper_cluster_test(r, "text/sundanese.txt"); }
DEF_TEST(Shaper_cluster_syriac,     r) { shaper_cluster_test(r, "text/syriac.txt"); }
DEF_TEST(Shaper_cluster_thaana,     r) { shaper_cluster_test(r, "text/thaana.txt"); }
DEF_TEST(Shaper_cluster_thai,       r) { shaper_cluster_test(r, "text/thai.txt"); }
DEF_TEST(Shaper_cluster_tibetan,    r) { shaper_cluster_test(r, "text/tibetan.txt"); }
DEF_TEST(Shaper_cluster_tifnagh,    r) { shaper_cluster_test(r, "text/tifnagh.txt"); }
DEF_TEST(Shaper_cluster_vai,        r) { shaper_cluster_test(r, "text/vai.txt"); }

// TODO(bungeman): fix these broken tests. (https://bugs.skia.org/9050)
//DEF_TEST(Shaper_cluster_bengali,    r) { shaper_cluster_test(r, "text/bengali.txt"); }
//DEF_TEST(Shaper_cluster_devanagari, r) { shaper_cluster_test(r, "text/devanagari.txt"); }
//DEF_TEST(Shaper_cluster_khmer,      r) { shaper_cluster_test(r, "text/khmer.txt"); }
//DEF_TEST(Shaper_cluster_myanmar,    r) { shaper_cluster_test(r, "text/myanmar.txt"); }
//DEF_TEST(Shaper_cluster_taitham,    r) { shaper_cluster_test(r, "text/taitham.txt"); }
//DEF_TEST(Shaper_cluster_tamil,      r) { shaper_cluster_test(r, "text/tamil.txt"); }

#endif  // !SK_BUILD_FOR_ANDROID_FRAMEWORK
