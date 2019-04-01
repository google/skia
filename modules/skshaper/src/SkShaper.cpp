/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"
#include "SkTextBlobPriv.h"
#include "SkFontMetrics.h"

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

void SkTextBlobBuilderRunHandler::beginLine() {
    fCurrentPosition = fOffset;
    fMaxRunAscent = 0;
    fMaxRunDescent = 0;
    fMaxRunLeading = 0;
}
void SkTextBlobBuilderRunHandler::runInfo(const RunInfo& info) {
    SkFontMetrics metrics;
    info.fFont.getMetrics(&metrics);
    fMaxRunAscent = SkTMin(fMaxRunAscent, metrics.fAscent);
    fMaxRunDescent = SkTMax(fMaxRunDescent, metrics.fDescent);
    fMaxRunLeading = SkTMax(fMaxRunLeading, metrics.fLeading);
}

void SkTextBlobBuilderRunHandler::commitRunInfo() {
    fCurrentPosition.fY -= fMaxRunAscent;
}

SkShaper::RunHandler::Buffer SkTextBlobBuilderRunHandler::runBuffer(const RunInfo& info) {
    int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;
    int utf8RangeSize = SkTFitsIn<int>(info.utf8Range.size()) ? info.utf8Range.size() : INT_MAX;

    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, info.fFont, glyphCount,
                                                                   utf8RangeSize, SkString());
    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, fUtf8Text + info.utf8Range.begin(), utf8RangeSize);
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = info.utf8Range.begin();

    return { runBuffer.glyphs,
             runBuffer.points(),
             nullptr,
             runBuffer.clusters,
             fCurrentPosition };
}

void SkTextBlobBuilderRunHandler::commitRunBuffer(const RunInfo& info) {
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
    fCurrentPosition += info.fAdvance;
}
void SkTextBlobBuilderRunHandler::commitLine() {
    fOffset += { 0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent };
}

sk_sp<SkTextBlob> SkTextBlobBuilderRunHandler::makeBlob() {
    return fBuilder.make();
}
