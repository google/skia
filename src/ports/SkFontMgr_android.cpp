/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigParser_android.h"
#include "SkFontDescriptor.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkTypeface_android.h"
#include "SkTypefaceCache.h"

#include <limits>
#include <stdlib.h>

#ifndef SK_FONT_FILE_PREFIX
#    define SK_FONT_FILE_PREFIX "/fonts/"
#endif

#ifndef SK_DEBUG_FONTS
    #define SK_DEBUG_FONTS 0
#endif

#if SK_DEBUG_FONTS
#    define DEBUG_FONT(args) SkDebugf args
#else
#    define DEBUG_FONT(args)
#endif

// For test only.
static const char* gTestMainConfigFile = NULL;
static const char* gTestFallbackConfigFile = NULL;
static const char* gTestFontFilePrefix = NULL;

class SkTypeface_Android : public SkTypeface_FreeType {
public:
    SkTypeface_Android(int index,
                       Style style,
                       bool isFixedPitch,
                       const SkString familyName)
        : INHERITED(style, SkTypefaceCache::NewFontID(), isFixedPitch)
        , fIndex(index)
        , fFamilyName(familyName) { }

protected:
    virtual void onGetFamilyName(SkString* familyName) const SK_OVERRIDE {
        *familyName = fFamilyName;
    }

    int fIndex;
    SkString fFamilyName;

private:
    typedef SkTypeface_FreeType INHERITED;
};

class SkTypeface_AndroidSystem : public SkTypeface_Android {
public:
    SkTypeface_AndroidSystem(const SkString pathName,
                             int index,
                             Style style,
                             bool isFixedPitch,
                             const SkString familyName,
                             const SkLanguage& lang,
                             FontVariant variantStyle)
        : INHERITED(index, style, isFixedPitch, familyName)
        , fPathName(pathName)
        , fLang(lang)
        , fVariantStyle(variantStyle) { }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc,
                                     bool* serialize) const SK_OVERRIDE {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setFontFileName(fPathName.c_str());
        desc->setFontIndex(fIndex);
        *serialize = false;
    }
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = fIndex;
        return SkStream::NewFromFile(fPathName.c_str());
    }

    const SkString fPathName;
    const SkLanguage fLang;
    const FontVariant fVariantStyle;

    typedef SkTypeface_Android INHERITED;
};

class SkTypeface_AndroidStream : public SkTypeface_Android {
public:
    SkTypeface_AndroidStream(SkStream* stream,
                             int index,
                             Style style,
                             bool isFixedPitch,
                             const SkString familyName)
        : INHERITED(index, style, isFixedPitch, familyName)
        , fStream(SkRef(stream)) { }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc,
                                     bool* serialize) const SK_OVERRIDE {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setFontFileName(NULL);
        *serialize = true;
    }

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = fIndex;
        return fStream->duplicate();
    }

private:
    SkAutoTUnref<SkStream> fStream;

    typedef SkTypeface_Android INHERITED;
};

void get_path_for_sys_fonts(const char* basePath, const SkString& name, SkString* full) {
    if (basePath) {
        full->set(basePath);
    } else {
        full->set(getenv("ANDROID_ROOT"));
        full->append(SK_FONT_FILE_PREFIX);
    }
    full->append(name);
}

class SkFontStyleSet_Android : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Android(const FontFamily& family, const char* basePath) {
        const SkString* cannonicalFamilyName = NULL;
        if (family.fNames.count() > 0) {
            cannonicalFamilyName = &family.fNames[0];
        }
        // TODO? make this lazy
        for (int i = 0; i < family.fFonts.count(); ++i) {
            const FontFileInfo& fontFile = family.fFonts[i];

            SkString pathName;
            get_path_for_sys_fonts(basePath, fontFile.fFileName, &pathName);

            SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(pathName.c_str()));
            if (!stream.get()) {
                DEBUG_FONT(("---- SystemFonts[%d] file=%s (NOT EXIST)", i, pathName.c_str()));
                continue;
            }

            const int ttcIndex = fontFile.fIndex;
            SkString familyName;
            SkTypeface::Style style;
            bool isFixedWidth;
            if (!SkTypeface_FreeType::ScanFont(stream.get(), ttcIndex,
                                               &familyName, &style, &isFixedWidth)) {
                DEBUG_FONT(("---- SystemFonts[%d] file=%s (INVALID)", i, pathName.c_str()));
                continue;
            }

            const SkLanguage& lang = family.fLanguage;
            uint32_t variant = family.fVariant;
            if (kDefault_FontVariant == variant) {
                variant = kCompact_FontVariant | kElegant_FontVariant;
            }

            // The first specified family name overrides the family name found in the font.
            // TODO: SkTypeface_AndroidSystem::onCreateFamilyNameIterator should return
            // all of the specified family names in addition to the names found in the font.
            if (cannonicalFamilyName != NULL) {
                familyName = *cannonicalFamilyName;
            }

            fStyles.push_back().reset(SkNEW_ARGS(SkTypeface_AndroidSystem,
                                                 (pathName, ttcIndex,
                                                  style, isFixedWidth, familyName,
                                                  lang, variant)));
        }
    }

    virtual int count() SK_OVERRIDE {
        return fStyles.count();
    }
    virtual void getStyle(int index, SkFontStyle* style, SkString* name) SK_OVERRIDE {
        if (index < 0 || fStyles.count() <= index) {
            return;
        }
        if (style) {
            *style = this->style(index);
        }
        if (name) {
            name->reset();
        }
    }
    virtual SkTypeface_AndroidSystem* createTypeface(int index) SK_OVERRIDE {
        if (index < 0 || fStyles.count() <= index) {
            return NULL;
        }
        return SkRef(fStyles[index].get());
    }

    /** Find the typeface in this style set that most closely matches the given pattern.
     *  TODO: consider replacing with SkStyleSet_Indirect::matchStyle();
     *  this simpler version using match_score() passes all our tests.
     */
    virtual SkTypeface_AndroidSystem* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE {
        if (0 == fStyles.count()) {
            return NULL;
        }
        SkTypeface_AndroidSystem* closest = fStyles[0];
        int minScore = std::numeric_limits<int>::max();
        for (int i = 0; i < fStyles.count(); ++i) {
            SkFontStyle style = this->style(i);
            int score = match_score(pattern, style);
            if (score < minScore) {
                closest = fStyles[i];
                minScore = score;
            }
        }
        return SkRef(closest);
    }

private:
    SkFontStyle style(int index) {
        return SkFontStyle(this->weight(index), SkFontStyle::kNormal_Width,
                           this->slant(index));
    }
    SkFontStyle::Weight weight(int index) {
        if (fStyles[index]->isBold()) return SkFontStyle::kBold_Weight;
        return SkFontStyle::kNormal_Weight;
    }
    SkFontStyle::Slant slant(int index) {
        if (fStyles[index]->isItalic()) return SkFontStyle::kItalic_Slant;
        return SkFontStyle::kUpright_Slant;
    }
    static int match_score(const SkFontStyle& pattern, const SkFontStyle& candidate) {
        int score = 0;
        score += abs((pattern.width() - candidate.width()) * 100);
        score += abs((pattern.isItalic() == candidate.isItalic()) ? 0 : 1000);
        score += abs(pattern.weight() - candidate.weight());
        return score;
    }

    SkTArray<SkAutoTUnref<SkTypeface_AndroidSystem>, true> fStyles;

    friend struct NameToFamily;
    friend class SkFontMgr_Android;

    typedef SkFontStyleSet INHERITED;
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
    SkFontMgr_Android() {
        SkTDArray<FontFamily*> fontFamilies;
        SkFontConfigParser::GetFontFamilies(fontFamilies);
        this->buildNameToFamilyMap(fontFamilies, NULL);
        this->findDefaultFont();
    }
    SkFontMgr_Android(const char* mainConfigFile, const char* fallbackConfigFile,
                      const char* basePath)
    {
        SkTDArray<FontFamily*> fontFamilies;
        SkFontConfigParser::GetTestFontFamilies(fontFamilies, mainConfigFile, fallbackConfigFile);
        this->buildNameToFamilyMap(fontFamilies, basePath);
        this->findDefaultFont();
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    virtual int onCountFamilies() const SK_OVERRIDE {
        return fNameToFamilyMap.count();
    }

    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    virtual SkFontStyleSet* onCreateStyleSet(int index) const SK_OVERRIDE {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            return NULL;
        }
        return SkRef(fNameToFamilyMap[index].styleSet);
    }

    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) const SK_OVERRIDE {
        if (!familyName) {
            return NULL;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.count(); ++i) {
            if (fNameToFamilyMap[i].name.equals(tolc.lc())) {
                return SkRef(fNameToFamilyMap[i].styleSet);
            }
        }
        // TODO: eventually we should not need to name fallback families.
        for (int i = 0; i < fFallbackNameToFamilyMap.count(); ++i) {
            if (fFallbackNameToFamilyMap[i].name.equals(tolc.lc())) {
                return SkRef(fFallbackNameToFamilyMap[i].styleSet);
            }
        }
        return NULL;
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& style) const SK_OVERRIDE {
        SkAutoTUnref<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(style);
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* typeface,
                                         const SkFontStyle& style) const SK_OVERRIDE {
        for (int i = 0; i < fFontStyleSets.count(); ++i) {
            for (int j = 0; j < fFontStyleSets[i]->fStyles.count(); ++j) {
                if (fFontStyleSets[i]->fStyles[j] == typeface) {
                    return fFontStyleSets[i]->matchStyle(style);
                }
            }
        }
        return NULL;
    }

static SkTypeface_AndroidSystem* find_family_style_character(
        const SkTDArray<NameToFamily>& fallbackNameToFamilyMap,
        const SkFontStyle& style, bool elegant,
        const SkString& langTag, SkUnichar character)
{
    for (int i = 0; i < fallbackNameToFamilyMap.count(); ++i) {
        SkFontStyleSet_Android* family = fallbackNameToFamilyMap[i].styleSet;
        SkAutoTUnref<SkTypeface_AndroidSystem> face(family->matchStyle(style));

        if (!langTag.isEmpty() && !face->fLang.getTag().startsWith(langTag.c_str())) {
            continue;
        }

        if (SkToBool(face->fVariantStyle & kElegant_FontVariant) != elegant) {
            continue;
        }

        SkPaint paint;
        paint.setTypeface(face);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        uint16_t glyphID;
        paint.textToGlyphs(&character, sizeof(character), &glyphID);
        if (glyphID != 0) {
            return face.detach();
        }
    }
    return NULL;
}
#ifdef SK_FM_NEW_MATCH_FAMILY_STYLE_CHARACTER
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character) const SK_OVERRIDE
    {
#else
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char bcp47_val[],
                                                    SkUnichar character) const SK_OVERRIDE
    {
        const char** bcp47 = &bcp47_val;
        int bcp47Count = bcp47_val ? 1 : 0;
#endif
        // The variant 'elegant' is 'not squashed', 'compact' is 'stays in ascent/descent'.
        // The variant 'default' means 'compact and elegant'.
        // As a result, it is not possible to know the variant context from the font alone.
        // TODO: add 'is_elegant' and 'is_compact' bits to 'style' request.

        // The first time match anything elegant, second time anything not elegant.
        for (int elegant = 2; elegant --> 0;) {
            for (int bcp47Index = bcp47Count; bcp47Index --> 0;) {
                SkLanguage lang(bcp47[bcp47Index]);
                while (!lang.getTag().isEmpty()) {
                    SkTypeface_AndroidSystem* matchingTypeface =
                        find_family_style_character(fFallbackNameToFamilyMap,
                                                    style, SkToBool(elegant),
                                                    lang.getTag(), character);
                    if (matchingTypeface) {
                        return matchingTypeface;
                    }

                    lang = lang.getParent();
                }
            }
            SkTypeface_AndroidSystem* matchingTypeface =
                find_family_style_character(fFallbackNameToFamilyMap,
                                            style, SkToBool(elegant),
                                            SkString(), character);
            if (matchingTypeface) {
                return matchingTypeface;
            }
        }
        return NULL;
    }

    virtual SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(new SkMemoryStream(data));
        return this->createFromStream(stream, ttcIndex);
    }

    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
        return stream.get() ? this->createFromStream(stream, ttcIndex) : NULL;
    }

    virtual SkTypeface* onCreateFromStream(SkStream* stream, int ttcIndex) const SK_OVERRIDE {
        bool isFixedPitch;
        SkTypeface::Style style;
        SkString name;
        if (!SkTypeface_FreeType::ScanFont(stream, ttcIndex, &name, &style, &isFixedPitch)) {
            return NULL;
        }
        return SkNEW_ARGS(SkTypeface_AndroidStream, (stream, ttcIndex,
                                                     style, isFixedPitch, name));
    }


    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) const SK_OVERRIDE {
        SkTypeface::Style oldStyle = (SkTypeface::Style)styleBits;
        SkFontStyle style = SkFontStyle(oldStyle & SkTypeface::kBold
                                                 ? SkFontStyle::kBold_Weight
                                                 : SkFontStyle::kNormal_Weight,
                                        SkFontStyle::kNormal_Width,
                                        oldStyle & SkTypeface::kItalic
                                                 ? SkFontStyle::kItalic_Slant
                                                 : SkFontStyle::kUpright_Slant);

        if (familyName) {
            // On Android, we must return NULL when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return this->onMatchFamilyStyle(familyName, style);
        }
        return fDefaultFamily->matchStyle(style);
    }


private:

    SkTArray<SkAutoTUnref<SkFontStyleSet_Android>, true> fFontStyleSets;
    SkFontStyleSet* fDefaultFamily;
    SkTypeface* fDefaultTypeface;

    SkTDArray<NameToFamily> fNameToFamilyMap;
    SkTDArray<NameToFamily> fFallbackNameToFamilyMap;

    void buildNameToFamilyMap(SkTDArray<FontFamily*> families, const char* basePath) {
        for (int i = 0; i < families.count(); i++) {
            FontFamily& family = *families[i];

            SkTDArray<NameToFamily>* nameToFamily = &fNameToFamilyMap;
            if (family.fIsFallbackFont) {
                nameToFamily = &fFallbackNameToFamilyMap;

                if (0 == family.fNames.count()) {
                    SkString& fallbackName = family.fNames.push_back();
                    fallbackName.printf("%.2x##fallback", i);
                }
            }

            SkFontStyleSet_Android* newSet = SkNEW_ARGS(SkFontStyleSet_Android, (family, basePath));
            if (0 == newSet->count()) {
                SkDELETE(newSet);
                continue;
            }
            fFontStyleSets.push_back().reset(newSet);

            for (int j = 0; j < family.fNames.count(); j++) {
                NameToFamily* nextEntry = nameToFamily->append();
                SkNEW_PLACEMENT_ARGS(&nextEntry->name, SkString, (family.fNames[j]));
                nextEntry->styleSet = newSet;
            }
        }
    }

    void findDefaultFont() {
        SkASSERT(!fFontStyleSets.empty());

        static const char* gDefaultNames[] = { "sans-serif" };
        for (size_t i = 0; i < SK_ARRAY_COUNT(gDefaultNames); ++i) {
            SkFontStyleSet* set = this->onMatchFamily(gDefaultNames[i]);
            if (NULL == set) {
                continue;
            }
            SkTypeface* tf = set->matchStyle(SkFontStyle());
            if (NULL == tf) {
                continue;
            }
            fDefaultFamily = set;
            fDefaultTypeface = tf;
            break;
        }
        if (NULL == fDefaultTypeface) {
            fDefaultFamily = fFontStyleSets[0];
            fDefaultTypeface = fDefaultFamily->createTypeface(0);
        }
        SkASSERT(fDefaultFamily);
        SkASSERT(fDefaultTypeface);
    }

    typedef SkFontMgr INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() {
    // The call to SkGetTestFontConfiguration is so that Chromium can override the environment.
    // TODO: these globals need to be removed, in favor of a constructor / separate Factory
    // which can be used instead.
    const char* mainConfigFile;
    const char* fallbackConfigFile;
    const char* basePath;
    SkGetTestFontConfiguration(&mainConfigFile, &fallbackConfigFile, &basePath);
    if (mainConfigFile) {
        return SkNEW_ARGS(SkFontMgr_Android, (mainConfigFile, fallbackConfigFile, basePath));
    }

    return SkNEW(SkFontMgr_Android);
}

void SkUseTestFontConfigFile(const char* mainconf, const char* fallbackconf,
                             const char* fontsdir) {
    gTestMainConfigFile = mainconf;
    gTestFallbackConfigFile = fallbackconf;
    gTestFontFilePrefix = fontsdir;
    SkASSERT(gTestMainConfigFile);
    SkASSERT(gTestFallbackConfigFile);
    SkASSERT(gTestFontFilePrefix);
    SkDEBUGF(("Use Test Config File Main %s, Fallback %s, Font Dir %s",
              gTestMainConfigFile, gTestFallbackConfigFile, gTestFontFilePrefix));
}

void SkGetTestFontConfiguration(const char** mainconf, const char** fallbackconf,
                                const char** fontsdir) {
    *mainconf = gTestMainConfigFile;
    *fallbackconf = gTestFallbackConfigFile;
    *fontsdir = gTestFontFilePrefix;
}
