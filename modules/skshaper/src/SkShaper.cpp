/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"
#include "SkSpan.h"
#include "SkTextBlobPriv.h"

std::unique_ptr<SkShaper> SkShaper::Make() {
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    std::unique_ptr<SkShaper> shaper = SkShaper::MakeHarfBuzz();
    if (shaper) {
        return shaper;
    }
#endif
    return SkShaper::MakePrimitive();
}

SkShaper::SkShaper() {}
SkShaper::~SkShaper() {}

SkShaper::RunHandler::Buffer SkTextBlobBuilderRunHandler::newRunBuffer(const RunInfo&,
                                                                       const SkFont& font,
                                                                       int glyphCount,
                                                                       SkSpan<const char> utf8) {
    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, font, glyphCount,
                                                                   utf8.size(), SkString());
    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, utf8.data(), utf8.size());
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = utf8.data() - fUtf8Text;

    return { runBuffer.glyphs,
             runBuffer.points(),
             runBuffer.clusters };
}

void SkTextBlobBuilderRunHandler::commitRun() {
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
}

sk_sp<SkTextBlob> SkTextBlobBuilderRunHandler::makeBlob() {
    return fBuilder.make();
}
