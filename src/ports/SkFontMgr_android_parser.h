/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_android_parser_DEFINED
#define SkFontMgr_android_parser_DEFINED

#include "SkFontMgr.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTypes.h"

#include <climits>
#include <limits>

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

// Must remain trivially movable (can be memmoved).
struct FontFileInfo {
    FontFileInfo() : fIndex(0), fWeight(0), fStyle(Style::kAuto) { }

    SkString fFileName;
    int fIndex;
    int fWeight;
    enum class Style { kAuto, kNormal, kItalic } fStyle;
    SkTArray<SkFontArguments::VariationPosition::Coordinate, true> fVariationDesignPosition;
};

/**
 * A font family provides one or more names for a collection of fonts, each of
 * which has a different style (normal, italic) or weight (thin, light, bold,
 * etc).
 * Some fonts may occur in compact variants for use in the user interface.
 * Android distinguishes "fallback" fonts to support non-ASCII character sets.
 */
struct FontFamily {
    FontFamily(const SkString& basePath, bool isFallbackFont)
        : fVariant(kDefault_FontVariant)
        , fOrder(-1)
        , fIsFallbackFont(isFallbackFont)
        , fBasePath(basePath)
    { }

    SkTArray<SkString, true> fNames;
    SkTArray<FontFileInfo, true> fFonts;
    SkLanguage fLanguage;
    FontVariant fVariant;
    int fOrder; // internal to the parser, not useful to users.
    bool fIsFallbackFont;
    const SkString fBasePath;
};

namespace SkFontMgr_Android_Parser {

/** Parses system font configuration files and appends result to fontFamilies. */
void GetSystemFontFamilies(SkTDArray<FontFamily*>& fontFamilies);

/** Parses font configuration files and appends result to fontFamilies. */
void GetCustomFontFamilies(SkTDArray<FontFamily*>& fontFamilies,
                           const SkString& basePath,
                           const char* fontsXml,
                           const char* fallbackFontsXml,
                           const char* langFallbackFontsDir = nullptr);

} // SkFontMgr_Android_Parser namespace


/** Parses a null terminated string into an integer type, checking for overflow.
 *  http://www.w3.org/TR/html-markup/datatypes.html#common.data.integer.non-negative-def
 *
 *  If the string cannot be parsed into 'value', returns false and does not change 'value'.
 */
template <typename T> static bool parse_non_negative_integer(const char* s, T* value) {
    static_assert(std::numeric_limits<T>::is_integer, "T_must_be_integer");

    if (*s == '\0') {
        return false;
    }

    const T nMax = std::numeric_limits<T>::max() / 10;
    const T dMax = std::numeric_limits<T>::max() - (nMax * 10);
    T n = 0;
    for (; *s; ++s) {
        // Check if digit
        if (*s < '0' || '9' < *s) {
            return false;
        }
        T d = *s - '0';
        // Check for overflow
        if (n > nMax || (n == nMax && d > dMax)) {
            return false;
        }
        n = (n * 10) + d;
    }
    *value = n;
    return true;
}

/** Parses a null terminated string into a signed fixed point value with bias N.
 *
 *  Like http://www.w3.org/TR/html-markup/datatypes.html#common.data.float-def ,
 *  but may start with '.' and does not support 'e'. '-?((:digit:+(.:digit:+)?)|(.:digit:+))'
 *
 *  Checks for overflow.
 *  Low bit rounding is not defined (is currently truncate).
 *  Bias (N) required to allow for the sign bit and 4 bits of integer.
 *
 *  If the string cannot be parsed into 'value', returns false and does not change 'value'.
 */
template <int N, typename T> static bool parse_fixed(const char* s, T* value) {
    static_assert(std::numeric_limits<T>::is_integer, "T_must_be_integer");
    static_assert(std::numeric_limits<T>::is_signed, "T_must_be_signed");
    static_assert(sizeof(T) * CHAR_BIT - N >= 5, "N_must_leave_four_bits_plus_sign");

    bool negate = false;
    if (*s == '-') {
        ++s;
        negate = true;
    }
    if (*s == '\0') {
        return false;
    }

    const T nMax = (std::numeric_limits<T>::max() >> N) / 10;
    const T dMax = (std::numeric_limits<T>::max() >> N) - (nMax * 10);
    T n = 0;
    T frac = 0;
    for (; *s; ++s) {
        // Check if digit
        if (*s < '0' || '9' < *s) {
            // If it wasn't a digit, check if it is a '.' followed by something.
            if (*s != '.' || s[1] == '\0') {
                return false;
            }
            // Find the end, verify digits.
            for (++s; *s; ++s) {
                if (*s < '0' || '9' < *s) {
                    return false;
                }
            }
            // Read back toward the '.'.
            for (--s; *s != '.'; --s) {
                T d = *s - '0';
                frac = (frac + (d << N)) / 10; // This requires four bits overhead.
            }
            break;
        }
        T d = *s - '0';
        // Check for overflow
        if (n > nMax || (n == nMax && d > dMax)) {
            return false;
        }
        n = (n * 10) + d;
    }
    if (negate) {
        n = -n;
        frac = -frac;
    }
    *value = SkLeftShift(n, N) + frac;
    return true;
}

#endif /* SkFontMgr_android_parser_DEFINED */
