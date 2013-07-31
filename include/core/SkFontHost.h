
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFontHost_DEFINED
#define SkFontHost_DEFINED

#include "SkTypeface.h"

//#define SK_FONTHOST_USES_FONTMGR

class SkDescriptor;
class SkScalerContext;
struct SkScalerContextRec;
class SkStream;
class SkWStream;

/** \class SkFontHost

    This class is ported to each environment. It is responsible for bridging
    the gap between the (sort of) abstract class SkTypeface and the
    platform-specific implementation that provides access to font files.

    One basic task is for each create (subclass of) SkTypeface, the FontHost is
    responsible for assigning a uniqueID. The ID should be unique for the
    underlying font file/data, not unique per typeface instance. Thus it is
    possible/common to request a typeface for the same font more than once
    (e.g. asking for the same font by name several times). The FontHost may
    return seperate typeface instances in that case, or it may choose to use a
    cache and return the same instance (but calling typeface->ref(), since the
    caller is always responsible for calling unref() on each instance that is
    returned). Either way, the fontID for those instance(s) will be the same.
    In addition, the fontID should never be set to 0. That value is used as a
    sentinel to indicate no-font-id.

    The major aspects are:
    1) Given either a name/style, return a subclass of SkTypeface that
        references the closest matching font available on the host system.
    2) Given the data for a font (either in a stream or a file name), return
        a typeface that allows access to that data.
    3) Each typeface instance carries a 32bit ID for its corresponding font.
        SkFontHost turns that ID into a stream to access the font's data.
    4) Given a font ID, return a subclass of SkScalerContext, which connects a
        font scaler (e.g. freetype or other) to the font's data.
    5) Utilites to manage the font cache (budgeting) and gamma correction
*/
class SK_API SkFontHost {
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
        kVertical_LCDOrientation   = 1
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
        kNONE_LCDOrder = 2
    };

    /** @deprecated set on Device creation. */
    static void SetSubpixelOrder(LCDOrder order);
    /** @deprecated get from Device. */
    static LCDOrder GetSubpixelOrder();

private:
    /** Return a new, closest matching typeface given either an existing family
        (specified by a typeface in that family) or by a familyName and a
        requested style.
        1) If familyFace is null, use familyName.
        2) If familyName is null, use data (UTF-16 to cover).
        3) If all are null, return the default font that best matches style
     */
    static SkTypeface* CreateTypeface(const SkTypeface* familyFace,
                                      const char familyName[],
                                      SkTypeface::Style style);

    /** Return a new typeface given the data buffer. If the data does not
        represent a valid font, returns null.

        If a typeface instance is returned, the caller is responsible for
        calling unref() on the typeface when they are finished with it.

        The returned typeface may or may not have called ref() on the stream
        parameter. If the typeface has not called ref(), then it may have made
        a copy of the releveant data. In either case, the caller is still
        responsible for its refcnt ownership of the stream.
     */
    static SkTypeface* CreateTypefaceFromStream(SkStream*);

    /** Return a new typeface from the specified file path. If the file does not
        represent a valid font, this returns null. If a typeface is returned,
        the caller is responsible for calling unref() when it is no longer used.
     */
    static SkTypeface* CreateTypefaceFromFile(const char path[]);

    ///////////////////////////////////////////////////////////////////////////

    friend class SkScalerContext;
    friend class SkTypeface;
};

#endif
