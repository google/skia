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
#include "src/core/SkZip.h"
#include "tests/Test.h"

#include <stdarg.h>
#include <string>
#include <vector>

#if 0
static void SkMaybeDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#else
static void SkMaybeDebugf(const char format[], ...) { }
#endif

DEF_TEST(FontMgrMac, reporter) {
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

    // Setting the initila opsz <= min, the reported wght axis is strange, but draws the same?
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

    const int original_position_count = typeface->getVariationDesignPosition(nullptr, 0);
    std::vector<Coordinate> original_position(original_position_count);
    const int retrieved_original_position_count =
        typeface->getVariationDesignPosition(original_position.data(), original_position.size());
    if (!(retrieved_original_position_count == original_position_count)) {
        REPORTER_ASSERT(reporter, retrieved_original_position_count == original_position_count);
        return;
    }

    constexpr SkFourByteTag kWghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
    constexpr SkFourByteTag kWdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
    constexpr SkFourByteTag kOpszTag = SkSetFourByteTag('o', 'p', 's', 'z');

    SkMaybeDebugf("Original: ");
    for (auto& original_coordinate : original_position) {
        SkMaybeDebugf("(%s: %f) ", tagToString(original_coordinate.axis).c_str(),
                                   original_coordinate.value);
    }
    SkMaybeDebugf("\n\n");

    const int original_axis_count = typeface->getVariationDesignParameters(nullptr, 0);
    std::vector<Axis> original_axes(original_axis_count);
    const int returned_original_axis_count =
        typeface->getVariationDesignParameters(original_axes.data(), original_axes.size());
    if (!(returned_original_axis_count == original_axis_count)) {
        REPORTER_ASSERT(reporter, returned_original_axis_count == original_axis_count);
        return;
    }

    for (bool omit_opsz : {false, true}) {
    for (SkFourByteTag axis_to_bump : { 0u, kOpszTag, kWdthTag }) {
    for (float test_coordinate : {100, 200, 300, 400, 500, 600, 700, 800, 900}) {
        std::vector<Coordinate> expected_position;
        std::vector<Coordinate> request_position;
        SkMaybeDebugf("Request : ");
        for (auto& original_coordinate : original_position) {
            float request_value = original_coordinate.value;
            if (original_coordinate.axis == kOpszTag && omit_opsz) {
                SkMaybeDebugf("#%s: %f# ", tagToString(original_coordinate.axis).c_str(),
                                           request_value);
            } else {
                if (original_coordinate.axis == axis_to_bump) {
                    request_value += 0.0001f;
                }
                if (original_coordinate.axis == kWghtTag) {
                    request_value = test_coordinate;
                }
                SkMaybeDebugf("(%s: %f) ", tagToString(original_coordinate.axis).c_str(),
                                           request_value);
                request_position.push_back({original_coordinate.axis, request_value});
            }

            float expected_value = request_value;
            for (auto& original_axis : original_axes) {
                if (original_axis.tag == original_coordinate.axis) {
                    expected_value = std::min(expected_value, original_axis.max);
                    expected_value = std::max(expected_value, original_axis.min);
                }
            }
            expected_position.push_back({original_coordinate.axis, expected_value});
        }
        SkMaybeDebugf("\n");

        SkMaybeDebugf("Expected: ");
        for (auto& expected_coordinate : expected_position) {
             SkMaybeDebugf("(%s: %f) ", tagToString(expected_coordinate.axis).c_str(),
                                        expected_coordinate.value);
        }
        SkMaybeDebugf("\n");

        SkFontArguments::VariationPosition variation_position =
            { request_position.data(), (int)request_position.size() };
        sk_sp<SkTypeface> cloned_typeface(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(variation_position)));

        const int cloned_axis_count = cloned_typeface->getVariationDesignPosition(nullptr, 0);
        std::vector<Coordinate> cloned_position(cloned_axis_count);
        const int retrieved_cloned_axis_count =
            cloned_typeface->getVariationDesignPosition(cloned_position.data(), cloned_position.size());
        if (!(retrieved_cloned_axis_count == cloned_axis_count)) {
            REPORTER_ASSERT(reporter, retrieved_cloned_axis_count == cloned_axis_count);
            continue;
        }

        SkMaybeDebugf("Result  : ");
        for (auto& cloned_coordinate : cloned_position) {
             SkMaybeDebugf("(%s: %f) ", tagToString(cloned_coordinate.axis).c_str(),
                                        cloned_coordinate.value);
        }
        SkMaybeDebugf("\n");

        if (cloned_position.size() != expected_position.size()) {
            REPORTER_ASSERT(reporter, cloned_position.size() == expected_position.size());
            continue;
        }

        auto compareCoordinate = [](const Coordinate& a, const Coordinate& b) -> bool {
            return a.axis < b.axis;
        };
        std::sort(cloned_position.begin(), cloned_position.end(), compareCoordinate);
        std::sort(expected_position.begin(), expected_position.end(), compareCoordinate);
        for (const auto&& [cloned, expected] : SkMakeZip(cloned_position, expected_position)) {
            REPORTER_ASSERT(reporter, cloned.axis == expected.axis);
            REPORTER_ASSERT(reporter, cloned.value == expected.value);
        }

        SkMaybeDebugf("\n");
    }
    }
    }
}
