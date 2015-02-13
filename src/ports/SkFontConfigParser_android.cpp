/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigParser_android.h"
#include "SkFontMgr_android.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTypeface.h"

#include <expat.h>
#include <dirent.h>
#include <stdio.h>

#include <limits>
#include <stdlib.h>

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

#ifndef SK_FONT_FILE_PREFIX
#    define SK_FONT_FILE_PREFIX "/fonts/"
#endif

/**
 * This file contains TWO parsers: one for JB and earlier (system_fonts.xml /
 * fallback_fonts.xml), one for LMP and later (fonts.xml).
 * We start with the JB parser, and if we detect a <familyset> tag with
 * version 21 or higher we switch to the LMP parser.
 */

/** Used to track which tag is currently populating with data.
 *  Only nameset and fileset are of interest for now.
 */
enum CurrentTag {
    kNo_CurrentTag,
    kNameSet_CurrentTag,
    kFileSet_CurrentTag
};

/**
 * The FamilyData structure is passed around by the parser so that each handler
 * can read these variables that are relevant to the current parsing.
 */
struct FamilyData {
    FamilyData(XML_Parser parser, SkTDArray<FontFamily*>& families,
               const SkString& basePath, bool isFallback)
        : fParser(parser)
        , fFamilies(families)
        , fCurrentFamily(NULL)
        , fCurrentFontInfo(NULL)
        , fCurrentTag(kNo_CurrentTag)
        , fVersion(0)
        , fBasePath(basePath)
        , fIsFallback(isFallback)
    { };

    XML_Parser fParser;                       // The expat parser doing the work, owned by caller
    SkTDArray<FontFamily*>& fFamilies;        // The array to append families, owned by caller
    SkAutoTDelete<FontFamily> fCurrentFamily; // The family being created, owned by this
    FontFileInfo* fCurrentFontInfo;           // The fontInfo being created, owned by fCurrentFamily
    CurrentTag fCurrentTag;                   // The kind of tag currently being parsed.
    int fVersion;                             // The version of the file parsed.
    const SkString& fBasePath;                // The current base path.
    const bool fIsFallback;                   // Indicates the file being parsed is a fallback file
};

/** Parses a null terminated string into an integer type, checking for overflow.
 *  http://www.w3.org/TR/html-markup/datatypes.html#common.data.integer.non-negative-def
 *
 *  If the string cannot be parsed into 'value', returns false and does not change 'value'.
 */
template <typename T> static bool parse_non_negative_integer(const char* s, T* value) {
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

static bool memeq(const char* s1, const char* s2, size_t n1, size_t n2) {
    return n1 == n2 && 0 == memcmp(s1, s2, n1);
}
#define MEMEQ(c, s, n) memeq(c, s, sizeof(c) - 1, n)

#define ATTS_NON_NULL(a, i) (a[i] != NULL && a[i+1] != NULL)

namespace lmpParser {

static void family_element_handler(FontFamily* family, const char** attributes) {
    // A non-fallback <family> tag must have a canonical name attribute.
    // A fallback <family> tag has no name, and may have lang and variant
    // attributes.
    family->fIsFallbackFont = true;
    for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        size_t valueLen = strlen(value);
        if (MEMEQ("name", name, nameLen)) {
            SkAutoAsciiToLC tolc(value);
            family->fNames.push_back().set(tolc.lc());
            family->fIsFallbackFont = false;
        } else if (MEMEQ("lang", name, nameLen)) {
            family->fLanguage = SkLanguage(value, valueLen);
        } else if (MEMEQ("variant", name, nameLen)) {
            // Value should be either elegant or compact.
            if (MEMEQ("elegant", value, valueLen)) {
                family->fVariant = kElegant_FontVariant;
            } else if (MEMEQ("compact", value, valueLen)) {
                family->fVariant = kCompact_FontVariant;
            }
        }
    }
}

static void XMLCALL font_file_name_handler(void* data, const char* s, int len) {
    FamilyData* self = static_cast<FamilyData*>(data);
    self->fCurrentFontInfo->fFileName.append(s, len);
}

static void font_element_handler(FamilyData* self, FontFileInfo* file, const char** attributes) {
    // A <font> should have weight (integer) and style (normal, italic) attributes.
    // NOTE: we ignore the style.
    // The element should contain a filename.
    for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        if (MEMEQ("weight", name, nameLen)) {
            if (!parse_non_negative_integer(value, &file->fWeight)) {
                SkDebugf("---- Font weight %s (INVALID)", value);
            }
        }
    }
    XML_SetCharacterDataHandler(self->fParser, font_file_name_handler);
}

static FontFamily* find_family(FamilyData* self, const SkString& familyName) {
    for (int i = 0; i < self->fFamilies.count(); i++) {
        FontFamily* candidate = self->fFamilies[i];
        for (int j = 0; j < candidate->fNames.count(); j++) {
            if (candidate->fNames[j] == familyName) {
                return candidate;
            }
        }
    }
    return NULL;
}

static void alias_element_handler(FamilyData* self, const char** attributes) {
    // An <alias> must have name and to attributes.
    //   It may have weight (integer).
    // If it *does not* have a weight, it is a variant name for a <family>.
    // If it *does* have a weight, it names the <font>(s) of a specific weight
    //   from a <family>.

    SkString aliasName;
    SkString to;
    int weight = 0;
    for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
        const char* name = attributes[i];
        const char* value = attributes[i+1];
        size_t nameLen = strlen(name);
        if (MEMEQ("name", name, nameLen)) {
            SkAutoAsciiToLC tolc(value);
            aliasName.set(tolc.lc());
        } else if (MEMEQ("to", name, nameLen)) {
            to.set(value);
        } else if (MEMEQ("weight", name, nameLen)) {
            if (!parse_non_negative_integer(value, &weight)) {
                SkDebugf("---- Font weight %s (INVALID)", value);
            }
        }
    }

    // Assumes that the named family is already declared
    FontFamily* targetFamily = find_family(self, to);
    if (!targetFamily) {
        SkDebugf("---- Font alias target %s (NOT FOUND)", to.c_str());
        return;
    }

    if (weight) {
        FontFamily* family = new FontFamily(targetFamily->fBasePath, self->fIsFallback);
        family->fNames.push_back().set(aliasName);

        for (int i = 0; i < targetFamily->fFonts.count(); i++) {
            if (targetFamily->fFonts[i].fWeight == weight) {
                family->fFonts.push_back(targetFamily->fFonts[i]);
            }
        }
        *self->fFamilies.append() = family;
    } else {
        targetFamily->fNames.push_back().set(aliasName);
    }
}

static void XMLCALL start_element_handler(void* data, const char* tag, const char** attributes) {
    FamilyData* self = static_cast<FamilyData*>(data);
    size_t len = strlen(tag);
    if (MEMEQ("family", tag, len)) {
        self->fCurrentFamily.reset(new FontFamily(self->fBasePath, self->fIsFallback));
        family_element_handler(self->fCurrentFamily, attributes);
    } else if (MEMEQ("font", tag, len)) {
        FontFileInfo* file = &self->fCurrentFamily->fFonts.push_back();
        self->fCurrentFontInfo = file;
        font_element_handler(self, file, attributes);
    } else if (MEMEQ("alias", tag, len)) {
        alias_element_handler(self, attributes);
    }
}

static void XMLCALL end_element_handler(void* data, const char* tag) {
    FamilyData* self = static_cast<FamilyData*>(data);
    size_t len = strlen(tag);
    if (MEMEQ("family", tag, len)) {
        *self->fFamilies.append() = self->fCurrentFamily.detach();
    } else if (MEMEQ("font", tag, len)) {
        XML_SetCharacterDataHandler(self->fParser, NULL);
    }
}

} // lmpParser

namespace jbParser {

/**
 * Handler for arbitrary text. This is used to parse the text inside each name
 * or file tag. The resulting strings are put into the fNames or FontFileInfo arrays.
 */
static void XMLCALL text_handler(void* data, const char* s, int len) {
    FamilyData* self = static_cast<FamilyData*>(data);
    // Make sure we're in the right state to store this name information
    if (self->fCurrentFamily.get()) {
        switch (self->fCurrentTag) {
        case kNameSet_CurrentTag: {
            SkAutoAsciiToLC tolc(s, len);
            self->fCurrentFamily->fNames.back().append(tolc.lc(), len);
            break;
        }
        case kFileSet_CurrentTag:
            if (self->fCurrentFontInfo) {
                self->fCurrentFontInfo->fFileName.append(s, len);
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
static void font_file_element_handler(FamilyData* self, const char** attributes) {
    FontFamily& currentFamily = *self->fCurrentFamily.get();
    FontFileInfo& newFileInfo = currentFamily.fFonts.push_back();
    if (attributes) {
        for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
            const char* attributeName = attributes[i];
            const char* attributeValue = attributes[i+1];
            size_t nameLength = strlen(attributeName);
            size_t valueLength = strlen(attributeValue);
            if (MEMEQ("variant", attributeName, nameLength)) {
                const FontVariant prevVariant = currentFamily.fVariant;
                if (MEMEQ("elegant", attributeValue, valueLength)) {
                    currentFamily.fVariant = kElegant_FontVariant;
                } else if (MEMEQ("compact", attributeValue, valueLength)) {
                    currentFamily.fVariant = kCompact_FontVariant;
                }
                if (currentFamily.fFonts.count() > 1 && currentFamily.fVariant != prevVariant) {
                    SkDebugf("Every font file within a family must have identical variants");
                }

            } else if (MEMEQ("lang", attributeName, nameLength)) {
                SkLanguage prevLang = currentFamily.fLanguage;
                currentFamily.fLanguage = SkLanguage(attributeValue, valueLength);
                if (currentFamily.fFonts.count() > 1 && currentFamily.fLanguage != prevLang) {
                    SkDebugf("Every font file within a family must have identical languages");
                }

            } else if (MEMEQ("index", attributeName, nameLength)) {
                if (!parse_non_negative_integer(attributeValue, &newFileInfo.fIndex)) {
                    SkDebugf("---- SystemFonts index=%s (INVALID)", attributeValue);
                }
            }
        }
    }
    self->fCurrentFontInfo = &newFileInfo;
    XML_SetCharacterDataHandler(self->fParser, text_handler);
}

/**
 * Handler for the start of a tag. The only tags we expect are familyset, family,
 * nameset, fileset, name, and file.
 */
static void XMLCALL start_element_handler(void* data, const char* tag, const char** attributes) {
    FamilyData* self = static_cast<FamilyData*>(data);
    size_t len = strlen(tag);
    if (MEMEQ("familyset", tag, len)) {
        // The familyset tag has an optional "version" attribute with an integer value >= 0
        for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
            size_t nameLen = strlen(attributes[i]);
            if (!MEMEQ("version", attributes[i], nameLen)) continue;
            const char* valueString = attributes[i+1];
            int version;
            if (parse_non_negative_integer(valueString, &version) && (version >= 21)) {
                XML_SetElementHandler(self->fParser,
                                      lmpParser::start_element_handler,
                                      lmpParser::end_element_handler);
                self->fVersion = version;
            }
        }
    } else if (MEMEQ("family", tag, len)) {
        self->fCurrentFamily.reset(new FontFamily(self->fBasePath, self->fIsFallback));
        // The Family tag has an optional "order" attribute with an integer value >= 0
        // If this attribute does not exist, the default value is -1
        for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
            const char* valueString = attributes[i+1];
            int value;
            if (parse_non_negative_integer(valueString, &value)) {
                self->fCurrentFamily->fOrder = value;
            }
        }
    } else if (MEMEQ("nameset", tag, len)) {
        self->fCurrentTag = kNameSet_CurrentTag;
    } else if (MEMEQ("fileset", tag, len)) {
        self->fCurrentTag = kFileSet_CurrentTag;
    } else if (MEMEQ("name", tag, len) && self->fCurrentTag == kNameSet_CurrentTag) {
        // If it's a Name, parse the text inside
        self->fCurrentFamily->fNames.push_back();
        XML_SetCharacterDataHandler(self->fParser, text_handler);
    } else if (MEMEQ("file", tag, len) && self->fCurrentTag == kFileSet_CurrentTag) {
        // If it's a file, parse the attributes, then parse the text inside
        font_file_element_handler(self, attributes);
    }
}

/**
 * Handler for the end of tags. We only care about family, nameset, fileset,
 * name, and file.
 */
static void XMLCALL end_element_handler(void* data, const char* tag) {
    FamilyData* self = static_cast<FamilyData*>(data);
    size_t len = strlen(tag);
    if (MEMEQ("family", tag, len)) {
        // Done parsing a Family - store the created currentFamily in the families array
        *self->fFamilies.append() = self->fCurrentFamily.detach();
    } else if (MEMEQ("nameset", tag, len)) {
        self->fCurrentTag = kNo_CurrentTag;
    } else if (MEMEQ("fileset", tag, len)) {
        self->fCurrentTag = kNo_CurrentTag;
    } else if ((MEMEQ("name", tag, len) && self->fCurrentTag == kNameSet_CurrentTag) ||
               (MEMEQ("file", tag, len) && self->fCurrentTag == kFileSet_CurrentTag)) {
        // Disable the arbitrary text handler installed to load Name data
        XML_SetCharacterDataHandler(self->fParser, NULL);
    }
}

} // namespace jbParser

static void XMLCALL xml_entity_decl_handler(void *data,
                                            const XML_Char *entityName,
                                            int is_parameter_entity,
                                            const XML_Char *value,
                                            int value_length,
                                            const XML_Char *base,
                                            const XML_Char *systemId,
                                            const XML_Char *publicId,
                                            const XML_Char *notationName)
{
    FamilyData* self = static_cast<FamilyData*>(data);
    SkDebugf("Entity declaration %s found, stopping processing.", entityName);
    XML_StopParser(self->fParser, XML_FALSE);
}

static const XML_Memory_Handling_Suite sk_XML_alloc = {
    sk_malloc_throw,
    sk_realloc_throw,
    sk_free
};

template<typename T> struct remove_ptr {typedef T type;};
template<typename T> struct remove_ptr<T*> {typedef T type;};

/**
 * This function parses the given filename and stores the results in the given
 * families array. Returns the version of the file, negative if the file does not exist.
 */
static int parse_config_file(const char* filename, SkTDArray<FontFamily*>& families,
                             const SkString& basePath, bool isFallback)
{
    SkFILEStream file(filename);

    // Some of the files we attempt to parse (in particular, /vendor/etc/fallback_fonts.xml)
    // are optional - failure here is okay because one of these optional files may not exist.
    if (!file.isValid()) {
        SkDebugf("File %s could not be opened.\n", filename);
        return -1;
    }

    SkAutoTCallVProc<remove_ptr<XML_Parser>::type, XML_ParserFree> parser(
        XML_ParserCreate_MM(NULL, &sk_XML_alloc, NULL));
    if (!parser) {
        SkDebugf("Could not create XML parser.\n");
        return -1;
    }

    FamilyData self(parser, families, basePath, isFallback);
    XML_SetUserData(parser, &self);

    // Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340
    XML_SetEntityDeclHandler(parser, xml_entity_decl_handler);

    // Start parsing oldschool; switch these in flight if we detect a newer version of the file.
    XML_SetElementHandler(parser, jbParser::start_element_handler, jbParser::end_element_handler);

    // One would assume it would be faster to have a buffer on the stack and call XML_Parse.
    // But XML_Parse will call XML_GetBuffer anyway and memmove the passed buffer into it.
    // (Unless XML_CONTEXT_BYTES is undefined, but all users define it.)
    // In debug, buffer a small odd number of bytes to detect slicing in XML_CharacterDataHandler.
    static const int bufferSize = 512 SkDEBUGCODE( - 507);
    bool done = false;
    while (!done) {
        void* buffer = XML_GetBuffer(parser, bufferSize);
        if (!buffer) {
            SkDebugf("Could not buffer enough to continue.\n");
            return -1;
        }
        size_t len = file.read(buffer, bufferSize);
        done = file.isAtEnd();
        XML_Status status = XML_ParseBuffer(parser, len, done);
        if (XML_STATUS_ERROR == status) {
            XML_Error error = XML_GetErrorCode(parser);
            int line = XML_GetCurrentLineNumber(parser);
            int column = XML_GetCurrentColumnNumber(parser);
            int index = XML_GetCurrentByteIndex(parser);
            const XML_LChar* errorString = XML_ErrorString(error);
            SkDebugf("Line: %d Column: %d (Offset: %d) Error %d: %s.\n",
                          line,    column,      index,    error, errorString);
            return -1;
        }
    }
    return self.fVersion;
}

/** Returns the version of the system font file actually found, negative if none. */
static int append_system_font_families(SkTDArray<FontFamily*>& fontFamilies,
                                       const SkString& basePath)
{
    int initialCount = fontFamilies.count();
    int version = parse_config_file(LMP_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
    if (version < 0 || fontFamilies.count() == initialCount) {
        version = parse_config_file(OLD_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
    }
    return version;
}

/**
 * In some versions of Android prior to Android 4.2 (JellyBean MR1 at API
 * Level 17) the fallback fonts for certain locales were encoded in their own
 * XML files with a suffix that identified the locale.  We search the provided
 * directory for those files,add all of their entries to the fallback chain, and
 * include the locale as part of each entry.
 */
static void append_fallback_font_families_for_locale(SkTDArray<FontFamily*>& fallbackFonts,
                                                     const char* dir,
                                                     const SkString& basePath)
{
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    // The framework is beyond Android 4.2 and can therefore skip this function
    return;
#endif

    SkAutoTCallIProc<DIR, closedir> fontDirectory(opendir(dir));
    if (NULL == fontDirectory) {
        return;
    }

    for (struct dirent* dirEntry; (dirEntry = readdir(fontDirectory));) {
        // The size of the prefix and suffix.
        static const size_t fixedLen = sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1
                                     + sizeof(LOCALE_FALLBACK_FONTS_SUFFIX) - 1;

        // The size of the prefix, suffix, and a minimum valid language code
        static const size_t minSize = fixedLen + 2;

        SkString fileName(dirEntry->d_name);
        if (fileName.size() < minSize ||
            !fileName.startsWith(LOCALE_FALLBACK_FONTS_PREFIX) ||
            !fileName.endsWith(LOCALE_FALLBACK_FONTS_SUFFIX))
        {
            continue;
        }

        SkString locale(fileName.c_str() + sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1,
                        fileName.size() - fixedLen);

        SkString absoluteFilename;
        absoluteFilename.printf("%s/%s", dir, fileName.c_str());

        SkTDArray<FontFamily*> langSpecificFonts;
        parse_config_file(absoluteFilename.c_str(), langSpecificFonts, basePath, true);

        for (int i = 0; i < langSpecificFonts.count(); ++i) {
            FontFamily* family = langSpecificFonts[i];
            family->fLanguage = SkLanguage(locale);
            *fallbackFonts.append() = family;
        }
    }
}

static void append_system_fallback_font_families(SkTDArray<FontFamily*>& fallbackFonts,
                                                 const SkString& basePath)
{
    parse_config_file(FALLBACK_FONTS_FILE, fallbackFonts, basePath, true);
    append_fallback_font_families_for_locale(fallbackFonts,
                                             LOCALE_FALLBACK_FONTS_SYSTEM_DIR,
                                             basePath);
}

static void mixin_vendor_fallback_font_families(SkTDArray<FontFamily*>& fallbackFonts,
                                                const SkString& basePath)
{
    SkTDArray<FontFamily*> vendorFonts;
    parse_config_file(VENDOR_FONTS_FILE, vendorFonts, basePath, true);
    append_fallback_font_families_for_locale(vendorFonts,
                                             LOCALE_FALLBACK_FONTS_VENDOR_DIR,
                                             basePath);

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

void SkFontConfigParser::GetSystemFontFamilies(SkTDArray<FontFamily*>& fontFamilies) {
    // Version 21 of the system font configuration does not need any fallback configuration files.
    SkString basePath(getenv("ANDROID_ROOT"));
    basePath.append(SK_FONT_FILE_PREFIX, sizeof(SK_FONT_FILE_PREFIX) - 1);

    if (append_system_font_families(fontFamilies, basePath) >= 21) {
        return;
    }

    // Append all the fallback fonts to system fonts
    SkTDArray<FontFamily*> fallbackFonts;
    append_system_fallback_font_families(fallbackFonts, basePath);
    mixin_vendor_fallback_font_families(fallbackFonts, basePath);
    fontFamilies.append(fallbackFonts.count(), fallbackFonts.begin());
}

void SkFontConfigParser::GetCustomFontFamilies(SkTDArray<FontFamily*>& fontFamilies,
                                               const SkString& basePath,
                                               const char* fontsXml,
                                               const char* fallbackFontsXml,
                                               const char* langFallbackFontsDir)
{
    if (fontsXml) {
        parse_config_file(fontsXml, fontFamilies, basePath, false);
    }
    if (fallbackFontsXml) {
        parse_config_file(fallbackFontsXml, fontFamilies, basePath, true);
    }
    if (langFallbackFontsDir) {
        append_fallback_font_families_for_locale(fontFamilies,
                                                 langFallbackFontsDir,
                                                 basePath);
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
