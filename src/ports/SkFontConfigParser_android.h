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
 * A font family provides one or more names for a collection of fonts, each of
 * which has a different style (normal, italic) or weight (thin, light, bold,
 * etc).
 * Some fonts may occur in compact variants for use in the user interface.
 * Android distinguishes "fallback" fonts to support non-ASCII character sets.
 */
struct FontFamily {
    FontFamily()
        : fVariant(kDefault_FontVariant)
        , fOrder(-1)
        , fIsFallbackFont(false) { }

    SkTArray<SkString>                 fNames;
    SkTArray<FontFileInfo>             fFonts;
    SkLanguage                         fLanguage;
    FontVariant                        fVariant;
    int                                fOrder; // internal to SkFontConfigParser
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
