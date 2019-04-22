/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "Test.h"

#include "src/core/SkSpan.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkUTF.h"

/*
 *  Look at SkClusterator
 */

/*
static const char gText[] =
        "When in the Course of human events it becomes necessary for one people "
        "to dissolve the political bands which have connected them with another "
        "and to assume among the powers of the earth, the separate and equal "
        "station to which the Laws of Nature and of Nature's God entitle them, "
        "a decent respect to the opinions of mankind requires that they should "
        "declare the causes which impel them to the separation.";

*/
template<typename MToN>
static void breakup_clusters(SkSpan<const char> utf8,
                             SkSpan<const SkGlyphID> glyphIDs,
                             SkSpan<const uint32_t> clusters,
                             MToN&& processMToN) {
    std::string utf1to1;
    std::vector<SkGlyphID> glyph1to1;

    auto handle1To1 = [&utf1to1, &glyph1to1, &processMToN]() {
        if (!utf1to1.empty()) {
            processMToN(glyph1to1.size(),
                        SkSpan<const char>{utf1to1},
                        SkSpan<const SkGlyphID>{glyph1to1});
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

DEF_TEST(SkShaperTest_cluster, reporter) {
    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    SkFont font(nullptr, 14);

    SkDynamicMemoryWStream out;
    SkJSONWriter jsonWriter{&out, SkJSONWriter::Mode::kPretty};

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
            {"ABC", {7, 8}, {3, 3, 0}},
            {"ABCD", {7, 8, 9, 10}, {4, 3, 2, 1, 0}},
            {"المادة", {246, 268, 241, 205, 240}, {12, 10, 8, 6, 2, 0}}
    };

    jsonWriter.beginObject();
    jsonWriter.beginArray("testcases");
    for (auto& oneCase : cases) {
        jsonWriter.beginObject();
        breakup_clusters(SkSpan<const char>{oneCase.utf8},
                         SkSpan<const SkGlyphID>{oneCase.glyphIDs},
                         SkSpan<const uint32_t>{oneCase.clusters},
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
    SkShaperJSONWriter(SkJSONWriter* JSONWriter, const char* utf8, size_t size)
        : fJSONWriter{JSONWriter}
        , fUTF8{utf8, size} {}
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
        return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0, 0}};
    }

    void commitRunBuffer(const RunInfo& info) override {
        fJSONWriter->beginObject("run", true);

        int codePointCount = SkUTF::CountUTF8(
                &fUTF8[info.utf8Range.begin()], info.utf8Range.size());
        std::string nString = std::to_string(codePointCount);
        std::string mString = std::to_string(info.glyphCount);
        std::string countCompare = nString + " to " + mString;

        fJSONWriter->appendString("counts", countCompare.c_str());

        std::string utf8{&fUTF8[info.utf8Range.begin()], info.utf8Range.size()};
        fJSONWriter->appendString("UTF8", utf8.c_str());

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


        uint32_t firstCluster = fClusters.front();
        uint32_t lastCluster = fClusters.back();
        if (firstCluster < lastCluster) {
            fClusters.push_back(firstCluster + info.utf8Range.size());
        } else {
            fClusters.insert(fClusters.begin(), lastCluster + info.utf8Range.size());
        }
#endif


        fJSONWriter->beginArray("clusters", false);
        for (auto cluster : fClusters) {
            fJSONWriter->appendU32(cluster);
        }
        fJSONWriter->endArray();
        fJSONWriter->endObject();
    }

    void commitLine() override {

    }

private:
    SkJSONWriter* fJSONWriter;
    std::vector<SkGlyphID> fGlyphs;
    std::vector<SkPoint> fPositions;
    std::vector<uint32_t> fClusters;

    std::string fUTF8
;
};

DEF_TEST(SkShaperTest_basic, reporter) {
    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    SkFont font(nullptr, 14);

    SkDynamicMemoryWStream out;
    SkJSONWriter jsonWriter{&out, SkJSONWriter::Mode::kPretty};
    std::string s = "المادة 1 يولد جميع الناس أحرارًا متساوين في الكرامة والحقوق. وقد وهبوا عقلاً وضميرًا وعليهم أن يعامل بعضهم بعضًا بروح الإخاء.";

    SkShaperJSONWriter shaperJSON{&jsonWriter, s.c_str(), s.size()};

    jsonWriter.beginObject();
    shaper->shape(s.c_str(), s.size(), font, false /* left to right*/,  256, &shaperJSON);
    jsonWriter.endObject();
    jsonWriter.flush();

    std::string sout(out.bytesWritten(), 0);
    out.copyTo(&sout[0]);
    SkDebugf("%s", sout.c_str());

}