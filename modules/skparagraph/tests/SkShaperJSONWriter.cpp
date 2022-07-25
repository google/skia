/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skparagraph/tests/SkShaperJSONWriter.h"

#include "include/core/SkFont.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTo.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkUTF.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>

SkShaperJSONWriter::SkShaperJSONWriter(SkJSONWriter* JSONWriter, const char* utf8, size_t size)
        : fJSONWriter{JSONWriter}
        , fUTF8{utf8, size} {}

void SkShaperJSONWriter::beginLine() { }

void SkShaperJSONWriter::runInfo(const SkShaper::RunHandler::RunInfo& info) { }

void SkShaperJSONWriter::commitRunInfo() { }

SkShaper::RunHandler::Buffer
SkShaperJSONWriter::runBuffer(const SkShaper::RunHandler::RunInfo& info) {
    fGlyphs.resize(info.glyphCount);
    fPositions.resize(info.glyphCount);
    fClusters.resize(info.glyphCount);
    return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0, 0}};
}

static bool is_one_to_one(const char utf8[], size_t utf8Begin, size_t utf8End,
        std::vector<uint32_t>& clusters) {
    size_t lastUtf8Index = utf8End;

    auto checkCluster = [&](size_t clusterIndex) {
        if (clusters[clusterIndex] >= lastUtf8Index) {
            return false;
        }
        size_t utf8ClusterSize = lastUtf8Index - clusters[clusterIndex];
        if (SkUTF::CountUTF8(&utf8[clusters[clusterIndex]], utf8ClusterSize) != 1) {
            return false;
        }
        lastUtf8Index = clusters[clusterIndex];
        return true;
    };

    if (clusters.front() <= clusters.back()) {
        // left-to-right clusters
        size_t clusterCursor = clusters.size();
        while (clusterCursor > 0) {
            if (!checkCluster(--clusterCursor)) { return false; }
        }
    } else {
        // right-to-left clusters
        size_t clusterCursor = 0;
        while (clusterCursor < clusters.size()) {
            if (!checkCluster(clusterCursor++)) { return false; }
        }
    }

    return true;
}

void SkShaperJSONWriter::commitRunBuffer(const SkShaper::RunHandler::RunInfo& info) {
    fJSONWriter->beginObject("run", true);

    // Font name
    SkString fontName;
    info.fFont.getTypeface()->getFamilyName(&fontName);
    fJSONWriter->appendString("font name", fontName);

    // Font size
    fJSONWriter->appendFloat("font size", info.fFont.getSize());

    if (info.fBidiLevel > 0) {
        std::string bidiType = info.fBidiLevel % 2 == 0 ? "left-to-right" : "right-to-left";
        std::string bidiOutput = bidiType + " lvl " + std::to_string(info.fBidiLevel);
        fJSONWriter->appendString("BiDi", bidiOutput);
    }

    if (is_one_to_one(fUTF8.c_str(), info.utf8Range.begin(), info.utf8Range.end(), fClusters)) {
        std::string utf8{&fUTF8[info.utf8Range.begin()], info.utf8Range.size()};
        fJSONWriter->appendString("UTF8", utf8);

        fJSONWriter->beginArray("glyphs", false);
        for (auto glyphID : fGlyphs) {
            fJSONWriter->appendU32(glyphID);
        }
        fJSONWriter->endArray();

        fJSONWriter->beginArray("clusters", false);
        for (auto cluster : fClusters) {
            fJSONWriter->appendU32(cluster);
        }
        fJSONWriter->endArray();
    } else {
        VisualizeClusters(fUTF8.c_str(),
                          info.utf8Range.begin(), info.utf8Range.end(),
                          SkSpan(fGlyphs),
                          SkSpan(fClusters),
                          [this](size_t codePointCount, SkSpan<const char> utf1to1,
                                 SkSpan<const SkGlyphID> glyph1to1) {
                              this->displayMToN(codePointCount, utf1to1, glyph1to1);
                          });
    }

    if (info.glyphCount > 1) {
        fJSONWriter->beginArray("horizontal positions", false);
        for (auto position : fPositions) {
            fJSONWriter->appendFloat(position.x());
        }
        fJSONWriter->endArray();
    }

    fJSONWriter->beginArray("advances", false);
    for (size_t i = 1; i < info.glyphCount; i++) {
        fJSONWriter->appendFloat(fPositions[i].fX - fPositions[i-1].fX);
    }
    SkPoint lastAdvance = info.fAdvance - (fPositions.back() - fPositions.front());
    fJSONWriter->appendFloat(lastAdvance.fX);
    fJSONWriter->endArray();

    fJSONWriter->endObject();
}

void SkShaperJSONWriter::BreakupClusters(size_t utf8Begin, size_t utf8End,
                                         SkSpan<const uint32_t> clusters,
                                         const BreakupClustersCallback& processMToN) {

    if (clusters.front() <= clusters.back()) {
        // Handle left-to-right text direction
        size_t glyphStartIndex = 0;
        for (size_t glyphEndIndex = 0; glyphEndIndex < clusters.size(); glyphEndIndex++) {

            if (clusters[glyphStartIndex] == clusters[glyphEndIndex]) { continue; }

            processMToN(glyphStartIndex, glyphEndIndex,
                        clusters[glyphStartIndex], clusters[glyphEndIndex]);

            glyphStartIndex = glyphEndIndex;
        }

        processMToN(glyphStartIndex, clusters.size(), clusters[glyphStartIndex], utf8End);

    } else {
        // Handle right-to-left text direction.
        SkASSERT(clusters.size() >= 2);
        size_t glyphStartIndex = 0;
        uint32_t utf8EndIndex = utf8End;
        for (size_t glyphEndIndex = 0; glyphEndIndex < clusters.size(); glyphEndIndex++) {

            if (clusters[glyphStartIndex] == clusters[glyphEndIndex]) { continue; }

            processMToN(glyphStartIndex, glyphEndIndex,
                        clusters[glyphStartIndex], utf8EndIndex);

            utf8EndIndex = clusters[glyphStartIndex];
            glyphStartIndex = glyphEndIndex;
        }
        processMToN(glyphStartIndex, clusters.size(), utf8Begin, clusters[glyphStartIndex-1]);
    }
}

void SkShaperJSONWriter::VisualizeClusters(const char* utf8, size_t utf8Begin, size_t utf8End,
                                           SkSpan<const SkGlyphID> glyphIDs,
                                           SkSpan<const uint32_t> clusters,
                                           const VisualizeClustersCallback& processMToN) {

    size_t glyphRangeStart, glyphRangeEnd;
    uint32_t utf8RangeStart, utf8RangeEnd;

    auto resetRanges = [&]() {
        glyphRangeStart = std::numeric_limits<size_t>::max();
        glyphRangeEnd   = 0;
        utf8RangeStart  = std::numeric_limits<uint32_t>::max();
        utf8RangeEnd    = 0;
    };

    auto checkRangesAndProcess = [&]() {
        if (glyphRangeStart < glyphRangeEnd) {
            size_t glyphRangeCount = glyphRangeEnd - glyphRangeStart;
            SkSpan<const char> utf8Span{&utf8[utf8RangeStart], utf8RangeEnd - utf8RangeStart};
            SkSpan<const SkGlyphID> glyphSpan{&glyphIDs[glyphRangeStart], glyphRangeCount};

            // Glyph count is the same as codepoint count for 1:1.
            processMToN(glyphRangeCount, utf8Span, glyphSpan);
        }
        resetRanges();
    };

    auto gatherRuns = [&](size_t glyphStartIndex, size_t glyphEndIndex,
                          uint32_t utf8StartIndex, uint32_t utf8EndIndex) {
        int possibleCount = SkUTF::CountUTF8(&utf8[utf8StartIndex], utf8EndIndex - utf8StartIndex);
        if (possibleCount == -1) { return; }
        size_t codePointCount = SkTo<size_t>(possibleCount);
        if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
            glyphRangeStart = std::min(glyphRangeStart, glyphStartIndex);
            glyphRangeEnd   = std::max(glyphRangeEnd,   glyphEndIndex  );
            utf8RangeStart  = std::min(utf8RangeStart,  utf8StartIndex );
            utf8RangeEnd    = std::max(utf8RangeEnd,    utf8EndIndex   );
        } else {
            checkRangesAndProcess();

            SkSpan<const char> utf8Span{&utf8[utf8StartIndex], utf8EndIndex - utf8StartIndex};
            SkSpan<const SkGlyphID> glyphSpan{&glyphIDs[glyphStartIndex],
                                              glyphEndIndex - glyphStartIndex};

            processMToN(codePointCount, utf8Span, glyphSpan);
        }
    };

    resetRanges();
    BreakupClusters(utf8Begin, utf8End, clusters, gatherRuns);
    checkRangesAndProcess();
}

void SkShaperJSONWriter::displayMToN(size_t codePointCount,
                                     SkSpan<const char> utf8,
                                     SkSpan<const SkGlyphID> glyphIDs) {
    std::string nString = std::to_string(codePointCount);
    std::string mString = std::to_string(glyphIDs.size());
    std::string clusterName = "cluster " + nString + " to " + mString;
    fJSONWriter->beginObject(clusterName.c_str(), true);
    std::string utf8String{utf8.data(), utf8.size()};
    fJSONWriter->appendString("UTF", utf8String);
    fJSONWriter->beginArray("glyphsIDs", false);
    for (auto glyphID : glyphIDs) {
        fJSONWriter->appendU32(glyphID);
    }
    fJSONWriter->endArray();
    fJSONWriter->endObject();
}
