// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/run_handler.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkString.h"
#include "include/private/SkTFitsIn.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkTextBlobPriv.h"

#include <limits.h>
#include <string.h>

using namespace editor;

void RunHandler::beginLine() {
    fCurrentPosition = fOffset;
    fMaxRunAscent = 0;
    fMaxRunDescent = 0;
    fMaxRunLeading = 0;
}

void RunHandler::runInfo(const SkShaper::RunHandler::RunInfo& info) {
    SkFontMetrics metrics;
    info.fFont.getMetrics(&metrics);
    fMaxRunAscent = SkTMin(fMaxRunAscent, metrics.fAscent);
    fMaxRunDescent = SkTMax(fMaxRunDescent, metrics.fDescent);
    fMaxRunLeading = SkTMax(fMaxRunLeading, metrics.fLeading);
}

void RunHandler::commitRunInfo() {
    fCurrentPosition.fY -= fMaxRunAscent;
}

SkShaper::RunHandler::Buffer RunHandler::runBuffer(const RunInfo& info) {
    int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;
    int utf8RangeSize = SkTFitsIn<int>(info.utf8Range.size()) ? info.utf8Range.size() : INT_MAX;

    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, info.fFont, glyphCount,
                                                                   utf8RangeSize, SkString());
    fCurrentGlyphs = runBuffer.glyphs;
    fCurrentPoints = runBuffer.points();

    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, fUtf8Text + info.utf8Range.begin(), utf8RangeSize);
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = info.utf8Range.begin();

    return {runBuffer.glyphs,
            runBuffer.points(),
            nullptr,
            runBuffer.clusters,
            fCurrentPosition};
}

void RunHandler::commitRunBuffer(const RunInfo& info) {
    // for (size_t i = 0; i < info.glyphCount; ++i) {
    //     SkASSERT(fClusters[i] >= info.utf8Range.begin());
    //     // this fails for khmer example.
    //     SkASSERT(fClusters[i] <  info.utf8Range.end());
    // }
    if (fCallbackFunction) {
        fCallbackFunction(fCallbackContext,
                          fUtf8Text,
                          info.utf8Range.end(),
                          info.glyphCount,
                          fCurrentGlyphs,
                          fCurrentPoints,
                          fClusters,
                          info.fFont);
    }
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
    fCurrentPosition += info.fAdvance;
    fTextOffset = SkTMax(fTextOffset, SkToUInt(info.utf8Range.end()));
}

void RunHandler::commitLine() {
    if (fLineEndOffsets.empty() || fTextOffset > fLineEndOffsets.back()) {
        // Ensure that fLineEndOffsets is monotonic.
        fLineEndOffsets.push_back(fTextOffset);
    }
    fOffset += { 0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent };
}

sk_sp<SkTextBlob> RunHandler::makeBlob() {
    return fBuilder.make();
}


