/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaint_DEFINED
#define SkPaint_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFilterQuality.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"

class SkAutoDescriptor;
class SkAutoGlyphCache;
class SkColorFilter;
class SkData;
class SkDescriptor;
class SkDrawLooper;
class SkReadBuffer;
class SkWriteBuffer;
class SkGlyph;
struct SkRect;
class SkGlyphCache;
class SkImageFilter;
class SkMaskFilter;
class SkPath;
class SkPathEffect;
struct SkPoint;
class SkRasterizer;
struct SkScalerContextEffects;
class SkShader;
class SkSurfaceProps;
class SkTextBlob;
class SkTypeface;

/** \class SkPaint
    SkPaint controls options applied when drawing and measuring. SkPaint collects all
    options outside of the SkCanvas clip and SkCanvas matrix.

    Various options apply to text, strokes and fills, and images.

    Some options may not be implemented on all platforms; in these cases, setting
    the option has no effect. Some options are conveniences that duplicate SkCanvas
    functionality; for instance, text size is identical to matrix scale.

    SkPaint options are rarely exclusive; each option modifies a stage of the drawing
    pipeline and multiple pipeline stages may be affected by a single SkPaint.

    SkPaint collects effects and filters that describe single-pass and multiple-pass
    algorithms that alter the drawing geometry, color, and transparency. For instance,
    SkPaint does not directly implement dashing or blur, but contains the objects that do so.

    The objects contained by SkPaint are opaque, and cannot be edited outside of the SkPaint
    to affect it. The implementation is free to defer computations associated with the
    SkPaint, or ignore them altogether. For instance, some GPU implementations draw all
    SkPath geometries with anti-aliasing, regardless of how SkPaint::kAntiAlias_Flag
    is set in SkPaint.

    SkPaint describes a single color, a single font, a single image quality, and so on.
    Multiple colors are drawn either by using multiple paints or with objects like
    SkShader attached to SkPaint.
*/
class SK_API SkPaint {
public:

    /** Constructs SkPaint with default values.

        @return  default initialized SkPaint
    */
    SkPaint();

    /** Makes a shallow copy of SkPaint. SkTypeface, SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkRasterizer, SkDrawLooper, and SkImageFilter are shared
        between the original paint and the copy. Objects containing SkRefCnt increment
        their references by one.

        The referenced objects SkPathEffect, SkShader, SkMaskFilter, SkColorFilter, SkRasterizer,
        SkDrawLooper, and SkImageFilter cannot be modified after they are created.
        This prevents objects with SkRefCnt from being modified once SkPaint refers to them.

        @param paint  original to copy
        @return       shallow copy of paint
    */
    SkPaint(const SkPaint& paint);

    /** Implements a move constructor to avoid increasing the reference counts
        of objects referenced by the paint.

        After the call, paint is undefined, and can be safely destructed.

        @param paint  original to move
        @return       content of paint
    */
    SkPaint(SkPaint&& paint);

    /** Decreases SkPaint SkRefCnt of owned objects: SkTypeface, SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkRasterizer, SkDrawLooper, and SkImageFilter. If the
        objects containing SkRefCnt go to zero, they are deleted.
    */
    ~SkPaint();

    /** Makes a shallow copy of SkPaint. SkTypeface, SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkRasterizer, SkDrawLooper, and SkImageFilter are shared
        between the original paint and the copy. Objects containing SkRefCnt in the
        prior destination are decreased by one, and the referenced objects are deleted if the
        resulting count is zero. Objects containing SkRefCnt in the parameter paint
        are increased by one. paint is unmodified.

        @param paint  original to copy
        @return       content of paint
    */
    SkPaint& operator=(const SkPaint& paint);

    /** Moves the paint to avoid increasing the reference counts
        of objects referenced by the paint parameter. Objects containing SkRefCnt in the
        prior destination are decreased by one; those objects are deleted if the resulting count
        is zero.

        After the call, paint is undefined, and can be safely destructed.

        @param paint  original to move
        @return       content of paint
    */
    SkPaint& operator=(SkPaint&& paint);

    /** Compares a and b, and returns true if a and b are equivalent. May return false
        if SkTypeface, SkPathEffect, SkShader, SkMaskFilter, SkColorFilter, SkRasterizer,
        SkDrawLooper, or SkImageFilter have identical contents but different pointers.

        @param a  SkPaint to compare
        @param b  SkPaint to compare
        @return   true if SkPaint pair are equivalent
    */
    SK_API friend bool operator==(const SkPaint& a, const SkPaint& b);

    /** Compares a and b, and returns true if a and b are not equivalent. May return true
        if SkTypeface, SkPathEffect, SkShader, SkMaskFilter, SkColorFilter, SkRasterizer,
        SkDrawLooper, or SkImageFilter have identical contents but different pointers.

        @param a  SkPaint to compare
        @param b  SkPaint to compare
        @return   true if SkPaint pair are not equivalent
    */
    friend bool operator!=(const SkPaint& a, const SkPaint& b) {
        return !(a == b);
    }

    /** Returns a hash generated from SkPaint values and pointers.
        Identical hashes guarantee that the paints are
        equivalent, but differing hashes do not guarantee that the paints have differing
        contents.

        If operator==(const SkPaint& a, const SkPaint& b) returns true for two paints,
        their hashes are also equal.

        The hash returned is platform and implementation specific.

        @return  a shallow hash
    */
    uint32_t getHash() const;

    /** Serializes SkPaint into a buffer. A companion unflatten() call
        can reconstitute the paint at a later time.

        @param buffer  SkWriteBuffer receiving the flattened SkPaint data
    */
    void flatten(SkWriteBuffer& buffer) const;

    /** Populates SkPaint, typically from a serialized stream, created by calling
        flatten() at an earlier time.

        SkReadBuffer class is not public, so unflatten() cannot be meaningfully called
        by the client.

        @param buffer  serialized data describing SkPaint content
    */
    void unflatten(SkReadBuffer& buffer);

    /** Sets all SkPaint contents to their initial values. This is equivalent to replacing
        SkPaint with the result of SkPaint().
    */
    void reset();

    /** \enum SkPaint::Hinting
        Hinting adjusts the glyph outlines so that the shape provides a uniform
        look at a given point size on font engines that support it. Hinting may have a
        muted effect or no effect at all depending on the platform.

        The four levels roughly control corresponding features on platforms that use FreeType
        as the font engine.
    */
    enum Hinting {
        /** Leaves glyph outlines unchanged from their native representation.
            With FreeType, this is equivalent to the FT_LOAD_NO_HINTING
            bit-field constant supplied to FT_Load_Glyph, which indicates that the vector
            outline being loaded should not be fitted to the pixel grid but simply scaled
            to 26.6 fractional pixels.
        */
        kNo_Hinting     = 0,

        /** Modifies glyph outlines minimally to improve constrast.
            With FreeType, this is equivalent in spirit to the
            FT_LOAD_TARGET_LIGHT value supplied to FT_Load_Glyph. It chooses a
            lighter hinting algorithm for non-monochrome modes.
            Generated glyphs may be fuzzy but better resemble their original shape.
        */
        kSlight_Hinting = 1,

        /** Modifies glyph outlines to improve constrast. This is the default.
            With FreeType, this supplies FT_LOAD_TARGET_NORMAL to FT_Load_Glyph,
            choosing the default hinting algorithm, which is optimized for standard
            gray-level rendering.
        */
        kNormal_Hinting = 2,

        /** Modifies glyph outlines for maxiumum constrast. With FreeType, this selects
            FT_LOAD_TARGET_LCD or FT_LOAD_TARGET_LCD_V if kLCDRenderText_Flag is set.
            FT_LOAD_TARGET_LCD is a variant of FT_LOAD_TARGET_NORMAL optimized for
            horizontally decimated LCD displays; FT_LOAD_TARGET_LCD_V is a
            variant of FT_LOAD_TARGET_NORMAL optimized for vertically decimated LCD displays.
        */
        kFull_Hinting   = 3,
    };

    /** Returns level of glyph outline adjustment.

        @return  one of: kNo_Hinting, kSlight_Hinting, kNormal_Hinting, kFull_Hinting
    */
    Hinting getHinting() const {
        return static_cast<Hinting>(fBitfields.fHinting);
    }

    /** Sets level of glyph outline adjustment.
        Does not check for valid values of hintingLevel.

        @param hintingLevel  one of: kNo_Hinting, kSlight_Hinting, kNormal_Hinting, kFull_Hinting
    */
    void setHinting(Hinting hintingLevel);

    /** \enum SkPaint::Flags
        The bit values stored in Flags.
        The default value for Flags, normally zero, can be changed at compile time
        with a custom definition of SkPaintDefaults_Flags.
        All flags can be read and written explicitly; Flags allows manipulating
        multiple settings at once.
    */
    enum Flags {
        kAntiAlias_Flag          = 0x01,   //!< mask for setting anti-alias
        kDither_Flag             = 0x04,   //!< mask for setting dither
        kFakeBoldText_Flag       = 0x20,   //!< mask for setting fake bold
        kLinearText_Flag         = 0x40,   //!< mask for setting linear text
        kSubpixelText_Flag       = 0x80,   //!< mask for setting subpixel text
        kDevKernText_Flag        = 0x100,  //!< mask for setting full hinting spacing
        kLCDRenderText_Flag      = 0x200,  //!< mask for setting lcd text
        kEmbeddedBitmapText_Flag = 0x400,  //!< mask for setting font embedded bitmaps
        kAutoHinting_Flag        = 0x800,  //!< mask for setting auto-hinting
        kVerticalText_Flag       = 0x1000, //!< mask for setting vertical text

        /** Hack for GDI -- do not use if you can help it */
        kGenA8FromLCD_Flag       = 0x2000,

        /** mask of all Flags, including private flags and flags reserved for future use */
        kAllFlags                = 0xFFFF,
    };

    #ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    enum ReserveFlags {
        kUnderlineText_ReserveFlag  = 0x08, //!< mask for underline text
        kStrikeThruText_ReserveFlag = 0x10, //!< mask for strike-thru text
    };
    #endif

    /** Returns paint settings described by SkPaint::Flags. Each setting uses one
        bit, and can be tested with SkPaint::Flags members.

        @return  zero, one, or more bits described by SkPaint::Flags
    */
    uint32_t getFlags() const { return fBitfields.fFlags; }

    /** Replaces SkPaint::Flags with flags, the union of the SkPaint::Flags members.
        All SkPaint::Flags members may be cleared, or one or more may be set.

        @param flags  union of SkPaint::Flags for SkPaint
    */
    void setFlags(uint32_t flags);

    /** If true, pixels on the active edges of SkPath may be drawn with partial transparency.

        Equivalent to getFlags() masked with kAntiAlias_Flag.

        @return  kAntiAlias_Flag state
    */
    bool isAntiAlias() const {
        return SkToBool(this->getFlags() & kAntiAlias_Flag);
    }

    /** Requests, but does not require, that SkPath edge pixels draw opaque or with
        partial transparency.

        Sets kAntiAlias_Flag if aa is true.
        Clears kAntiAlias_Flag if aa is false.

        @param aa  setting for kAntiAlias_Flag
    */
    void setAntiAlias(bool aa);

    /** If true, color error may be distributed to smooth color transition.

        Equivalent to getFlags() masked with kDither_Flag.

        @return  kDither_Flag state
    */
    bool isDither() const {
        return SkToBool(this->getFlags() & kDither_Flag);
    }

    /** Requests, but does not require, to distribute color error.

        Sets kDither_Flag if dither is true.
        Clears kDither_Flag if dither is false.

        @param dither  setting for kDither_Flag
    */
    void setDither(bool dither);

    /** If true, text is converted to SkPath before drawing and measuring.

        Equivalent to getFlags() masked with kLinearText_Flag.

        @return  kLinearText_Flag state
    */
    bool isLinearText() const {
        return SkToBool(this->getFlags() & kLinearText_Flag);
    }

    /** If true, text is converted to SkPath before drawing and measuring.
        By default, kLinearText_Flag is clear.

        Sets kLinearText_Flag if linearText is true.
        Clears kLinearText_Flag if linearText is false.

        @param linearText  setting for kLinearText_Flag
    */
    void setLinearText(bool linearText);

    /** If true, glyphs at different sub-pixel positions may differ on pixel edge coverage.

        Equivalent to getFlags() masked with kSubpixelText_Flag.

        @return  kSubpixelText_Flag state
    */
    bool isSubpixelText() const {
        return SkToBool(this->getFlags() & kSubpixelText_Flag);
    }

    /** Requests, but does not require, that glyphs respect sub-pixel positioning.

        Sets kSubpixelText_Flag if subpixelText is true.
        Clears kSubpixelText_Flag if subpixelText is false.

        @param subpixelText  setting for kSubpixelText_Flag
    */
    void setSubpixelText(bool subpixelText);

    /** If true, glyphs may use LCD striping to improve glyph edges.

        Returns true if SkPaint::Flags kLCDRenderText_Flag is set.

        @return  kLCDRenderText_Flag state
    */
    bool isLCDRenderText() const {
        return SkToBool(this->getFlags() & kLCDRenderText_Flag);
    }

    /** Requests, but does not require, that glyphs use LCD striping for glyph edges.

        Sets kLCDRenderText_Flag if lcdText is true.
        Clears kLCDRenderText_Flag if lcdText is false.

        @param lcdText  setting for kLCDRenderText_Flag
    */
    void setLCDRenderText(bool lcdText);

    /** If true, font engine may return glyphs from font bitmaps instead of from outlines.

        Equivalent to getFlags() masked with kEmbeddedBitmapText_Flag.

        @return  kEmbeddedBitmapText_Flag state
    */
    bool isEmbeddedBitmapText() const {
        return SkToBool(this->getFlags() & kEmbeddedBitmapText_Flag);
    }

    /** Requests, but does not require, to use bitmaps in fonts instead of outlines.

        Sets kEmbeddedBitmapText_Flag if useEmbeddedBitmapText is true.
        Clears kEmbeddedBitmapText_Flag if useEmbeddedBitmapText is false.

        @param useEmbeddedBitmapText  setting for kEmbeddedBitmapText_Flag
    */
    void setEmbeddedBitmapText(bool useEmbeddedBitmapText);

    /** If true, and if SkPaint::Hinting is set to kNormal_Hinting or kFull_Hinting, and if
        platform uses FreeType as the font manager, instruct the font manager to always hint
        glyphs.

        Equivalent to getFlags() masked with kAutoHinting_Flag.

        @return  kAutoHinting_Flag state
    */
    bool isAutohinted() const {
        return SkToBool(this->getFlags() & kAutoHinting_Flag);
    }

    /** If SkPaint::Hinting is set to kNormal_Hinting or kFull_Hinting and useAutohinter is set,
        instruct the font manager to always hint glyphs.
        auto-hinting has no effect if SkPaint::Hinting is set to kNo_Hinting or
        kSlight_Hinting.

        Only affects platforms that use FreeType as the font manager.

        Sets kAutoHinting_Flag if useAutohinter is true.
        Clears kAutoHinting_Flag if useAutohinter is false.

        @param useAutohinter  setting for kAutoHinting_Flag
    */
    void setAutohinted(bool useAutohinter);

    /** If true, glyphs are drawn top to bottom instead of left to right.

        Equivalent to getFlags() masked with kVerticalText_Flag.

        @return  kVerticalText_Flag state
    */
    bool isVerticalText() const {
        return SkToBool(this->getFlags() & kVerticalText_Flag);
    }

    /** If true, text advance positions the next glyph below the previous glyph instead of to the
        right of previous glyph.

        Sets kVerticalText_Flag if vertical is true.
        Clears kVerticalText_Flag if vertical is false.

        @param verticalText  setting for kVerticalText_Flag
    */
    void setVerticalText(bool verticalText);

    /** If true, approximate bold by increasing the stroke width when creating glyph bitmaps
        from outlines.

        Equivalent to getFlags() masked with kFakeBoldText_Flag.

        @return  kFakeBoldText_Flag state
    */
    bool isFakeBoldText() const {
        return SkToBool(this->getFlags() & kFakeBoldText_Flag);
    }

    /** Use increased stroke width when creating glyph bitmaps to approximate a bold typeface.

        Sets kFakeBoldText_Flag if fakeBoldText is true.
        Clears kFakeBoldText_Flag if fakeBoldText is false.

        @param fakeBoldText  setting for kFakeBoldText_Flag
    */
    void setFakeBoldText(bool fakeBoldText);

    /** Returns if character spacing may be adjusted by the hinting difference.

        Equivalent to getFlags() masked with kDevKernText_Flag.

        @return  kDevKernText_Flag state
    */
    bool isDevKernText() const {
        return SkToBool(this->getFlags() & kDevKernText_Flag);
    }

    /** Requests, but does not require, to use hinting to adjust glyph spacing.

        Sets kDevKernText_Flag if devKernText is true.
        Clears kDevKernText_Flag if devKernText is false.

        @param devKernText  setting for devKernText
    */
    void setDevKernText(bool devKernText);

    /** Returns SkFilterQuality, the image filtering level. A lower setting
        draws faster; a higher setting looks better when the image is scaled.

        @return  one of: kNone_SkFilterQuality, kLow_SkFilterQuality,
                 kMedium_SkFilterQuality, kHigh_SkFilterQuality
    */
    SkFilterQuality getFilterQuality() const {
        return (SkFilterQuality)fBitfields.fFilterQuality;
    }

    /** Sets SkFilterQuality, the image filtering level. A lower setting
        draws faster; a higher setting looks better when the image is scaled.
        Does not check to see if quality is valid.

        @param quality  one of: kNone_SkFilterQuality, kLow_SkFilterQuality,
                        kMedium_SkFilterQuality, kHigh_SkFilterQuality
    */
    void setFilterQuality(SkFilterQuality quality);

    /** \enum SkPaint::Style
        Set Style to fill, stroke, or both fill and stroke geometry.
        The stroke and fill
        share all paint attributes; for instance, they are drawn with the same color.

        Use kStrokeAndFill_Style to avoid hitting the same pixels twice with a stroke draw and
        a fill draw.
    */
    enum Style {
        /** Set to fill geometry.
            Applies to SkRect, SkRegion, SkRRect, circles, ovals, SkPath, and text.
            SkBitmap, SkImage, patches, SkRegion, sprites, and vertices are painted as if
            kFill_Style is set, and ignore the set Style.
            The FillType specifies additional rules to fill the area outside the path edge,
            and to create an unfilled hole inside the shape.
            Style is set to kFill_Style by default.
        */
        kFill_Style,

        /** Set to stroke geometry.
            Applies to SkRect, SkRegion, SkRRect, arcs, circles, ovals, SkPath, and text.
            Arcs, lines, and points, are always drawn as if kStroke_Style is set,
            and ignore the set Style.
            The stroke construction is unaffected by the FillType.
        */
        kStroke_Style,

        /** Set to stroke and fill geometry.
            Applies to SkRect, SkRegion, SkRRect, circles, ovals, SkPath, and text.
            SkPath is treated as if it is set to SkPath::kWinding_FillType,
            and the set FillType is ignored.
        */
        kStrokeAndFill_Style,
    };

    enum {
        /** The number of different Style values defined.
            May be used to verify that Style is a legal value.
        */
        kStyleCount = kStrokeAndFill_Style + 1,
    };

    /** Whether the geometry is filled, stroked, or filled and stroked.

        @return  one of:kFill_Style, kStroke_Style, kStrokeAndFill_Style
    */
    Style getStyle() const { return (Style)fBitfields.fStyle; }

    /** Sets whether the geometry is filled, stroked, or filled and stroked.
        Has no effect if style is not a legal SkPaint::Style value.

        @param style  one of: kFill_Style, kStroke_Style, kStrokeAndFill_Style
    */
    void setStyle(Style style);

    /** Retrieves alpha and RGB, unpremultiplied, packed into 32 bits.
        Use helpers SkColorGetA(), SkColorGetR(), SkColorGetG(), and SkColorGetB() to extract
        a color component.

        @return  unpremultiplied ARGB
    */
    SkColor getColor() const { return fColor; }

    /** Sets alpha and RGB used when stroking and filling. The color is a 32-bit value,
        unpremultiplied, packing 8-bit components for alpha, red, blue, and green.

        @param color  unpremultiplied ARGB
    */
    void setColor(SkColor color);

    /** Retrieves alpha from the color used when stroking and filling.

        @return  alpha ranging from zero, fully transparent, to 255, fully opaque
    */
    uint8_t getAlpha() const { return SkToU8(SkColorGetA(fColor)); }

    /** Replaces alpha, leaving RGB
        unchanged. An out of range value triggers an assert in the debug
        build. a is a value from zero to 255.
        a set to zero makes color fully transparent; a set to 255 makes color
        fully opaque.

        @param a  alpha component of color
    */
    void setAlpha(U8CPU a);

    /** Sets color used when drawing solid fills. The color components range from 0 to 255.
        The color is unpremultiplied; alpha sets the transparency independent of RGB.

        @param a  amount of color alpha, from fully transparent (0) to fully opaque (255)
        @param r  amount of color rgb red, from no red (0) to full red (255)
        @param g  amount of color rgb green, from no green (0) to full green (255)
        @param b  amount of color rgb blue, from no blue (0) to full blue (255)
    */
    void setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

    /** Returns the thickness of the pen used by SkPaint to
        outline the shape.

        @return  zero for hairline, greater than zero for pen thickness
    */
    SkScalar getStrokeWidth() const { return fWidth; }

    /** Sets the thickness of the pen used by the paint to
        outline the shape.
        Has no effect if width is less than zero.

        @param width  zero thickness for hairline; greater than zero for pen thickness
    */
    void setStrokeWidth(SkScalar width);

    /** The limit at which a sharp corner is drawn beveled.

        @return  zero and greater miter limit
    */
    SkScalar getStrokeMiter() const { return fMiterLimit; }

    /** The limit at which a sharp corner is drawn beveled.
        Valid values are zero and greater.
        Has no effect if miter is less than zero.

        @param miter  zero and greater miter limit
    */
    void setStrokeMiter(SkScalar miter);

    /** \enum SkPaint::Cap
        Cap draws at the beginning and end of an open path contour.
    */
    enum Cap {
        kButt_Cap,                  //!< Does not extend the stroke past the beginning or the end.

        /** Adds a circle with a diameter equal to stroke width at the beginning
            and end.
        */
        kRound_Cap,

        /** Adds a square with sides equal to stroke width at the beginning
            and end. The square sides are parallel to the initial and final direction
            of the stroke.
        */
        kSquare_Cap,
        kLast_Cap    = kSquare_Cap, //!< Equivalent to the largest value for Cap.

        /** Equivalent to kButt_Cap.
            Cap is set to kButt_Cap by default.
        */
        kDefault_Cap = kButt_Cap,
    };

    static constexpr int kCapCount = kLast_Cap + 1;

    /** \enum SkPaint::Join
        Join specifies how corners are drawn when a shape is stroked. Join
        affects the four corners of a stroked rectangle, and the connected segments in a
        stroked path.

        Choose miter join to draw sharp corners. Choose round join to draw a circle with a
        radius equal to the stroke width on top of the corner. Choose bevel join to minimally
        connect the thick strokes.

        The fill path constructed to describe the stroked path respects the join setting but may
        not contain the actual join. For instance, a fill path constructed with round joins does
        not necessarily include circles at each connected segment.
    */
    enum Join {
        /** Extends the outside corner to the extent allowed by miter limit.
            If the extension exceeds miter limit, kBevel_Join is used instead.
        */
        kMiter_Join,

        /** Adds a circle with a diameter of stroke width at the sharp corner. */
        kRound_Join,
        kBevel_Join,                 //!< Connects the outside edges of the sharp corner.
        kLast_Join    = kBevel_Join, //!< Equivalent to the largest value for Join.

        /** Equivalent to kMiter_Join.
            Join is set to kMiter_Join by default.
        */
        kDefault_Join = kMiter_Join,
    };

    static constexpr int kJoinCount = kLast_Join + 1;

    /** The geometry drawn at the beginning and end of strokes.

        @return  one of: kButt_Cap, kRound_Cap, kSquare_Cap
    */
    Cap getStrokeCap() const { return (Cap)fBitfields.fCapType; }

    /** The geometry drawn at the beginning and end of strokes.

        @param cap  one of: kButt_Cap, kRound_Cap, kSquare_Cap;
                    has no effect if cap is not valid
    */
    void setStrokeCap(Cap cap);

    /** The geometry drawn at the corners of strokes.

        @return  one of: kMiter_Join, kRound_Join, kBevel_Join
    */
    Join getStrokeJoin() const { return (Join)fBitfields.fJoinType; }

    /** The geometry drawn at the corners of strokes.

        @param join  one of: kMiter_Join, kRound_Join, kBevel_Join;
                     otherwise, has no effect
    */
    void setStrokeJoin(Join join);

    /** The filled equivalent of the stroked path.

        @param src       SkPath read to create a filled version
        @param dst       resulting SkPath; may be the same as src, but may not be nullptr
        @param cullRect  optional limit passed to SkPathEffect
        @param resScale  if > 1, increase precision, else if (0 < res < 1) reduce precision
                         to favor speed and size
        @return          true if the path represents style fill, or false if it represents hairline
    */
    bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                     SkScalar resScale = 1) const;

    /** The filled equivalent of the stroked path.

        Replaces dst with the src path modified by SkPathEffect and style stroke.
        SkPathEffect, if any, is not culled. stroke width is created with default precision.

        @param src  SkPath read to create a filled version
        @param dst  resulting SkPath dst may be the same as src, but may not be nullptr
        @return     true if the path represents style fill, or false if it represents hairline
    */
    bool getFillPath(const SkPath& src, SkPath* dst) const {
        return this->getFillPath(src, dst, nullptr, 1);
    }

    /** Optional colors used when filling a path, such as a gradient.

        Does not alter SkShader SkRefCnt.

        @return  SkShader if previously set, nullptr otherwise
    */
    SkShader* getShader() const { return fShader.get(); }

    /** Optional colors used when filling a path, such as a gradient.

        Increases SkShader SkRefCnt by one.

        @return  SkShader if previously set, nullptr otherwise
    */
    sk_sp<SkShader> refShader() const;

    /** Optional colors used when filling a path, such as a gradient.

        Sets SkShader to shader, decreasing SkRefCnt of the previous SkShader.
        Increments shader SkRefCnt by one.

        @param shader  how geometry is filled with color; if nullptr, color is used instead
    */
    void setShader(sk_sp<SkShader> shader);

    /** Returns SkColorFilter if set, or nullptr.
        Does not alter SkColorFilter SkRefCnt.

        @return  SkColorFilter if previously set, nullptr otherwise
    */
    SkColorFilter* getColorFilter() const { return fColorFilter.get(); }

    /** Returns SkColorFilter if set, or nullptr.
        Increases SkColorFilter SkRefCnt by one.

        @return  SkColorFilter if set, or nullptr
    */
    sk_sp<SkColorFilter> refColorFilter() const;

    /** Sets SkColorFilter to filter, decreasing SkRefCnt of the previous
        SkColorFilter. Pass nullptr to clear SkColorFilter.

        Increments filter SkRefCnt by one.

        @param colorFilter  SkColorFilter to apply to subsequent draw
    */
    void setColorFilter(sk_sp<SkColorFilter> colorFilter);

    /** Returns SkBlendMode.
        By default, returns SkBlendMode::kSrcOver.

        @return  mode used to combine source color with destination color
    */
    SkBlendMode getBlendMode() const { return (SkBlendMode)fBlendMode; }

    /** Returns true if SkBlendMode is SkBlendMode::kSrcOver, the default.

        @return  true if SkBlendMode is SkBlendMode::kSrcOver
    */
    bool isSrcOver() const { return (SkBlendMode)fBlendMode == SkBlendMode::kSrcOver; }

    /** Sets SkBlendMode to mode.
        Does not check for valid input.

        @param mode  SkBlendMode used to combine source color and destination
    */
    void setBlendMode(SkBlendMode mode) { fBlendMode = (unsigned)mode; }

    /** Returns SkPathEffect if set, or nullptr.
        Does not alter SkPathEffect SkRefCnt.

        @return  SkPathEffect if previously set, nullptr otherwise
    */
    SkPathEffect* getPathEffect() const { return fPathEffect.get(); }

    /** Returns SkPathEffect if set, or nullptr.
        Increases SkPathEffect SkRefCnt by one.

        @return  SkPathEffect if previously set, nullptr otherwise
    */
    sk_sp<SkPathEffect> refPathEffect() const;

    /** Sets SkPathEffect to pathEffect, decreasing SkRefCnt of the previous
        SkPathEffect. Pass nullptr to leave the path geometry unaltered.

        Increments pathEffect SkRefCnt by one.

        @param pathEffect  replace SkPath with a modification when drawn
    */
    void setPathEffect(sk_sp<SkPathEffect> pathEffect);

    /** Returns SkMaskFilter if set, or nullptr.
        Does not alter SkMaskFilter SkRefCnt.

        @return  SkMaskFilter if previously set, nullptr otherwise
    */
    SkMaskFilter* getMaskFilter() const { return fMaskFilter.get(); }

    /** Returns SkMaskFilter if set, or nullptr.

        Increases SkMaskFilter SkRefCnt by one.

        @return  SkMaskFilter if previously set, nullptr otherwise
    */
    sk_sp<SkMaskFilter> refMaskFilter() const;

    /** Sets SkMaskFilter to maskFilter, decreasing SkRefCnt of the previous
        SkMaskFilter. Pass nullptr to clear SkMaskFilter and leave SkMaskFilter effect on
        mask alpha unaltered.

        Does not affect SkRasterizer.
        Increments maskFilter SkRefCnt by one.

        @param maskFilter  modifies clipping mask generated from drawn geometry
    */
    void setMaskFilter(sk_sp<SkMaskFilter> maskFilter);

    /** Returns SkTypeface if set, or nullptr.
        Increments SkTypeface SkRefCnt by one.

        @return  SkTypeface if previously set, nullptr otherwise
    */
    SkTypeface* getTypeface() const { return fTypeface.get(); }

    /** Increases SkTypeface SkRefCnt by one.

        @return  SkTypeface if previously set, nullptr otherwise
    */
    sk_sp<SkTypeface> refTypeface() const;

    /** Sets SkTypeface to typeface, decreasing SkRefCnt of the previous SkTypeface.
        Pass nullptr to clear SkTypeface and use the default typeface. Increments
        typeface SkRefCnt by one.

        @param typeface  font and style used to draw text
    */
    void setTypeface(sk_sp<SkTypeface> typeface);

    /** Returns SkRasterizer if set, or nullptr.
        Does not alter SkRasterizer SkRefCnt.

        @return  SkRasterizer if previously set, nullptr otherwise
    */
    SkRasterizer* getRasterizer() const { return fRasterizer.get(); }

    /** Returns SkRasterizer if set, or nullptr.
        Increases SkRasterizer SkRefCnt by one.

        @return  SkRasterizer if previously set, nullptr otherwise
    */
    sk_sp<SkRasterizer> refRasterizer() const;

    /** Sets SkRasterizer to rasterizer, decreasing SkRefCnt of the previous
        SkRasterizer. Pass nullptr to clear SkRasterizer and leave SkRasterizer effect on
        mask alpha unaltered.

        Does not affect SkMaskFilter.
        Increments rasterizer SkRefCnt by one.

        @param rasterizer  how geometry is converted to mask alpha
    */
    void setRasterizer(sk_sp<SkRasterizer> rasterizer);

    /** Returns SkImageFilter if set, or nullptr.
        Does not alter SkImageFilter SkRefCnt.

        @return  SkImageFilter if previously set, nullptr otherwise
    */
    SkImageFilter* getImageFilter() const { return fImageFilter.get(); }

    /** Returns SkImageFilter if set, or nullptr.
        Increases SkImageFilter SkRefCnt by one.

        @return  SkImageFilter if previously set, nullptr otherwise
    */
    sk_sp<SkImageFilter> refImageFilter() const;

    /** Sets SkImageFilter to imageFilter, decreasing SkRefCnt of the previous
        SkImageFilter. Pass nullptr to clear SkImageFilter, and remove SkImageFilter effect
        on drawing.

        Does not affect SkRasterizer or SkMaskFilter.
        Increments imageFilter SkRefCnt by one.

        @param imageFilter  how SkImage is sampled when transformed
    */
    void setImageFilter(sk_sp<SkImageFilter> imageFilter);

    /** Returns SkDrawLooper if set, or nullptr.
        Does not alter SkDrawLooper SkRefCnt.

        @return  SkDrawLooper if previously set, nullptr otherwise
    */
    SkDrawLooper* getDrawLooper() const { return fDrawLooper.get(); }

    /** Returns SkDrawLooper if set, or nullptr.
        Increases SkDrawLooper SkRefCnt by one.

        @return  SkDrawLooper if previously set, nullptr otherwise
    */
    sk_sp<SkDrawLooper> refDrawLooper() const;

    /** Deprecated.
        (see bug.skia.org/6259)

        @return  SkDrawLooper if previously set, nullptr otherwise
    */
    SkDrawLooper* getLooper() const { return fDrawLooper.get(); }

    /** Sets SkDrawLooper to drawLooper, decreasing SkRefCnt of the previous
        drawLooper.  Pass nullptr to clear SkDrawLooper and leave SkDrawLooper effect on
        drawing unaltered.

        Increments drawLooper SkRefCnt by one.

        @param drawLooper  iterates through drawing one or more time, altering SkPaint
    */
    void setDrawLooper(sk_sp<SkDrawLooper> drawLooper);

    /** Deprecated.
        (see bug.skia.org/6259)

        @param drawLooper  sets SkDrawLooper to drawLooper
    */
    void setLooper(sk_sp<SkDrawLooper> drawLooper);

    /** \enum SkPaint::Align
        Align adjusts the text relative to the text position.
        Align affects glyphs drawn with: SkCanvas::drawText, SkCanvas::drawPosText,
        SkCanvas::drawPosTextH, SkCanvas::drawTextOnPath,
        SkCanvas::drawTextOnPathHV, SkCanvas::drawTextRSXform, SkCanvas::drawTextBlob,
        and SkCanvas::drawString;
        as well as calls that place text glyphs like getTextWidths() and getTextPath().

        The text position is set by the font for both horizontal and vertical text.
        Typically, for horizontal text, the position is to the left side of the glyph on the
        base line; and for vertical text, the position is the horizontal center of the glyph
        at the caps height.

        Align adjusts the glyph position to center it or move it to abut the position
        using the metrics returned by the font.

        Align defaults to kLeft_Align.
    */
    enum Align {
        /** Leaves the glyph at the position computed by the font offset by the text position. */
        kLeft_Align,

        /** Moves the glyph half its width if Flags has kVerticalText_Flag clear, and
            half its height if Flags has kVerticalText_Flag set.
        */
        kCenter_Align,

        /** Moves the glyph by its width if Flags has kVerticalText_Flag clear,
            and by its height if Flags has kVerticalText_Flag set.
        */
        kRight_Align,
    };

    enum {
        kAlignCount = 3, //!< The number of different Align values defined.
    };

    /** Returns SkPaint::Align.
        Returns kLeft_Align if SkPaint::Align has not been set.

        @return  text placement relative to position
    */
    Align   getTextAlign() const { return (Align)fBitfields.fTextAlign; }

    /** Sets SkPaint::Align to align.
        Has no effect if align is an invalid value.

        @param align  text placement relative to position
    */
    void    setTextAlign(Align align);

    /** Returns text size in points.

        @return  typographic height of text
    */
    SkScalar getTextSize() const { return fTextSize; }

    /** Sets text size in points.
        Has no effect if textSize is not greater than or equal to zero.

        @param textSize  typographic height of text
    */
    void setTextSize(SkScalar textSize);

    /** Returns text scale x.
        Default value is 1.

        @return  text horizontal scale
    */
    SkScalar getTextScaleX() const { return fTextScaleX; }

    /** Sets text scale x.
        Default value is 1.

        @param scaleX  text horizontal scale
    */
    void setTextScaleX(SkScalar scaleX);

    /** Returns text skew x.
        Default value is zero.

        @return  additional shear in x-axis relative to y-axis
    */
    SkScalar getTextSkewX() const { return fTextSkewX; }

    /** Sets text skew x.
        Default value is zero.

        @param skewX  additional shear in x-axis relative to y-axis
    */
    void setTextSkewX(SkScalar skewX);

    /** \enum SkPaint::TextEncoding
        TextEncoding determines whether text specifies character codes and their encoded
        size, or glyph indices. Characters are encoded as specified by the Unicode standard.

        Character codes encoded size are specified by UTF-8, UTF-16, or UTF-32.
        All character code formats are able to represent all of Unicode, differing only
        in the total storage required.

        UTF-8 (RFC 3629) encodes each character as one or more 8-bit bytes.

        UTF-16 (RFC 2781) encodes each character as one or two 16-bit words.

        UTF-32 encodes each character as one 32-bit word.

        font manager uses font data to convert character code points into glyph indices.
        A glyph index is a 16-bit word.

        TextEncoding is set to kUTF8_TextEncoding by default.
    */
    enum TextEncoding {
        kUTF8_TextEncoding,    //!< Uses bytes to represent UTF-8 or ASCII.
        kUTF16_TextEncoding,   //!< Uses two byte words to represent most of Unicode.
        kUTF32_TextEncoding,   //!< Uses four byte words to represent all of Unicode.
        kGlyphID_TextEncoding, //!< Uses two byte words to represent glyph indices.
    };

    /** Returns SkPaint::TextEncoding.
        SkPaint::TextEncoding determines how character code points are mapped to font glyph indices.

        @return  one of: kUTF8_TextEncoding, kUTF16_TextEncoding, kUTF32_TextEncoding, or
                 kGlyphID_TextEncoding
    */
    TextEncoding getTextEncoding() const {
      return (TextEncoding)fBitfields.fTextEncoding;
    }

    /** Sets SkPaint::TextEncoding to encoding.
        SkPaint::TextEncoding determines how character code points are mapped to font glyph indices.
        Invalid values for encoding are ignored.

        @param encoding  one of: kUTF8_TextEncoding, kUTF16_TextEncoding, kUTF32_TextEncoding, or
                         kGlyphID_TextEncoding
    */
    void setTextEncoding(TextEncoding encoding);

    /** \struct SkPaint::FontMetrics
        FontMetrics is filled out by getFontMetrics(). FontMetrics contents reflect the values
        computed by font manager using SkTypeface. Values are set to zero if they are
        not available.

        All vertical values relative to the baseline are given y-down. As such, zero is on the
        baseline, negative values are above the baseline, and positive values are below the
        baseline.

        fUnderlineThickness and fUnderlinePosition have a bit set in fFlags if their values
        are valid, since their value may be zero.

        fStrikeoutThickness and fStrikeoutPosition have a bit set in fFlags if their values
        are valid, since their value may be zero.
    */
    struct FontMetrics {

        /** \enum SkPaint::FontMetrics::FontMetricsFlags
            FontMetricsFlags are set in fFlags when underline and strikeout metrics are valid;
            the underline or strikeout metric may be valid and zero.
            Fonts with embedded bitmaps may not have valid underline or strikeout metrics.
        */
        enum FontMetricsFlags {
            kUnderlineThicknessIsValid_Flag = 1 << 0, //!< Set if fUnderlineThickness is valid.
            kUnderlinePositionIsValid_Flag  = 1 << 1, //!< Set if fUnderlinePosition is valid.
            kStrikeoutThicknessIsValid_Flag = 1 << 2, //!< Set if fStrikeoutThickness is valid.
            kStrikeoutPositionIsValid_Flag  = 1 << 3, //!< Set if fStrikeoutPosition is valid.
        };

        uint32_t fFlags;              //!< fFlags is set when underline metrics are valid.

        /** Greatest extent above the baseline for any glyph.
            Typically less than zero.
        */
        SkScalar fTop;

        /** Recommended distance above the baseline to reserve for a line of text.
            Typically less than zero.
        */
        SkScalar fAscent;

        /** Recommended distance below the baseline to reserve for a line of text.
            Typically greater than zero.
        */
        SkScalar fDescent;

        /** Greatest extent below the baseline for any glyph.
            Typically greater than zero.
        */
        SkScalar fBottom;

        /** Recommended distance to add between lines of text.
            Typically greater than or equal to zero.
        */
        SkScalar fLeading;

        /** Average character width, if it is available.
            Zero if no average width is stored in the font.
        */
        SkScalar fAvgCharWidth;

        SkScalar fMaxCharWidth;       //!< Maximum character width.

        /** Minimum bounding box x value for all glyphs.
            Typically less than zero.
        */
        SkScalar fXMin;

        /** Maximum bounding box x value for all glyphs.
            Typically greater than zero.
        */
        SkScalar fXMax;

        /** Height of a lower-case 'x'.
            May be zero if no lower-case height is stored in the font.
        */
        SkScalar fXHeight;

        /** Height of an upper-case letter.
            May be zero if no upper-case height is stored in the font.
        */
        SkScalar fCapHeight;

        /** Underline thickness.

            If the metric is valid, the kUnderlineThicknessIsValid_Flag is set in fFlags.
            If kUnderlineThicknessIsValid_Flag is clear, fUnderlineThickness is zero.
        */
        SkScalar fUnderlineThickness;

        /** Position of the top of the underline stroke relative to the baseline.
            Typically positive when valid.

            If the metric is valid, the kUnderlinePositionIsValid_Flag is set in fFlags.
            If kUnderlinePositionIsValid_Flag is clear, fUnderlinePosition is zero.
        */
        SkScalar fUnderlinePosition;

        /** Strikeout thickness.

            If the metric is valid, the kStrikeoutThicknessIsValid_Flag is set in fFlags.
            If kStrikeoutThicknessIsValid_Flag is clear, fStrikeoutThickness is zero.
        */
        SkScalar fStrikeoutThickness;

        /** Position of the bottom of the strikeout stroke relative to the baseline.
            Typically negative when valid.

            If the metric is valid, the kStrikeoutPositionIsValid_Flag is set in fFlags.
            If kStrikeoutPositionIsValid_Flag is clear, fStrikeoutPosition is zero.
        */
        SkScalar fStrikeoutPosition;

        /** If SkPaint::FontMetrics has a valid underline thickness, return true, and set
            thickness to that value. If the underline thickness is not valid,
            return false, and ignore thickness.

            @param thickness  storage for underline width
            @return           true if font specifies underline width
        */
        bool hasUnderlineThickness(SkScalar* thickness) const {
            if (SkToBool(fFlags & kUnderlineThicknessIsValid_Flag)) {
                *thickness = fUnderlineThickness;
                return true;
            }
            return false;
        }

        /** If SkPaint::FontMetrics has a valid underline position, return true, and set
            position to that value. If the underline position is not valid,
            return false, and ignore position.

            @param position  storage for underline position
            @return          true if font specifies underline position
        */
        bool hasUnderlinePosition(SkScalar* position) const {
            if (SkToBool(fFlags & kUnderlinePositionIsValid_Flag)) {
                *position = fUnderlinePosition;
                return true;
            }
            return false;
        }

        /** If SkPaint::FontMetrics has a valid strikeout thickness, return true, and set
            thickness to that value. If the underline thickness is not valid,
            return false, and ignore thickness.

            @param thickness  storage for strikeout width
            @return           true if font specifies strikeout width
        */
        bool hasStrikeoutThickness(SkScalar* thickness) const {
            if (SkToBool(fFlags & kStrikeoutThicknessIsValid_Flag)) {
                *thickness = fStrikeoutThickness;
                return true;
            }
            return false;
        }

        /** If SkPaint::FontMetrics has a valid strikeout position, return true, and set
            position to that value. If the underline position is not valid,
            return false, and ignore position.

            @param position  storage for strikeout position
            @return          true if font specifies strikeout position
        */
        bool hasStrikeoutPosition(SkScalar* position) const {
            if (SkToBool(fFlags & kStrikeoutPositionIsValid_Flag)) {
                *position = fStrikeoutPosition;
                return true;
            }
            return false;
        }

    };

    /** Returns SkPaint::FontMetrics associated with SkTypeface.
        The return value is the recommended spacing between lines: the sum of metrics
        descent, ascent, and leading.
        If metrics is not nullptr, SkPaint::FontMetrics is copied to metrics.
        Results are scaled by text size but does not take into account
        dimensions required by text scale x, text skew x, fake bold,
        style stroke, and SkPathEffect.
        Results can be additionally scaled by scale; a scale of zero
        is ignored.

        @param metrics  storage for SkPaint::FontMetrics from SkTypeface; may be nullptr
        @param scale    additional multiplier for returned values
        @return         recommended spacing between lines
    */
    SkScalar getFontMetrics(FontMetrics* metrics, SkScalar scale = 0) const;

    /** Returns the recommended spacing between lines: the sum of metrics
        descent, ascent, and leading.
        Result is scaled by text size but does not take into account
        dimensions required by stroking and SkPathEffect.
        Returns the same result as getFontMetrics().

        @return  recommended spacing between lines
    */
    SkScalar getFontSpacing() const { return this->getFontMetrics(nullptr, 0); }

    /** Converts text into glyph indices.
        Returns the number of glyph indices represented by text.
        SkPaint::TextEncoding specifies how text represents characters or glyphs.
        glyphs may be nullptr, to compute the glyph count.

        Does not check text for valid character codes or valid glyph indices.

        If byteLength equals zero, returns zero.
        If byteLength includes a partial character, the partial character is ignored.

        If SkPaint::TextEncoding is kUTF8_TextEncoding and
        text contains an invalid UTF-8 sequence, zero is returned.

        @param text        character storage encoded with SkPaint::TextEncoding
        @param byteLength  length of character storage in bytes
        @param glyphs      storage for glyph indices; may be nullptr
        @return            number of glyphs represented by text of length byteLength
    */
    int textToGlyphs(const void* text, size_t byteLength,
                     SkGlyphID glyphs[]) const;

    /** Returns true if all text corresponds to a non-zero glyph index.
        Returns false if any characters in text are not supported in
        SkTypeface.

        If SkPaint::TextEncoding is kGlyphID_TextEncoding,
        returns true if all glyph indices in text are non-zero;
        does not check to see if text contains valid glyph indices for SkTypeface.

        Returns true if byteLength is zero.

        @param text        array of characters or glyphs
        @param byteLength  number of bytes in text array
        @return            true if all text corresponds to a non-zero glyph index
    */
    bool containsText(const void* text, size_t byteLength) const;

    /** Converts glyphs into text if possible.
        Glyph values without direct Unicode equivalents are mapped to zero.
        Uses the SkTypeface, but is unaffected
        by SkPaint::TextEncoding; the text values returned are equivalent to kUTF32_TextEncoding.

        Only supported on platforms that use FreeType as the font engine.

        @param glyphs  array of indices into font
        @param count   length of glyph array
        @param text    storage for character codes, one per glyph
    */
    void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[]) const;

    /** Returns the number of glyphs in text.
        Uses SkPaint::TextEncoding to count the glyphs.
        Returns the same result as textToGlyphs().

        @param text        character storage encoded with SkPaint::TextEncoding
        @param byteLength  length of character storage in bytes
        @return            number of glyphs represented by text of length byteLength
    */
    int countText(const void* text, size_t byteLength) const {
        return this->textToGlyphs(text, byteLength, nullptr);
    }

    /** Returns the advance width of text if kVerticalText_Flag is clear,
        and the height of text if kVerticalText_Flag is set.
        The advance is the normal distance to move before drawing additional text.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the font metrics,
        and text size, text scale x, text skew x, stroke width, and
        SkPathEffect to scale the metrics and bounds.
        Returns the bounding box of text if bounds is not nullptr.
        The bounding box is computed as if the text was drawn at the origin.

        @param text    character codes or glyph indices to be measured
        @param length  number of bytes of text to measure
        @param bounds  returns bounding box relative to (0, 0) if not nullptr
        @return        advance width or height
    */
    SkScalar measureText(const void* text, size_t length, SkRect* bounds) const;

    /** Returns the advance width of text if kVerticalText_Flag is clear,
        and the height of text if kVerticalText_Flag is set.
        The advance is the normal distance to move before drawing additional text.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the font metrics,
        and text size to scale the metrics.
        Does not scale the advance or bounds by fake bold or SkPathEffect.

        @param text    character codes or glyph indices to be measured
        @param length  number of bytes of text to measure
        @return        advance width or height
    */
    SkScalar measureText(const void* text, size_t length) const {
        return this->measureText(text, length, nullptr);
    }

    /** Returns the bytes of text that fit within maxWidth.
        If kVerticalText_Flag is clear, the text fragment fits if its advance width is less than or
        equal to maxWidth.
        If kVerticalText_Flag is set, the text fragment fits if its advance height is less than or
        equal to maxWidth.
        Measures only while the advance is less than or equal to maxWidth.
        Returns the advance or the text fragment in measuredWidth if it not nullptr.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the font metrics,
        and text size to scale the metrics.
        Does not scale the advance or bounds by fake bold or SkPathEffect.

        @param text           character codes or glyph indices to be measured
        @param length         number of bytes of text to measure
        @param maxWidth       advance limit; text is measured while advance is less than maxWidth
        @param measuredWidth  returns the width of the text less than or equal to maxWidth
        @return               bytes of text that fit, always less than or equal to length
    */
    size_t  breakText(const void* text, size_t length, SkScalar maxWidth,
                      SkScalar* measuredWidth = nullptr) const;

    /** Retrieves the advance and bounds for each glyph in text, and returns
        the glyph count in text.
        Both widths and bounds may be nullptr.
        If widths is not nullptr, widths must be an array of glyph count entries.
        if bounds is not nullptr, bounds must be an array of glyph count entries.
        If kVerticalText_Flag is clear, widths returns the horizontal advance.
        If kVerticalText_Flag is set, widths returns the vertical advance.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the font metrics,
        and text size to scale the widths and bounds.
        Does not scale the advance by fake bold or SkPathEffect.
        Does include fake bold and SkPathEffect in the bounds.

        @param text        character codes or glyph indices to be measured
        @param byteLength  number of bytes of text to measure
        @param widths      returns text advances for each glyph; may be nullptr
        @param bounds      returns bounds for each glyph relative to (0, 0); may be nullptr
        @return            glyph count in text
    */
    int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                      SkRect bounds[] = nullptr) const;

    /** Returns the geometry as SkPath equivalent to the drawn text.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        All of the glyph paths are stored in path.
        Uses x, y, and SkPaint::Align to position path.

        @param text    character codes or glyph indices
        @param length  number of bytes of text
        @param x       x-coordinate of the origin of the text
        @param y       y-coordinate of the origin of the text
        @param path    geometry of the glyphs
    */
    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                     SkPath* path) const;

    /** Returns the geometry as SkPath equivalent to the drawn text.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        All of the glyph paths are stored in path.
        Uses pos array and SkPaint::Align to position path.
        pos contains a position for each glyph.

        @param text    character codes or glyph indices
        @param length  number of bytes of text
        @param pos     positions of each glyph
        @param path    geometry of the glyphs
    */
    void getPosTextPath(const void* text, size_t length,
                        const SkPoint pos[], SkPath* path) const;

    /** Returns the number of intervals that intersect bounds.
        bounds describes a pair of lines parallel to the text advance.
        The return count is zero or a multiple of two, and is at most twice the number of glyphs in
        the string.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        Uses x, y, and SkPaint::Align to position intervals.

        Pass nullptr for intervals to determine the size of the interval array.

        intervals are cached to improve performance for multiple calls.

        @param text       character codes or glyph indices
        @param length     number of bytes of text
        @param x          x-coordinate of the origin of the text
        @param y          y-coordinate of the origin of the text
        @param bounds     lower and upper line parallel to the advance
        @param intervals  returned intersections; may be nullptr
        @return           number of intersections; may be zero
    */
    int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,
                          const SkScalar bounds[2], SkScalar* intervals) const;

    /** Returns the number of intervals that intersect bounds.
        bounds describes a pair of lines parallel to the text advance.
        The return count is zero or a multiple of two, and is at most twice the number of glyphs in
        the string.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        Uses pos array and SkPaint::Align to position intervals.

        Pass nullptr for intervals to determine the size of the interval array.

        intervals are cached to improve performance for multiple calls.

        @param text       character codes or glyph indices
        @param length     number of bytes of text
        @param pos        positions of each glyph
        @param bounds     lower and upper line parallel to the advance
        @param intervals  returned intersections; may be nullptr
        @return           number of intersections; may be zero
    */
    int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],
                             const SkScalar bounds[2], SkScalar* intervals) const;

    /** Returns the number of intervals that intersect bounds.
        bounds describes a pair of lines parallel to the text advance.
        The return count is zero or a multiple of two, and is at most twice the number of glyphs in
        the string.
        Uses SkPaint::TextEncoding to decode text, SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        Uses xpos array, constY, and SkPaint::Align to position intervals.

        Pass nullptr for intervals to determine the size of the interval array.

        intervals are cached to improve performance for multiple calls.

        @param text       character codes or glyph indices
        @param length     number of bytes of text
        @param xpos       positions of each glyph in x
        @param constY     position of each glyph in y
        @param bounds     lower and upper line parallel to the advance
        @param intervals  returned intersections; may be nullptr
        @return           number of intersections; may be zero
    */
    int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],
                              SkScalar constY, const SkScalar bounds[2], SkScalar* intervals) const;

    /** Returns the number of intervals that intersect bounds.
        bounds describes a pair of lines parallel to the text advance.
        The return count is zero or a multiple of two, and is at most twice the number of glyphs in
        the string.
        Uses SkTypeface to get the glyph paths,
        and text size, fake bold, and SkPathEffect to scale and modify the glyph paths.
        Uses run array and SkPaint::Align to position intervals.

        SkPaint::TextEncoding must be set to SkPaint::kGlyphID_TextEncoding.

        Pass nullptr for intervals to determine the size of the interval array.

        intervals are cached to improve performance for multiple calls.

        @param blob       glyphs, positions, and text paint attributes
        @param bounds     lower and upper line parallel to the advance
        @param intervals  returned intersections; may be nullptr
        @return           number of intersections; may be zero
    */
    int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                              SkScalar* intervals) const;

    /** Returns the union of bounds of all glyphs.
        Returned dimensions are computed by font manager from font data,
        ignoring SkPaint::Hinting. Includes text size, text scale x,
        and text skew x, but not fake bold or SkPathEffect.

        If text size is large, text scale x is one, and text skew x is zero,
        returns the same bounds as SkPaint::FontMetrics { FontMetrics::fXMin,
        FontMetrics::fTop, FontMetrics::fXMax, FontMetrics::fBottom }.

        @return  union of bounds of all glyphs
    */
    SkRect getFontBounds() const;

    /** Returns true if SkPaint prevents all drawing;
        otherwise, the SkPaint may or may not allow drawing.

        Returns true if, for example, SkBlendMode combined with color alpha computes a
        new alpha of zero.

        @return  true if SkPaint prevents all drawing
    */
    bool nothingToDraw() const;

    /**     (to be made private)
        Returns true if SkPaint does not include elements requiring extensive computation
        to compute SkBaseDevice bounds of drawn geometry. For instance, SkPaint with SkPathEffect
        always returns false.

        @return  true if SkPaint allows for fast computation of bounds
    */
    bool canComputeFastBounds() const;

    /**     (to be made private)
        Only call this if canComputeFastBounds() returned true. This takes a
        raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
        effects in the paint (e.g. stroking). If needed, it uses the storage
        parameter. It returns the adjusted bounds that can then be used
        for SkCanvas::quickReject tests.

        The returned SkRect will either be orig or storage, thus the caller
        should not rely on storage being set to the result, but should always
        use the returned value. It is legal for orig and storage to be the same
        SkRect.
            e.g.
            if (paint.canComputeFastBounds()) {
            SkRect r, storage;
            path.computeBounds(&r, SkPath::kFast_BoundsType);
            const SkRect& fastR = paint.computeFastBounds(r, &storage);
            if (canvas->quickReject(fastR, ...)) {
            // don't draw the path
            }
            }

        @param orig     geometry modified by SkPaint when drawn
        @param storage  computed bounds of geometry; may not be nullptr
        @return         fast computed bounds
    */
    const SkRect& computeFastBounds(const SkRect& orig, SkRect* storage) const {
        // Things like stroking, etc... will do math on the bounds rect, assuming that it's sorted.
        SkASSERT(orig.isSorted());
        SkPaint::Style style = this->getStyle();
        // ultra fast-case: filling with no effects that affect geometry
        if (kFill_Style == style) {
            uintptr_t effects = reinterpret_cast<uintptr_t>(this->getLooper());
            effects |= reinterpret_cast<uintptr_t>(this->getMaskFilter());
            effects |= reinterpret_cast<uintptr_t>(this->getPathEffect());
            effects |= reinterpret_cast<uintptr_t>(this->getImageFilter());
            if (!effects) {
                return orig;
            }
        }

        return this->doComputeFastBounds(orig, storage, style);
    }

    /**     (to be made private)

        @param orig     geometry modified by SkPaint when drawn
        @param storage  computed bounds of geometry
        @return         fast computed bounds
    */
    const SkRect& computeFastStrokeBounds(const SkRect& orig,
                                          SkRect* storage) const {
        return this->doComputeFastBounds(orig, storage, kStroke_Style);
    }

    /**     (to be made private)
        Computes the bounds, overriding the SkPaint SkPaint::Style. This can be used to
        account for additional width required by stroking orig, without
        altering SkPaint::Style set to fill.

        @param orig     geometry modified by SkPaint when drawn
        @param storage  computed bounds of geometry
        @param style    overrides SkPaint::Style
        @return         fast computed bounds
    */
    const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage,
                                      Style style) const;

    /** macro expands to: void toString(SkString* str) const;
        Creates string representation of SkPaint. The representation is read by
        internal debugging tools. The interface and implementation may be
        suppressed by defining SK_IGNORE_TO_STRING.

        @param str  storage for string representation of SkPaint
    */
    SK_TO_STRING_NONVIRT()

private:
    typedef const SkGlyph& (*GlyphCacheProc)(SkGlyphCache*, const char**);

    sk_sp<SkTypeface>     fTypeface;
    sk_sp<SkPathEffect>   fPathEffect;
    sk_sp<SkShader>       fShader;
    sk_sp<SkMaskFilter>   fMaskFilter;
    sk_sp<SkColorFilter>  fColorFilter;
    sk_sp<SkRasterizer>   fRasterizer;
    sk_sp<SkDrawLooper>   fDrawLooper;
    sk_sp<SkImageFilter>  fImageFilter;

    SkScalar        fTextSize;
    SkScalar        fTextScaleX;
    SkScalar        fTextSkewX;
    SkColor         fColor;
    SkScalar        fWidth;
    SkScalar        fMiterLimit;
    uint32_t        fBlendMode; // just need 5-6 bits
    union {
        struct {
            // all of these bitfields should add up to 32
            unsigned        fFlags : 16;
            unsigned        fTextAlign : 2;
            unsigned        fCapType : 2;
            unsigned        fJoinType : 2;
            unsigned        fStyle : 2;
            unsigned        fTextEncoding : 2;  // 3 values
            unsigned        fHinting : 2;
            unsigned        fFilterQuality : 2;
            //unsigned      fFreeBits : 2;
        } fBitfields;
        uint32_t fBitfieldsUInt;
    };

    static GlyphCacheProc GetGlyphCacheProc(TextEncoding encoding,
                                            bool isDevKern,
                                            bool needFullMetrics);

    SkScalar measure_text(SkGlyphCache*, const char* text, size_t length,
                          int* count, SkRect* bounds) const;

    enum ScalerContextFlags : uint32_t {
        kNone_ScalerContextFlags = 0,

        kFakeGamma_ScalerContextFlag = 1 << 0,
        kBoostContrast_ScalerContextFlag = 1 << 1,

        kFakeGammaAndBoostContrast_ScalerContextFlags =
            kFakeGamma_ScalerContextFlag | kBoostContrast_ScalerContextFlag,
    };

    /*
     * Allocs an SkDescriptor on the heap and return it to the caller as a refcnted
     * SkData.  Caller is responsible for managing the lifetime of this object.
     */
    void getScalerContextDescriptor(SkScalerContextEffects*, SkAutoDescriptor*,
                                    const SkSurfaceProps& surfaceProps,
                                    uint32_t scalerContextFlags, const SkMatrix*) const;

    SkGlyphCache* detachCache(const SkSurfaceProps* surfaceProps, uint32_t scalerContextFlags,
                              const SkMatrix*) const;

    void descriptorProc(const SkSurfaceProps* surfaceProps, uint32_t scalerContextFlags,
                        const SkMatrix* deviceMatrix,
                        void (*proc)(SkTypeface*, const SkScalerContextEffects&,
                                     const SkDescriptor*, void*),
                        void* context) const;

    /*
     * The luminance color is used to determine which Gamma Canonical color to map to.  This is
     * really only used by backends which want to cache glyph masks, and need some way to know if
     * they need to generate new masks based off a given color.
     */
    SkColor computeLuminanceColor() const;

    enum {
        /*  This is the size we use when we ask for a glyph's path. We then
         *  post-transform it as we draw to match the request.
         *  This is done to try to re-use cache entries for the path.
         *
         *  This value is somewhat arbitrary. In theory, it could be 1, since
         *  we store paths as floats. However, we get the path from the font
         *  scaler, and it may represent its paths as fixed-point (or 26.6),
         *  so we shouldn't ask for something too big (might overflow 16.16)
         *  or too small (underflow 26.6).
         *
         *  This value could track kMaxSizeForGlyphCache, assuming the above
         *  constraints, but since we ask for unhinted paths, the two values
         *  need not match per-se.
         */
        kCanonicalTextSizeForPaths  = 64,
    };

    static bool TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM, SkScalar maxLimit);

    // Set flags/hinting/textSize up to use for drawing text as paths.
    // Returns scale factor to restore the original textSize, since will will
    // have change it to kCanonicalTextSizeForPaths.
    SkScalar setupForAsPaths();

    static SkScalar MaxCacheSize2(SkScalar maxLimit);

    friend class SkAutoGlyphCache;
    friend class SkAutoGlyphCacheNoGamma;
    friend class SkCanvas;
    friend class SkDraw;
    friend class SkPDFDevice;
    friend class GrAtlasTextBlob;
    friend class GrAtlasTextContext;
    friend class GrStencilAndCoverTextContext;
    friend class GrPathRendering;
    friend class GrTextUtils;
    friend class GrGLPathRendering;
    friend class SkScalerContext;
    friend class SkTextBaseIter;
    friend class SkCanonicalizePaint;
};

#endif
