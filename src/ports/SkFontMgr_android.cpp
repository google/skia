/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkFontMgr_android.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "src/ports/SkTypeface_proxy.h"

#include <algorithm>
#include <limits>

using namespace skia_private;

class SkData;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

class SkTypeface_AndroidSystem : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_AndroidSystem> Make(sk_sp<SkTypeface> realTypeface,
                                                const SkFontStyle& style,
                                                bool isFixedPitch,
                                                const SkString& familyName,
                                                const TArray<SkLanguage, true>& lang,
                                                FontVariant variantStyle) {
        SkASSERT(realTypeface);
        return sk_sp(new SkTypeface_AndroidSystem(std::move(realTypeface),
                                                  style, isFixedPitch,
                                                  familyName, lang, variantStyle));
    }

    const SkString fFamilyName;
    const STArray<4, SkLanguage, true> fLang;
    const FontVariant fVariantStyle;

protected:
    SkTypeface_AndroidSystem(sk_sp<SkTypeface> realTypeface,
                             const SkFontStyle& style,
                             bool isFixedPitch,
                             const SkString& familyName,
                             const TArray<SkLanguage, true>& lang,
                             FontVariant variantStyle)
            : SkTypeface_proxy(std::move(realTypeface), style, isFixedPitch)
            , fFamilyName(familyName)
            , fLang(lang)
            , fVariantStyle(variantStyle)
    {}

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        auto proxy = SkTypeface_proxy::onMakeClone(args);
        if (proxy == nullptr) {
            return nullptr;
        }
        return SkTypeface_AndroidSystem::Make(std::move(proxy),
                                              this->fontStyle(), this->isFixedPitch(),
                                              fFamilyName, fLang, fVariantStyle);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkTypeface_proxy::onGetFontDescriptor(desc, serialize);

        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }

    SkFontStyle onGetFontStyle() const override {
        return SkTypeface::onGetFontStyle();
    }

    bool onGetFixedPitch() const override {
        return SkTypeface::onGetFixedPitch();
    }
};

template <typename D, typename S> sk_sp<D> sk_sp_static_cast(sk_sp<S>&& s) {
    return sk_sp<D>(static_cast<D*>(s.release()));
}

using StreamForPathCache = skia_private::THashMap<SkString, std::unique_ptr<SkStreamAsset>>;

class SkFontStyleSet_Android : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Android(const FontFamily& family, const SkFontScanner* scanner,
                                    const bool cacheFontFiles, StreamForPathCache& streamForPath) {
        const SkString* cannonicalFamilyName = nullptr;
        if (!family.fNames.empty()) {
            cannonicalFamilyName = &family.fNames[0];
        }
        fFallbackFor = family.fFallbackFor;

        // TODO? make this lazy
        for (int i = 0; i < family.fFonts.size(); ++i) {
            const FontFileInfo& fontFile = family.fFonts[i];

            SkString pathName(family.fBasePath);
            pathName.append(fontFile.fFileName);

            std::unique_ptr<SkStreamAsset>* streamPtr = streamForPath.find(pathName);
            if (!streamPtr) {
                streamPtr = streamForPath.set(pathName, SkStream::MakeFromFile(pathName.c_str()));
            }
            if (!*streamPtr) {
                SkDEBUGF("Requested font file %s cannot be opened.\n", pathName.c_str());
                continue;
            }
            std::unique_ptr<SkStreamAsset> stream = (*streamPtr)->duplicate();
            if (!stream) {
                SkDEBUGF("Requested font file %s could not be duplicated.\n", pathName.c_str());
                continue;
            }

            SkFontArguments::VariationPosition position = {
                    fontFile.fVariationDesignPosition.begin(),
                    fontFile.fVariationDesignPosition.size()
            };
            // TODO: this creates the proxy with the given stream, so always cacheFontFiles.
            auto proxy = scanner->MakeFromStream(
                std::move(stream),
                SkFontArguments().setCollectionIndex(fontFile.fIndex)
                                 .setVariationDesignPosition(position));
            if (!proxy) {
                SkDEBUGF("Requested font file %s does not have valid font data.\n",
                         pathName.c_str());
                continue;
            }

            uint32_t variant = family.fVariant;
            if (kDefault_FontVariant == variant) {
                variant = kCompact_FontVariant | kElegant_FontVariant;
            }

            // The first specified family name overrides the family name found in the font.
            // TODO: SkTypeface_AndroidSystem::onCreateFamilyNameIterator should return
            // all of the specified family names in addition to the names found in the font.
            SkString familyName;
            proxy->getFamilyName(&familyName);
            if (cannonicalFamilyName != nullptr) {
                familyName = *cannonicalFamilyName;
            }

            SkFontStyle fontStyle = proxy->fontStyle();
            int weight = fontFile.fWeight != 0 ? fontFile.fWeight : fontStyle.weight();
            SkFontStyle::Slant slant = fontStyle.slant();
            switch (fontFile.fStyle) {
                case FontFileInfo::Style::kAuto: slant = fontStyle.slant(); break;
                case FontFileInfo::Style::kNormal: slant = SkFontStyle::kUpright_Slant; break;
                case FontFileInfo::Style::kItalic: slant = SkFontStyle::kItalic_Slant; break;
                default: SkASSERT(false); break;
            }
            fontStyle = SkFontStyle(weight, fontStyle.width(), slant);

            fStyles.push_back(
                SkTypeface_AndroidSystem::Make(proxy,
                                               fontStyle, proxy->isFixedPitch(),
                                               familyName, family.fLanguages, variant));
        }
    }

    int count() override {
        return fStyles.size();
    }
    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fStyles.size() <= index) {
            return;
        }
        if (style) {
            *style = fStyles[index]->fontStyle();
        }
        if (name) {
            name->reset();
        }
    }
    sk_sp<SkTypeface> createTypeface(int index) override {
        if (index < 0 || fStyles.size() <= index) {
            return nullptr;
        }
        return fStyles[index];
    }

    sk_sp<SkTypeface_AndroidSystem> matchAStyle(const SkFontStyle& pattern) {
        return sk_sp_static_cast<SkTypeface_AndroidSystem>(this->matchStyleCSS3(pattern));
    }
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        return this->matchAStyle(pattern);
    }

private:
    TArray<sk_sp<SkTypeface_AndroidSystem>> fStyles;
    SkString fFallbackFor;

    friend struct NameToFamily;
    friend class SkFontMgr_Android;

    using INHERITED = SkFontStyleSet;
};

/** On Android a single family can have many names, but our API assumes unique names.
 *  Map names to the back end so that all names for a given family refer to the same
 *  (non-replicated) set of typefaces.
 *  SkTDict<> doesn't let us do index-based lookup, so we write our own mapping.
 */
struct NameToFamily {
    SkString name;
    SkFontStyleSet_Android* styleSet;
};

class SkFontMgr_Android : public SkFontMgr {
public:
    SkFontMgr_Android(const SkFontMgr_Android_CustomFonts* custom,
                      std::unique_ptr<SkFontScanner> scanner)
        : fScanner(std::move(scanner)) {
        std::vector<std::unique_ptr<FontFamily>> families;
        if (custom && SkFontMgr_Android_CustomFonts::kPreferSystem != custom->fSystemFontUse) {
            SkString base(custom->fBasePath);
            SkFontMgr_Android_Parser::GetCustomFontFamilies(
                families, base, custom->fFontsXml, custom->fFallbackFontsXml);
        }
        if (!custom ||
            (custom && SkFontMgr_Android_CustomFonts::kOnlyCustom != custom->fSystemFontUse))
        {
            SkFontMgr_Android_Parser::GetSystemFontFamilies(families);
        }
        if (custom && SkFontMgr_Android_CustomFonts::kPreferSystem == custom->fSystemFontUse) {
            SkString base(custom->fBasePath);
            SkFontMgr_Android_Parser::GetCustomFontFamilies(
                families, base, custom->fFontsXml, custom->fFallbackFontsXml);
        }
        this->buildNameToFamilyMap(families, custom ? custom->fIsolated : false);
        this->findDefaultStyleSet();
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    int onCountFamilies() const override {
        return fNameToFamilyMap.size();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            return nullptr;
        }
        return sk_ref_sp(fNameToFamilyMap[index].styleSet);
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            if (fNameToFamilyMap[i].name.equals(tolc.lc())) {
                return sk_ref_sp(fNameToFamilyMap[i].styleSet);
            }
        }
        // TODO: eventually we should not need to name fallback families.
        for (int i = 0; i < fFallbackNameToFamilyMap.size(); ++i) {
            if (fFallbackNameToFamilyMap[i].name.equals(tolc.lc())) {
                return sk_ref_sp(fFallbackNameToFamilyMap[i].styleSet);
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(style);
    }

    enum class NameType { Self, Fallback };
    static sk_sp<SkTypeface_AndroidSystem> find_family_style_character(
            const SkString& familyName,
            const TArray<NameToFamily, true>& nameToFamilyMap,
            NameType nameType,
            const SkFontStyle& style, bool elegant,
            const SkString& langTag, SkUnichar character)
    {
        for (auto&& nameToFamily : nameToFamilyMap) {
            SkFontStyleSet_Android* family = nameToFamily.styleSet;
            if (!familyName.isEmpty()) {
                const SkString& name = nameType == NameType::Self ? nameToFamily.name
                                                                  : family->fFallbackFor;
                if (familyName != name) {
                    continue;
                }
            }
            sk_sp<SkTypeface_AndroidSystem> face(family->matchAStyle(style));

            if (!langTag.isEmpty() &&
                std::none_of(face->fLang.begin(), face->fLang.end(), [&](const SkLanguage& lang){
                    return lang.getTag().startsWith(langTag.c_str());
                }))
            {
                continue;
            }

            if (SkToBool(face->fVariantStyle & kElegant_FontVariant) != elegant) {
                continue;
            }

            if (face->unicharToGlyph(character) != 0) {
                return face;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        // The variant 'elegant' is 'not squashed', 'compact' is 'stays in ascent/descent'.
        // The variant 'default' means 'compact and elegant'.
        // As a result, it is not possible to know the variant context from the font alone.
        // TODO: add 'is_elegant' and 'is_compact' bits to 'style' request.
        sk_sp<SkTypeface_AndroidSystem> matchingTypeface;
        SkString familyNameString(familyName);
        for (const SkString& currentFamilyName : { familyNameString, SkString() }) {
            // The first time match anything elegant, second time anything not elegant.
            for (int elegant = 2; elegant --> 0;) {
                for (int bcp47Index = bcp47Count; bcp47Index --> 0;) {
                    SkLanguage lang(bcp47[bcp47Index]);
                    while (!lang.getTag().isEmpty()) {
                        matchingTypeface = find_family_style_character(
                            currentFamilyName, fNameToFamilyMap, NameType::Self,
                            style, SkToBool(elegant), lang.getTag(), character);
                        if (matchingTypeface) {
                            return matchingTypeface;
                        }
                        matchingTypeface = find_family_style_character(
                            currentFamilyName, fFallbackNameToFamilyMap, NameType::Fallback,
                            style, SkToBool(elegant), lang.getTag(), character);
                        if (matchingTypeface) {
                            return matchingTypeface;
                        }

                        lang = lang.getParent();
                    }
                }
                matchingTypeface = find_family_style_character(
                    currentFamilyName, fNameToFamilyMap, NameType::Self,
                    style, SkToBool(elegant), SkString(), character);
                if (matchingTypeface) {
                    return matchingTypeface;
                }
                matchingTypeface = find_family_style_character(
                    currentFamilyName, fFallbackNameToFamilyMap, NameType::Fallback,
                    style, SkToBool(elegant), SkString(), character);
                if (matchingTypeface) {
                    return matchingTypeface;
                }
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))),
                                    ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return fScanner->MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        if (familyName) {
            // On Android, we must return nullptr when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return sk_sp<SkTypeface>(this->onMatchFamilyStyle(familyName, style));
        }
        if (fDefaultStyleSet) {
            return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
        }
        return SkTypeface::MakeEmpty();
    }


private:

    std::unique_ptr<SkFontScanner> fScanner;

    TArray<sk_sp<SkFontStyleSet_Android>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    TArray<NameToFamily, true> fNameToFamilyMap;
    TArray<NameToFamily, true> fFallbackNameToFamilyMap;

    void addFamily(FontFamily& family, const bool isolated, int familyIndex,
                   StreamForPathCache& streamForPath)
    {
        TArray<NameToFamily, true>* nameToFamily = &fNameToFamilyMap;
        if (family.fIsFallbackFont) {
            nameToFamily = &fFallbackNameToFamilyMap;

            if (family.fNames.empty()) {
                SkString& fallbackName = family.fNames.push_back();
                fallbackName.printf("%.2x##fallback", (uint32_t)familyIndex);
            }
        }

        sk_sp<SkFontStyleSet_Android> newSet =
            sk_make_sp<SkFontStyleSet_Android>(family, fScanner.get(), isolated, streamForPath);
        if (0 == newSet->count()) {
            return;
        }

        for (const SkString& name : family.fNames) {
            nameToFamily->emplace_back(NameToFamily{name, newSet.get()});
        }
        fStyleSets.emplace_back(std::move(newSet));
    }
    void buildNameToFamilyMap(std::vector<std::unique_ptr<FontFamily>>& families,
                              const bool isolated) {
        StreamForPathCache streamForPath;
        int familyIndex = 0;
        for (std::unique_ptr<FontFamily>& family : families) {
            addFamily(*family, isolated, familyIndex++, streamForPath);
            for (const auto& [unused, fallbackFamily] : family->fallbackFamilies) {
                addFamily(*fallbackFamily, isolated, familyIndex++, streamForPath);
            }
        }
    }

    void findDefaultStyleSet() {
        static const char* defaultNames[] = { "sans-serif" };
        for (const char* defaultName : defaultNames) {
            fDefaultStyleSet = this->onMatchFamily(defaultName);
            if (fDefaultStyleSet) {
                break;
            }
        }
        if (!fDefaultStyleSet && !fStyleSets.empty()) {
            fDefaultStyleSet = fStyleSets[0];
        }
    }

    using INHERITED = SkFontMgr;
};

#ifdef SK_DEBUG
static char const * const gSystemFontUseStrings[] = {
    "OnlyCustom", "PreferCustom", "PreferSystem"
};
#endif

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_Android(const SkFontMgr_Android_CustomFonts* custom,
                                       std::unique_ptr<SkFontScanner> scanner) {
    if (custom) {
        SkASSERT(0 <= custom->fSystemFontUse);
        SkASSERT(custom->fSystemFontUse < std::size(gSystemFontUseStrings));
        if constexpr (kSkFontMgrVerbose) {
            SkDEBUGF("SystemFontUse: %s BasePath: %s Fonts: %s FallbackFonts: %s\n",
                     gSystemFontUseStrings[custom->fSystemFontUse],
                     custom->fBasePath,
                     custom->fFontsXml,
                     custom->fFallbackFontsXml);
        }
    }
    return sk_make_sp<SkFontMgr_Android>(custom, std::move(scanner));
}
