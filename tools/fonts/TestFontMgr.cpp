/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontDescriptor.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/TestFontMgr.h"
#include "tools/fonts/TestTypeface.h"

#ifdef SK_XML
#include "tools/fonts/TestSVGTypeface.h"
#endif

#include <vector>

namespace {

#include "tools/fonts/test_font_monospace.inc"
#include "tools/fonts/test_font_sans_serif.inc"
#include "tools/fonts/test_font_serif.inc"

#include "tools/fonts/test_font_index.inc"

class FontStyleSet final : public SkFontStyleSet {
public:
    FontStyleSet(const char* familyName) : fFamilyName(familyName) {}
    struct TypefaceEntry {
        TypefaceEntry(sk_sp<SkTypeface> typeface, SkFontStyle style, const char* styleName)
                : fTypeface(std::move(typeface)), fStyle(style), fStyleName(styleName) {}
        sk_sp<SkTypeface> fTypeface;
        SkFontStyle       fStyle;
        const char*       fStyleName;
    };

    int count() override { return fTypefaces.size(); }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (style) {
            *style = fTypefaces[index].fStyle;
        }
        if (name) {
            *name = fTypefaces[index].fStyleName;
        }
    }

    SkTypeface* createTypeface(int index) override {
        return SkRef(fTypefaces[index].fTypeface.get());
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        return this->matchStyleCSS3(pattern);
    }

    SkString getFamilyName() { return fFamilyName; }

    std::vector<TypefaceEntry> fTypefaces;
    SkString                   fFamilyName;
};

class FontMgr final : public SkFontMgr {
public:
    FontMgr() {
        for (const auto& sub : gSubFonts) {
            sk_sp<TestTypeface> typeface =
                    sk_make_sp<TestTypeface>(sk_make_sp<SkTestFont>(sub.fFont), sub.fStyle);
            bool defaultFamily = false;
            if (&sub - gSubFonts == gDefaultFontIndex) {
                defaultFamily    = true;
                fDefaultTypeface = typeface;
            }
            bool found = false;
            for (const auto& family : fFamilies) {
                if (family->getFamilyName().equals(sub.fFamilyName)) {
                    family->fTypefaces.emplace_back(
                            std::move(typeface), sub.fStyle, sub.fStyleName);
                    found = true;
                    if (defaultFamily) {
                        fDefaultFamily = family;
                    }
                    break;
                }
            }
            if (!found) {
                fFamilies.emplace_back(sk_make_sp<FontStyleSet>(sub.fFamilyName));
                fFamilies.back()->fTypefaces.emplace_back(
                        // NOLINTNEXTLINE(bugprone-use-after-move)
                        std::move(typeface),
                        sub.fStyle,
                        sub.fStyleName);
                if (defaultFamily) {
                    fDefaultFamily = fFamilies.back();
                }
            }
        }
#if defined(SK_ENABLE_SVG)
        fFamilies.emplace_back(sk_make_sp<FontStyleSet>("Emoji"));
        fFamilies.back()->fTypefaces.emplace_back(
                TestSVGTypeface::Default(), SkFontStyle::Normal(), "Normal");

        fFamilies.emplace_back(sk_make_sp<FontStyleSet>("Planet"));
        fFamilies.back()->fTypefaces.emplace_back(
                TestSVGTypeface::Planets(), SkFontStyle::Normal(), "Normal");
#endif
    }

    int onCountFamilies() const override { return fFamilies.size(); }

    void onGetFamilyName(int index, SkString* familyName) const override {
        *familyName = fFamilies[index]->getFamilyName();
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        sk_sp<SkFontStyleSet> ref = fFamilies[index];
        return ref.release();
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (familyName) {
            if (strstr(familyName, "ono")) {
                return this->createStyleSet(0);
            }
            if (strstr(familyName, "ans")) {
                return this->createStyleSet(1);
            }
            if (strstr(familyName, "erif")) {
                return this->createStyleSet(2);
            }
#ifdef SK_XML
            if (strstr(familyName, "oji")) {
                return this->createStyleSet(6);
            }
            if (strstr(familyName, "Planet")) {
                return this->createStyleSet(7);
            }
#endif
        }
        return nullptr;
    }

    SkTypeface* onMatchFamilyStyle(const char         familyName[],
                                   const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> styleSet(this->matchFamily(familyName));
        return styleSet->matchStyle(style);
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char         familyName[],
                                            const SkFontStyle& style,
                                            const char*        bcp47[],
                                            int                bcp47Count,
                                            SkUnichar          character) const override {
        (void)bcp47;
        (void)bcp47Count;
        (void)character;
        return this->matchFamilyStyle(familyName, style);
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                            int ttcIndex) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char  familyName[],
                                           SkFontStyle style) const override {
        if (familyName == nullptr) {
            return sk_sp<SkTypeface>(fDefaultFamily->matchStyle(style));
        }
        sk_sp<SkTypeface> typeface = sk_sp<SkTypeface>(this->matchFamilyStyle(familyName, style));
        if (!typeface) {
            typeface = fDefaultTypeface;
        }
        return typeface;
    }

private:
    std::vector<sk_sp<FontStyleSet>> fFamilies;
    sk_sp<FontStyleSet>              fDefaultFamily;
    sk_sp<SkTypeface>                fDefaultTypeface;
};
}  // namespace

namespace ToolUtils {
sk_sp<SkFontMgr> MakePortableFontMgr() { return sk_make_sp<FontMgr>(); }
}  // namespace ToolUtils
