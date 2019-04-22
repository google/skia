/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "Test.h"

#include "src/core/SkSpan.h"
#include "src/utils/SkShaperJSONWriter.h"
#include "src/utils/SkUTF.h"

DEF_TEST(SkShaperTest_cluster, reporter) {

    struct Answer {
        size_t glyphStartIndex, glyphEndIndex;
        uint32_t utf8StartIndex, utf8EndIndex;
    };

    struct TestCase {
        size_t utf8Len;
        std::vector<uint32_t> clusters;
        std::vector<Answer> answers;
    };

    std::vector<TestCase> cases = {
            /*1:1*/ { 1, {0}, {{0, 1, 0, 1}} },
            /*1:2*/ { 1, {0, 0}, {{0, 2, 0, 1}} },
            /*2:1*/ { 2, {0}, {{0, 1, 0, 2}} },
            /*2:3*/ { 2, {0, 0, 0}, {{0, 3, 0, 2}} },
            /*3:2*/ { 3, {0, 0}, {{0, 2, 0, 3}} },

            // cluster runs
            { 2, {0, 1}, {{0, 1, 0, 1}, {1, 2, 1, 2}} },
            { 2, {1, 0}, {{0, 1, 1, 2}, {1, 2, 0, 1}} },
            { 2, {0, 0, 1}, {{0, 2, 0, 1}, {2, 3, 1, 2}} },
            { 2, {1, 0, 0}, {{0, 1, 1, 2}, {1, 3, 0, 1}} },
            { 2, {0, 1, 1}, {{0, 1, 0, 1}, {1, 3, 1, 2}} },
            { 2, {1, 1, 0}, {{0, 2, 1, 2}, {2, 3, 0, 1}} },
            { 3, {0, 0, 1}, {{0, 2, 0, 1}, {2, 3, 1, 3}} },
            { 3, {1, 0, 0}, {{0, 1, 1, 3}, {1, 3, 0, 1}} },
            { 3, {0, 1, 1}, {{0, 1, 0, 1}, {1, 3, 1, 3}} },
            { 3, {1, 1, 0}, {{0, 2, 1, 3}, {2, 3, 0, 1}} },
            { 4, {3, 2, 1, 0}, {{0, 1, 3, 4}, {1, 2, 2, 3}, {2, 3, 1, 2}, {3, 4, 0, 1}} },
    };

    for (auto& oneCase : cases) {
        size_t answerCount = 0;
        auto checker = [&](size_t glyphStartIndex, size_t glyphEndIndex,
                           uint32_t utf8StartIndex, uint32_t utf8EndIndex) {
            if (answerCount < oneCase.answers.size()) {
                Answer a = oneCase.answers[answerCount];
                REPORTER_ASSERT(reporter, a.glyphStartIndex == glyphStartIndex);
                REPORTER_ASSERT(reporter,   a.glyphEndIndex == glyphEndIndex  );
                REPORTER_ASSERT(reporter,  a.utf8StartIndex == utf8StartIndex );
                REPORTER_ASSERT(reporter,    a.utf8EndIndex == utf8EndIndex   );

            } else {
                REPORTER_ASSERT(reporter, false, "Too many clusters");
            }
            answerCount++;
        };

        SkShaperJSONWriter::BreakupClusters(
                0, oneCase.utf8Len, SkSpan<const uint32_t>{oneCase.clusters}, checker);
        REPORTER_ASSERT(reporter, answerCount == oneCase.answers.size());
    }
}

DEF_TEST(SkShaperTest_VisualizeCluster, reporter) {
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
        std::string utf8String{utf1to1.data(), utf1to1.size()};
        jsonWriter.appendString("UTF", utf8String.c_str());
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
            {"A", {7}, {0}},
            {"ABCD", {7, 8, 9, 10}, {0, 1, 2, 3}},
            {"A", {7, 8}, {0, 0}},
            {"AB", {7}, {0}},
            {"AB", {7, 8, 9}, {0, 0, 0}},
            {"ABC", {7, 8}, {0, 0}},
            {"ABC", {7, 8}, {0, 0}},
            {"ABCD", {7, 8, 9, 10}, {3, 2, 1, 0}},
            {"المادة", {246, 268, 241, 205, 240}, {10, 8, 6, 2, 0}}
    };

    jsonWriter.beginObject();
    jsonWriter.beginArray("testcases");
    for (auto& oneCase : cases) {
        jsonWriter.beginObject();
        SkShaperJSONWriter::VisualizeClusters(oneCase.utf8.c_str(),
                                              0, oneCase.utf8.size(),
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

DEF_TEST(SkShaperTest_basic, reporter) {
    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    SkFont font(nullptr, 14);

    SkDynamicMemoryWStream out;
    SkJSONWriter jsonWriter{&out, SkJSONWriter::Mode::kPretty};
    std::string s = "المادة 1 يولد جميع الناس أحرارًا متساوين في الكرامة والحقوق. وقد وهبوا "
                    "عقلاً وضميرًا وعليهم أن يعامل بعضهم بعضًا بروح الإخاء.";

    SkShaperJSONWriter shaperJSON{&jsonWriter, s.c_str(), s.size()};

    jsonWriter.beginObject();
    shaper->shape(s.c_str(), s.size(), font, true /* right to left */,  256, &shaperJSON);
    jsonWriter.endObject();
    jsonWriter.flush();

    std::string sout(out.bytesWritten(), 0);
    out.copyTo(&sout[0]);
    SkDebugf("%s", sout.c_str());
}