/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "src/base/SkZip.h"
#include "src/utils/SkFloatUtils.h"
#include "tests/Test.h"

#include <stdarg.h>
#include <string>
#include <vector>

static void SkMaybeDebugf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);

static void SkMaybeDebugf(const char* format, ...) {
    if ((false)) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

DEF_TEST(TypefaceMacVariation, reporter) {
    auto makeSystemFont = [](float size) -> CTFontRef {
        // kCTFontUIFontSystem, kCTFontUIFontMessage
        return CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr);
    };

    auto tagToString = [](SkFourByteTag tag) -> std::string {
      char buffer[5];
      buffer[0] = (tag & 0xff000000) >> 24;
      buffer[1] = (tag & 0xff0000) >> 16;
      buffer[2] = (tag & 0xff00) >> 8;
      buffer[3] = tag & 0xff;
      buffer[4] = 0;
      return std::string(buffer);
    };

    // This typeface has the issue.
    sk_sp<SkTypeface> typeface(SkMakeTypefaceFromCTFont(makeSystemFont(30)));

    // Since MakeFromFile creates at default size 12, these two are more comparable.
    // The first one has the issue and the second does not.
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(12));
    //typeface = SkTypeface::MakeFromFile("/System/Library/Fonts/SFNS.ttf");

    // Setting the initial opsz <= min, the reported wght axis is strange, but draws the same?
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(17));
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(17.01));

    // Setting the initial opsz >= max, the requested variation doesn't take effect?
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(95.9));
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(96));

    if (!typeface) {
        REPORTER_ASSERT(reporter, typeface);
        return;
    }
    using Coordinate = SkFontArguments::VariationPosition::Coordinate;
    using Axis = SkFontParameters::Variation::Axis;

    const int originalPositionCount = typeface->getVariationDesignPosition({});
    std::vector<Coordinate> originalPosition(originalPositionCount);
    const int retrievedOriginalPositionCount =
        typeface->getVariationDesignPosition(originalPosition);
    if (!(retrievedOriginalPositionCount == originalPositionCount)) {
        REPORTER_ASSERT(reporter, retrievedOriginalPositionCount == originalPositionCount);
        return;
    }

    constexpr SkFourByteTag kGRADTag = SkSetFourByteTag('G', 'R', 'A', 'D');
    constexpr SkFourByteTag kWghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
    constexpr SkFourByteTag kWdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
    constexpr SkFourByteTag kOpszTag = SkSetFourByteTag('o', 'p', 's', 'z');

    SkMaybeDebugf("Original: ");
    for (auto& originalCoordinate : originalPosition) {
        SkMaybeDebugf("(%s: %f) ", tagToString(originalCoordinate.axis).c_str(),
                                   originalCoordinate.value);
    }
    SkMaybeDebugf("\n\n");

    const int originalAxisCount = typeface->getVariationDesignParameters({});
    std::vector<Axis> originalAxes(originalAxisCount);
    const int returnedOriginalAxisCount =
        typeface->getVariationDesignParameters(originalAxes);
    if (!(returnedOriginalAxisCount == originalAxisCount)) {
        REPORTER_ASSERT(reporter, returnedOriginalAxisCount == originalAxisCount);
        return;
    }

    for (bool omitOpsz : {false, true}) {
    for (SkFourByteTag axisToBump : { 0u, kOpszTag, kWdthTag, kGRADTag }) {
    for (float testCoordinate : {100, 300, 400, 500, 700, 900}) {
        std::vector<Coordinate> expectedPosition;
        std::vector<Coordinate> requestPosition;
        SkMaybeDebugf("Request : ");
        for (auto& originalCoordinate : originalPosition) {
            float requestValue = originalCoordinate.value;
            if (originalCoordinate.axis == kOpszTag && omitOpsz) {
                SkMaybeDebugf("#%s: %f# ", tagToString(originalCoordinate.axis).c_str(),
                                           requestValue);
            } else {
                if (originalCoordinate.axis == axisToBump) {
                    // CoreText floats for the variation coordinates have limited precision.
                    // 'opsz' sems to have more precision since it is set somewhat independently.
                    // Though in mocOS 14 this seems to have changed and it can be more rounded.
                    //requestValue = nextafter(requestValue, HUGE_VALF); // Does not work.
                    requestValue += requestValue / 1024.0f; // Expect at least 10 bits.
                }
                if (originalCoordinate.axis == kWghtTag) {
                    requestValue = testCoordinate;
                }
                SkMaybeDebugf("(%s: %f) ", tagToString(originalCoordinate.axis).c_str(),
                                           requestValue);
                requestPosition.push_back({originalCoordinate.axis, requestValue});
            }

            float expectedValue = requestValue;
            for (auto& originalAxis : originalAxes) {
                if (originalAxis.tag == originalCoordinate.axis) {
                    expectedValue = std::min(expectedValue, originalAxis.max);
                    expectedValue = std::max(expectedValue, originalAxis.min);
                }
            }
            expectedPosition.push_back({originalCoordinate.axis, expectedValue});
        }
        SkMaybeDebugf("\n");

        SkMaybeDebugf("Expected: ");
        for (auto& expectedCoordinate : expectedPosition) {
             SkMaybeDebugf("(%s: %f) ", tagToString(expectedCoordinate.axis).c_str(),
                                        expectedCoordinate.value);
        }
        SkMaybeDebugf("\n");

        SkFontArguments::VariationPosition variationPosition =
            { requestPosition.data(), (int)requestPosition.size() };
        sk_sp<SkTypeface> cloneTypeface(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(variationPosition)));

        const int cloneAxisCount = cloneTypeface->getVariationDesignPosition({});
        std::vector<Coordinate> clonePosition(cloneAxisCount);
        const int retrievedCloneAxisCount =
            cloneTypeface->getVariationDesignPosition(clonePosition);
        if (!(retrievedCloneAxisCount == cloneAxisCount)) {
            REPORTER_ASSERT(reporter, retrievedCloneAxisCount == cloneAxisCount);
            continue;
        }

        SkMaybeDebugf("Result  : ");
        for (auto& cloneCoordinate : clonePosition) {
             SkMaybeDebugf("(%s: %f) ", tagToString(cloneCoordinate.axis).c_str(),
                                        cloneCoordinate.value);
        }
        SkMaybeDebugf("\n");

        if (clonePosition.size() != expectedPosition.size()) {
            REPORTER_ASSERT(reporter, clonePosition.size() == expectedPosition.size());
            continue;
        }

        auto compareCoordinate = [](const Coordinate& a, const Coordinate& b) -> bool {
            return a.axis < b.axis;
        };
        std::sort(clonePosition.begin(), clonePosition.end(), compareCoordinate);
        std::sort(expectedPosition.begin(), expectedPosition.end(), compareCoordinate);
        for (const auto&& [clone, expected] : SkMakeZip(clonePosition, expectedPosition)) {
            REPORTER_ASSERT(reporter, clone.axis == expected.axis, "%s == %s",
                            tagToString(clone.axis).c_str(), tagToString(expected.axis).c_str());

            // Allow a lot of slop here, ignoring the bottom 6 of the 23 bits.
            // CoreText appears to round `opsz` a lot starting in macOS 14.
            const SkFloatingPoint<float, 64> cloneValue(clone.value), expectedValue(expected.value);
            REPORTER_ASSERT(reporter, cloneValue.AlmostEquals(expectedValue), "%s:%f == %s:%f",
                            tagToString(clone.axis).c_str(), clone.value,
                            tagToString(expected.axis).c_str(), expected.value);
        }

        SkMaybeDebugf("\n");
    }
    }
    }
}
