
/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPaintOptionsAndroid_DEFINED
#define SkPaintOptionsAndroid_DEFINED

#include "SkTypes.h"
#include "SkString.h"

class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;

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

class SkPaintOptionsAndroid {
public:
    SkPaintOptionsAndroid() {
        fFontVariant = kDefault_Variant;
        fUseFontFallbacks = false;
    }

    SkPaintOptionsAndroid& operator=(const SkPaintOptionsAndroid& b) {
        fLanguage = b.fLanguage;
        fFontVariant = b.fFontVariant;
        fUseFontFallbacks = b.fUseFontFallbacks;
        return *this;
    }

    bool operator==(const SkPaintOptionsAndroid& b) const {
        return !(*this != b);
    }

    bool operator!=(const SkPaintOptionsAndroid& b) const {
        return fLanguage != b.fLanguage ||
               fFontVariant != b.fFontVariant ||
               fUseFontFallbacks != b.fUseFontFallbacks;
    }

    void flatten(SkFlattenableWriteBuffer&) const;
    void unflatten(SkFlattenableReadBuffer&);

    /** Return the paint's language value used for drawing text.
        @return the paint's language value used for drawing text.
    */
    const SkLanguage& getLanguage() const { return fLanguage; }

    /** Set the paint's language value used for drawing text.
        @param language set the paint's language value for drawing text.
    */
    void setLanguage(const SkLanguage& language) { fLanguage = language; }
    void setLanguage(const char* languageTag) { fLanguage = SkLanguage(languageTag); }


    enum FontVariant {
       kDefault_Variant = 0x01,
       kCompact_Variant = 0x02,
       kElegant_Variant = 0x04,
       kLast_Variant = kElegant_Variant,
    };

    /** Return the font variant
        @return the font variant used by this paint object
    */
    FontVariant getFontVariant() const { return fFontVariant; }

    /** Set the font variant
      @param fontVariant set the paint's font variant for choosing fonts
    */
    void setFontVariant(FontVariant fontVariant) {
        SkASSERT((unsigned)fontVariant <= kLast_Variant);
        fFontVariant = fontVariant;
    }

    bool isUsingFontFallbacks() const { return fUseFontFallbacks; }

    void setUseFontFallbacks(bool useFontFallbacks) {
        fUseFontFallbacks = useFontFallbacks;
    }

private:
    SkLanguage fLanguage;
    FontVariant fFontVariant;
    bool fUseFontFallbacks;
};

#endif // #ifndef SkPaintOptionsAndroid_DEFINED
