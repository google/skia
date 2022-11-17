/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "modules/skottie/src/text/SkottieShaper.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

using namespace skottie;

DEF_TEST(Skottie_Shaper_Clusters, r) {
    const SkString text("Foo \rbar \rBaz.");

    auto check_clusters = [](skiatest::Reporter* r, const SkString& text, Shaper::Flags flags,
                             const std::vector<size_t>& expected_clusters) {
        const Shaper::TextDesc desc = {
            ToolUtils::create_portable_typeface("Serif", SkFontStyle()),
            18,
            0, 18,
            18,
             0,
             0,
            SkTextUtils::Align::kCenter_Align,
            Shaper::VAlign::kTop,
            Shaper::ResizePolicy::kNone,
            Shaper::LinebreakPolicy::kParagraph,
            Shaper::Direction::kLTR,
            Shaper::Capitalization::kNone,
            0,
            flags
        };
        const auto result = Shaper::Shape(text, desc, SkRect::MakeWH(1000, 1000),
                                          SkFontMgr::RefDefault());
        REPORTER_ASSERT(r, !result.fFragments.empty());

        size_t i = 0;
        for (const auto& frag : result.fFragments) {
            const auto& glyphs = frag.fGlyphs;

            if (flags & Shaper::kClusters) {
                REPORTER_ASSERT(r, glyphs.fClusters.size() == glyphs.fGlyphIDs.size());
            }

            for (const auto& utf_cluster : glyphs.fClusters) {
                REPORTER_ASSERT(r, i < expected_clusters.size());
                REPORTER_ASSERT(r, utf_cluster == expected_clusters[i++]);
            }
        }

        REPORTER_ASSERT(r, i == expected_clusters.size());
    };

    check_clusters(r, text, Shaper::kNone, {});
    check_clusters(r, text, Shaper::kFragmentGlyphs, {});
    check_clusters(r, text, Shaper::kClusters,
                   {0, 1, 2, 3,    5, 6, 7, 8,    10, 11, 12, 13});
    check_clusters(r, text, (Shaper::Flags)(Shaper::kClusters | Shaper::kFragmentGlyphs),
                   {0, 1, 2, 3,    5, 6, 7, 8,    10, 11, 12, 13});
}
