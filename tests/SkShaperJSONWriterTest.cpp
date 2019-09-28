/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkShaperJSONWriter.h"

#include "tests/Test.h"

#include "src/core/SkSpan.h"
#include "src/utils/SkJSONWriter.h"
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
                0, oneCase.utf8Len, SkMakeSpan(oneCase.clusters), checker);
        REPORTER_ASSERT(reporter, answerCount == oneCase.answers.size());
    }
}

DEF_TEST(SkShaperTest_VisualizeCluster, reporter) {

    struct Answer {
        std::string utf8;
        std::vector<SkGlyphID> glyphIDs;
    };
    struct TestCase {
        std::string utf8;
        std::vector<SkGlyphID> glyphIDs;
        std::vector<uint32_t> clusters;
        std::vector<Answer> answers;
    };

    std::vector<TestCase> cases = {
            { "A", {7}, {0}, {{"A", {7}}} },
            { "ABCD", {7, 8, 9, 10}, {0, 1, 2, 3}, {{"ABCD", {7, 8, 9, 10}}} },
            { "A", {7, 8}, {0, 0}, {{"A", {7, 8}}} },
            { "AB", {7}, {0}, {{"AB", {7}}} },
            { "AB", {7, 8, 9}, {0, 0, 0}, {{"AB", {7, 8, 9}}} },
            { "ABC", {7, 8}, {0, 0}, {{"ABC", {7, 8}}} },
            { "ABCD", {7, 8, 9, 10}, {3, 2, 1, 0}, {{"ABCD", {7, 8, 9, 10}}} },
            { "المادة", {246, 268, 241, 205, 240}, {10, 8, 6, 2, 0},
                        {{"ادة",  {246, 268, 241}}, {"لم", {205}}, {"ا", {240}}} },
    };

    for (auto& oneCase : cases) {
        size_t answerCount = 0;
        auto checker = [&](
                int codePointCount, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
            if (answerCount < oneCase.answers.size()) {
                Answer a = oneCase.answers[answerCount];
                std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                REPORTER_ASSERT(reporter, a.utf8 == toCheckUtf8);
                std::vector<SkGlyphID> toCheckGlyphIDs{glyph1to1.begin(), glyph1to1.end()};
                REPORTER_ASSERT(reporter, a.glyphIDs == toCheckGlyphIDs);

            } else {
                REPORTER_ASSERT(reporter, false, "Too many clusters");
            }
            answerCount++;
        };

        SkShaperJSONWriter::VisualizeClusters(oneCase.utf8.c_str(),
                                              0, oneCase.utf8.size(),
                                              SkMakeSpan(oneCase.glyphIDs),
                                              SkMakeSpan(oneCase.clusters),
                                              checker);
    }
}

// Example use of the SkShaperJSONWriter.
// Set to 1 to see use.
#if 0
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
    // Uncomment below to show the JSON.
    SkDebugf("%s", sout.c_str());
}
#endif
