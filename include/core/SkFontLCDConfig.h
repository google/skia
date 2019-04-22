/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontLCDConfig_DEFINED
#define SkFontLCDConfig_DEFINED

#include "include/core/SkTypes.h"

class SK_API SkFontLCDConfig {
public:
    /** LCDs either have their color elements arranged horizontally or
        vertically. When rendering subpixel glyphs we need to know which way
        round they are.

        Note, if you change this after startup, you'll need to flush the glyph
        cache because it'll have the wrong type of masks cached.

        @deprecated use SkPixelGeometry instead.
    */
    enum LCDOrientation {
        kHorizontal_LCDOrientation = 0,    //!< this is the default
        kVertical_LCDOrientation   = 1,
    };

    /** @deprecated set on Device creation. */
    static void SetSubpixelOrientation(LCDOrientation orientation);
    /** @deprecated get from Device. */
    static LCDOrientation GetSubpixelOrientation();

    /** LCD color elements can vary in order. For subpixel text we need to know
        the order which the LCDs uses so that the color fringes are in the
        correct place.

        Note, if you change this after startup, you'll need to flush the glyph
        cache because it'll have the wrong type of masks cached.

        kNONE_LCDOrder means that the subpixel elements are not spatially
        separated in any usable fashion.

        @deprecated use SkPixelGeometry instead.
     */
    enum LCDOrder {
        kRGB_LCDOrder = 0,    //!< this is the default
        kBGR_LCDOrder = 1,
        kNONE_LCDOrder = 2,
    };

    /** @deprecated set on Device creation. */
    static void SetSubpixelOrder(LCDOrder order);
    /** @deprecated get from Device. */
    static LCDOrder GetSubpixelOrder();
};

#endif
