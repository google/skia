/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "Test.h"

#include "SkJSONWriter.h"
#include "SkSpan.h"
#include "SkUTF.h"

static const char gText[] =
        "When in the Course of human events it becomes necessary for one people "
        "to dissolve the political bands which have connected them with another "
        "and to assume among the powers of the earth, the separate and equal "
        "station to which the Laws of Nature and of Nature's God entitle them, "
        "a decent respect to the opinions of mankind requires that they should "
        "declare the causes which impel them to the separation.";


template<typename OneToOne, typename MToN>
static void breakup_clusters(SkSpan<const char> utf8,
                             SkSpan<const SkGlyphID> glyphIDs,
                             SkSpan<const uint32_t> clusters,
                             OneToOne&& processOneToOne,
                             MToN&& processMToN) {
    std::string utf1to1;
    std::vector<SkGlyphID> glyph1to1;

    auto handle1To1 = [&utf1to1, &glyph1to1, &processOneToOne]() {
        if (!utf1to1.empty()) {
            processOneToOne(SkSpan<const char>{utf1to1}, SkSpan<const SkGlyphID>{glyph1to1});
            utf1to1.clear();
            glyph1to1.clear();
        }
    };

    auto handleMToN = [&handle1To1, processMToN](
            int codePointCount, SkSpan<const char> utf, SkSpan<const SkGlyphID> glyphs) {
        handle1To1();
        processMToN(codePointCount, utf, glyphs);
    };

    uint32_t glyphStartIndex = 0;
    uint32_t n = clusters.size();
    for (uint32_t glyphEndIndex = 1; glyphEndIndex < n; glyphEndIndex++) {
        uint32_t utf8Start = clusters[glyphEndIndex-1];
        uint32_t utf8End = clusters[glyphEndIndex];
        int32_t utf8Count = utf8End - utf8Start;
        if (utf8Count > 0) {
            int codePointCount = SkUTF::CountUTF8(&utf8[utf8Start], utf8Count);
            if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                utf1to1 += std::string(&utf8[utf8Start], utf8Count);
                glyph1to1.push_back(glyphIDs[glyphEndIndex-1]);
            } else {
                handleMToN(codePointCount,
                           SkSpan<const char>(&utf8[utf8Start], utf8Count),
                           SkSpan<const SkGlyphID>(
                                   &glyphIDs[glyphStartIndex], glyphEndIndex - glyphStartIndex));
            }
            glyphStartIndex = glyphEndIndex;
        } else if (utf8Count < 0) {
            int codePointCount = SkUTF::CountUTF8(&utf8[utf8End], -utf8Count);
            if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                utf1to1 += std::string(&utf8[utf8End], -utf8Count);
                glyph1to1.push_back(glyphIDs[glyphEndIndex-1]);
            } else {
                handleMToN(codePointCount,
                           SkSpan<const char>(&utf8[utf8End], -utf8Count),
                           SkSpan<const SkGlyphID>(
                                   &glyphIDs[glyphStartIndex], glyphEndIndex - glyphStartIndex));
            }
            glyphStartIndex = glyphEndIndex;
        } else {
            continue;
        }
    }

    handle1To1();
}

static void display_1_to_1 (
        SkJSONWriter* fJSONWriter, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
    std::string countString = std::to_string(glyph1to1.size());
    std::string clusterName = countString + " to " + countString;

    fJSONWriter->beginObject(clusterName.c_str(), true);
    fJSONWriter->appendString("UTF", utf1to1.data());
    fJSONWriter->beginArray("glyphs", false);
    for (auto glyphID : glyph1to1) {
        fJSONWriter->appendU32(glyphID);
    }
    fJSONWriter->endArray();
    fJSONWriter->endObject();
};

DEF_TEST(SkShaperTest_cluster, reporter) {
    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    SkFont font(nullptr, 14);

    SkDynamicMemoryWStream out;
    SkJSONWriter jsonWriter{&out, SkJSONWriter::Mode::kPretty};

    auto display1To1 = [&jsonWriter]
            (SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
        display_1_to_1(&jsonWriter, utf1to1, glyph1to1);
    };

    auto displayMToN = [&jsonWriter](
            int codePointCount, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
        std::string nString = std::to_string(codePointCount);
        std::string mString = std::to_string(glyph1to1.size());
        std::string clusterName = nString + " to " + mString;
        jsonWriter.beginObject(clusterName.c_str(), true);
        jsonWriter.appendString("UTF", utf1to1.data());
        jsonWriter.beginArray("glyphsIDs", false);
        for (auto glyphID : glyph1to1) {
            jsonWriter.appendU32(glyphID);
        }
        jsonWriter.endArray();
        jsonWriter.endObject();

    };

    struct TestCase {
        std::string utf8;
        std::vector<SkGlyphID> glyphIDs;
        std::vector<uint32_t> clusters;
    };

    std::vector<TestCase> cases = {
            {"A", {7}, {0, 1}},
            {"ABCD", {7, 8, 9, 10}, {0, 1, 2, 3, 4}},
            {"A", {7, 8}, {0, 0, 1}},
            {"AB", {7}, {0, 2}},
            {"AB", {7, 8, 9}, {0, 0, 0, 2}},
            {"ABC", {7, 8}, {0, 0, 3}},
    };

    jsonWriter.beginObject();
    jsonWriter.beginArray("testcases");
    for (auto& oneCase : cases) {
        jsonWriter.beginObject();
        breakup_clusters(SkSpan<const char>{oneCase.utf8},
                         SkSpan<const SkGlyphID>{oneCase.glyphIDs},
                         SkSpan<const uint32_t>{oneCase.clusters},
                         display1To1,
                         displayMToN);
        jsonWriter.endObject();
    }
    jsonWriter.endArray();
    jsonWriter.endObject();
    jsonWriter.flush();

    std::string sout(out.bytesWritten(), 0);
    out.copyTo(&sout[0]);
    SkDebugf("%s", sout.c_str());
}


class SkShaperJSONWriter final : public SkShaper::RunHandler {
public:
    SkShaperJSONWriter(SkJSONWriter* JSONWriter) : fJSONWriter{JSONWriter} {}
    void beginLine() override {

    }

    void runInfo(const RunInfo& info) override {

    }

    void commitRunInfo() override {

    }

    Buffer runBuffer(const RunInfo& info) override {
        fGlyphs.resize(info.glyphCount);
        fPositions.resize(info.glyphCount);
        fClusters.resize(info.glyphCount);
        fUTF8.resize(info.utf8Range.size());
        memcpy(fUTF8.data(), gText + info.utf8Range.begin(), info.utf8Range.size());
        return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0, 0}};
    }

    void commitRunBuffer(const RunInfo& info) override {
        fJSONWriter->beginObject("run", true);
        fJSONWriter->beginArray("glyphs", false);
        for (auto glyphID : fGlyphs) {
            fJSONWriter->appendU32(glyphID);
        }
        fJSONWriter->endArray();

#if 0
        fJSONWriter->beginArray("positions", false);
        for (auto position : fPositions) {
            fJSONWriter->beginArray(nullptr, false);
            fJSONWriter->appendFloat(position.x());
            fJSONWriter->appendFloat(position.y());
            fJSONWriter->endArray();
        }
        fJSONWriter->endArray();
#endif

        uint32_t firstCluster = fClusters.front();
        uint32_t lastCluster = fClusters.back();
        if (firstCluster < lastCluster) {
            fClusters.push_back(firstCluster + info.utf8Range.size());
        } else {
            fClusters.insert(fClusters.begin(), lastCluster + info.utf8Range.size());
        }

        fJSONWriter->beginArray("clusters", false);
        for (auto cluster : fClusters) {
            fJSONWriter->appendU32(cluster);
        }
        fJSONWriter->endArray();

        // Simplify everything by adding a last element to the cluster vector.

        std::string utf1to1;
        std::vector<SkGlyphID> glyph1to1;

        auto display1To1 = [this, &utf1to1, &glyph1to1]() {
            std::string countString = std::to_string(glyph1to1.size());
            std::string clusterName = countString + "-" + countString;

            fJSONWriter->beginObject(clusterName.c_str(), true);
            fJSONWriter->appendString("UTF", utf1to1.c_str());
            fJSONWriter->beginArray("glyphs", false);
            for (auto glyphID : glyph1to1) {
                fJSONWriter->appendU32(glyphID);
            }
            fJSONWriter->endArray();
            fJSONWriter->endObject();
            utf1to1.clear();
            glyph1to1.clear();
        };

        auto nToM = [&display1To1](uint32_t startUtf8, uint32_t endUtf8,
                                   uint32_t startGlyphIndex, uint32_t endGlyphIndex) {
            display1To1();
        };

        fJSONWriter->beginObject("ClusterVisualize", true);
        uint32_t glyphStartIndex = 0;
        uint32_t n = fClusters.size();
        for (uint32_t glyphEndIndex = 1; glyphEndIndex < n; glyphEndIndex++) {
            uint32_t utf8Start = fClusters[glyphEndIndex-1];
            uint32_t utf8End = fClusters[glyphEndIndex];
            int32_t utf8Count = utf8End - utf8Start;
            if (utf8Count > 0) {
                int codePointCount = SkUTF::CountUTF8(&gText[utf8Start], utf8Count);
                if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                    utf1to1 += std::string(&gText[utf8Start], utf8Count);
                    glyph1to1.push_back(fGlyphs[glyphEndIndex-1]);
                } else {
                    nToM(utf8Start, utf8End, glyphStartIndex, glyphEndIndex);
                }
                glyphStartIndex = glyphEndIndex;
            } else if (utf8Count < 0) {
                int codePointCount = SkUTF::CountUTF8(&gText[utf8End], -utf8Count);
                if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                    utf1to1 += std::string(&gText[utf8End], -utf8Count);
                    glyph1to1.push_back(fGlyphs[glyphEndIndex-1]);
                } else {
                    nToM(utf8End, utf8Start, glyphStartIndex, glyphEndIndex);
                }
                glyphStartIndex = glyphEndIndex;
            } else {
                continue;
            }
        }

        if (!utf1to1.empty()) {
            display1To1();
        }

        fJSONWriter->endObject();

        fJSONWriter->endObject();
    }

    void commitLine() override {

    }

private:
    std::vector<SkGlyphID> fGlyphs;
    std::vector<SkPoint> fPositions;
    std::vector<char> fUTF8;
    std::vector<uint32_t> fClusters;

    SkJSONWriter* fJSONWriter;
};

DEF_TEST(SkShaperTest_basic, reporter) {
    return;
    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    SkFont font(nullptr, 14);

    SkDynamicMemoryWStream out;
    SkJSONWriter jsonWriter{&out, SkJSONWriter::Mode::kPretty};

    SkShaperJSONWriter shaperJSON{&jsonWriter};

    jsonWriter.beginObject();
    shaper->shape(gText, strlen(gText) - 1, font, true /* left to right*/,  256, &shaperJSON);
    jsonWriter.endObject();
    jsonWriter.flush();

    std::string sout(out.bytesWritten(), 0);
    out.copyTo(&sout[0]);
    SkDebugf("%s", sout.c_str());

}