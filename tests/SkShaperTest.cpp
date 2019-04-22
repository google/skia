/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "Test.h"

#include "SkJSONWriter.h"
#include "SkUTF.h"

static const char gText[] =
        "When in the Course of human events it becomes necessary for one people "
        "to dissolve the political bands which have connected them with another "
        "and to assume among the powers of the earth, the separate and equal "
        "station to which the Laws of Nature and of Nature's God entitle them, "
        "a decent respect to the opinions of mankind requires that they should "
        "declare the causes which impel them to the separation.";



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
        fJSONWriter->beginArray("positions", false);
        for (auto position : fPositions) {
            fJSONWriter->beginArray(nullptr, false);
            fJSONWriter->appendFloat(position.x());
            fJSONWriter->appendFloat(position.y());
            fJSONWriter->endArray();
        }
        fJSONWriter->endArray();
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
                int codePointCount = SkUTF::CountUTF8(&fUTF8[utf8Start], utf8Count);
                if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                    utf1to1 += std::string(&fUTF8[utf8Start], utf8Count);
                    glyph1to1.push_back(fGlyphs[glyphEndIndex-1]);
                } else {
                    nToM(utf8Start, utf8End, glyphStartIndex, glyphEndIndex);
                }
                glyphStartIndex = glyphEndIndex;
            } else if (utf8Count < 0) {
                int codePointCount = SkUTF::CountUTF8(&fUTF8[utf8End], -utf8Count);
                if (codePointCount == 1 && glyphEndIndex - glyphStartIndex == 1) {
                    utf1to1 += std::string(&fUTF8[utf8End], -utf8Count);
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