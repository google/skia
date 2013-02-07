/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "FontHostConfiguration_android.h"
#include "SkString.h"
#include "SkTDArray.h"
#include <expat.h>
#include <sys/system_properties.h>

#define SYSTEM_FONTS_FILE "/system/etc/system_fonts.xml"
#define FALLBACK_FONTS_FILE "/system/etc/fallback_fonts.xml"
#define VENDOR_FONTS_FILE "/vendor/etc/fallback_fonts.xml"


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
    FamilyData(XML_Parser *parserRef, SkTDArray<FontFamily*> &familiesRef, const AndroidLocale &localeRef) :
            parser(parserRef), families(familiesRef), currentTag(NO_TAG),
            locale(localeRef), currentFamilyLangMatch(false), familyLangMatchCount(0) {}

    XML_Parser *parser;                // The expat parser doing the work
    SkTDArray<FontFamily*> &families;  // The array that each family is put into as it is parsed
    FontFamily *currentFamily;         // The current family being created
    int currentTag;                    // A flag to indicate whether we're in nameset/fileset tags
    const AndroidLocale &locale;       // The locale to which we compare the "lang" attribute of File.
    bool currentFamilyLangMatch;       // If currentFamily's File has a "lang" attribute and matches locale.
    int familyLangMatchCount;          // Number of families containing File which has a "lang" attribute and matches locale.
};

/**
 * Handler for arbitrary text. This is used to parse the text inside each name
 * or file tag. The resulting strings are put into the fNames or fFileNames arrays.
 */
void textHandler(void *data, const char *s, int len) {
    FamilyData *familyData = (FamilyData*) data;
    // Make sure we're in the right state to store this name information
    if (familyData->currentFamily &&
            (familyData->currentTag == NAMESET_TAG || familyData->currentTag == FILESET_TAG)) {
        // Malloc new buffer to store the string
        char *buff;
        buff = (char*) malloc((len + 1) * sizeof(char));
        strncpy(buff, s, len);
        buff[len] = '\0';
        switch (familyData->currentTag) {
        case NAMESET_TAG:
            *(familyData->currentFamily->fNames.append()) = buff;
            break;
        case FILESET_TAG:
            *(familyData->currentFamily->fFileNames.append()) = buff;
            break;
        default:
            // Noop - don't care about any text that's not in the Fonts or Names list
            break;
        }
    }
}

/**
 * Handler for the start of a tag. The only tags we expect are family, nameset,
 * fileset, name, and file.
 */
void startElementHandler(void *data, const char *tag, const char **atts) {
    FamilyData *familyData = (FamilyData*) data;
    int len = strlen(tag);
    if (strncmp(tag, "family", len)== 0) {
        familyData->currentFamily = new FontFamily();
        familyData->currentFamily->order = -1;
        // The Family tag has an optional "order" attribute with an integer value >= 0
        // If this attribute does not exist, the default value is -1
        for (int i = 0; atts[i] != NULL; i += 2) {
            const char* valueString = atts[i+1];
            int value;
            int len = sscanf(valueString, "%d", &value);
            if (len > 0) {
                familyData->currentFamily->order = value;
            }
        }
    } else if (len == 7 && strncmp(tag, "nameset", len)== 0) {
        familyData->currentTag = NAMESET_TAG;
    } else if (len == 7 && strncmp(tag, "fileset", len) == 0) {
        familyData->currentTag = FILESET_TAG;
    } else if (strncmp(tag, "name", len) == 0 && familyData->currentTag == NAMESET_TAG) {
        XML_SetCharacterDataHandler(*familyData->parser, textHandler);
    } else if (strncmp(tag, "file", len) == 0 && familyData->currentTag == FILESET_TAG) {
        // From JB MR1, the File tag has a "lang" attribute to specify a language specific font file
        // and the family entry has higher priority than the others without "lang" attribute.
        bool includeTheEntry = true;
        for (int i = 0; atts[i] != NULL; i += 2) {
            const char* attribute = atts[i];
            const char* value = atts[i+1];
            if (strncmp(attribute, "lang", 4) == 0) {
                if (strcmp(value, familyData->locale.language) == 0) {
                    // Found matching "lang" attribute. The current Family will have higher priority in the family list.
                    familyData->currentFamilyLangMatch = true;
                } else {
                    // Don't include the entry if "lang" is specified but not matching.
                    includeTheEntry = false;
                }
            }
        }
        if (includeTheEntry) {
            XML_SetCharacterDataHandler(*familyData->parser, textHandler);
        }
    }
}

/**
 * Handler for the end of tags. We only care about family, nameset, fileset,
 * name, and file.
 */
void endElementHandler(void *data, const char *tag) {
    FamilyData *familyData = (FamilyData*) data;
    int len = strlen(tag);
    if (strncmp(tag, "family", len)== 0) {
        // Done parsing a Family - store the created currentFamily in the families array
        if (familyData->currentFamilyLangMatch) {
            *familyData->families.insert(familyData->familyLangMatchCount++) = familyData->currentFamily;
            familyData->currentFamilyLangMatch = false;
        } else {
            *familyData->families.append() = familyData->currentFamily;
        }
        familyData->currentFamily = NULL;
    } else if (len == 7 && strncmp(tag, "nameset", len)== 0) {
        familyData->currentTag = NO_TAG;
    } else if (len == 7 && strncmp(tag, "fileset", len)== 0) {
        familyData->currentTag = NO_TAG;
    } else if ((strncmp(tag, "name", len) == 0 && familyData->currentTag == NAMESET_TAG) ||
            (strncmp(tag, "file", len) == 0 && familyData->currentTag == FILESET_TAG)) {
        // Disable the arbitrary text handler installed to load Name data
        XML_SetCharacterDataHandler(*familyData->parser, NULL);
    }
}

/**
 * Read the persistent locale.
 */
void getLocale(AndroidLocale &locale)
{
    char propLang[PROP_VALUE_MAX], propRegn[PROP_VALUE_MAX];
    __system_property_get("persist.sys.language", propLang);
    __system_property_get("persist.sys.country", propRegn);

    if (*propLang == 0 && *propRegn == 0) {
        /* Set to ro properties, default is en_US */
        __system_property_get("ro.product.locale.language", propLang);
        __system_property_get("ro.product.locale.region", propRegn);
        if (*propLang == 0 && *propRegn == 0) {
            strcpy(propLang, "en");
            strcpy(propRegn, "US");
        }
    }
    strncpy(locale.language, propLang, 2);
    locale.language[2] = '\0';
    strncpy(locale.region, propRegn, 2);
    locale.region[2] = '\0';
}

/**
 * Use the current system locale (language and region) to open the best matching
 * customization. For example, when the language is Japanese, the sequence might be:
 *      /system/etc/fallback_fonts-ja-JP.xml
 *      /system/etc/fallback_fonts-ja.xml
 *      /system/etc/fallback_fonts.xml
 */
FILE* openLocalizedFile(const char* origname, const AndroidLocale& locale) {
    FILE* file = 0;
    SkString basename;
    SkString filename;

    basename.set(origname);
    // Remove the .xml suffix. We'll add it back in a moment.
    if (basename.endsWith(".xml")) {
        basename.resize(basename.size()-4);
    }
    // Try first with language and region
    filename.printf("%s-%s-%s.xml", basename.c_str(), locale.language, locale.region);
    file = fopen(filename.c_str(), "r");
    if (!file) {
        // If not found, try next with just language
        filename.printf("%s-%s.xml", basename.c_str(), locale.language);
        file = fopen(filename.c_str(), "r");

        if (!file) {
            // If still not found, try just the original name
            file = fopen(origname, "r");
        }
    }
    return file;
}

/**
 * This function parses the given filename and stores the results in the given
 * families array.
 */
void parseConfigFile(const char *filename, SkTDArray<FontFamily*> &families) {
    AndroidLocale locale;
    getLocale(locale);
    XML_Parser parser = XML_ParserCreate(NULL);
    FamilyData familyData(&parser, families, locale);
    XML_SetUserData(parser, &familyData);
    XML_SetElementHandler(parser, startElementHandler, endElementHandler);
    FILE *file = openLocalizedFile(filename, locale);
    // Some of the files we attempt to parse (in particular, /vendor/etc/fallback_fonts.xml)
    // are optional - failure here is okay because one of these optional files may not exist.
    if (file == NULL) {
        return;
    }
    char buffer[512];
    bool done = false;
    while (!done) {
        fgets(buffer, sizeof(buffer), file);
        int len = strlen(buffer);
        if (feof(file) != 0) {
            done = true;
        }
        XML_Parse(parser, buffer, len, done);
    }
    fclose(file);
    XML_ParserFree(parser);
}

void getSystemFontFamilies(SkTDArray<FontFamily*> &fontFamilies) {
    parseConfigFile(SYSTEM_FONTS_FILE, fontFamilies);
}

void getFallbackFontFamilies(SkTDArray<FontFamily*> &fallbackFonts) {
    SkTDArray<FontFamily*> vendorFonts;
    parseConfigFile(FALLBACK_FONTS_FILE, fallbackFonts);
    parseConfigFile(VENDOR_FONTS_FILE, vendorFonts);

    // This loop inserts the vendor fallback fonts in the correct order in the
    // overall fallbacks list.
    int currentOrder = -1;
    for (int i = 0; i < vendorFonts.count(); ++i) {
        FontFamily* family = vendorFonts[i];
        int order = family->order;
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
void getFontFamilies(SkTDArray<FontFamily*> &fontFamilies) {
    SkTDArray<FontFamily*> fallbackFonts;

    getSystemFontFamilies(fontFamilies);
    getFallbackFontFamilies(fallbackFonts);

    // Append all fallback fonts to system fonts
    for (int i = 0; i < fallbackFonts.count(); ++i) {
        *fontFamilies.append() = fallbackFonts[i];
    }
}

void getTestFontFamilies(SkTDArray<FontFamily*> &fontFamilies,
                         const char* testMainConfigFile,
                         const char* testFallbackConfigFile) {
    parseConfigFile(testMainConfigFile, fontFamilies);

    SkTDArray<FontFamily*> fallbackFonts;
    parseConfigFile(testFallbackConfigFile, fallbackFonts);

    // Append all fallback fonts to system fonts
    for (int i = 0; i < fallbackFonts.count(); ++i) {
        *fontFamilies.append() = fallbackFonts[i];
    }
}
