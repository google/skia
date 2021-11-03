/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkScalerContext.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"

#include <initializer_list>
#include <limits>
#include <vector>

DEFINE_bool(verboseFontMgr, false, "FontMgr will be very verbose.");

DEF_TEST(FontMgr_Font, reporter) {
    SkFont font(nullptr, 24);

    //REPORTER_ASSERT(reporter, SkTypeface::GetDefaultTypeface() == font.getTypeface());
    REPORTER_ASSERT(reporter, 24 == font.getSize());
    REPORTER_ASSERT(reporter, 1 == font.getScaleX());
    REPORTER_ASSERT(reporter, 0 == font.getSkewX());

    uint16_t glyphs[5];
    sk_bzero(glyphs, sizeof(glyphs));

    // Check that no glyphs are copied with insufficient storage.
    int count = font.textToGlyphs("Hello", 5, SkTextEncoding::kUTF8, glyphs, 2);
    REPORTER_ASSERT(reporter, 5 == count);
    for (const auto glyph : glyphs) { REPORTER_ASSERT(reporter, glyph == 0); }

    SkAssertResult(font.textToGlyphs("Hello", 5, SkTextEncoding::kUTF8, glyphs,
                                     SK_ARRAY_COUNT(glyphs)) == count);

    for (int i = 0; i < count; ++i) {
        REPORTER_ASSERT(reporter, 0 != glyphs[i]);
    }
    REPORTER_ASSERT(reporter, glyphs[0] != glyphs[1]); // 'h' != 'e'
    REPORTER_ASSERT(reporter, glyphs[2] == glyphs[3]); // 'l' == 'l'

    const SkFont newFont(font.makeWithSize(36));
    REPORTER_ASSERT(reporter, font.getTypefaceOrDefault() == newFont.getTypefaceOrDefault());
    REPORTER_ASSERT(reporter, 36 == newFont.getSize());   // double check we haven't changed
    REPORTER_ASSERT(reporter, 24 == font.getSize());   // double check we haven't changed
}

/*
 *  If the font backend is going to "alias" some font names to other fonts
 *  (e.g. sans -> Arial) then we want to at least get the same typeface back
 *  if we request the alias name multiple times.
 */
DEF_TEST(FontMgr_AliasNames, reporter) {
    const char* inNames[] = {
        "sans", "sans-serif", "serif", "monospace", "times", "helvetica"
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(inNames); ++i) {
        sk_sp<SkTypeface> first(SkTypeface::MakeFromName(inNames[i], SkFontStyle()));
        if (nullptr == first.get()) {
            continue;
        }
        SkString firstName;
        first->getFamilyName(&firstName);
        for (int j = 0; j < 10; ++j) {
            sk_sp<SkTypeface> face(SkTypeface::MakeFromName(inNames[i], SkFontStyle()));

            SkString name;
            face->getFamilyName(&name);
            REPORTER_ASSERT(reporter, first->uniqueID() == face->uniqueID(),
                "Request \"%s\" First Name: \"%s\" Id: %x Received Name \"%s\" Id %x",
                inNames[i], firstName.c_str(), first->uniqueID(), name.c_str(), face->uniqueID());
        }
    }
}

DEF_TEST(FontMgr_Iter, reporter) {
    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    int count = fm->countFamilies();

    for (int i = 0; i < count; ++i) {
        SkString fname;
        fm->getFamilyName(i, &fname);

        sk_sp<SkFontStyleSet> fnset(fm->matchFamily(fname.c_str()));
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        REPORTER_ASSERT(reporter, fnset->count() == set->count());

        if (FLAGS_verboseFontMgr) {
            SkDebugf("[%2d] %s\n", i, fname.c_str());
        }

        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);
//            REPORTER_ASSERT(reporter, sname.size() > 0);

            sk_sp<SkTypeface> face(set->createTypeface(j));
//            REPORTER_ASSERT(reporter, face.get());

            if (FLAGS_verboseFontMgr) {
                SkDebugf("\t[%d] %s [%3d %d %d]\n", j, sname.c_str(),
                         fs.weight(), fs.width(), fs.slant());
            }
        }
    }
}

DEF_TEST(FontMgr_Match, reporter) {
    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    sk_sp<SkFontStyleSet> styleSet(fm->matchFamily(nullptr));
    REPORTER_ASSERT(reporter, styleSet);
}

DEF_TEST(FontMgr_MatchStyleCSS3, reporter) {
    static const SkFontStyle invalidFontStyle(101, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);

    class TestTypeface : public SkTypeface {
    public:
        TestTypeface(const SkFontStyle& fontStyle) : SkTypeface(fontStyle, false){}
    protected:
        std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
        sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
            return sk_ref_sp(this);
        }
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
            const SkScalerContextEffects& effects, const SkDescriptor* desc) const override
        {
            return SkScalerContext::MakeEmpty(
                    sk_ref_sp(const_cast<TestTypeface*>(this)), effects, desc);
        }
        void onFilterRec(SkScalerContextRec*) const override { }
        std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
            return nullptr;
        }
        void onGetFontDescriptor(SkFontDescriptor*, bool*) const override { }
        void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
            sk_bzero(glyphs, count * sizeof(glyphs[0]));
        }
        int onCountGlyphs() const override { return 0; }
        void getPostScriptGlyphNames(SkString*) const override {}
        void getGlyphToUnicodeMap(SkUnichar*) const override {}
        int onGetUPEM() const override { return 0; }
        class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
        public:
            bool next(SkTypeface::LocalizedString*) override { return false; }
        };
        void onGetFamilyName(SkString* familyName) const override {
            familyName->reset();
        }
        bool onGetPostScriptName(SkString*) const override { return false; }
        SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
            return new EmptyLocalizedStrings;
        }
        bool onGlyphMaskNeedsCurrentColor() const override { return false; }
        int onGetVariationDesignPosition(
                SkFontArguments::VariationPosition::Coordinate coordinates[],
                int coordinateCount) const override
        {
            return 0;
        }
        int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                           int parameterCount) const override
        {
            return -1;
        }
        int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
        size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override {
            return 0;
        }
    };

    class TestFontStyleSet : public SkFontStyleSet {
    public:
        TestFontStyleSet(std::initializer_list<SkFontStyle> styles) : fStyles(styles) {}
        int count() override { return static_cast<int>(fStyles.size()); }
        void getStyle(int index, SkFontStyle* style, SkString*) override {
            if (style) {
                *style = fStyles[index];
            }
        }
        SkTypeface* createTypeface(int index) override {
            if (index < 0 || this->count() <= index) {
                return new TestTypeface(invalidFontStyle);
            }
            return new TestTypeface(fStyles[index]);
        }
        SkTypeface* matchStyle(const SkFontStyle& pattern) override {
            return this->matchStyleCSS3(pattern);
        }
    private:
        std::vector<SkFontStyle> fStyles;
    };

    SkFontStyle condensed_normal_100(SkFontStyle::kThin_Weight,  SkFontStyle::kCondensed_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle condensed_normal_900(SkFontStyle::kBlack_Weight, SkFontStyle::kCondensed_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle condensed_italic_100(SkFontStyle::kThin_Weight,  SkFontStyle::kCondensed_Width, SkFontStyle::kItalic_Slant);
    SkFontStyle condensed_italic_900(SkFontStyle::kBlack_Weight, SkFontStyle::kCondensed_Width, SkFontStyle::kItalic_Slant);
    SkFontStyle condensed_obliqu_100(SkFontStyle::kThin_Weight,  SkFontStyle::kCondensed_Width, SkFontStyle::kOblique_Slant);
    SkFontStyle condensed_obliqu_900(SkFontStyle::kBlack_Weight, SkFontStyle::kCondensed_Width, SkFontStyle::kOblique_Slant);
    SkFontStyle  expanded_normal_100(SkFontStyle::kThin_Weight,  SkFontStyle::kExpanded_Width,  SkFontStyle::kUpright_Slant);
    SkFontStyle  expanded_normal_900(SkFontStyle::kBlack_Weight, SkFontStyle::kExpanded_Width,  SkFontStyle::kUpright_Slant);
    SkFontStyle  expanded_italic_100(SkFontStyle::kThin_Weight,  SkFontStyle::kExpanded_Width,  SkFontStyle::kItalic_Slant);
    SkFontStyle  expanded_italic_900(SkFontStyle::kBlack_Weight, SkFontStyle::kExpanded_Width,  SkFontStyle::kItalic_Slant);
    SkFontStyle  expanded_obliqu_100(SkFontStyle::kThin_Weight,  SkFontStyle::kExpanded_Width,  SkFontStyle::kOblique_Slant);
    SkFontStyle  expanded_obliqu_900(SkFontStyle::kBlack_Weight, SkFontStyle::kExpanded_Width,  SkFontStyle::kOblique_Slant);

    SkFontStyle normal_normal_100(SkFontStyle::kThin_Weight,       SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle normal_normal_300(SkFontStyle::kLight_Weight,      SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle normal_normal_400(SkFontStyle::kNormal_Weight,     SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle normal_normal_500(SkFontStyle::kMedium_Weight,     SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle normal_normal_600(SkFontStyle::kSemiBold_Weight,   SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    SkFontStyle normal_normal_900(SkFontStyle::kBlack_Weight,      SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);

    struct StyleSetTest {
        TestFontStyleSet styleSet;
        struct Case {
            SkFontStyle pattern;
            SkFontStyle expectedResult;
        };
        std::vector<Case> cases;
    } tests[] = {
        {
            { normal_normal_500, normal_normal_400 },
            {
                { normal_normal_400, normal_normal_400 },
                { normal_normal_500, normal_normal_500 },
            },
        },

        {
            { normal_normal_500, normal_normal_300 },
            {
                { normal_normal_300, normal_normal_300 },
                { normal_normal_400, normal_normal_500 },
                { normal_normal_500, normal_normal_500 },
            },
        },

        {
            { condensed_normal_100,condensed_normal_900,condensed_italic_100,condensed_italic_900,
               expanded_normal_100, expanded_normal_900, expanded_italic_100, expanded_italic_900 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { condensed_normal_100,condensed_italic_100,expanded_normal_100,expanded_italic_100 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_100 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_100 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_100 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_100 },
            },
        },

        {
            { condensed_normal_900,condensed_italic_900,expanded_normal_900,expanded_italic_900 },
            {
                { condensed_normal_100, condensed_normal_900 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_italic_900 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, expanded_normal_900 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_italic_900 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { condensed_normal_100,condensed_normal_900,expanded_normal_100,expanded_normal_900 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_normal_100 },
                { condensed_italic_900, condensed_normal_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_normal_100 },
                { expanded_italic_900, expanded_normal_900 },
            },
        },

        {
            { condensed_normal_100,expanded_normal_100 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_100 },
                { condensed_italic_100, condensed_normal_100 },
                { condensed_italic_900, condensed_normal_100 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_100 },
                { expanded_italic_100, expanded_normal_100 },
                { expanded_italic_900, expanded_normal_100 },
            },
        },

        {
            { condensed_normal_900,expanded_normal_900 },
            {
                { condensed_normal_100, condensed_normal_900 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_normal_900 },
                { condensed_italic_900, condensed_normal_900 },
                { expanded_normal_100, expanded_normal_900 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_normal_900 },
                { expanded_italic_900, expanded_normal_900 },
            },
        },

        {
            { condensed_italic_100,condensed_italic_900,expanded_italic_100,expanded_italic_900 },
            {
                { condensed_normal_100, condensed_italic_100 },
                { condensed_normal_900, condensed_italic_900 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, expanded_italic_100 },
                { expanded_normal_900, expanded_italic_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { condensed_italic_100,expanded_italic_100 },
            {
                { condensed_normal_100, condensed_italic_100 },
                { condensed_normal_900, condensed_italic_100 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_100 },
                { expanded_normal_100, expanded_italic_100 },
                { expanded_normal_900, expanded_italic_100 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_100 },
            },
        },

        {
            { condensed_italic_900,expanded_italic_900 },
            {
                { condensed_normal_100, condensed_italic_900 },
                { condensed_normal_900, condensed_italic_900 },
                { condensed_italic_100, condensed_italic_900 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, expanded_italic_900 },
                { expanded_normal_900, expanded_italic_900 },
                { expanded_italic_100, expanded_italic_900 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { condensed_normal_100,condensed_normal_900,condensed_italic_100,condensed_italic_900 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, condensed_normal_100 },
                { expanded_normal_900, condensed_normal_900 },
                { expanded_italic_100, condensed_italic_100 },
                { expanded_italic_900, condensed_italic_900 },
            },
        },

        {
            { condensed_normal_100,condensed_italic_100 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_100 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_100 },
                { expanded_normal_100, condensed_normal_100 },
                { expanded_normal_900, condensed_normal_100 },
                { expanded_italic_100, condensed_italic_100 },
                { expanded_italic_900, condensed_italic_100 },
            },
        },

        {
            { condensed_normal_900,condensed_italic_900 },
            {
                { condensed_normal_100, condensed_normal_900 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_italic_900 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, condensed_normal_900 },
                { expanded_normal_900, condensed_normal_900 },
                { expanded_italic_100, condensed_italic_900 },
                { expanded_italic_900, condensed_italic_900 },
            },
        },

        {
            { condensed_normal_100,condensed_normal_900 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_normal_100 },
                { condensed_italic_900, condensed_normal_900 },
                { expanded_normal_100, condensed_normal_100 },
                { expanded_normal_900, condensed_normal_900 },
                { expanded_italic_100, condensed_normal_100 },
                { expanded_italic_900, condensed_normal_900 },
            },
        },

        {
            { condensed_normal_100 },
            {
                { condensed_normal_100, condensed_normal_100 },
                { condensed_normal_900, condensed_normal_100 },
                { condensed_italic_100, condensed_normal_100 },
                { condensed_italic_900, condensed_normal_100 },
                { expanded_normal_100, condensed_normal_100 },
                { expanded_normal_900, condensed_normal_100 },
                { expanded_italic_100, condensed_normal_100 },
                { expanded_italic_900, condensed_normal_100 },
            },
        },

        {
            { condensed_normal_900 },
            {
                { condensed_normal_100, condensed_normal_900 },
                { condensed_normal_900, condensed_normal_900 },
                { condensed_italic_100, condensed_normal_900 },
                { condensed_italic_900, condensed_normal_900 },
                { expanded_normal_100, condensed_normal_900 },
                { expanded_normal_900, condensed_normal_900 },
                { expanded_italic_100, condensed_normal_900 },
                { expanded_italic_900, condensed_normal_900 },
            },
        },

        {
            { condensed_italic_100,condensed_italic_900 },
            {
                { condensed_normal_100, condensed_italic_100 },
                { condensed_normal_900, condensed_italic_900 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, condensed_italic_100 },
                { expanded_normal_900, condensed_italic_900 },
                { expanded_italic_100, condensed_italic_100 },
                { expanded_italic_900, condensed_italic_900 },
            },
        },

        {
            { condensed_italic_100 },
            {
                { condensed_normal_100, condensed_italic_100 },
                { condensed_normal_900, condensed_italic_100 },
                { condensed_italic_100, condensed_italic_100 },
                { condensed_italic_900, condensed_italic_100 },
                { expanded_normal_100, condensed_italic_100 },
                { expanded_normal_900, condensed_italic_100 },
                { expanded_italic_100, condensed_italic_100 },
                { expanded_italic_900, condensed_italic_100 },
            },
        },

        {
            { condensed_italic_900 },
            {
                { condensed_normal_100, condensed_italic_900 },
                { condensed_normal_900, condensed_italic_900 },
                { condensed_italic_100, condensed_italic_900 },
                { condensed_italic_900, condensed_italic_900 },
                { expanded_normal_100, condensed_italic_900 },
                { expanded_normal_900, condensed_italic_900 },
                { expanded_italic_100, condensed_italic_900 },
                { expanded_italic_900, condensed_italic_900 },
            },
        },

        {
            { expanded_normal_100,expanded_normal_900,
              expanded_italic_100,expanded_italic_900 },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_900 },
                { condensed_obliqu_100, expanded_italic_100 },
                { condensed_obliqu_900, expanded_italic_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
                { expanded_obliqu_100, expanded_italic_100 },
                { expanded_obliqu_900, expanded_italic_900 },
            },
        },

        {
            { expanded_normal_100,expanded_italic_100 },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_100 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_100 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_100 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_100 },
            },
        },

        {
            { expanded_normal_900,expanded_italic_900 },
            {
                { condensed_normal_100, expanded_normal_900 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_italic_900 },
                { condensed_italic_900, expanded_italic_900 },
                { expanded_normal_100, expanded_normal_900 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_italic_900 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { expanded_normal_100,expanded_normal_900 },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_normal_100 },
                { condensed_italic_900, expanded_normal_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_normal_100 },
                { expanded_italic_900, expanded_normal_900 },
            },
        },

        {
            { expanded_normal_100 },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_100 },
                { condensed_italic_100, expanded_normal_100 },
                { condensed_italic_900, expanded_normal_100 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_100 },
                { expanded_italic_100, expanded_normal_100 },
                { expanded_italic_900, expanded_normal_100 },
            },
        },

        {
            { expanded_normal_900 },
            {
                { condensed_normal_100, expanded_normal_900 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_normal_900 },
                { condensed_italic_900, expanded_normal_900 },
                { expanded_normal_100, expanded_normal_900 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_normal_900 },
                { expanded_italic_900, expanded_normal_900 },
            },
        },

        {
            { expanded_italic_100,expanded_italic_900 },
            {
                { condensed_normal_100, expanded_italic_100 },
                { condensed_normal_900, expanded_italic_900 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_900 },
                { expanded_normal_100, expanded_italic_100 },
                { expanded_normal_900, expanded_italic_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { expanded_italic_100 },
            {
                { condensed_normal_100, expanded_italic_100 },
                { condensed_normal_900, expanded_italic_100 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_100 },
                { expanded_normal_100, expanded_italic_100 },
                { expanded_normal_900, expanded_italic_100 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_100 },
            },
        },

        {
            { expanded_italic_900 },
            {
                { condensed_normal_100, expanded_italic_900 },
                { condensed_normal_900, expanded_italic_900 },
                { condensed_italic_100, expanded_italic_900 },
                { condensed_italic_900, expanded_italic_900 },
                { expanded_normal_100, expanded_italic_900 },
                { expanded_normal_900, expanded_italic_900 },
                { expanded_italic_100, expanded_italic_900 },
                { expanded_italic_900, expanded_italic_900 },
            },
        },

        {
            { normal_normal_100, normal_normal_900 },
            {
                { normal_normal_300, normal_normal_100 },
                { normal_normal_400, normal_normal_100 },
                { normal_normal_500, normal_normal_100 },
                { normal_normal_600, normal_normal_900 },
            },
        },

        {
            { normal_normal_100, normal_normal_400, normal_normal_900 },
            {
                { normal_normal_300, normal_normal_100 },
                { normal_normal_400, normal_normal_400 },
                { normal_normal_500, normal_normal_400 },
                { normal_normal_600, normal_normal_900 },
            },
        },

        {
            { normal_normal_100, normal_normal_500, normal_normal_900 },
            {
                { normal_normal_300, normal_normal_100 },
                { normal_normal_400, normal_normal_500 },
                { normal_normal_500, normal_normal_500 },
                { normal_normal_600, normal_normal_900 },
            },
        },

        {
            { },
            {
                { normal_normal_300, invalidFontStyle },
                { normal_normal_400, invalidFontStyle },
                { normal_normal_500, invalidFontStyle },
                { normal_normal_600, invalidFontStyle },
            },
        },
        {
            { expanded_normal_100,expanded_normal_900,
              expanded_italic_100,expanded_italic_900,
              expanded_obliqu_100,expanded_obliqu_900, },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_900 },
                { condensed_obliqu_100, expanded_obliqu_100 },
                { condensed_obliqu_900, expanded_obliqu_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
                { expanded_obliqu_100, expanded_obliqu_100 },
                { expanded_obliqu_900, expanded_obliqu_900 },
            },
        },
        {
            { expanded_normal_100,expanded_normal_900,
              expanded_obliqu_100,expanded_obliqu_900, },
            {
                { condensed_normal_100, expanded_normal_100 },
                { condensed_normal_900, expanded_normal_900 },
                { condensed_italic_100, expanded_obliqu_100 },
                { condensed_italic_900, expanded_obliqu_900 },
                { condensed_obliqu_100, expanded_obliqu_100 },
                { condensed_obliqu_900, expanded_obliqu_900 },
                { expanded_normal_100, expanded_normal_100 },
                { expanded_normal_900, expanded_normal_900 },
                { expanded_italic_100, expanded_obliqu_100 },
                { expanded_italic_900, expanded_obliqu_900 },
                { expanded_obliqu_100, expanded_obliqu_100 },
                { expanded_obliqu_900, expanded_obliqu_900 },
            },
        },
        {
            { expanded_italic_100,expanded_italic_900,
              expanded_obliqu_100,expanded_obliqu_900, },
            {
                { condensed_normal_100, expanded_obliqu_100 },
                { condensed_normal_900, expanded_obliqu_900 },
                { condensed_italic_100, expanded_italic_100 },
                { condensed_italic_900, expanded_italic_900 },
                { condensed_obliqu_100, expanded_obliqu_100 },
                { condensed_obliqu_900, expanded_obliqu_900 },
                { expanded_normal_100, expanded_obliqu_100 },
                { expanded_normal_900, expanded_obliqu_900 },
                { expanded_italic_100, expanded_italic_100 },
                { expanded_italic_900, expanded_italic_900 },
                { expanded_obliqu_100, expanded_obliqu_100 },
                { expanded_obliqu_900, expanded_obliqu_900 },
            },
        },
    };

    for (StyleSetTest& test : tests) {
        for (const StyleSetTest::Case& testCase : test.cases) {
            sk_sp<SkTypeface> typeface(test.styleSet.matchStyle(testCase.pattern));
            if (typeface) {
                REPORTER_ASSERT(reporter, typeface->fontStyle() == testCase.expectedResult);
            } else {
                REPORTER_ASSERT(reporter, invalidFontStyle == testCase.expectedResult);
            }
        }
    }
}

DEF_TEST(FontMgr_MatchCharacter, reporter) {
    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    // 0xD800 <= codepoint <= 0xDFFF || 0x10FFFF < codepoint are invalid
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, 0x0));
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, 0xD800));
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, 0xDFFF));
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, 0x110000));
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, 0x1FFFFF));
    SkSafeUnref(fm->matchFamilyStyleCharacter("Blah", SkFontStyle::Normal(), nullptr, 0, -1));
}
