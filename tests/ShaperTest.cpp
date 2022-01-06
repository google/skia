// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tests/Test.h"

#if defined(SKSHAPER_IMPLEMENTATION) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "tools/Resources.h"

#include <cstdint>
#include <memory>

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
        fGlyphs = std::make_unique<SkGlyphID[]>(info.glyphCount);
        fPositions = std::make_unique<SkPoint[]>(info.glyphCount);
        fClusters = std::make_unique<uint32_t[]>(info.glyphCount);
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

void shaper_test(skiatest::Reporter* reporter, const char* name, SkData* data) {
    auto shaper = SkShaper::Make();
    if (!shaper) {
        ERRORF(reporter, "Could not create shaper.");
        return;
    }

    constexpr float kWidth = 400;
    SkFont font(SkTypeface::MakeDefault());
    RunHandler rh(name, reporter);
    shaper->shape((const char*)data->data(), data->size(), font, true, kWidth, &rh);

    constexpr SkFourByteTag latn = SkSetFourByteTag('l','a','t','n');
    auto fontIterator = SkShaper::TrivialFontRunIterator(font, data->size());
    auto bidiIterator = SkShaper::TrivialBiDiRunIterator(0, data->size());
    auto scriptIterator = SkShaper::TrivialScriptRunIterator(latn, data->size());
    auto languageIterator = SkShaper::TrivialLanguageRunIterator("en-US", data->size());
    shaper->shape((const char*)data->data(), data->size(),
                  fontIterator, bidiIterator, scriptIterator, languageIterator, kWidth, &rh);
}

void cluster_test(skiatest::Reporter* reporter, const char* resource) {
    auto data = GetResourceAsData(resource);
    if (!data) {
        ERRORF(reporter, "Could not get resource %s.", resource);
        return;
    }

    shaper_test(reporter, resource, data.get());
}

}  // namespace

DEF_TEST(Shaper_cluster_empty, r) { shaper_test(r, "empty", SkData::MakeEmpty().get()); }

#define SHAPER_TEST(X) DEF_TEST(Shaper_cluster_ ## X, r) { cluster_test(r, "text/" #X ".txt"); }
SHAPER_TEST(arabic)
SHAPER_TEST(armenian)
SHAPER_TEST(balinese)
SHAPER_TEST(buginese)
SHAPER_TEST(cherokee)
SHAPER_TEST(cyrillic)
SHAPER_TEST(emoji)
SHAPER_TEST(english)
SHAPER_TEST(ethiopic)
SHAPER_TEST(greek)
SHAPER_TEST(hangul)
SHAPER_TEST(han_simplified)
SHAPER_TEST(han_traditional)
SHAPER_TEST(hebrew)
SHAPER_TEST(javanese)
SHAPER_TEST(kana)
SHAPER_TEST(lao)
SHAPER_TEST(mandaic)
SHAPER_TEST(newtailue)
SHAPER_TEST(nko)
SHAPER_TEST(sinhala)
SHAPER_TEST(sundanese)
SHAPER_TEST(syriac)
SHAPER_TEST(thaana)
SHAPER_TEST(thai)
SHAPER_TEST(tibetan)
SHAPER_TEST(tifnagh)
SHAPER_TEST(vai)

// TODO(bungeman): fix these broken tests. (https://bugs.skia.org/9050)
//SHAPER_TEST(bengali)
//SHAPER_TEST(devanagari)
//SHAPER_TEST(khmer)
//SHAPER_TEST(myanmar)
//SHAPER_TEST(taitham)
//SHAPER_TEST(tamil)
#undef SHAPER_TEST

#endif  // defined(SKSHAPER_IMPLEMENTATION) && !defined(SK_BUILD_FOR_GOOGLE3)
