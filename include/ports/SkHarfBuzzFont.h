/*
 * Copyright 2009 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHarfBuzzFont_DEFINED
#define SkHarfBuzzFont_DEFINED

extern "C" {
#include "harfbuzz-shaper.h"
//#include "harfbuzz-unicode.h"
}

#include "SkTypes.h"

class SkPaint;
class SkTypeface;

class SkHarfBuzzFont {
public:
    /** The subclass returns the typeface for this font, or NULL
     */
    virtual SkTypeface* getTypeface() const = 0;
    /** The subclass sets the text related attributes of the paint.
        e.g. textSize, typeface, textSkewX, etc.
        All of the attributes that could effect how the text is measured.
        Color information (e.g. color, xfermode, shader, etc.) are not required.
     */
    virtual void setupPaint(SkPaint*) const = 0;
    
    /** Implementation of HB_GetFontTableFunc, using SkHarfBuzzFont* as
        the first parameter.
     */
    static HB_Error GetFontTableFunc(void* skharfbuzzfont, const HB_Tag tag,
                                     HB_Byte* buffer, HB_UInt* len);

    static const HB_FontClass& GetFontClass();
};

#endif
