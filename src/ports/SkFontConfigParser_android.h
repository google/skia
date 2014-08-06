/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTCONFIGPARSER_ANDROID_H_
#define SKFONTCONFIGPARSER_ANDROID_H_

#include "SkString.h"
#include "SkTDArray.h"

/** \class SkLanguage

    The SkLanguage class represents a human written language, and is used by
    text draw operations to determine which glyph to draw when drawing
    characters with variants (ie Han-derived characters).
*/
class SkLanguage {
public:
    SkLanguage() { }
    SkLanguage(const SkString& tag) : fTag(tag) { }
    SkLanguage(const char* tag) : fTag(tag) { }
    SkLanguage(const char* tag, size_t len) : fTag(tag, len) { }
    SkLanguage(const SkLanguage& b) : fTag(b.fTag) { }

    /** Gets a BCP 47 language identifier for this SkLanguage.
        @return a BCP 47 language identifier representing this language
    */
    const SkString& getTag() const { return fTag; }

    /** Performs BCP 47 fallback to return an SkLanguage one step more general.
        @return an SkLanguage one step more general
    */
    SkLanguage getParent() const;

    bool operator==(const SkLanguage& b) const {
        return fTag == b.fTag;
    }
    bool operator!=(const SkLanguage& b) const {
        return fTag != b.fTag;
    }
    SkLanguage& operator=(const SkLanguage& b) {
        fTag = b.fTag;
        return *this;
    }

private:
    //! BCP 47 language identifier
    SkString fTag;
};

enum FontVariants {
   kDefault_FontVariant = 0x01,
   kCompact_FontVariant = 0x02,
   kElegant_FontVariant = 0x04,
   kLast_FontVariant = kElegant_FontVariant,
};
typedef uint32_t FontVariant;

struct FontFileInfo {
    FontFileInfo() : fIndex(0), fWeight(0) { }

    SkString              fFileName;
    int                   fIndex;
    int                   fWeight;
};

/**
 * The FontFamily data structure is created during parsing and handed back to
 * Skia to fold into its representation of font families. fNames is the list of
 * font names that alias to a font family. fontFileArray is the list of information
 * about each file.  Order is the priority order for the font. This is
 * used internally to determine the order in which to place fallback fonts as
 * they are read from the configuration files.
 */
struct FontFamily {
    FontFamily()
        : fVariant(kDefault_FontVariant)
        , order(-1)
        , fIsFallbackFont(false) { }

    SkTArray<SkString>                 fNames;
    SkTArray<FontFileInfo>             fFontFiles;
    SkLanguage                         fLanguage;
    FontVariant                        fVariant;
    int                                order; // only used internally by SkFontConfigParser
    bool                               fIsFallbackFont;
};

namespace SkFontConfigParser {

/**
 * Parses all system font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void GetFontFamilies(SkTDArray<FontFamily*> &fontFamilies);

/**
 * Parses all test font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void GetTestFontFamilies(SkTDArray<FontFamily*> &fontFamilies,
                         const char* testMainConfigFile,
                         const char* testFallbackConfigFile);

} // SkFontConfigParser namespace

#endif /* SKFONTCONFIGPARSER_ANDROID_H_ */
