/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommandLineFlags.h"
#include "SkAdvancedTypefaceMetrics.h"
#include "SkFont.h"
#include "SkFontMgr.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "Test.h"

#include <initializer_list>
#include <limits>
#include <vector>

static void test_font(skiatest::Reporter* reporter) {
    SkFont font(nullptr, 24);

    //REPORTER_ASSERT(reporter, SkTypeface::GetDefaultTypeface() == font.getTypeface());
    REPORTER_ASSERT(reporter, 24 == font.getSize());
    REPORTER_ASSERT(reporter, 1 == font.getScaleX());
    REPORTER_ASSERT(reporter, 0 == font.getSkewX());

    uint16_t glyphs[5];
    sk_bzero(glyphs, sizeof(glyphs));

    // Check that no glyphs are copied with insufficient storage.
    int count = font.textToGlyphs("Hello", 5, kUTF8_SkTextEncoding, glyphs, 2);
    REPORTER_ASSERT(reporter, 5 == count);
    for (const auto glyph : glyphs) { REPORTER_ASSERT(reporter, glyph == 0); }

    SkAssertResult(font.textToGlyphs("Hello", 5, kUTF8_SkTextEncoding, glyphs,
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
static void test_alias_names(skiatest::Reporter* reporter) {
    const char* inNames[] = {
        "sans", "sans-serif", "serif", "monospace", "times", "helvetica"
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(inNames); ++i) {
        sk_sp<SkTypeface> first(SkTypeface::MakeFromName(inNames[i], SkFontStyle()));
        if (nullptr == first.get()) {
            continue;
        }
        for (int j = 0; j < 10; ++j) {
            sk_sp<SkTypeface> face(SkTypeface::MakeFromName(inNames[i], SkFontStyle()));
    #if 0
            SkString name;
            face->getFamilyName(&name);
            printf("request %s, received %s, first id %x received %x\n",
                   inNames[i], name.c_str(), first->uniqueID(), face->uniqueID());
    #endif
            REPORTER_ASSERT(reporter, first->uniqueID() == face->uniqueID());
        }
    }
}

static void test_fontiter(skiatest::Reporter* reporter, bool verbose) {
    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    int count = fm->countFamilies();

    for (int i = 0; i < count; ++i) {
        SkString fname;
        fm->getFamilyName(i, &fname);

        sk_sp<SkFontStyleSet> fnset(fm->matchFamily(fname.c_str()));
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        REPORTER_ASSERT(reporter, fnset->count() == set->count());

        if (verbose) {
            SkDebugf("[%2d] %s\n", i, fname.c_str());
        }

        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);
//            REPORTER_ASSERT(reporter, sname.size() > 0);

            sk_sp<SkTypeface> face(set->createTypeface(j));
//            REPORTER_ASSERT(reporter, face.get());

            if (verbose) {
                SkDebugf("\t[%d] %s [%3d %d %d]\n", j, sname.c_str(),
                         fs.weight(), fs.width(), fs.slant());
            }
        }
    }
}

static void test_match(skiatest::Reporter* reporter) {
    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    sk_sp<SkFontStyleSet> styleSet(fm->matchFamily(nullptr));
    REPORTER_ASSERT(reporter, styleSet);
}

static void test_matchStyleCSS3(skiatest::Reporter* reporter) {
    static const SkFontStyle invalidFontStyle(101, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);

    class TestTypeface : public SkTypeface {
    public:
        TestTypeface(const SkFontStyle& fontStyle) : SkTypeface(fontStyle, false){}
    protected:
        std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
        sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
            return sk_ref_sp(this);
        }
        SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                               const SkDescriptor*) const override {
            return nullptr;
        }
        void onFilterRec(SkScalerContextRec*) const override { }
        std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
            return nullptr;
        }
        void onGetFontDescriptor(SkFontDescriptor*, bool*) const override { }
        virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
            uint16_t glyphs[], int glyphCount) const override {
            if (glyphs && glyphCount > 0) {
                sk_bzero(glyphs, glyphCount * sizeof(glyphs[0]));
            }
            return 0;
        }
        int onCountGlyphs() const override { return 0; }
        int onGetUPEM() const override { return 0; }
        class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
        public:
            bool next(SkTypeface::LocalizedString*) override { return false; }
        };
        void onGetFamilyName(SkString* familyName) const override {
            familyName->reset();
        }
        SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
            return new EmptyLocalizedStrings;
        }
        int onGetVariationDesignPosition(
                SkFontArguments::VariationPosition::Coordinate coordinates[],
                int coordinateCount) const override
        {
            return 0;
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

DEFINE_bool(verboseFontMgr, false, "run verbose fontmgr tests.");

DEF_TEST(FontMgr, reporter) {
    test_match(reporter);
    test_matchStyleCSS3(reporter);
    test_fontiter(reporter, FLAGS_verboseFontMgr);
    test_alias_names(reporter);
    test_font(reporter);
}
