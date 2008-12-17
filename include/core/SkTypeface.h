/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkTypeface_DEFINED
#define SkTypeface_DEFINED

#include "SkRefCnt.h"

class SkStream;
class SkWStream;

/** \class SkTypeface

    The SkTypeface class specifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).

    Typeface objects are immutable, and so they can be shred between threads.
    To enable this, Typeface inherits from the thread-safe version of SkRefCnt.
*/
class SkTypeface : public SkRefCnt {
public:
    /** Style specifies the intrinsic style attributes of a given typeface
    */
    enum Style {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,

        // helpers
        kBoldItalic = 0x03
    };

    /** Returns the typeface's intrinsic style attributes
    */
    Style style() const { return fStyle; }
    
    /** DEPRECATED */
    Style getStyle() const { return this->style(); }

    /** Returns true if getStyle() has the kBold bit set.
    */
    bool isBold() const { return (fStyle & kBold) != 0; }

    /** Returns true if getStyle() has the kItalic bit set.
    */
    bool isItalic() const { return (fStyle & kItalic) != 0; }
    
    uint32_t uniqueID() const { return fUniqueID; }

    /** Return the uniqueID for the specified typeface. If the face is null,
        resolve it to the default font and return its uniqueID.
    */
    static uint32_t UniqueID(const SkTypeface* face);

    /** Return a new reference to the typeface that most closely matches the
        requested familyName and style. Pass null as the familyName to return
        the default font for the requested style. Will never return null
        
        @param familyName  May be NULL. The name of the font family.
        @param style       The style (normal, bold, italic) of the typeface.
        @return reference to the closest-matching typeface. Call must call
                unref() when they are done.
    */
    static SkTypeface* Create(const char familyName[], Style style = kNormal);

    /** Return a new reference to the typeface that most closely matches the
        requested typeface and specified Style. Use this call if you want to
        pick a new style from the same family of the existing typeface.
        If family is NULL, this selects from the default font's family.
        
        @param family  May be NULL. The name of the existing type face.
        @param s       The style (normal, bold, italic) of the type face.
        @return reference to the closest-matching typeface. Call must call
                unref() when they are done.
    */
    static SkTypeface* CreateFromTypeface(const SkTypeface* family, Style s);

    /** Returns true if the two typefaces reference the same underlying font,
        even if one is null (which maps to the default font).
    */
    static bool Equal(const SkTypeface* facea, const SkTypeface* faceb);
    
    /** Returns a 32bit hash value for the typeface. Takes care of mapping null
        to the default typeface.
    */
    static uint32_t Hash(const SkTypeface* face);

    /** Return a new typeface given a file. If the file does not exist, or is
        not a valid font file, returns null.
    */
    static SkTypeface* CreateFromFile(const char path[]);
    
    /** Return a new typeface given a stream. If the stream is
        not a valid font file, returns null. Ownership of the stream is
        transferred, so the caller must not reference it again.
    */
    static SkTypeface* CreateFromStream(SkStream* stream);

    // Serialization
    void serialize(SkWStream*) const;
    static SkTypeface* Deserialize(SkStream*);

protected:
    /** uniqueID must be unique (please!) and non-zero
    */
    SkTypeface(Style style, uint32_t uniqueID)
        : fUniqueID(uniqueID), fStyle(style) {}

private:
    uint32_t    fUniqueID;
    Style       fStyle;
    
    typedef SkRefCnt INHERITED;
};

#endif
