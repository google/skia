
/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkTypeface_android.h"

#include "SkFontConfigParser_android.h"
#include "SkFontConfigTypeface.h"
#include "SkFontMgr.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTypefaceCache.h"
#include "SkTArray.h"
#include "SkTDict.h"
#include "SkTSearch.h"

#include <stdio.h>
#include <string.h>

#ifndef SK_DEBUG_FONTS
    #define SK_DEBUG_FONTS 0
#endif

#if SK_DEBUG_FONTS
    #define DEBUG_FONT(args) SkDebugf args
#else
    #define DEBUG_FONT(args)
#endif

///////////////////////////////////////////////////////////////////////////////

// For test only.
static const char* gTestMainConfigFile = NULL;
static const char* gTestFallbackConfigFile = NULL;
static const char* gTestFontFilePrefix = NULL;

///////////////////////////////////////////////////////////////////////////////

typedef int32_t FontRecID;
#define INVALID_FONT_REC_ID -1

typedef int32_t FamilyRecID;
#define INVALID_FAMILY_REC_ID -1

// used to record our notion of the pre-existing fonts
struct FontRec {
    SkRefPtr<SkTypeface> fTypeface;
    SkString fFileName;
    SkTypeface::Style fStyle;
    bool fIsValid;
    FamilyRecID fFamilyRecID;
};

struct FamilyRec {
    FamilyRec() {
        memset(fFontRecID, INVALID_FONT_REC_ID, sizeof(fFontRecID));
    }

    static const int FONT_STYLE_COUNT = 4;
    FontRecID fFontRecID[FONT_STYLE_COUNT];
    bool fIsFallbackFont;
    SkString fFallbackName;
    SkPaintOptionsAndroid fPaintOptions;
};


typedef SkTDArray<FamilyRecID> FallbackFontList;

class SkFontConfigInterfaceAndroid : public SkFontConfigInterface {
public:
    SkFontConfigInterfaceAndroid(SkTDArray<FontFamily*>& fontFamilies);
    virtual ~SkFontConfigInterfaceAndroid();

    virtual bool matchFamilyName(const char familyName[],
                                 SkTypeface::Style requested,
                                 FontIdentity* outFontIdentifier,
                                 SkString* outFamilyName,
                                 SkTypeface::Style* outStyle) SK_OVERRIDE;
    virtual SkStream* openStream(const FontIdentity&) SK_OVERRIDE;

    // new APIs
    virtual SkDataTable* getFamilyNames() SK_OVERRIDE;
    virtual bool matchFamilySet(const char inFamilyName[],
                                SkString* outFamilyName,
                                SkTArray<FontIdentity>*) SK_OVERRIDE;

    /**
     *  Get the family name of the font in the default fallback font list that
     *  contains the specified chararacter. if no font is found, returns false.
     */
    bool getFallbackFamilyNameForChar(SkUnichar uni, const char* lang, SkString* name);
    /**
     *
     */
    SkTypeface* getTypefaceForChar(SkUnichar uni, SkTypeface::Style style,
                                   SkPaintOptionsAndroid::FontVariant fontVariant);
    SkTypeface* nextLogicalTypeface(SkFontID currFontID, SkFontID origFontID,
                                    const SkPaintOptionsAndroid& options);
    SkTypeface* getTypefaceForGlyphID(uint16_t glyphID, const SkTypeface* origTypeface,
                                      const SkPaintOptionsAndroid& options,
                                      int* lowerBounds, int* upperBounds);

private:
    void addFallbackFamily(FamilyRecID fontRecID);
    SkTypeface* getTypefaceForFontRec(FontRecID fontRecID);
    FallbackFontList* getCurrentLocaleFallbackFontList();
    FallbackFontList* findFallbackFontList(const SkLanguage& lang, bool isOriginal = true);

    SkTArray<FontRec> fFonts;
    SkTArray<FamilyRec> fFontFamilies;
    SkTDict<FamilyRecID> fFamilyNameDict;
    FamilyRecID fDefaultFamilyRecID;

    // (SkLanguage)<->(fallback chain index) translation
    SkTDict<FallbackFontList*> fFallbackFontDict;
    SkTDict<FallbackFontList*> fFallbackFontAliasDict;
    FallbackFontList fDefaultFallbackList;

    // fallback info for current locale
    SkString fCachedLocale;
    FallbackFontList* fLocaleFallbackFontList;
};

///////////////////////////////////////////////////////////////////////////////

static SkFontConfigInterfaceAndroid* getSingletonInterface() {
    SK_DECLARE_STATIC_MUTEX(gMutex);
    static SkFontConfigInterfaceAndroid* gFontConfigInterface;

    SkAutoMutexAcquire ac(gMutex);
    if (NULL == gFontConfigInterface) {
        // load info from a configuration file that we can use to populate the
        // system/fallback font structures
        SkTDArray<FontFamily*> fontFamilies;
        if (!gTestMainConfigFile) {
            SkFontConfigParser::GetFontFamilies(fontFamilies);
        } else {
            SkFontConfigParser::GetTestFontFamilies(fontFamilies, gTestMainConfigFile,
                                                    gTestFallbackConfigFile);
        }

        gFontConfigInterface = new SkFontConfigInterfaceAndroid(fontFamilies);

        // cleanup the data we received from the parser
        fontFamilies.deleteAll();
    }
    return gFontConfigInterface;
}

SkFontConfigInterface* SkFontConfigInterface::GetSingletonDirectInterface() {
    return getSingletonInterface();
}

///////////////////////////////////////////////////////////////////////////////

static bool has_font(const SkTArray<FontRec>& array, const SkString& filename) {
    for (int i = 0; i < array.count(); i++) {
        if (array[i].fFileName == filename) {
            return true;
        }
    }
    return false;
}

#ifndef SK_FONT_FILE_PREFIX
    #define SK_FONT_FILE_PREFIX          "/fonts/"
#endif

static void get_path_for_sys_fonts(SkString* full, const char name[]) {
    if (gTestFontFilePrefix) {
        full->set(gTestFontFilePrefix);
    } else {
        full->set(getenv("ANDROID_ROOT"));
        full->append(SK_FONT_FILE_PREFIX);
    }
    full->append(name);
}

static void insert_into_name_dict(SkTDict<FamilyRecID>& familyNameDict,
                                  const char* name, FamilyRecID familyRecID) {
    SkAutoAsciiToLC tolc(name);
    if (familyNameDict.find(tolc.lc())) {
        SkDebugf("---- system font attempting to use a the same name [%s] for"
                 "multiple families. skipping subsequent occurrences", tolc.lc());
    } else {
        familyNameDict.set(tolc.lc(), familyRecID);
    }
}

// Defined in SkFontHost_FreeType.cpp
bool find_name_and_attributes(SkStream* stream, SkString* name,
                              SkTypeface::Style* style, bool* isFixedWidth);

///////////////////////////////////////////////////////////////////////////////

SkFontConfigInterfaceAndroid::SkFontConfigInterfaceAndroid(SkTDArray<FontFamily*>& fontFamilies) :
        fFonts(fontFamilies.count()),
        fFontFamilies(fontFamilies.count() / FamilyRec::FONT_STYLE_COUNT),
        fFamilyNameDict(1024),
        fDefaultFamilyRecID(INVALID_FAMILY_REC_ID),
        fFallbackFontDict(128),
        fFallbackFontAliasDict(128),
        fLocaleFallbackFontList(NULL) {

    for (int i = 0; i < fontFamilies.count(); ++i) {
        FontFamily* family = fontFamilies[i];

        // defer initializing the familyRec until we can be sure that at least
        // one of it's children contains a valid font file
        FamilyRec* familyRec = NULL;
        FamilyRecID familyRecID = INVALID_FAMILY_REC_ID;

        for (int j = 0; j < family->fFontFiles.count(); ++j) {
            SkString filename;
            get_path_for_sys_fonts(&filename, family->fFontFiles[j]->fFileName);

            if (has_font(fFonts, filename)) {
                SkDebugf("---- system font and fallback font files specify a duplicate "
                        "font %s, skipping the second occurrence", filename.c_str());
                continue;
            }

            FontRec& fontRec = fFonts.push_back();
            fontRec.fFileName = filename;
            fontRec.fStyle = SkTypeface::kNormal;
            fontRec.fIsValid = false;
            fontRec.fFamilyRecID = familyRecID;

            const FontRecID fontRecID = fFonts.count() - 1;

            SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(filename.c_str()));
            if (stream.get() != NULL) {
                bool isFixedWidth;
                SkString name;
                fontRec.fIsValid = find_name_and_attributes(stream.get(), &name,
                                                            &fontRec.fStyle, &isFixedWidth);
            } else {
                if (!family->fIsFallbackFont) {
                    SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
                }
            }

            if (fontRec.fIsValid) {
                DEBUG_FONT(("---- SystemFonts[%d][%d] fallback=%d file=%s",
                           i, fFonts.count() - 1, family->fIsFallbackFont, filename.c_str()));
            } else {
                DEBUG_FONT(("---- SystemFonts[%d][%d] fallback=%d file=%s (INVALID)",
                           i, fFonts.count() - 1, family->fIsFallbackFont, filename.c_str()));
                continue;
            }

            // create a familyRec now that we know that at least one font in
            // the family is valid
            if (familyRec == NULL) {
                familyRec = &fFontFamilies.push_back();
                familyRecID = fFontFamilies.count() - 1;
                fontRec.fFamilyRecID = familyRecID;

                familyRec->fIsFallbackFont = family->fIsFallbackFont;
                familyRec->fPaintOptions = family->fFontFiles[j]->fPaintOptions;

            } else if (familyRec->fPaintOptions != family->fFontFiles[j]->fPaintOptions) {
                SkDebugf("Every font file within a family must have identical"
                         "language and variant attributes");
                sk_throw();
            }

            // add this font to the current familyRec
            if (INVALID_FONT_REC_ID != familyRec->fFontRecID[fontRec.fStyle]) {
                DEBUG_FONT(("Overwriting familyRec for style[%d] old,new:(%d,%d)",
                            fontRec.fStyle, familyRec->fFontRecID[fontRec.fStyle],
                            fontRecID));
            }
            familyRec->fFontRecID[fontRec.fStyle] = fontRecID;
        }

        if (familyRec) {
            if (familyRec->fIsFallbackFont) {
                // add the font to the appropriate fallback chains and also insert a
                // unique name into the familyNameDict for internal usage
                addFallbackFamily(familyRecID);
            } else {
                // add the names that map to this family to the dictionary for easy lookup
                const SkTDArray<const char*>& names = family->fNames;
                if (names.isEmpty()) {
                    SkDEBUGFAIL("ERROR: non-fallback font with no name");
                    continue;
                }

                for (int i = 0; i < names.count(); i++) {
                    insert_into_name_dict(fFamilyNameDict, names[i], familyRecID);
                }
            }
        }
    }

    DEBUG_FONT(("---- We have %d system fonts", fFonts.count()));

    if (fFontFamilies.count() > 0) {
        fDefaultFamilyRecID = 0;
    }

    // scans the default fallback font chain, adding every entry to every other
    // fallback font chain to which it does not belong. this results in every
    // language-specific fallback font chain having all of its fallback fonts at
    // the front of the chain, and everything else at the end.
    FallbackFontList* fallbackList;
    SkTDict<FallbackFontList*>::Iter iter(fFallbackFontDict);
    const char* fallbackLang = iter.next(&fallbackList);
    while(fallbackLang != NULL) {
        for (int i = 0; i < fDefaultFallbackList.count(); i++) {
            FamilyRecID familyRecID = fDefaultFallbackList[i];
            const SkString& fontLang = fFontFamilies[familyRecID].fPaintOptions.getLanguage().getTag();
            if (strcmp(fallbackLang, fontLang.c_str()) != 0) {
                fallbackList->push(familyRecID);
            }
        }
        // move to the next fallback list in the dictionary
        fallbackLang = iter.next(&fallbackList);
    }
}

SkFontConfigInterfaceAndroid::~SkFontConfigInterfaceAndroid() {
    // iterate through and cleanup fFallbackFontDict
    SkTDict<FallbackFontList*>::Iter iter(fFallbackFontDict);
    FallbackFontList* fallbackList;
    while(iter.next(&fallbackList) != NULL) {
        SkDELETE(fallbackList);
    }
}

void SkFontConfigInterfaceAndroid::addFallbackFamily(FamilyRecID familyRecID) {
    SkASSERT(familyRecID < fFontFamilies.count());
    FamilyRec& familyRec = fFontFamilies[familyRecID];
    SkASSERT(familyRec.fIsFallbackFont);

    // add the fallback family to the name dictionary.  This is
    // needed by getFallbackFamilyNameForChar() so that fallback
    // families can be identified by a unique name. The unique
    // identifier that we've chosen is the familyID in hex (e.g. '0F##fallback').
    familyRec.fFallbackName.printf("%.2x##fallback", familyRecID);
    insert_into_name_dict(fFamilyNameDict, familyRec.fFallbackName.c_str(), familyRecID);

    // add to the default fallback list
    fDefaultFallbackList.push(familyRecID);

    // stop here if it is the default language tag
    const SkString& languageTag = familyRec.fPaintOptions.getLanguage().getTag();
    if (languageTag.isEmpty()) {
        return;
    }

    // add to the appropriate language's custom fallback list
    FallbackFontList* customList = NULL;
    if (!fFallbackFontDict.find(languageTag.c_str(), &customList)) {
        DEBUG_FONT(("----  Created fallback list for \"%s\"", languageTag.c_str()));
        customList = SkNEW(FallbackFontList);
        fFallbackFontDict.set(languageTag.c_str(), customList);
    }
    SkASSERT(customList != NULL);
    customList->push(familyRecID);
}


static FontRecID find_best_style(const FamilyRec& family, SkTypeface::Style style) {

    const FontRecID* fontRecIDs = family.fFontRecID;

    if (fontRecIDs[style] != INVALID_FONT_REC_ID) { // exact match
        return fontRecIDs[style];
    }
    // look for a matching bold
    style = (SkTypeface::Style)(style ^ SkTypeface::kItalic);
    if (fontRecIDs[style] != INVALID_FONT_REC_ID) {
        return fontRecIDs[style];
    }
    // look for the plain
    if (fontRecIDs[SkTypeface::kNormal] != INVALID_FONT_REC_ID) {
        return fontRecIDs[SkTypeface::kNormal];
    }
    // look for anything
    for (int i = 0; i < FamilyRec::FONT_STYLE_COUNT; i++) {
        if (fontRecIDs[i] != INVALID_FONT_REC_ID) {
            return fontRecIDs[i];
        }
    }
    // should never get here, since the fontRecID list should not be empty
    SkDEBUGFAIL("No valid fonts exist for this family");
    return -1;
}

bool SkFontConfigInterfaceAndroid::matchFamilyName(const char familyName[],
                                                   SkTypeface::Style style,
                                                   FontIdentity* outFontIdentifier,
                                                   SkString* outFamilyName,
                                                   SkTypeface::Style* outStyle) {
    // clip to legal style bits
    style = (SkTypeface::Style)(style & SkTypeface::kBoldItalic);

    bool exactNameMatch = false;

    FamilyRecID familyRecID = INVALID_FAMILY_REC_ID;
    if (NULL != familyName) {
        SkAutoAsciiToLC tolc(familyName);
        if (fFamilyNameDict.find(tolc.lc(), &familyRecID)) {
            exactNameMatch = true;
        }
    } else {
        familyRecID = fDefaultFamilyRecID;

    }

    // If no matching family name is found then return false. This allows clients
    // to be able to search for other fonts instead of forcing them to use the
    // default font.
    if (INVALID_FAMILY_REC_ID == familyRecID) {
        return false;
    }

    FontRecID fontRecID = find_best_style(fFontFamilies[familyRecID], style);
    FontRec& fontRec = fFonts[fontRecID];

    if (NULL != outFontIdentifier) {
        outFontIdentifier->fID = fontRecID;
        outFontIdentifier->fTTCIndex = 0;
        outFontIdentifier->fString.set(fontRec.fFileName);
//        outFontIdentifier->fStyle = fontRec.fStyle;
    }

    if (NULL != outFamilyName) {
        if (exactNameMatch) {
            outFamilyName->set(familyName);
        } else {
            // find familyName from list of names
            const char* familyName = NULL;
            SkAssertResult(fFamilyNameDict.findKey(familyRecID, &familyName));
            SkASSERT(familyName);
            outFamilyName->set(familyName);
        }
    }

    if (NULL != outStyle) {
        *outStyle = fontRec.fStyle;
    }

    return true;
}

SkStream* SkFontConfigInterfaceAndroid::openStream(const FontIdentity& identity) {
    return SkStream::NewFromFile(identity.fString.c_str());
}

SkDataTable* SkFontConfigInterfaceAndroid::getFamilyNames() {
    SkTDArray<const char*> names;
    SkTDArray<size_t> sizes;

    SkTDict<FamilyRecID>::Iter iter(fFamilyNameDict);
    const char* familyName = iter.next(NULL);
    while(familyName != NULL) {
        *names.append() = familyName;
        *sizes.append() = strlen(familyName) + 1;

        // move to the next familyName in the dictionary
        familyName = iter.next(NULL);
    }

    return SkDataTable::NewCopyArrays((const void*const*)names.begin(),
                                      sizes.begin(), names.count());
}

bool SkFontConfigInterfaceAndroid::matchFamilySet(const char inFamilyName[],
                                                  SkString* outFamilyName,
                                                  SkTArray<FontIdentity>*) {
    return false;
}

static bool find_proc(SkTypeface* face, SkTypeface::Style style, void* ctx) {
    const FontRecID* fontRecID = (const FontRecID*)ctx;
    FontRecID currFontRecID = ((FontConfigTypeface*)face)->getIdentity().fID;
    return currFontRecID == *fontRecID;
}

SkTypeface* SkFontConfigInterfaceAndroid::getTypefaceForFontRec(FontRecID fontRecID) {
    FontRec& fontRec = fFonts[fontRecID];
    SkTypeface* face = fontRec.fTypeface.get();
    if (!face) {
        // look for it in the typeface cache
        face = SkTypefaceCache::FindByProcAndRef(find_proc, &fontRecID);

        // if it is not in the cache then create it
        if (!face) {
            const char* familyName = NULL;
            SkAssertResult(fFamilyNameDict.findKey(fontRec.fFamilyRecID, &familyName));
            SkASSERT(familyName);
            face = SkTypeface::CreateFromName(familyName, fontRec.fStyle);
        }

        // store the result for subsequent lookups
        fontRec.fTypeface = face;
    }
    SkASSERT(face);
    return face;
}

bool SkFontConfigInterfaceAndroid::getFallbackFamilyNameForChar(SkUnichar uni,
                                                                const char* lang,
                                                                SkString* name) {
    FallbackFontList* fallbackFontList = NULL;
    const SkString langTag(lang);
    if (langTag.isEmpty()) {
        fallbackFontList = this->getCurrentLocaleFallbackFontList();
    } else {
        fallbackFontList = this->findFallbackFontList(langTag);
    }

    for (int i = 0; i < fallbackFontList->count(); i++) {
        FamilyRecID familyRecID = fallbackFontList->getAt(i);

        // if it is not one of the accepted variants then move to the next family
        int32_t acceptedVariants = SkPaintOptionsAndroid::kDefault_Variant |
                                   SkPaintOptionsAndroid::kElegant_Variant;
        if (!(fFontFamilies[familyRecID].fPaintOptions.getFontVariant() & acceptedVariants)) {
            continue;
        }

        FontRecID fontRecID = find_best_style(fFontFamilies[familyRecID], SkTypeface::kNormal);
        SkTypeface* face = this->getTypefaceForFontRec(fontRecID);

        SkPaint paint;
        paint.setTypeface(face);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        uint16_t glyphID;
        paint.textToGlyphs(&uni, sizeof(uni), &glyphID);
        if (glyphID != 0) {
            name->set(fFontFamilies[familyRecID].fFallbackName);
            return true;
        }
    }
    return false;
}

SkTypeface* SkFontConfigInterfaceAndroid::getTypefaceForChar(SkUnichar uni,
                                                             SkTypeface::Style style,
                                                             SkPaintOptionsAndroid::FontVariant fontVariant) {
    FontRecID fontRecID = find_best_style(fFontFamilies[fDefaultFamilyRecID], style);
    SkTypeface* face = this->getTypefaceForFontRec(fontRecID);

    SkPaintOptionsAndroid paintOptions;
    paintOptions.setFontVariant(fontVariant);
    paintOptions.setUseFontFallbacks(true);

    SkPaint paint;
    paint.setTypeface(face);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    paint.setPaintOptionsAndroid(paintOptions);

    SkAutoGlyphCache autoCache(paint, NULL, NULL);
    SkGlyphCache*    cache = autoCache.getCache();

    SkScalerContext* ctx = cache->getScalerContext();
    if (ctx) {
        SkFontID fontID = ctx->findTypefaceIdForChar(uni);
        return SkTypefaceCache::FindByID(fontID);
    }
    return NULL;
}

FallbackFontList* SkFontConfigInterfaceAndroid::getCurrentLocaleFallbackFontList() {
    SkString locale = SkFontConfigParser::GetLocale();
    if (NULL == fLocaleFallbackFontList || locale != fCachedLocale) {
        fCachedLocale = locale;
        fLocaleFallbackFontList = this->findFallbackFontList(locale);
    }
    return fLocaleFallbackFontList;
}

FallbackFontList* SkFontConfigInterfaceAndroid::findFallbackFontList(const SkLanguage& lang,
                                                                     bool isOriginal) {
    const SkString& langTag = lang.getTag();
    if (langTag.isEmpty()) {
        return &fDefaultFallbackList;
    }

    FallbackFontList* fallbackFontList;
    if (fFallbackFontDict.find(langTag.c_str(), langTag.size(), &fallbackFontList) ||
        fFallbackFontAliasDict.find(langTag.c_str(), langTag.size(), &fallbackFontList)) {
        return fallbackFontList;
    }

    // attempt a recursive fuzzy match
    SkLanguage parent = lang.getParent();
    fallbackFontList = findFallbackFontList(parent, false);

    // cache the original lang so we don't have to do the recursion again.
    if (isOriginal) {
        DEBUG_FONT(("----  Created fallback list alias for \"%s\"", langTag.c_str()));
        fFallbackFontAliasDict.set(langTag.c_str(), fallbackFontList);
    }
    return fallbackFontList;
}

SkTypeface* SkFontConfigInterfaceAndroid::nextLogicalTypeface(SkFontID currFontID,
                                                              SkFontID origFontID,
                                                              const SkPaintOptionsAndroid& opts) {
    // Skia does not support font fallback by default. This enables clients such
    // as WebKit to customize their font selection. In any case, clients can use
    // GetFallbackFamilyNameForChar() to get the fallback font for individual
    // characters.
    if (!opts.isUsingFontFallbacks()) {
        return NULL;
    }

    FallbackFontList* currentFallbackList = findFallbackFontList(opts.getLanguage());
    SkASSERT(currentFallbackList);

    SkTypeface::Style origStyle = SkTypeface::kNormal;
    const SkTypeface* origTypeface = SkTypefaceCache::FindByID(origFontID);
    if (NULL != origTypeface) {
        origStyle = origTypeface->style();
    }

    // we must convert currTypeface into a FontRecID
    FontRecID currFontRecID = INVALID_FONT_REC_ID;
    const SkTypeface* currTypeface = SkTypefaceCache::FindByID(currFontID);
    // non-system fonts are not in the font cache so if we are asked to fallback
    // for a non-system font we will start at the front of the chain.
    if (NULL != currTypeface) {
        currFontRecID = ((FontConfigTypeface*)currTypeface)->getIdentity().fID;
        SkASSERT(INVALID_FONT_REC_ID != currFontRecID);
    }

    FamilyRecID currFamilyRecID = INVALID_FAMILY_REC_ID;
    if (INVALID_FONT_REC_ID != currFontRecID) {
        currFamilyRecID = fFonts[currFontRecID].fFamilyRecID;
    }

    // lookup the index next font in the chain
    int currFallbackFontIndex = currentFallbackList->find(currFamilyRecID);
    // We add 1 to the returned index for 2 reasons: (1) if find succeeds it moves
    // our index to the next entry in the list; (2) if find() fails it returns
    // -1 and incrementing it will set our starting index to 0 (the head of the list)
    int nextFallbackFontIndex = currFallbackFontIndex + 1;

    if(nextFallbackFontIndex >= currentFallbackList->count()) {
        return NULL;
    }

    // If a rec object is set to prefer "kDefault_Variant" it means they have no preference
    // In this case, we set the value to "kCompact_Variant"
    SkPaintOptionsAndroid::FontVariant variant = opts.getFontVariant();
    if (variant == SkPaintOptionsAndroid::kDefault_Variant) {
        variant = SkPaintOptionsAndroid::kCompact_Variant;
    }

    int32_t acceptedVariants = SkPaintOptionsAndroid::kDefault_Variant | variant;

    SkTypeface* nextLogicalTypeface = 0;
    while (nextFallbackFontIndex < currentFallbackList->count()) {
        FamilyRecID familyRecID = currentFallbackList->getAt(nextFallbackFontIndex);
        if ((fFontFamilies[familyRecID].fPaintOptions.getFontVariant() & acceptedVariants) != 0) {
            FontRecID matchedFont = find_best_style(fFontFamilies[familyRecID], origStyle);
            nextLogicalTypeface = this->getTypefaceForFontRec(matchedFont);
            break;
        }
        nextFallbackFontIndex++;
    }

    DEBUG_FONT(("---- nextLogicalFont: currFontID=%d, origFontID=%d, currRecID=%d, "
                "lang=%s, variant=%d, nextFallbackIndex[%d,%d] => nextLogicalTypeface=%d",
                currFontID, origFontID, currFontRecID, opts.getLanguage().getTag().c_str(),
                variant, nextFallbackFontIndex, currentFallbackList->getAt(nextFallbackFontIndex),
                (nextLogicalTypeface) ? nextLogicalTypeface->uniqueID() : 0));
    return SkSafeRef(nextLogicalTypeface);
}

SkTypeface* SkFontConfigInterfaceAndroid::getTypefaceForGlyphID(uint16_t glyphID,
                                                                const SkTypeface* origTypeface,
                                                                const SkPaintOptionsAndroid& opts,
                                                                int* lBounds, int* uBounds) {
    // If we aren't using fallbacks then we shouldn't be calling this
    SkASSERT(opts.isUsingFontFallbacks());
    SkASSERT(origTypeface);

    SkTypeface* currentTypeface = NULL;
    int lowerBounds = 0; //inclusive
    int upperBounds = origTypeface->countGlyphs(); //exclusive

    // check to see if the glyph is in the bounds of the origTypeface
    if (glyphID < upperBounds) {
        currentTypeface = const_cast<SkTypeface*>(origTypeface);
    } else {
        FallbackFontList* currentFallbackList = findFallbackFontList(opts.getLanguage());
        SkASSERT(currentFallbackList);

        // If an object is set to prefer "kDefault_Variant" it means they have no preference
        // In this case, we set the value to "kCompact_Variant"
        SkPaintOptionsAndroid::FontVariant variant = opts.getFontVariant();
        if (variant == SkPaintOptionsAndroid::kDefault_Variant) {
            variant = SkPaintOptionsAndroid::kCompact_Variant;
        }

        int32_t acceptedVariants = SkPaintOptionsAndroid::kDefault_Variant | variant;
        SkTypeface::Style origStyle = origTypeface->style();

        for (int x = 0; x < currentFallbackList->count(); ++x) {
            const FamilyRecID familyRecID = currentFallbackList->getAt(x);
            const SkPaintOptionsAndroid& familyOptions = fFontFamilies[familyRecID].fPaintOptions;
            if ((familyOptions.getFontVariant() & acceptedVariants) != 0) {
                FontRecID matchedFont = find_best_style(fFontFamilies[familyRecID], origStyle);
                currentTypeface = this->getTypefaceForFontRec(matchedFont);
                lowerBounds = upperBounds;
                upperBounds += currentTypeface->countGlyphs();
                if (glyphID < upperBounds) {
                    break;
                }
            }
        }
    }

    if (NULL != currentTypeface) {
        if (lBounds) {
            *lBounds = lowerBounds;
        }
        if (uBounds) {
            *uBounds = upperBounds;
        }
    }
    return currentTypeface;
}

///////////////////////////////////////////////////////////////////////////////

bool SkGetFallbackFamilyNameForChar(SkUnichar uni, SkString* name) {
    SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
    return fontConfig->getFallbackFamilyNameForChar(uni, NULL, name);
}

bool SkGetFallbackFamilyNameForChar(SkUnichar uni, const char* lang, SkString* name) {
    SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
    return fontConfig->getFallbackFamilyNameForChar(uni, lang, name);
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

SkTypeface* SkAndroidNextLogicalTypeface(SkFontID currFontID, SkFontID origFontID,
                                         const SkPaintOptionsAndroid& options) {
    SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
    return fontConfig->nextLogicalTypeface(currFontID, origFontID, options);

}

SkTypeface* SkGetTypefaceForGlyphID(uint16_t glyphID, const SkTypeface* origTypeface,
                                    const SkPaintOptionsAndroid& options,
                                    int* lowerBounds, int* upperBounds) {
    SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
    return fontConfig->getTypefaceForGlyphID(glyphID, origTypeface, options,
                                             lowerBounds, upperBounds);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

struct HB_UnicodeMapping {
    hb_script_t script;
    const SkUnichar unicode;
};

/*
 * The following scripts are not complex fonts and we do not expect them to be parsed by this table
 * HB_SCRIPT_COMMON,
 * HB_SCRIPT_GREEK,
 * HB_SCRIPT_CYRILLIC,
 * HB_SCRIPT_HANGUL
 * HB_SCRIPT_INHERITED
 */

/* Harfbuzz (old) is missing a number of scripts in its table. For these,
 * we include a value which can never happen. We won't get complex script
 * shaping in these cases, but the library wouldn't know how to shape
 * them anyway. */
#define HB_Script_Unknown HB_ScriptCount

static HB_UnicodeMapping HB_UnicodeMappingArray[] = {
    {HB_SCRIPT_ARMENIAN,    0x0531},
    {HB_SCRIPT_HEBREW,      0x0591},
    {HB_SCRIPT_ARABIC,      0x0600},
    {HB_SCRIPT_SYRIAC,      0x0710},
    {HB_SCRIPT_THAANA,      0x0780},
    {HB_SCRIPT_NKO,         0x07C0},
    {HB_SCRIPT_DEVANAGARI,  0x0901},
    {HB_SCRIPT_BENGALI,     0x0981},
    {HB_SCRIPT_GURMUKHI,    0x0A10},
    {HB_SCRIPT_GUJARATI,    0x0A90},
    {HB_SCRIPT_ORIYA,       0x0B10},
    {HB_SCRIPT_TAMIL,       0x0B82},
    {HB_SCRIPT_TELUGU,      0x0C10},
    {HB_SCRIPT_KANNADA,     0x0C90},
    {HB_SCRIPT_MALAYALAM,   0x0D10},
    {HB_SCRIPT_SINHALA,     0x0D90},
    {HB_SCRIPT_THAI,        0x0E01},
    {HB_SCRIPT_LAO,         0x0E81},
    {HB_SCRIPT_TIBETAN,     0x0F00},
    {HB_SCRIPT_MYANMAR,     0x1000},
    {HB_SCRIPT_GEORGIAN,    0x10A0},
    {HB_SCRIPT_ETHIOPIC,    0x1200},
    {HB_SCRIPT_CHEROKEE,    0x13A0},
    {HB_SCRIPT_OGHAM,       0x1680},
    {HB_SCRIPT_RUNIC,       0x16A0},
    {HB_SCRIPT_KHMER,       0x1780},
    {HB_SCRIPT_TAI_LE,      0x1950},
    {HB_SCRIPT_NEW_TAI_LUE, 0x1980},
    {HB_SCRIPT_TAI_THAM,    0x1A20},
    {HB_SCRIPT_CHAM,        0xAA00},
};

// returns 0 for "Not Found"
static SkUnichar getUnicodeFromHBScript(hb_script_t script) {
    SkUnichar unichar = 0;
    int numSupportedFonts = sizeof(HB_UnicodeMappingArray) / sizeof(HB_UnicodeMapping);
    for (int i = 0; i < numSupportedFonts; i++) {
        if (script == HB_UnicodeMappingArray[i].script) {
            unichar = HB_UnicodeMappingArray[i].unicode;
            break;
        }
    }
    return unichar;
}

struct TypefaceLookupStruct {
    hb_script_t script;
    SkTypeface::Style style;
    SkPaintOptionsAndroid::FontVariant fontVariant;
    SkTypeface* typeface;
};

SK_DECLARE_STATIC_MUTEX(gTypefaceTableMutex);  // This is the mutex for gTypefaceTable
static SkTDArray<TypefaceLookupStruct> gTypefaceTable;  // This is protected by gTypefaceTableMutex

static int typefaceLookupCompare(const TypefaceLookupStruct& first,
                                 const TypefaceLookupStruct& second) {
    if (first.script != second.script) {
        return (first.script > second.script) ? 1 : -1;
    }
    if (first.style != second.style) {
        return (first.style > second.style) ? 1 : -1;
    }
    if (first.fontVariant != second.fontVariant) {
        return (first.fontVariant > second.fontVariant) ? 1 : -1;
    }
    return 0;
}

SkTypeface* SkCreateTypefaceForScript(hb_script_t script, SkTypeface::Style style,
                                      SkPaintOptionsAndroid::FontVariant fontVariant) {
    SkAutoMutexAcquire ac(gTypefaceTableMutex);

    TypefaceLookupStruct key;
    key.script = script;
    key.style = style;
    key.fontVariant = fontVariant;

    int index = SkTSearch<TypefaceLookupStruct>(
            (const TypefaceLookupStruct*) gTypefaceTable.begin(),
            gTypefaceTable.count(), key, sizeof(TypefaceLookupStruct),
            typefaceLookupCompare);

    SkTypeface* retTypeface = NULL;
    if (index >= 0) {
        retTypeface = gTypefaceTable[index].typeface;
    }
    else {
        SkUnichar unichar = getUnicodeFromHBScript(script);
        if (!unichar) {
            return NULL;
        }

        SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
        retTypeface = fontConfig->getTypefaceForChar(unichar, style, fontVariant);

        // add to the lookup table
        key.typeface = retTypeface;
        *gTypefaceTable.insert(~index) = key;
    }

    // we ref(), the caller is expected to unref when they are done
    return SkSafeRef(retTypeface);
}

#endif

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() {
    return NULL;
}
