/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaperJSONWriter_DEFINED
#define SkShaperJSONWriter_DEFINED

#include <string>

#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkUTF.h"

class SkShaperJSONWriter final : public SkShaper::RunHandler {
public:
    SkShaperJSONWriter(SkJSONWriter* JSONWriter, const char* utf8, size_t size)
            : fJSONWriter{JSONWriter}
            , fUTF8{utf8, size} {}
    void beginLine() override { }

    void runInfo(const RunInfo& info) override { }

    void commitRunInfo() override { }

    Buffer runBuffer(const RunInfo& info) override {
        fGlyphs.resize(info.glyphCount);
        fPositions.resize(info.glyphCount);
        fClusters.resize(info.glyphCount);
        return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0, 0}};
    }

    void commitRunBuffer(const RunInfo& info) override {
        fJSONWriter->beginObject("run", true);

        // Font name
        SkString fontName;
        info.fFont.getTypeface()->getFamilyName(&fontName);
        fJSONWriter->appendString("font name", fontName.c_str());

        // Font size
        fJSONWriter->appendFloat("font size", info.fFont.getSize());

        if (info.fBidiLevel > 0) {
            std::string bidiType = info.fBidiLevel % 2 == 0 ? "left-to-right" : "right-to-left";
            std::string bidiOutput = bidiType + " lvl " + std::to_string(info.fBidiLevel);
            fJSONWriter->appendString("BiDi", bidiOutput.c_str());
        }

        int codePointCount = SkUTF::CountUTF8(
                &fUTF8[info.utf8Range.begin()], info.utf8Range.size());
        if (SkTo<size_t>(codePointCount) == info.glyphCount) {
            std::string utf8{&fUTF8[info.utf8Range.begin()], info.utf8Range.size()};
            fJSONWriter->appendString("UTF8", utf8.c_str());

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
                              SkSpan<const SkGlyphID>{fGlyphs},
                              SkSpan<const uint32_t>{fClusters},
                              [this](int codePointCount, SkSpan<const char> utf1to1,
                                     SkSpan<const SkGlyphID> glyph1to1) {
                                  this->displayMToN(codePointCount, utf1to1, glyph1to1);
                              });
        }

        fJSONWriter->beginArray("horizontal positions", false);
        for (auto position : fPositions) {
            fJSONWriter->appendFloat(position.x());
        }
        fJSONWriter->endArray();

        if (info.glyphCount > 1) {
            fJSONWriter->beginArray("advances", false);
            for (size_t i = 1; i < info.glyphCount; i++) {
                fJSONWriter->appendFloat(fPositions[i].fX - fPositions[i-1].fX);
            }
            fJSONWriter->endArray();
        }

        fJSONWriter->endObject();
    }

    void commitLine() override {}

    template<typename MToN>
    static void BreakupClusters(size_t utf8Begin, size_t utf8End,
                                SkSpan<const uint32_t> clusters,
                                MToN&& processMToN) {

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

    template<typename MToN>
    static void VisualizeClusters(const char utf8[],
                                  size_t utf8Begin, size_t utf8End,
                                  SkSpan<const SkGlyphID> glyphIDs,
                                  SkSpan<const uint32_t> clusters,
                                  MToN&& processMToN) {

        size_t glyphRangeStart, glyphRangeEnd;
        uint32_t utf8RangeStart, utf8RangeEnd;

        auto resetRanges = [&]() {
            glyphRangeStart = SIZE_T_MAX;
            glyphRangeEnd   = 0;
            utf8RangeStart  = UINT32_MAX;
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
            int codePointCount = SkUTF::CountUTF8(&utf8[utf8StartIndex], utf8EndIndex - utf8StartIndex);
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

private:
    void displayMToN(
            int codePointCount, SkSpan<const char> utf1to1,SkSpan<const SkGlyphID> glyph1to1) {
        std::string nString = std::to_string(codePointCount);
        std::string mString = std::to_string(glyph1to1.size());
        std::string clusterName = "cluster " + nString + " to " + mString;
        fJSONWriter->beginObject(clusterName.c_str(), true);
        std::string utf8String{utf1to1.data(), utf1to1.size()};
        fJSONWriter->appendString("UTF", utf8String.c_str());
        fJSONWriter->beginArray("glyphsIDs", false);
        for (auto glyphID : glyph1to1) {
            fJSONWriter->appendU32(glyphID);
        }
        fJSONWriter->endArray();
        fJSONWriter->endObject();
    }

    SkJSONWriter* fJSONWriter;
    std::vector<SkGlyphID> fGlyphs;
    std::vector<SkPoint> fPositions;
    std::vector<uint32_t> fClusters;

    std::string fUTF8;
};

#endif  // SkShaperJSONWriter_DEFINED
