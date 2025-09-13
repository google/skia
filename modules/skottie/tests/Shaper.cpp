/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <initializer_list>
#include <memory>

#include "include/core/SkFontMgr.h"
#include "modules/skottie/include/TextShaper.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

using namespace skottie;

DEF_TEST(Skottie_Shaper_Clusters, r) {
    const SkString text("Foo \rbar \rBaz.");

    auto check_clusters = [](skiatest::Reporter* r, const SkString& text, Shaper::Flags flags,
                             const std::vector<size_t>& expected_clusters) {
        const Shaper::TextDesc desc = {
            ToolUtils::CreatePortableTypeface("Serif", SkFontStyle()),
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
            flags,
            nullptr,
        };
        const auto result =
                Shaper::Shape(text, desc, SkRect::MakeWH(1000, 1000), ToolUtils::TestFontMgr(),
                    SkShapers::BestAvailable());
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

DEF_TEST(Skottie_Shaper_HAlign, reporter) {
    sk_sp<SkTypeface> typeface = ToolUtils::DefaultTypeface();
    REPORTER_ASSERT(reporter, typeface);

    static constexpr struct {
        SkScalar text_size,
                 tolerance;
    } kTestSizes[] = {
        // These gross tolerances are required for the test to pass on NativeFonts bots.
        // Might be worth investigating why we need so much slack.
        {  5, 2.0f },
        { 10, 2.0f },
        { 15, 2.4f },
        { 25, 4.4f },
    };

    static constexpr struct {
        SkTextUtils::Align align;
        SkScalar           l_selector,
                           r_selector;
    } kTestAligns[] = {
        { SkTextUtils::  kLeft_Align, 0.0f, 1.0f },
        { SkTextUtils::kCenter_Align, 0.5f, 0.5f },
        { SkTextUtils:: kRight_Align, 1.0f, 0.0f },
    };

    const SkString text("Foo, bar.\rBaz.");
    const SkPoint  text_point = SkPoint::Make(100, 100);

    for (const auto& tsize : kTestSizes) {
        for (const auto& talign : kTestAligns) {
            const skottie::Shaper::TextDesc desc = {
                typeface,
                tsize.text_size,
                0, tsize.text_size,
                tsize.text_size,
                0,
                0,
                talign.align,
                Shaper::VAlign::kTopBaseline,
                Shaper::ResizePolicy::kNone,
                Shaper::LinebreakPolicy::kExplicit,
                Shaper::Direction::kLTR,
                Shaper::Capitalization::kNone,
                0,
                0,
                nullptr
            };

            const auto shape_result =
                    Shaper::Shape(text, desc, text_point, ToolUtils::TestFontMgr(), SkShapers::BestAvailable());
            REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
            REPORTER_ASSERT(reporter, !shape_result.fFragments[0].fGlyphs.fRuns.empty());

            const auto shape_bounds = shape_result.computeVisualBounds();
            REPORTER_ASSERT(reporter, !shape_bounds.isEmpty());

            const auto expected_l = text_point.x() - shape_bounds.width() * talign.l_selector;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.left() - expected_l) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.left(), expected_l, tsize.tolerance,
                                              tsize.text_size, talign.align);

            const auto expected_r = text_point.x() + shape_bounds.width() * talign.r_selector;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.right() - expected_r) < tsize.tolerance,
                            "%f %f %f %f %d", shape_bounds.right(), expected_r, tsize.tolerance,
                                              tsize.text_size, talign.align);

        }
    }
}

DEF_TEST(Skottie_Shaper_VAlign, reporter) {
    sk_sp<SkTypeface> typeface = ToolUtils::DefaultTypeface();
    REPORTER_ASSERT(reporter, typeface);

    static constexpr struct {
        SkScalar text_size,
                 tolerance;
    } kTestSizes[] = {
        // These gross tolerances are required for the test to pass on NativeFonts bots.
        // Might be worth investigating why we need so much slack.
        {  5, 2.0f },
        { 10, 4.0f },
        { 15, 5.5f },
        { 25, 8.0f },
    };

    struct {
        skottie::Shaper::VAlign align;
        SkScalar                topFactor;
    } kTestAligns[] = {
        { skottie::Shaper::VAlign::kHybridTop   , 0.0f },
        { skottie::Shaper::VAlign::kHybridCenter, 0.5f },
        // TODO: any way to test kTopBaseline?
    };

    const SkString text("Foo, bar.\rBaz.");
    const auto text_box = SkRect::MakeXYWH(100, 100, 1000, 1000); // large-enough to avoid breaks.


    for (const auto& tsize : kTestSizes) {
        for (const auto& talign : kTestAligns) {
            const skottie::Shaper::TextDesc desc = {
                typeface,
                tsize.text_size,
                0, tsize.text_size,
                tsize.text_size,
                0,
                0,
                SkTextUtils::Align::kCenter_Align,
                talign.align,
                Shaper::ResizePolicy::kNone,
                Shaper::LinebreakPolicy::kParagraph,
                Shaper::Direction::kLTR,
                Shaper::Capitalization::kNone,
                0,
                0,
                nullptr
            };

            const auto shape_result = Shaper::Shape(text, desc, text_box, ToolUtils::TestFontMgr(), SkShapers::BestAvailable());
            REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
            REPORTER_ASSERT(reporter, !shape_result.fFragments[0].fGlyphs.fRuns.empty());

            const auto shape_bounds = shape_result.computeVisualBounds();
            REPORTER_ASSERT(reporter, !shape_bounds.isEmpty());

            const auto v_diff = text_box.height() - shape_bounds.height();

            const auto expected_t = text_box.top() + v_diff * talign.topFactor;
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.top() - expected_t) < tsize.tolerance,
                            "%f %f %f %f %u", shape_bounds.top(), expected_t, tsize.tolerance,
                                              tsize.text_size, SkToU32(talign.align));

            const auto expected_b = text_box.bottom() - v_diff * (1 - talign.topFactor);
            REPORTER_ASSERT(reporter,
                            std::fabs(shape_bounds.bottom() - expected_b) < tsize.tolerance,
                            "%f %f %f %f %u", shape_bounds.bottom(), expected_b, tsize.tolerance,
                                              tsize.text_size, SkToU32(talign.align));
        }
    }
}

DEF_TEST(Skottie_Shaper_FragmentGlyphs, reporter) {
    skottie::Shaper::TextDesc desc = {
        ToolUtils::DefaultTypeface(),
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
        0,
        nullptr
    };

    const SkString text("Foo bar baz");
    const auto text_box = SkRect::MakeWH(100, 100);

    {
        const auto shape_result = Shaper::Shape(text, desc, text_box, ToolUtils::TestFontMgr(), SkShapers::BestAvailable());
        // Default/consolidated mode => single blob result.
        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        SkASSERT(!shape_result.fFragments.empty());
        REPORTER_ASSERT(reporter, !shape_result.fFragments[0].fGlyphs.fRuns.empty());
    }

    {
        desc.fFlags = Shaper::Flags::kFragmentGlyphs;
        const auto shape_result =
                skottie::Shaper::Shape(text, desc, text_box, ToolUtils::TestFontMgr(), SkShapers::BestAvailable());
        // Fragmented mode => one blob per glyph.
        const size_t expectedSize = text.size();
        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == expectedSize);
        SkASSERT(!shape_result.fFragments.empty());
        for (size_t i = 0; i < expectedSize; ++i) {
            REPORTER_ASSERT(reporter, !shape_result.fFragments[i].fGlyphs.fRuns.empty());
        }
    }
}

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && !defined(SK_BUILD_FOR_WIN)

DEF_TEST(Skottie_Shaper_ExplicitFontMgr, reporter) {
    class CountingFontMgr : public SkFontMgr {
    public:
        size_t fallbackCount() const { return fFallbackCount; }

    protected:
        int onCountFamilies() const override { return 0; }
        void onGetFamilyName(int index, SkString* familyName) const override {
            SkDEBUGFAIL("onGetFamilyName called with bad index");
        }
        sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
            SkDEBUGFAIL("onCreateStyleSet called with bad index");
            return nullptr;
        }
        sk_sp<SkFontStyleSet> onMatchFamily(const char[]) const override {
            return SkFontStyleSet::CreateEmpty();
        }

        sk_sp<SkTypeface> onMatchFamilyStyle(const char[], const SkFontStyle&) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                      const SkFontStyle& style,
                                                      const char* bcp47[],
                                                      int bcp47Count,
                                                      SkUnichar character) const override {
            fFallbackCount++;
            return nullptr;
        }

        sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                               const SkFontArguments&) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onMakeFromFile(const char[], int) const override {
            return nullptr;
        }
        sk_sp<SkTypeface> onLegacyMakeTypeface(const char [], SkFontStyle) const override {
            return nullptr;
        }
    private:
        mutable size_t fFallbackCount = 0;
    };

    auto fontmgr = sk_make_sp<CountingFontMgr>();

    skottie::Shaper::TextDesc desc = {
        ToolUtils::DefaultPortableTypeface(),
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
        0,
        nullptr
    };

    const auto text_box = SkRect::MakeWH(100, 100);

    {
        const auto shape_result = Shaper::Shape(SkString("foo bar"), desc, text_box, fontmgr, SkShapers::BestAvailable());

        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        REPORTER_ASSERT(reporter, !shape_result.fFragments[0].fGlyphs.fRuns.empty());
        REPORTER_ASSERT(reporter, fontmgr->fallbackCount() == 0ul);
        REPORTER_ASSERT(reporter, shape_result.fMissingGlyphCount == 0);
    }

    {
        // An unassigned codepoint should trigger fallback.
        const auto shape_result = skottie::Shaper::Shape(SkString("foo\U000DFFFFbar"),
                                                         desc, text_box, fontmgr, SkShapers::BestAvailable());

        REPORTER_ASSERT(reporter, shape_result.fFragments.size() == 1ul);
        REPORTER_ASSERT(reporter, !shape_result.fFragments[0].fGlyphs.fRuns.empty());
        REPORTER_ASSERT(reporter, fontmgr->fallbackCount() == 1ul);
        REPORTER_ASSERT(reporter, shape_result.fMissingGlyphCount == 1ul);
    }
}

namespace skottie {
std::unique_ptr<SkBreakIterator> MakeIntersectingBreakIteratorForTesting(
    std::unique_ptr<SkBreakIterator>, std::unique_ptr<SkBreakIterator>);
}

DEF_TEST(Skottie_Shaper_breakiter, r) {
    class MockBreakIterator final : public SkBreakIterator {
    public:
        explicit MockBreakIterator(std::vector<Position> brks) : fBreaks(std::move(brks)) {}

        Position first() override {
            this->reset();
            return fCurrent;
        }

        Position current() override {
            return fCurrent;
        }
        Position next() override {
            if (fIndex < fBreaks.size()) {
                fCurrent = fBreaks[fIndex++];
            } else {
                fCurrent = -1;
                fDone = true;
            }
            return fCurrent;
        }
        Status status() override { return 0; }
        bool isDone() override { return fDone; }
        bool setText(const char[], int) override {
            this->reset();
            return true;
        }
        bool setText(const char16_t[], int) override {
            this->reset();
            return true;
        }

    private:
        void reset() {
            fCurrent = 0;
            fIndex = 0;
            fDone = false;
        }

        const std::vector<Position> fBreaks;
        Position                    fCurrent = 0;
        bool                        fDone    = false;
        size_t                      fIndex   = 0;
    };

    auto do_do_check = [&](const std::unique_ptr<SkBreakIterator>& it,
                           std::initializer_list<SkBreakIterator::Position> expected) {
        // Initial state.
        REPORTER_ASSERT(r, it);
        REPORTER_ASSERT(r, !it->isDone());
        REPORTER_ASSERT(r, it->current() == 0);

        for (const auto& pos : expected) {
            REPORTER_ASSERT(r, it->next() == pos);
            REPORTER_ASSERT(r, it->current() == pos);
            REPORTER_ASSERT(r, !it->isDone());
        }

        // Final state.
        REPORTER_ASSERT(r, it->next() < 0);
        REPORTER_ASSERT(r, it->current() < 0);
        REPORTER_ASSERT(r, it->isDone());

        // One more time for good measure.
        REPORTER_ASSERT(r, it->next() < 0);
        REPORTER_ASSERT(r, it->current() < 0);
        REPORTER_ASSERT(r, it->isDone());
    };

    auto do_check = [&](std::initializer_list<SkBreakIterator::Position> a,
                        std::initializer_list<SkBreakIterator::Position> b,
                        std::initializer_list<SkBreakIterator::Position> expected) {
        auto it = MakeIntersectingBreakIteratorForTesting(
            std::make_unique<MockBreakIterator>(std::vector(a)),
            std::make_unique<MockBreakIterator>(std::vector(b)));

        do_do_check(it, expected);

        // first() resets state.
        REPORTER_ASSERT(r, it->first() == 0);
        do_do_check(it, expected);

        // as does setText()
        REPORTER_ASSERT(r, it->setText("foo", 3));
        do_do_check(it, expected);
    };

    auto check = [&](std::initializer_list<SkBreakIterator::Position> a,
                     std::initializer_list<SkBreakIterator::Position> b,
                     std::initializer_list<SkBreakIterator::Position> expected) {
        // Order should not matter.
        do_check(a, b, expected);
        do_check(b, a, expected);
    };

    check({  }, {  },   {  });
    check({42}, {  },   {  });
    check({42}, {43},   {  });
    check({42}, {42},   {42});

    check({1, 3   }, {1, 3},   {1, 3});
    check({1, 3, 5}, {1, 3},   {1, 3});
    check({1, 3, 5}, {3, 5},   {3, 5});
    check({1, 3, 5}, {1, 5},   {1, 5});

    check({1, 3, 5, 7}, {2, 4, 6, 8},   {       });
    check({1, 3, 5, 7}, {1         },   {1      });
    check({1, 3, 5, 7}, {1, 5      },   {1, 5   });
    check({1, 3, 5, 7}, {3, 7      },   {3, 7   });
    check({1, 3, 5, 7}, {1, 7      },   {1, 7   });
    check({1, 3, 5, 7}, {1, 3, 7   },   {1, 3, 7});
    check({1, 3, 5, 7}, {1, 5, 7   },   {1, 5, 7});

    check({1, 5, 9}, {2, 3, 4   },   {       });
    check({1, 5, 9}, {6, 7, 8   },   {       });
    check({1, 5, 9}, {2, 3, 7, 8},   {       });
    check({1, 5, 9}, {1, 2, 3   },   {1      });
    check({1, 5, 9}, {7, 8, 9   },   {9      });
    check({1, 5, 9}, {2, 3, 5, 7},   {5      });
    check({1, 5, 9}, {4, 5, 7, 8},   {5      });
    check({1, 5, 9}, {1, 2, 3, 5},   {1, 5   });
    check({1, 5, 9}, {1, 3, 5, 7},   {1, 5   });
    check({1, 5, 9}, {5, 7, 8, 9},   {5, 9   });
    check({1, 5, 9}, {3, 5, 7, 9},   {5, 9   });
    check({1, 5, 9}, {1, 3, 5, 9},   {1, 5, 9});
    check({1, 5, 9}, {1, 5, 7, 9},   {1, 5, 9});
}

#endif

