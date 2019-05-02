// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tests/Test.h"
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

DEF_TEST(Shaper_cluster, reporter) {
    const char* kResources[] = {
        "text/arabic.txt",
        "text/armenian.txt",
        "text/balinese.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/bengali.txt",
        "text/buginese.txt",
        "text/cherokee.txt",
        "text/cyrillic.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/devanagari.txt",
        "text/emoji.txt",
        "text/english.txt",
        "text/ethiopic.txt",
        "text/greek.txt",
        "text/hangul.txt",
        "text/han-simplified.txt",
        "text/han-traditional.txt",
        "text/hebrew.txt",
        "text/javanese.txt",
        "text/kana.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/khmer.txt",
        "text/lao.txt",
        "text/mandaic.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/myanmar.txt",
        "text/newtailue.txt",
        "text/nko.txt",
        "text/sinhala.txt",
        "text/sundanese.txt",
        "text/syriac.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/taitham.txt",
        // TODO(bungeman): https://bugs.skia.org/9050
        //"text/tamil.txt",
        "text/thaana.txt",
        "text/thai.txt",
        "text/tibetan.txt",
        "text/tifnagh.txt",
        "text/vai.txt",
    };
    auto shaper = SkShaper::Make();
    SkFont font;
    if (!shaper) {
        return;
    }
    for (const char* resource : kResources) {
        auto data = GetResourceAsData(resource);
        if (!data) {
            continue;
        }
        RunHandler rh(resource, reporter);
        shaper->shape((const char*)data->data(), data->size(), font, true, 400.0f, &rh);
    }
}
