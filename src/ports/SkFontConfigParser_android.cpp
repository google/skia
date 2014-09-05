/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigParser_android.h"
#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTypeface.h"

#include <expat.h>
#include <dirent.h>
#include <stdio.h>

#include <limits>



// From Android version LMP onwards, all font files collapse into
// /system/etc/fonts.xml. Instead of trying to detect which version
// we're on, try to open fonts.xml; if that fails, fall back to the
// older filename.
#define LMP_SYSTEM_FONTS_FILE "/system/etc/fonts.xml"
#define OLD_SYSTEM_FONTS_FILE "/system/etc/system_fonts.xml"
#define FALLBACK_FONTS_FILE "/system/etc/fallback_fonts.xml"
#define VENDOR_FONTS_FILE "/vendor/etc/fallback_fonts.xml"

#define LOCALE_FALLBACK_FONTS_SYSTEM_DIR "/system/etc"
#define LOCALE_FALLBACK_FONTS_VENDOR_DIR "/vendor/etc"
#define LOCALE_FALLBACK_FONTS_PREFIX "fallback_fonts-"
#define LOCALE_FALLBACK_FONTS_SUFFIX ".xml"

/**
 * This file contains TWO parsers: one for JB and earlier (system_fonts.xml /
 * fallback_fonts.xml), one for LMP and later (fonts.xml).
 * We start with the JB parser, and if we detect a <familyset> tag with
 * version 21 or higher we switch to the LMP parser.
 */

// These defines are used to determine the kind of tag that we're currently
// populating with data. We only care about the sibling tags nameset and fileset
// for now.
#define NO_TAG 0
#define NAMESET_TAG 1
#define FILESET_TAG 2

/**
 * The FamilyData structure is passed around by the parser so that each handler
 * can read these variables that are relevant to the current parsing.
 */
struct FamilyData {
    FamilyData(XML_Parser* parserRef, SkTDArray<FontFamily*> &familiesRef) :
        parser(parserRef),
        families(familiesRef),
        currentFamily(NULL),
        currentFontInfo(NULL),
        currentTag(NO_TAG) {};

    XML_Parser* parser;                // The expat parser doing the work
    SkTDArray<FontFamily*> &families;  // The array that each family is put into as it is parsed
    FontFamily* currentFamily;         // The current family being created
    FontFileInfo* currentFontInfo;     // The current fontInfo being created
    int currentTag;                    // A flag to indicate whether we're in nameset/fileset tags
};

/** http://www.w3.org/TR/html-markup/datatypes.html#common.data.integer.non-negative-def */
template <typename T> static bool parseNonNegativeInteger(const char* s, T* value) {
    SK_COMPILE_ASSERT(std::numeric_limits<T>::is_integer, T_must_be_integer);
    const T nMax = std::numeric_limits<T>::max() / 10;
    const T dMax = std::numeric_limits<T>::max() - (nMax * 10);
    T n = 0;
    for (; *s; ++s) {
        // Check if digit
        if (*s < '0' || '9' < *s) {
            return false;
        }
        int d = *s - '0';
        // Check for overflow
        if (n > nMax || (n == nMax && d > dMax)) {
            return false;
        }
        n = (n * 10) + d;
    }
    *value = n;
    return true;
}

namespace lmpParser {

void familyElementHandler(FontFamily* family, const char** attributes) {
    // A non-fallback <family> tag must have a canonical name attribute.
    // A fallback <family> tag has no name, and may have lang and variant
    // attributes.
    family->fIsFallbackFont = true;
    for (size_t i = 0; attributes[i] != NULL &&
                       attributes[i+1] != NULL; i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        size_t valueLen = strlen(value);
        if (nameLen == 4 && !strncmp("name", name, nameLen)) {
            SkAutoAsciiToLC tolc(value);
            family->fNames.push_back().set(tolc.lc());
            family->fIsFallbackFont = false;
        } else if (nameLen == 4 && !strncmp("lang", name, nameLen)) {
            family->fLanguage = SkLanguage (value);
        } else if (nameLen == 7 && !strncmp("variant", name, nameLen)) {
            // Value should be either elegant or compact.
            if (valueLen == 7 && !strncmp("elegant", value, valueLen)) {
                family->fVariant = kElegant_FontVariant;
            } else if (valueLen == 7 && !strncmp("compact", value, valueLen)) {
                family->fVariant = kCompact_FontVariant;
            }
        }
    }
}

void fontFileNameHandler(void* data, const char* s, int len) {
    FamilyData* familyData = (FamilyData*) data;
    familyData->currentFontInfo->fFileName.set(s, len);
}

void fontElementHandler(XML_Parser* parser, FontFileInfo* file, const char** attributes) {
    // A <font> should have weight (integer) and style (normal, italic) attributes.
    // NOTE: we ignore the style.
    // The element should contain a filename.
    for (size_t i = 0; attributes[i] != NULL &&
                       attributes[i+1] != NULL; i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        if (nameLen == 6 && !strncmp("weight", name, nameLen)) {
            if (!parseNonNegativeInteger(value, &file->fWeight)) {
                SkDebugf("---- Font weight %s (INVALID)", value);
                file->fWeight = 0;
            }
        }
    }
    XML_SetCharacterDataHandler(*parser, fontFileNameHandler);
}

FontFamily* findFamily(FamilyData* familyData, const char* familyName) {
    size_t nameLen = strlen(familyName);
    for (int i = 0; i < familyData->families.count(); i++) {
        FontFamily* candidate = familyData->families[i];
        for (int j = 0; j < candidate->fNames.count(); j++) {
            if (!strncmp(candidate->fNames[j].c_str(), familyName, nameLen) &&
                nameLen == strlen(candidate->fNames[j].c_str())) {
                return candidate;
            }
        }
    }

    return NULL;
}

void aliasElementHandler(FamilyData* familyData, const char** attributes) {
    // An <alias> must have name and to attributes.
    //   It may have weight (integer).
    // If it *does not* have a weight, it is a variant name for a <family>.
    // If it *does* have a weight, it names the <font>(s) of a specific weight
    //   from a <family>.

    SkString aliasName;
    SkString to;
    int weight = 0;
    for (size_t i = 0; attributes[i] != NULL &&
                       attributes[i+1] != NULL; i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        if (nameLen == 4 && !strncmp("name", name, nameLen)) {
            SkAutoAsciiToLC tolc(value);
            aliasName.set(tolc.lc());
        } else if (nameLen == 2 && !strncmp("to", name, nameLen)) {
            to.set(value);
        } else if (nameLen == 6 && !strncmp("weight", name, nameLen)) {
            parseNonNegativeInteger(value, &weight);
        }
    }

    // Assumes that the named family is already declared
    FontFamily* targetFamily = findFamily(familyData, to.c_str());
    if (!targetFamily) {
        SkDebugf("---- Font alias target %s (NOT FOUND)", to.c_str());
        return;
    }

    if (weight) {
        FontFamily* family = new FontFamily();
        family->fNames.push_back().set(aliasName);

        for (int i = 0; i < targetFamily->fFonts.count(); i++) {
            if (targetFamily->fFonts[i].fWeight == weight) {
                family->fFonts.push_back(targetFamily->fFonts[i]);
            }
        }
        *familyData->families.append() = family;
    } else {
        targetFamily->fNames.push_back().set(aliasName);
    }
}

bool findWeight400(FontFamily* family) {
    for (int i = 0; i < family->fFonts.count(); i++) {
        if (family->fFonts[i].fWeight == 400) {
            return true;
        }
    }
    return false;
}

bool desiredWeight(int weight) {
    return (weight == 400 || weight == 700);
}

int countDesiredWeight(FontFamily* family) {
    int count = 0;
    for (int i = 0; i < family->fFonts.count(); i++) {
        if (desiredWeight(family->fFonts[i].fWeight)) {
            count++;
        }
    }
    return count;
}

// To meet Skia's expectations, any family that contains weight=400
// fonts should *only* contain {400,700}
void purgeUndesiredWeights(FontFamily* family) {
    int count = countDesiredWeight(family);
    for (int i = 1, j = 0; i < family->fFonts.count(); i++) {
        if (desiredWeight(family->fFonts[j].fWeight)) {
            j++;
        }
        if ((i != j) && desiredWeight(family->fFonts[i].fWeight)) {
            family->fFonts[j] = family->fFonts[i];
        }
    }
    family->fFonts.resize_back(count);
}

void familysetElementEndHandler(FamilyData* familyData) {
    for (int i = 0; i < familyData->families.count(); i++) {
        if (findWeight400(familyData->families[i])) {
            purgeUndesiredWeights(familyData->families[i]);
        }
    }
}

void startElementHandler(void* data, const char* tag,
                         const char** attributes) {
    FamilyData* familyData = (FamilyData*) data;
    size_t len = strlen(tag);
    if (len == 6 && !strncmp(tag, "family", len)) {
        familyData->currentFamily = new FontFamily();
        familyElementHandler(familyData->currentFamily, attributes);
    } else if (len == 4 && !strncmp(tag, "font", len)) {
        FontFileInfo* file = &familyData->currentFamily->fFonts.push_back();
        familyData->currentFontInfo = file;
        fontElementHandler(familyData->parser, file, attributes);
    } else if (len == 5 && !strncmp(tag, "alias", len)) {
        aliasElementHandler(familyData, attributes);
    }
}

void endElementHandler(void* data, const char* tag) {
    FamilyData* familyData = (FamilyData*) data;
    size_t len = strlen(tag);
    if (len == 9 && strncmp(tag, "familyset", len) == 0) {
        familysetElementEndHandler(familyData);
    } else if (len == 6 && strncmp(tag, "family", len) == 0) {
        *familyData->families.append() = familyData->currentFamily;
        familyData->currentFamily = NULL;
    } else if (len == 4 && !strncmp(tag, "font", len)) {
        XML_SetCharacterDataHandler(*familyData->parser, NULL);
    }
}

} // lmpParser

namespace jbParser {

/**
 * Handler for arbitrary text. This is used to parse the text inside each name
 * or file tag. The resulting strings are put into the fNames or FontFileInfo arrays.
 */
static void textHandler(void* data, const char* s, int len) {
    FamilyData* familyData = (FamilyData*) data;
    // Make sure we're in the right state to store this name information
    if (familyData->currentFamily &&
            (familyData->currentTag == NAMESET_TAG || familyData->currentTag == FILESET_TAG)) {
        switch (familyData->currentTag) {
        case NAMESET_TAG: {
            SkAutoAsciiToLC tolc(s, len);
            familyData->currentFamily->fNames.push_back().set(tolc.lc(), len);
            break;
        }
        case FILESET_TAG:
            if (familyData->currentFontInfo) {
                familyData->currentFontInfo->fFileName.set(s, len);
            }
            break;
        default:
            // Noop - don't care about any text that's not in the Fonts or Names list
            break;
        }
    }
}

/**
 * Handler for font files. This processes the attributes for language and
 * variants then lets textHandler handle the actual file name
 */
static void fontFileElementHandler(FamilyData* familyData, const char** attributes) {
    FontFileInfo& newFileInfo = familyData->currentFamily->fFonts.push_back();
    if (attributes) {
        size_t currentAttributeIndex = 0;
        while (attributes[currentAttributeIndex] &&
               attributes[currentAttributeIndex + 1]) {
            const char* attributeName = attributes[currentAttributeIndex];
            const char* attributeValue = attributes[currentAttributeIndex+1];
            size_t nameLength = strlen(attributeName);
            size_t valueLength = strlen(attributeValue);
            if (nameLength == 7 && strncmp(attributeName, "variant", nameLength) == 0) {
                const FontVariant prevVariant = familyData->currentFamily->fVariant;
                if (valueLength == 7 && strncmp(attributeValue, "elegant", valueLength) == 0) {
                    familyData->currentFamily->fVariant = kElegant_FontVariant;
                } else if (valueLength == 7 &&
                           strncmp(attributeValue, "compact", valueLength) == 0) {
                    familyData->currentFamily->fVariant = kCompact_FontVariant;
                }
                if (familyData->currentFamily->fFonts.count() > 1 &&
                        familyData->currentFamily->fVariant != prevVariant) {
                    SkDebugf("Every font file within a family must have identical variants");
                    sk_throw();
                }

            } else if (nameLength == 4 && strncmp(attributeName, "lang", nameLength) == 0) {
                SkLanguage prevLang = familyData->currentFamily->fLanguage;
                familyData->currentFamily->fLanguage = SkLanguage(attributeValue);
                if (familyData->currentFamily->fFonts.count() > 1 &&
                        familyData->currentFamily->fLanguage != prevLang) {
                    SkDebugf("Every font file within a family must have identical languages");
                    sk_throw();
                }
            } else if (nameLength == 5 && strncmp(attributeName, "index", nameLength) == 0) {
                int value;
                if (parseNonNegativeInteger(attributeValue, &value)) {
                    newFileInfo.fIndex = value;
                } else {
                    SkDebugf("---- SystemFonts index=%s (INVALID)", attributeValue);
                }
            }
            //each element is a pair of attributeName/attributeValue string pairs
            currentAttributeIndex += 2;
        }
    }
    familyData->currentFontInfo = &newFileInfo;
    XML_SetCharacterDataHandler(*familyData->parser, textHandler);
}

/**
 * Handler for the start of a tag. The only tags we expect are familyset, family,
 * nameset, fileset, name, and file.
 */
static void startElementHandler(void* data, const char* tag, const char** atts) {
    FamilyData* familyData = (FamilyData*) data;
    size_t len = strlen(tag);
    if (len == 9 && strncmp(tag, "familyset", len) == 0) {
        // The familyset tag has an optional "version" attribute with an integer value >= 0
        for (size_t i = 0; atts[i] != NULL &&
                           atts[i+1] != NULL; i += 2) {
            size_t nameLen = strlen(atts[i]);
            if (nameLen == 7 && strncmp(atts[i], "version", nameLen)) continue;
            const char* valueString = atts[i+1];
            int version;
            if (parseNonNegativeInteger(valueString, &version) && (version >= 21)) {
                XML_SetElementHandler(*familyData->parser,
                                      lmpParser::startElementHandler,
                                      lmpParser::endElementHandler);
            }
        }
    } else if (len == 6 && strncmp(tag, "family", len) == 0) {
        familyData->currentFamily = new FontFamily();
        // The Family tag has an optional "order" attribute with an integer value >= 0
        // If this attribute does not exist, the default value is -1
        for (size_t i = 0; atts[i] != NULL &&
                           atts[i+1] != NULL; i += 2) {
            const char* valueString = atts[i+1];
            int value;
            if (parseNonNegativeInteger(valueString, &value)) {
                familyData->currentFamily->fOrder = value;
            }
        }
    } else if (len == 7 && strncmp(tag, "nameset", len) == 0) {
        familyData->currentTag = NAMESET_TAG;
    } else if (len == 7 && strncmp(tag, "fileset", len) == 0) {
        familyData->currentTag = FILESET_TAG;
    } else if (len == 4 && strncmp(tag, "name", len) == 0 && familyData->currentTag == NAMESET_TAG) {
        // If it's a Name, parse the text inside
        XML_SetCharacterDataHandler(*familyData->parser, textHandler);
    } else if (len == 4 && strncmp(tag, "file", len) == 0 && familyData->currentTag == FILESET_TAG) {
        // If it's a file, parse the attributes, then parse the text inside
        fontFileElementHandler(familyData, atts);
    }
}

/**
 * Handler for the end of tags. We only care about family, nameset, fileset,
 * name, and file.
 */
static void endElementHandler(void* data, const char* tag) {
    FamilyData* familyData = (FamilyData*) data;
    size_t len = strlen(tag);
    if (len == 6 && strncmp(tag, "family", len)== 0) {
        // Done parsing a Family - store the created currentFamily in the families array
        *familyData->families.append() = familyData->currentFamily;
        familyData->currentFamily = NULL;
    } else if (len == 7 && strncmp(tag, "nameset", len) == 0) {
        familyData->currentTag = NO_TAG;
    } else if (len == 7 && strncmp(tag, "fileset", len) == 0) {
        familyData->currentTag = NO_TAG;
    } else if ((len == 4 &&
                strncmp(tag, "name", len) == 0 &&
                familyData->currentTag == NAMESET_TAG) ||
               (len == 4 &&
                strncmp(tag, "file", len) == 0 &&
                familyData->currentTag == FILESET_TAG)) {
        // Disable the arbitrary text handler installed to load Name data
        XML_SetCharacterDataHandler(*familyData->parser, NULL);
    }
}

} // namespace jbParser

/**
 * This function parses the given filename and stores the results in the given
 * families array.
 */
static void parseConfigFile(const char* filename, SkTDArray<FontFamily*> &families) {

    FILE* file = fopen(filename, "r");

    // Some of the files we attempt to parse (in particular, /vendor/etc/fallback_fonts.xml)
    // are optional - failure here is okay because one of these optional files may not exist.
    if (NULL == file) {
        return;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    FamilyData* familyData = new FamilyData(&parser, families);
    XML_SetUserData(parser, familyData);
    // Start parsing oldschool; switch these in flight if we detect a newer version of the file.
    XML_SetElementHandler(parser, jbParser::startElementHandler, jbParser::endElementHandler);

    char buffer[512];
    bool done = false;
    while (!done) {
        fgets(buffer, sizeof(buffer), file);
        size_t len = strlen(buffer);
        if (feof(file) != 0) {
            done = true;
        }
        XML_Parse(parser, buffer, len, done);
    }
    XML_ParserFree(parser);
    fclose(file);
}

static void getSystemFontFamilies(SkTDArray<FontFamily*> &fontFamilies) {
    int initialCount = fontFamilies.count();
    parseConfigFile(LMP_SYSTEM_FONTS_FILE, fontFamilies);

    if (initialCount == fontFamilies.count()) {
        parseConfigFile(OLD_SYSTEM_FONTS_FILE, fontFamilies);
    }
}

/**
 * In some versions of Android prior to Android 4.2 (JellyBean MR1 at API
 * Level 17) the fallback fonts for certain locales were encoded in their own
 * XML files with a suffix that identified the locale.  We search the provided
 * directory for those files,add all of their entries to the fallback chain, and
 * include the locale as part of each entry.
 */
static void getFallbackFontFamiliesForLocale(SkTDArray<FontFamily*> &fallbackFonts, const char* dir) {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    // The framework is beyond Android 4.2 and can therefore skip this function
    return;
#endif

    DIR* fontDirectory = opendir(dir);
    if (fontDirectory != NULL){
        struct dirent* dirEntry = readdir(fontDirectory);
        while (dirEntry) {

            // The size of both the prefix, suffix, and a minimum valid language code
            static const size_t minSize = strlen(LOCALE_FALLBACK_FONTS_PREFIX) +
                                          strlen(LOCALE_FALLBACK_FONTS_SUFFIX) + 2;

            SkString fileName(dirEntry->d_name);
            if (fileName.size() >= minSize &&
                    fileName.startsWith(LOCALE_FALLBACK_FONTS_PREFIX) &&
                    fileName.endsWith(LOCALE_FALLBACK_FONTS_SUFFIX)) {

                static const size_t fixedLen = strlen(LOCALE_FALLBACK_FONTS_PREFIX) -
                                               strlen(LOCALE_FALLBACK_FONTS_SUFFIX);

                SkString locale(fileName.c_str() - strlen(LOCALE_FALLBACK_FONTS_PREFIX),
                                fileName.size() - fixedLen);

                SkString absoluteFilename;
                absoluteFilename.printf("%s/%s", dir, fileName.c_str());

                SkTDArray<FontFamily*> langSpecificFonts;
                parseConfigFile(absoluteFilename.c_str(), langSpecificFonts);

                for (int i = 0; i < langSpecificFonts.count(); ++i) {
                    FontFamily* family = langSpecificFonts[i];
                    family->fLanguage = SkLanguage(locale);
                    *fallbackFonts.append() = family;
                }
            }

            // proceed to the next entry in the directory
            dirEntry = readdir(fontDirectory);
        }
        // cleanup the directory reference
        closedir(fontDirectory);
    }
}

static void getFallbackFontFamilies(SkTDArray<FontFamily*> &fallbackFonts) {
    SkTDArray<FontFamily*> vendorFonts;
    parseConfigFile(FALLBACK_FONTS_FILE, fallbackFonts);
    parseConfigFile(VENDOR_FONTS_FILE, vendorFonts);

    getFallbackFontFamiliesForLocale(fallbackFonts, LOCALE_FALLBACK_FONTS_SYSTEM_DIR);
    getFallbackFontFamiliesForLocale(vendorFonts, LOCALE_FALLBACK_FONTS_VENDOR_DIR);

    // This loop inserts the vendor fallback fonts in the correct order in the
    // overall fallbacks list.
    int currentOrder = -1;
    for (int i = 0; i < vendorFonts.count(); ++i) {
        FontFamily* family = vendorFonts[i];
        int order = family->fOrder;
        if (order < 0) {
            if (currentOrder < 0) {
                // Default case - just add it to the end of the fallback list
                *fallbackFonts.append() = family;
            } else {
                // no order specified on this font, but we're incrementing the order
                // based on an earlier order insertion request
                *fallbackFonts.insert(currentOrder++) = family;
            }
        } else {
            // Add the font into the fallback list in the specified order. Set
            // currentOrder for correct placement of other fonts in the vendor list.
            *fallbackFonts.insert(order) = family;
            currentOrder = order + 1;
        }
    }
}

/**
 * Loads data on font families from various expected configuration files. The
 * resulting data is returned in the given fontFamilies array.
 */
void SkFontConfigParser::GetFontFamilies(SkTDArray<FontFamily*> &fontFamilies) {

    getSystemFontFamilies(fontFamilies);

    // Append all the fallback fonts to system fonts
    SkTDArray<FontFamily*> fallbackFonts;
    getFallbackFontFamilies(fallbackFonts);
    for (int i = 0; i < fallbackFonts.count(); ++i) {
        fallbackFonts[i]->fIsFallbackFont = true;
        *fontFamilies.append() = fallbackFonts[i];
    }
}

void SkFontConfigParser::GetTestFontFamilies(SkTDArray<FontFamily*> &fontFamilies,
                                             const char* testMainConfigFile,
                                             const char* testFallbackConfigFile) {
    parseConfigFile(testMainConfigFile, fontFamilies);

    SkTDArray<FontFamily*> fallbackFonts;
    if (testFallbackConfigFile) {
        parseConfigFile(testFallbackConfigFile, fallbackFonts);
    }

    // Append all fallback fonts to system fonts
    for (int i = 0; i < fallbackFonts.count(); ++i) {
        fallbackFonts[i]->fIsFallbackFont = true;
        *fontFamilies.append() = fallbackFonts[i];
    }
}

SkLanguage SkLanguage::getParent() const {
    SkASSERT(!fTag.isEmpty());
    const char* tag = fTag.c_str();

    // strip off the rightmost "-.*"
    const char* parentTagEnd = strrchr(tag, '-');
    if (parentTagEnd == NULL) {
        return SkLanguage();
    }
    size_t parentTagLen = parentTagEnd - tag;
    return SkLanguage(tag, parentTagLen);
}
