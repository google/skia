
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
#include "SkPaintOptionsAndroid.h"
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

// used to record our notion of the pre-existing fonts
struct FontRec {
    SkRefPtr<SkTypeface> fTypeface;
    SkString fFileName;
    SkTypeface::Style fStyle;
    SkPaintOptionsAndroid fPaintOptions;
    bool fIsFallbackFont;
    bool fIsValid;
};

typedef int32_t FontRecID;
#define INVALID_FONT_REC_ID -1

struct FamilyRec {
    FamilyRec() {
        memset(fFontRecID, INVALID_FONT_REC_ID, sizeof(fFontRecID));
    }

    static const int FONT_STYLE_COUNT = 4;
    FontRecID fFontRecID[FONT_STYLE_COUNT];
};

typedef int32_t FamilyRecID;
#define INVALID_FAMILY_REC_ID -1

typedef SkTDArray<FontRecID> FallbackFontList;

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
    bool getFallbackFamilyNameForChar(SkUnichar uni, SkString* name);
    /**
     *
     */
    SkTypeface* getTypefaceForChar(SkUnichar uni, SkTypeface::Style style,
                                   SkPaintOptionsAndroid::FontVariant fontVariant);
    SkTypeface* nextLogicalTypeface(SkFontID currFontID, SkFontID origFontID,
                                    const SkPaintOptionsAndroid& options);

private:
    void addFallbackFont(FontRecID fontRecID);
    FallbackFontList* findFallbackFontList(const SkLanguage& lang);

    SkTArray<FontRec> fFonts;
    SkTArray<FamilyRec> fFontFamilies;
    SkTDict<FamilyRecID> fFamilyNameDict;
    FamilyRecID fDefaultFamilyRecID;

    // (SkLanguage)<->(fallback chain index) translation
    SkTDict<FallbackFontList*> fFallbackFontDict;
    FallbackFontList fDefaultFallbackList;
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
    familyNameDict.set(tolc.lc(), familyRecID);
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
        fFallbackFontDict(128) {

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
            fontRec.fPaintOptions = family->fFontFiles[j]->fPaintOptions;
            fontRec.fIsFallbackFont = family->fIsFallbackFont;
            fontRec.fIsValid = false;

            const FontRecID fontRecID = fFonts.count() - 1;

            SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(filename.c_str()));
            if (stream.get() != NULL) {
                bool isFixedWidth;
                SkString name;
                fontRec.fIsValid = find_name_and_attributes(stream.get(), &name,
                                                            &fontRec.fStyle, &isFixedWidth);
            } else {
                if (!fontRec.fIsFallbackFont) {
                    SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
                }
            }

            if (fontRec.fIsValid) {
                DEBUG_FONT(("---- SystemFonts[%d][%d] fallback=%d file=%s",
                           i, fFonts.count() - 1, fontRec.fIsFallbackFont, filename.c_str()));
            } else {
                DEBUG_FONT(("---- SystemFonts[%d][%d] fallback=%d file=%s (INVALID)",
                           i, fFonts.count() - 1, fontRec.fIsFallbackFont, filename.c_str()));
                continue;
            }

            // create a familyRec now that we know that at least one font in
            // the family is valid
            if (familyRec == NULL) {
                familyRec = &fFontFamilies.push_back();
                familyRecID = fFontFamilies.count() - 1;
            }

            // add this font to the current familyRec
            if (INVALID_FONT_REC_ID != familyRec->fFontRecID[fontRec.fStyle]) {
                DEBUG_FONT(("Overwriting familyRec for style[%d] old,new:(%d,%d)",
                            fontRec.fStyle, familyRec->fFontRecID[fontRec.fStyle],
                            fontRecID));
            }
            familyRec->fFontRecID[fontRec.fStyle] = fontRecID;

            // if this is a fallback font then add it to the appropriate fallback chains
            if (fontRec.fIsFallbackFont) {
                addFallbackFont(fontRecID);
            }

            // add the fallback file name to the name dictionary.  This is needed
            // by getFallbackFamilyNameForChar() so that fallback families can be
            // requested by the filenames of the fonts they contain.
            if (family->fIsFallbackFont && familyRec) {
                insert_into_name_dict(fFamilyNameDict, fontRec.fFileName.c_str(), familyRecID);
            }
        }

        // add the names that map to this family to the dictionary for easy lookup
        if (familyRec && !family->fIsFallbackFont) {
            SkTDArray<const char*> names = family->fNames;
            if (names.isEmpty()) {
                SkDEBUGFAIL("ERROR: non-fallback font with no name");
                continue;
            }

            for (int i = 0; i < names.count(); i++) {
                insert_into_name_dict(fFamilyNameDict, names[i], familyRecID);
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
            FontRecID fontRecID = fDefaultFallbackList[i];
            const SkString& fontLang = fFonts[fontRecID].fPaintOptions.getLanguage().getTag();
            if (strcmp(fallbackLang, fontLang.c_str()) != 0) {
                fallbackList->push(fontRecID);
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

void SkFontConfigInterfaceAndroid::addFallbackFont(FontRecID fontRecID) {
    SkASSERT(fontRecID < fFonts.count());
    const FontRec& fontRec = fFonts[fontRecID];
    SkASSERT(fontRec.fIsFallbackFont);

    // add to the default fallback list
    fDefaultFallbackList.push(fontRecID);

    // stop here if it is the default language tag
    const SkString& languageTag = fontRec.fPaintOptions.getLanguage().getTag();
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
    customList->push(fontRecID);
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
                                                   SkTypeface::Style* outStyle) SK_OVERRIDE {
    // clip to legal style bits
    style = (SkTypeface::Style)(style & SkTypeface::kBoldItalic);

    bool exactNameMatch = false;

    FamilyRecID familyRecID = INVALID_FAMILY_REC_ID;
    if (NULL != familyName) {
        if (fFamilyNameDict.find(familyName, &familyRecID)) {
            exactNameMatch = true;
        }
    } else {
        familyRecID = fDefaultFamilyRecID;

    }

    if (INVALID_FAMILY_REC_ID == familyRecID) {
        //TODO this ensures that we always return something
        familyRecID = fDefaultFamilyRecID;
        //return false;
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

SkStream* SkFontConfigInterfaceAndroid::openStream(const FontIdentity& identity) SK_OVERRIDE {
    return SkStream::NewFromFile(identity.fString.c_str());
}

SkDataTable* SkFontConfigInterfaceAndroid::getFamilyNames() SK_OVERRIDE {
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
                                                  SkTArray<FontIdentity>*) SK_OVERRIDE {
    return false;
}

static SkTypeface* get_typeface_for_rec(FontRec& fontRec) {
    SkTypeface* face = fontRec.fTypeface.get();
    if (!face) {
        // TODO look for it in the typeface cache

        // if it is not in the cache then create it
        face = SkTypeface::CreateFromFile(fontRec.fFileName.c_str());

        // store the result for subsequent lookups
        fontRec.fTypeface = face;
    }
    SkASSERT(face);
    return face;
}

bool SkFontConfigInterfaceAndroid::getFallbackFamilyNameForChar(SkUnichar uni, SkString* name) {
    for (int i = 0; i < fDefaultFallbackList.count(); i++) {
        FontRecID fontRecID = fDefaultFallbackList[i];
        SkTypeface* face = get_typeface_for_rec(fFonts[fontRecID]);

        SkPaint paint;
        paint.setTypeface(face);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        uint16_t glyphID;
        paint.textToGlyphs(&uni, sizeof(uni), &glyphID);
        if (glyphID != 0) {
            name->set(fFonts[fontRecID].fFileName);
            return true;
        }
    }
    return false;
}

SkTypeface* SkFontConfigInterfaceAndroid::getTypefaceForChar(SkUnichar uni,
                                                             SkTypeface::Style style,
                                                             SkPaintOptionsAndroid::FontVariant fontVariant) {
    FontRecID fontRecID = find_best_style(fFontFamilies[fDefaultFamilyRecID], style);
    SkTypeface* face = get_typeface_for_rec(fFonts[fontRecID]);

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

FallbackFontList* SkFontConfigInterfaceAndroid::findFallbackFontList(const SkLanguage& lang) {
    const SkString& langTag = lang.getTag();
    if (langTag.isEmpty()) {
        return &fDefaultFallbackList;
    }

    FallbackFontList* fallbackFontList;
    if (fFallbackFontDict.find(langTag.c_str(), langTag.size(), &fallbackFontList)) {
        return fallbackFontList;
    }

    // attempt a recursive fuzzy match
    // TODO we could cache the intermediate parent so that we don't have to do
    //      the recursion again.
    SkLanguage parent = lang.getParent();
    return findFallbackFontList(parent);
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

    const SkTypeface* currTypeface = SkTypefaceCache::FindByID(currFontID);

    FallbackFontList* currentFallbackList = findFallbackFontList(opts.getLanguage());
    SkASSERT(currentFallbackList);

    SkASSERT(currTypeface != 0);

    // we must convert currTypeface into a FontRecID
    FontRecID currFontRecID = ((FontConfigTypeface*)currTypeface)->getIdentity().fID;

    // TODO lookup the index next font in the chain
    int currFallbackFontIndex = currentFallbackList->find(currFontRecID);
    int nextFallbackFontIndex = currFallbackFontIndex + 1;
    SkASSERT(-1 == nextFallbackFontIndex);

    if(nextFallbackFontIndex >= currentFallbackList->count() && -1 == currFallbackFontIndex) {
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
        FontRecID fontRecID = currentFallbackList->getAt(nextFallbackFontIndex);
        if (fFonts[fontRecID].fPaintOptions.getFontVariant() & acceptedVariants) {
            nextLogicalTypeface = get_typeface_for_rec(fFonts[fontRecID]);
            break;
        }
        nextFallbackFontIndex++;
    }

    DEBUG_FONT(("---- nextLogicalFont: currFontID=%d, origFontID=%d, currRecID=%d, "
                "lang=%s, variant=%d, nextFallbackIndex=%d => nextLogicalTypeface=%d",
                currFontID, origFontID, currFontRecID, opts.getLanguage().getTag().c_str(),
                variant, nextFallbackFontIndex,
                (nextLogicalTypeface) ? nextLogicalTypeface->uniqueID() : 0));
    return SkSafeRef(nextLogicalTypeface);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGetFallbackFamilyNameForChar(SkUnichar uni, SkString* name) {
    SkFontConfigInterfaceAndroid* fontConfig = getSingletonInterface();
    return fontConfig->getFallbackFamilyNameForChar(uni, name);
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

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() {
    return NULL;
}
