/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>

namespace {
const char kFontResource[] = "fonts/ahem.ttf";
const char kTtcResource[] = "fonts/test.ttc";
const char kVariableResource[] = "fonts/test_glyphs-glyf_colr_1_variable.ttf";
constexpr size_t kNumVariableAxes = 44;

struct AxisExpectation {
    SkFourByteTag tag;
    float minValue;
    float defValue;
    float maxValue;
} axisExpectations[] = {
        {SkSetFourByteTag('S', 'W', 'P', 'S'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'W', 'P', 'E'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'W', 'C', '1'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '2'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '3'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '4'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'C', 'O', 'X'), -200., 0.0, 200.},
        {SkSetFourByteTag('S', 'C', 'O', 'Y'), -200., 0.0, 200.},
        {SkSetFourByteTag('S', 'C', 'S', 'X'), -2.0, 0.0, 1.9999389648437},
        {SkSetFourByteTag('S', 'C', 'S', 'Y'), -2.0, 0.0, 1.9999389648437},
        {SkSetFourByteTag('G', 'R', 'X', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'X', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'X', '2'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '2'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'R', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'R', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('C', 'O', 'L', '1'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('C', 'O', 'L', '2'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('C', 'O', 'L', '3'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('R', 'O', 'T', 'A'), 0.0, 0.0, 539.989013671875},
        {SkSetFourByteTag('R', 'O', 'T', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('R', 'O', 'T', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('S', 'K', 'X', 'A'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'K', 'Y', 'A'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'K', 'C', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('S', 'K', 'C', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'R', 'X', 'X'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'Y', 'X'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'X', 'Y'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'Y', 'Y'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'D', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'R', 'D', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'L', 'D', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'L', 'D', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'X', 'I'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'Y', 'I'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'X', 'A'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'Y', 'A'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'I', 'O'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('A', 'P', 'H', '1'), -1.0, 0.0, 0.0},
        {SkSetFourByteTag('A', 'P', 'H', '2'), -1.0, 0.0, 0.0},
        {SkSetFourByteTag('A', 'P', 'H', '3'), -1.0, 0.0, 0.0},
};
}  // namespace

DEF_TEST(Fontations_DoNotMakeFromNull, reporter) {
    std::unique_ptr<SkStreamAsset> nullStream = SkMemoryStream::MakeDirect(nullptr, 0);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(nullStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromNonSfnt, reporter) {
    char notAnSfnt[] = "I_AM_NOT_AN_SFNT";
    std::unique_ptr<SkStreamAsset> notSfntStream =
            SkMemoryStream::MakeDirect(notAnSfnt, std::size(notAnSfnt));
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(notSfntStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_MakeFromFont, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollection, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollectionNonNullIndex, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromCollection_Invalid_Index, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1000);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_TableData, reporter) {
    constexpr size_t kNameTableSize = 11310;
    constexpr size_t kTestOffset = 1310;
    constexpr size_t kTestLength = 500;
    char destBuffer[kNameTableSize] = {0};
    sk_sp<SkTypeface> testTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    SkFourByteTag nameTableTag = SkSetFourByteTag('n', 'a', 'm', 'e');
    SkFourByteTag nonExistantTag = SkSetFourByteTag('0', 'X', '0', 'X');

    // Getting size without buffer.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize, nullptr) ==
                            kNameTableSize);
    // Reading full table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize, destBuffer) ==
                            kNameTableSize);
    // Reading restricted length.
    REPORTER_ASSERT(
            reporter,
            testTypeface->getTableData(nameTableTag, 0, kTestLength, destBuffer) == kTestLength);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kTestOffset, kTestLength, destBuffer) == kTestLength);
    // Reading at an offset.
    REPORTER_ASSERT(
            reporter,
            testTypeface->getTableData(nameTableTag, kTestOffset, kNameTableSize, destBuffer) ==
                    kNameTableSize - kTestOffset);

    // Reading from offset past table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kNameTableSize, kNameTableSize, destBuffer) == 0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, kNameTableSize, 0, nullptr) == 0);
    // Reading one byte before end of table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kNameTableSize - 1, kNameTableSize, destBuffer) == 1);
    // Trying to start reading at an offset past table start.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize + 10, destBuffer) ==
                            kNameTableSize);
    // Restricting length without target buffer.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kTestLength, nullptr) ==
                            kTestLength);

    // Trying to access non-existant table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, 0, kNameTableSize, destBuffer) ==
                            0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, 0, 0, nullptr) ==
                            0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, kTestOffset, 0, nullptr) == 0);
}

DEF_TEST(Fontations_TableTags, reporter) {
    constexpr size_t kNumTags = 11;
    SkFourByteTag tagsBuffer[kNumTags] = {0};
    sk_sp<SkTypeface> testTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    SkFourByteTag firstTag = SkSetFourByteTag('O', 'S', '/', '2');
    SkFourByteTag lastTag = SkSetFourByteTag('p', 'o', 's', 't');

    REPORTER_ASSERT(reporter, testTypeface->getTableTags(nullptr) == kNumTags);

    REPORTER_ASSERT(reporter, testTypeface->getTableTags(tagsBuffer) == kNumTags);
    REPORTER_ASSERT(reporter, tagsBuffer[0] == firstTag);
    REPORTER_ASSERT(reporter, tagsBuffer[kNumTags - 1] == lastTag);
}

DEF_TEST(Fontations_VariationPosition, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    // Everything at default.
    const int numAxes = variableTypeface->getVariationDesignPosition(nullptr, 0);
    REPORTER_ASSERT(reporter, numAxes == kNumVariableAxes, "numAxes: %d", numAxes);

    SkFontArguments::VariationPosition::Coordinate kSwpsCoordinateFirst = {SkSetFourByteTag('S', 'W', 'P', 'S'), 25};
    SkFontArguments::VariationPosition::Coordinate kSwpsCoordinateSecond = {SkSetFourByteTag('S', 'W', 'P', 'S'), 55};
    SkFontArguments::VariationPosition::Coordinate kSwpeCoordinate = {SkSetFourByteTag('S', 'W', 'P', 'E'), 45};
    SkFontArguments::VariationPosition::Coordinate kInvalidCoordinate = {SkSetFourByteTag('_', '_', '_', '_'), 0};

    // 'SWPS' and 'SWPE' exist. Second 'SWPS' should override first, invalid tag should be stripped.
    SkFontArguments::VariationPosition::Coordinate cloneCoordinates[4] = {
            kSwpsCoordinateFirst, kSwpsCoordinateSecond, kSwpeCoordinate, kInvalidCoordinate};

    SkFontArguments::VariationPosition clonePosition;
    clonePosition.coordinates = cloneCoordinates;
    clonePosition.coordinateCount = 4;

    sk_sp<SkTypeface> cloneTypeface = variableTypeface->makeClone(
            SkFontArguments().setVariationDesignPosition(clonePosition));
    const int cloneNumAxes = cloneTypeface->getVariationDesignPosition(nullptr, 0);
    REPORTER_ASSERT(reporter, cloneNumAxes == kNumVariableAxes, "clonedNumAxes: %d", cloneNumAxes);

    SkFontArguments::VariationPosition::Coordinate retrieveCoordinates[kNumVariableAxes] = {};

    // Error when providing too little space.
    const int badClonedNumAxes = cloneTypeface->getVariationDesignPosition(retrieveCoordinates, 1);
    REPORTER_ASSERT(reporter, badClonedNumAxes == -1, "badClonedNumAxes: %d", badClonedNumAxes);

    const int retrievedClonedNumAxes =
            cloneTypeface->getVariationDesignPosition(retrieveCoordinates, kNumVariableAxes);
    REPORTER_ASSERT(reporter, retrievedClonedNumAxes == kNumVariableAxes,
                    "retrievedClonedNumAxes: %d", retrievedClonedNumAxes);
    REPORTER_ASSERT(reporter,
                    retrieveCoordinates[0].axis  == kSwpsCoordinateSecond.axis &&
                    retrieveCoordinates[0].value == kSwpsCoordinateSecond.value);
    REPORTER_ASSERT(reporter,
                    retrieveCoordinates[1].axis  == kSwpeCoordinate.axis &&
                    retrieveCoordinates[1].value == kSwpeCoordinate.value);
}

DEF_TEST(Fontations_VariationParameters, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(nullptr, 0) == kNumVariableAxes);

    SkFontParameters::Variation::Axis axes[kNumVariableAxes] = {};
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(axes, kNumVariableAxes) ==
                            kNumVariableAxes);

    for (size_t i = 0; i < kNumVariableAxes; ++i) {
        REPORTER_ASSERT(reporter, axes[i].tag == axisExpectations[i].tag);
        REPORTER_ASSERT(reporter, axes[i].min == axisExpectations[i].minValue);
        REPORTER_ASSERT(reporter, axes[i].def == axisExpectations[i].defValue);
        REPORTER_ASSERT(reporter, axes[i].max == axisExpectations[i].maxValue);
    }
}

DEF_TEST(Fontations_VariationParameters_BufferTooSmall, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(nullptr, 0) == kNumVariableAxes);

    constexpr size_t kArrayTooSmall = 3;
    SkFontParameters::Variation::Axis axes[kArrayTooSmall] = {};
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(axes, kArrayTooSmall) == -1);
}
