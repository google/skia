/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectPriv_DEFINED
#define SkRectPriv_DEFINED

#include "SkRect.h"

class SkRectPriv {
public:
    // Returns true iff width and height are positive. Catches inverted, empty, and overflowing
    // (way too big) rects. This is used by clients that want a non-empty rect that they can also
    // actually use its computed width/height.
    //
    static bool PositiveDimensions(const SkIRect& r) {
        return r.width() > 0 && r.height() > 0;
    }

    static void SetLargestInverted(SkIRect* r) {
        r->fLeft = r->fTop = SK_MaxS32;
        r->fRight = r->fBottom = SK_MinS32;
    }

    static void SetLargest(SkRect* r) {
        r->fLeft = r->fTop = SK_ScalarMin;
        r->fRight = r->fBottom = SK_ScalarMax;
    }

    static void SetLargestInverted(SkRect* r) {
        r->fLeft = r->fTop = SK_ScalarMax;
        r->fRight = r->fBottom = SK_ScalarMin;
    }

    static SkRect MakeLargestS32() {
        const int32_t ihalf = SK_MaxS32 >> 1;
        const SkScalar half = SkIntToScalar(ihalf);

        return { -half, -half, half, half };
    }

#if 0
    /** Returns true if SkIRect encloses largest possible area.

     @return  true if equal to (SK_MinS32, SK_MinS32, SK_MaxS32, SK_MaxS32)
     */
    bool isLargest() const { return SK_MinS32 == fLeft &&
        SK_MinS32 == fTop &&
        SK_MaxS32 == fRight &&
        SK_MaxS32 == fBottom; }

    /** Sets rectangle left and top to most negative value, and sets
     right and bottom to most positive value.
     */
    void setLargest() {
        fLeft = fTop = SK_MinS32;
        fRight = fBottom = SK_MaxS32;
    }

    /** Sets rectangle left and top to most positive value, and sets
     right and bottom to most negative value. This is used internally to
     flag that a condition is met, but otherwise has no special purpose.
     */
    void setLargestInverted() {
        fLeft = fTop = SK_MaxS32;
        fRight = fBottom = SK_MinS32;
    }

    /** Returns constructed SkRect that can be represented exactly with SkIRect. The left
     and top are set to the most negative integer value that fits in a 32-bit float,
     and the right and bottom are set to the most positive finite value that fits in
     a 32-bit float.

     These are the largest values for which round() is well defined.

     @return  bounds (SK_MinS32FitsInFloat, SK_MinS32FitsInFloat,
     SK_MaxS32FitsInFloat, SK_MaxS32FitsInFloat)
     */
    static SkRect SK_WARN_UNUSED_RESULT MakeLargestS32() {
        const SkRect r = MakeLTRB(SK_MinS32FitsInFloat, SK_MinS32FitsInFloat,
                                  SK_MaxS32FitsInFloat, SK_MaxS32FitsInFloat);
        SkASSERT(r == Make(r.roundOut()));
        return r;
    }

    /** Returns true if SkRect encloses largest possible area.

     @return  true if equal to (SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)
     */
    bool isLargest() const { return SK_ScalarMin == fLeft &&
        SK_ScalarMin == fTop &&
        SK_ScalarMax == fRight &&
        SK_ScalarMax == fBottom; }

    /** Sets rectangle left and top to most negative finite value, and sets
     right and bottom to most positive finite value.
     */
    void setLargest() {
        fLeft = fTop = SK_ScalarMin;
        fRight = fBottom = SK_ScalarMax;
    }
#endif
};


#endif
