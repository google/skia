

/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPaint_DEFINED
#define SkPaint_DEFINED

#include "SkColor.h"
#include "SkDrawLooper.h"
#include "SkMatrix.h"
#include "SkXfermode.h"
#ifdef SK_BUILD_FOR_ANDROID
#include "SkPaintOptionsAndroid.h"
#endif

class SkAnnotation;
class SkAutoGlyphCache;
class SkColorFilter;
class SkDescriptor;
struct SkDeviceProperties;
class SkReadBuffer;
class SkWriteBuffer;
struct SkGlyph;
struct SkRect;
class SkGlyphCache;
class SkImageFilter;
class SkMaskFilter;
class SkPath;
class SkPathEffect;
struct SkPoint;
class SkRasterizer;
class SkShader;
class SkTypeface;

typedef const SkGlyph& (*SkDrawCacheProc)(SkGlyphCache*, const char**,
                                           SkFixed x, SkFixed y);

typedef const SkGlyph& (*SkMeasureCacheProc)(SkGlyphCache*, const char**);

#define kBicubicFilterBitmap_Flag kHighQualityFilterBitmap_Flag

/** \class SkPaint

    The SkPaint class holds the style and color information about how to draw
    geometries, text and bitmaps.
*/

class SK_API SkPaint {
    enum {
        // DEPRECATED -- use setFilterLevel instead
        kFilterBitmap_Flag    = 0x02, // temporary flag
        // DEPRECATED -- use setFilterLevel instead
        kHighQualityFilterBitmap_Flag = 0x4000, // temporary flag
        // DEPRECATED -- use setFilterLevel instead
        kHighQualityDownsampleBitmap_Flag = 0x8000, // temporary flag
    };
public:
    SkPaint();
    SkPaint(const SkPaint& paint);
    ~SkPaint();

    SkPaint& operator=(const SkPaint&);

    SK_API friend bool operator==(const SkPaint& a, const SkPaint& b);
    friend bool operator!=(const SkPaint& a, const SkPaint& b) {
        return !(a == b);
    }

    void flatten(SkWriteBuffer&) const;
    void unflatten(SkReadBuffer&);

    /** Restores the paint to its initial settings.
    */
    void reset();

    /** Specifies the level of hinting to be performed. These names are taken
        from the Gnome/Cairo names for the same. They are translated into
        Freetype concepts the same as in cairo-ft-font.c:
           kNo_Hinting     -> FT_LOAD_NO_HINTING
           kSlight_Hinting -> FT_LOAD_TARGET_LIGHT
           kNormal_Hinting -> <default, no option>
           kFull_Hinting   -> <same as kNormalHinting, unless we are rendering
                              subpixel glyphs, in which case TARGET_LCD or
                              TARGET_LCD_V is used>
    */
    enum Hinting {
        kNo_Hinting            = 0,
        kSlight_Hinting        = 1,
        kNormal_Hinting        = 2,     //!< this is the default
        kFull_Hinting          = 3
    };

    Hinting getHinting() const {
        return static_cast<Hinting>(fHinting);
    }

    void setHinting(Hinting hintingLevel);

    /** Specifies the bit values that are stored in the paint's flags.
    */
    enum Flags {
        kAntiAlias_Flag       = 0x01,   //!< mask to enable antialiasing
        kDither_Flag          = 0x04,   //!< mask to enable dithering
        kUnderlineText_Flag   = 0x08,   //!< mask to enable underline text
        kStrikeThruText_Flag  = 0x10,   //!< mask to enable strike-thru text
        kFakeBoldText_Flag    = 0x20,   //!< mask to enable fake-bold text
        kLinearText_Flag      = 0x40,   //!< mask to enable linear-text
        kSubpixelText_Flag    = 0x80,   //!< mask to enable subpixel text positioning
        kDevKernText_Flag     = 0x100,  //!< mask to enable device kerning text
        kLCDRenderText_Flag   = 0x200,  //!< mask to enable subpixel glyph renderering
        kEmbeddedBitmapText_Flag = 0x400, //!< mask to enable embedded bitmap strikes
        kAutoHinting_Flag     = 0x800,  //!< mask to force Freetype's autohinter
        kVerticalText_Flag    = 0x1000,
        kGenA8FromLCD_Flag    = 0x2000, // hack for GDI -- do not use if you can help it
        kDistanceFieldTextTEMP_Flag = 0x4000, //!< TEMPORARY mask to enable distance fields
                                              // currently overrides LCD and subpixel rendering
        // when adding extra flags, note that the fFlags member is specified
        // with a bit-width and you'll have to expand it.

        kAllFlags = 0xFFFF
    };

    /** Return the paint's flags. Use the Flag enum to test flag values.
        @return the paint's flags (see enums ending in _Flag for bit masks)
    */
    uint32_t getFlags() const { return fFlags; }

    /** Set the paint's flags. Use the Flag enum to specific flag values.
        @param flags    The new flag bits for the paint (see Flags enum)
    */
    void setFlags(uint32_t flags);

    /** Helper for getFlags(), returning true if kAntiAlias_Flag bit is set
        @return true if the antialias bit is set in the paint's flags.
        */
    bool isAntiAlias() const {
        return SkToBool(this->getFlags() & kAntiAlias_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kAntiAlias_Flag bit
        @param aa   true to enable antialiasing, false to disable it
        */
    void setAntiAlias(bool aa);

    /** Helper for getFlags(), returning true if kDither_Flag bit is set
        @return true if the dithering bit is set in the paint's flags.
        */
    bool isDither() const {
        return SkToBool(this->getFlags() & kDither_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kDither_Flag bit
        @param dither   true to enable dithering, false to disable it
        */
    void setDither(bool dither);

    /** Helper for getFlags(), returning true if kLinearText_Flag bit is set
        @return true if the lineartext bit is set in the paint's flags
    */
    bool isLinearText() const {
        return SkToBool(this->getFlags() & kLinearText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kLinearText_Flag bit
        @param linearText true to set the linearText bit in the paint's flags,
                          false to clear it.
    */
    void setLinearText(bool linearText);

    /** Helper for getFlags(), returning true if kSubpixelText_Flag bit is set
        @return true if the lineartext bit is set in the paint's flags
    */
    bool isSubpixelText() const {
        return SkToBool(this->getFlags() & kSubpixelText_Flag);
    }

    /**
     *  Helper for setFlags(), setting or clearing the kSubpixelText_Flag.
     *  @param subpixelText true to set the subpixelText bit in the paint's
     *                      flags, false to clear it.
     */
    void setSubpixelText(bool subpixelText);

    bool isLCDRenderText() const {
        return SkToBool(this->getFlags() & kLCDRenderText_Flag);
    }

    /**
     *  Helper for setFlags(), setting or clearing the kLCDRenderText_Flag.
     *  Note: antialiasing must also be on for lcd rendering
     *  @param lcdText true to set the LCDRenderText bit in the paint's flags,
     *                 false to clear it.
     */
    void setLCDRenderText(bool lcdText);

    bool isEmbeddedBitmapText() const {
        return SkToBool(this->getFlags() & kEmbeddedBitmapText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kEmbeddedBitmapText_Flag bit
        @param useEmbeddedBitmapText true to set the kEmbeddedBitmapText bit in the paint's flags,
                                     false to clear it.
    */
    void setEmbeddedBitmapText(bool useEmbeddedBitmapText);

    bool isAutohinted() const {
        return SkToBool(this->getFlags() & kAutoHinting_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kAutoHinting_Flag bit
        @param useAutohinter true to set the kEmbeddedBitmapText bit in the
                                  paint's flags,
                             false to clear it.
    */
    void setAutohinted(bool useAutohinter);

    bool isVerticalText() const {
        return SkToBool(this->getFlags() & kVerticalText_Flag);
    }

    /**
     *  Helper for setting or clearing the kVerticalText_Flag bit in
     *  setFlags(...).
     *
     *  If this bit is set, then advances are treated as Y values rather than
     *  X values, and drawText will places its glyphs vertically rather than
     *  horizontally.
     */
    void setVerticalText(bool);

    /** Helper for getFlags(), returning true if kUnderlineText_Flag bit is set
        @return true if the underlineText bit is set in the paint's flags.
    */
    bool isUnderlineText() const {
        return SkToBool(this->getFlags() & kUnderlineText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kUnderlineText_Flag bit
        @param underlineText true to set the underlineText bit in the paint's
                             flags, false to clear it.
    */
    void setUnderlineText(bool underlineText);

    /** Helper for getFlags(), returns true if kStrikeThruText_Flag bit is set
        @return true if the strikeThruText bit is set in the paint's flags.
    */
    bool isStrikeThruText() const {
        return SkToBool(this->getFlags() & kStrikeThruText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kStrikeThruText_Flag bit
        @param strikeThruText   true to set the strikeThruText bit in the
                                paint's flags, false to clear it.
    */
    void setStrikeThruText(bool strikeThruText);

    /** Helper for getFlags(), returns true if kFakeBoldText_Flag bit is set
        @return true if the kFakeBoldText_Flag bit is set in the paint's flags.
    */
    bool isFakeBoldText() const {
        return SkToBool(this->getFlags() & kFakeBoldText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kFakeBoldText_Flag bit
        @param fakeBoldText true to set the kFakeBoldText_Flag bit in the paint's
                            flags, false to clear it.
    */
    void setFakeBoldText(bool fakeBoldText);

    /** Helper for getFlags(), returns true if kDevKernText_Flag bit is set
        @return true if the kernText bit is set in the paint's flags.
    */
    bool isDevKernText() const {
        return SkToBool(this->getFlags() & kDevKernText_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kKernText_Flag bit
        @param kernText true to set the kKernText_Flag bit in the paint's
                            flags, false to clear it.
    */
    void setDevKernText(bool devKernText);

    /** Helper for getFlags(), returns true if kDistanceFieldTextTEMP_Flag bit is set
     @return true if the distanceFieldText bit is set in the paint's flags.
     */
    bool isDistanceFieldTextTEMP() const {
        return SkToBool(this->getFlags() & kDistanceFieldTextTEMP_Flag);
    }

    /** Helper for setFlags(), setting or clearing the kDistanceFieldTextTEMP_Flag bit
     @param distanceFieldText true to set the kDistanceFieldTextTEMP_Flag bit in the paint's
     flags, false to clear it.
     */
    void setDistanceFieldTextTEMP(bool distanceFieldText);

    enum FilterLevel {
        kNone_FilterLevel,
        kLow_FilterLevel,
        kMedium_FilterLevel,
        kHigh_FilterLevel
    };

    /**
     *  Return the filter level. This affects the quality (and performance) of
     *  drawing scaled images.
     */
    FilterLevel getFilterLevel() const;

    /**
     *  Set the filter level. This affects the quality (and performance) of
     *  drawing scaled images.
     */
    void setFilterLevel(FilterLevel);

    /**
     *  If the predicate is true, set the filterLevel to Low, else set it to
     *  None.
     */
    SK_ATTR_DEPRECATED("use setFilterLevel")
    void setFilterBitmap(bool doFilter) {
        this->setFilterLevel(doFilter ? kLow_FilterLevel : kNone_FilterLevel);
    }

    /**
     *  Returns true if getFilterLevel() returns anything other than None.
     */
    SK_ATTR_DEPRECATED("use getFilterLevel")
    bool isFilterBitmap() const {
        return kNone_FilterLevel != this->getFilterLevel();
    }

    /** Styles apply to rect, oval, path, and text.
        Bitmaps are always drawn in "fill", and lines are always drawn in
        "stroke".

        Note: strokeandfill implicitly draws the result with
        SkPath::kWinding_FillType, so if the original path is even-odd, the
        results may not appear the same as if it was drawn twice, filled and
        then stroked.
    */
    enum Style {
        kFill_Style,            //!< fill the geometry
        kStroke_Style,          //!< stroke the geometry
        kStrokeAndFill_Style,   //!< fill and stroke the geometry
    };
    enum {
        kStyleCount = kStrokeAndFill_Style + 1
    };

    /** Return the paint's style, used for controlling how primitives'
        geometries are interpreted (except for drawBitmap, which always assumes
        kFill_Style).
        @return the paint's Style
    */
    Style getStyle() const { return (Style)fStyle; }

    /** Set the paint's style, used for controlling how primitives'
        geometries are interpreted (except for drawBitmap, which always assumes
        Fill).
        @param style    The new style to set in the paint
    */
    void setStyle(Style style);

    /** Return the paint's color. Note that the color is a 32bit value
        containing alpha as well as r,g,b. This 32bit value is not
        premultiplied, meaning that its alpha can be any value, regardless of
        the values of r,g,b.
        @return the paint's color (and alpha).
    */
    SkColor getColor() const { return fColor; }

    /** Set the paint's color. Note that the color is a 32bit value containing
        alpha as well as r,g,b. This 32bit value is not premultiplied, meaning
        that its alpha can be any value, regardless of the values of r,g,b.
        @param color    The new color (including alpha) to set in the paint.
    */
    void setColor(SkColor color);

    /** Helper to getColor() that just returns the color's alpha value.
        @return the alpha component of the paint's color.
        */
    uint8_t getAlpha() const { return SkToU8(SkColorGetA(fColor)); }

    /** Helper to setColor(), that only assigns the color's alpha value,
        leaving its r,g,b values unchanged.
        @param a    set the alpha component (0..255) of the paint's color.
    */
    void setAlpha(U8CPU a);

    /** Helper to setColor(), that takes a,r,g,b and constructs the color value
        using SkColorSetARGB()
        @param a    The new alpha component (0..255) of the paint's color.
        @param r    The new red component (0..255) of the paint's color.
        @param g    The new green component (0..255) of the paint's color.
        @param b    The new blue component (0..255) of the paint's color.
    */
    void setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

    /** Return the width for stroking.
        <p />
        A value of 0 strokes in hairline mode.
        Hairlines always draw 1-pixel wide, regardless of the matrix.
        @return the paint's stroke width, used whenever the paint's style is
                Stroke or StrokeAndFill.
    */
    SkScalar getStrokeWidth() const { return fWidth; }

    /** Set the width for stroking.
        Pass 0 to stroke in hairline mode.
        Hairlines always draw 1-pixel wide, regardless of the matrix.
        @param width set the paint's stroke width, used whenever the paint's
                     style is Stroke or StrokeAndFill.
    */
    void setStrokeWidth(SkScalar width);

    /** Return the paint's stroke miter value. This is used to control the
        behavior of miter joins when the joins angle is sharp.
        @return the paint's miter limit, used whenever the paint's style is
                Stroke or StrokeAndFill.
    */
    SkScalar getStrokeMiter() const { return fMiterLimit; }

    /** Set the paint's stroke miter value. This is used to control the
        behavior of miter joins when the joins angle is sharp. This value must
        be >= 0.
        @param miter    set the miter limit on the paint, used whenever the
                        paint's style is Stroke or StrokeAndFill.
    */
    void setStrokeMiter(SkScalar miter);

    /** Cap enum specifies the settings for the paint's strokecap. This is the
        treatment that is applied to the beginning and end of each non-closed
        contour (e.g. lines).
    */
    enum Cap {
        kButt_Cap,      //!< begin/end contours with no extension
        kRound_Cap,     //!< begin/end contours with a semi-circle extension
        kSquare_Cap,    //!< begin/end contours with a half square extension

        kCapCount,
        kDefault_Cap = kButt_Cap
    };

    /** Join enum specifies the settings for the paint's strokejoin. This is
        the treatment that is applied to corners in paths and rectangles.
    */
    enum Join {
        kMiter_Join,    //!< connect path segments with a sharp join
        kRound_Join,    //!< connect path segments with a round join
        kBevel_Join,    //!< connect path segments with a flat bevel join

        kJoinCount,
        kDefault_Join = kMiter_Join
    };

    /** Return the paint's stroke cap type, controlling how the start and end
        of stroked lines and paths are treated.
        @return the line cap style for the paint, used whenever the paint's
                style is Stroke or StrokeAndFill.
    */
    Cap getStrokeCap() const { return (Cap)fCapType; }

    /** Set the paint's stroke cap type.
        @param cap  set the paint's line cap style, used whenever the paint's
                    style is Stroke or StrokeAndFill.
    */
    void setStrokeCap(Cap cap);

    /** Return the paint's stroke join type.
        @return the paint's line join style, used whenever the paint's style is
                Stroke or StrokeAndFill.
    */
    Join getStrokeJoin() const { return (Join)fJoinType; }

    /** Set the paint's stroke join type.
        @param join set the paint's line join style, used whenever the paint's
                    style is Stroke or StrokeAndFill.
    */
    void setStrokeJoin(Join join);

    /**
     *  Applies any/all effects (patheffect, stroking) to src, returning the
     *  result in dst. The result is that drawing src with this paint will be
     *  the same as drawing dst with a default paint (at least from the
     *  geometric perspective).
     *
     *  @param src  input path
     *  @param dst  output path (may be the same as src)
     *  @param cullRect If not null, the dst path may be culled to this rect.
     *  @return     true if the path should be filled, or false if it should be
     *              drawn with a hairline (width == 0)
     */
    bool getFillPath(const SkPath& src, SkPath* dst,
                     const SkRect* cullRect = NULL) const;

    /** Get the paint's shader object.
        <p />
      The shader's reference count is not affected.
        @return the paint's shader (or NULL)
    */
    SkShader* getShader() const { return fShader; }

    /** Set or clear the shader object.
     *  Shaders specify the source color(s) for what is being drawn. If a paint
     *  has no shader, then the paint's color is used. If the paint has a
     *  shader, then the shader's color(s) are use instead, but they are
     *  modulated by the paint's alpha. This makes it easy to create a shader
     *  once (e.g. bitmap tiling or gradient) and then change its transparency
     *  w/o having to modify the original shader... only the paint's alpha needs
     *  to be modified.
     *  <p />
     *  Pass NULL to clear any previous shader.
     *  As a convenience, the parameter passed is also returned.
     *  If a previous shader exists, its reference count is decremented.
     *  If shader is not NULL, its reference count is incremented.
     *  @param shader   May be NULL. The shader to be installed in the paint
     *  @return         shader
     */
    SkShader* setShader(SkShader* shader);

    /** Get the paint's colorfilter. If there is a colorfilter, its reference
        count is not changed.
        @return the paint's colorfilter (or NULL)
    */
    SkColorFilter* getColorFilter() const { return fColorFilter; }

    /** Set or clear the paint's colorfilter, returning the parameter.
        <p />
        If the paint already has a filter, its reference count is decremented.
        If filter is not NULL, its reference count is incremented.
        @param filter   May be NULL. The filter to be installed in the paint
        @return         filter
    */
    SkColorFilter* setColorFilter(SkColorFilter* filter);

    /** Get the paint's xfermode object.
        <p />
      The xfermode's reference count is not affected.
        @return the paint's xfermode (or NULL)
    */
    SkXfermode* getXfermode() const { return fXfermode; }

    /** Set or clear the xfermode object.
        <p />
        Pass NULL to clear any previous xfermode.
        As a convenience, the parameter passed is also returned.
        If a previous xfermode exists, its reference count is decremented.
        If xfermode is not NULL, its reference count is incremented.
        @param xfermode May be NULL. The new xfermode to be installed in the
                        paint
        @return         xfermode
    */
    SkXfermode* setXfermode(SkXfermode* xfermode);

    /** Create an xfermode based on the specified Mode, and assign it into the
        paint, returning the mode that was set. If the Mode is SrcOver, then
        the paint's xfermode is set to null.
     */
    SkXfermode* setXfermodeMode(SkXfermode::Mode);

    /** Get the paint's patheffect object.
        <p />
      The patheffect reference count is not affected.
        @return the paint's patheffect (or NULL)
    */
    SkPathEffect* getPathEffect() const { return fPathEffect; }

    /** Set or clear the patheffect object.
        <p />
        Pass NULL to clear any previous patheffect.
        As a convenience, the parameter passed is also returned.
        If a previous patheffect exists, its reference count is decremented.
        If patheffect is not NULL, its reference count is incremented.
        @param effect   May be NULL. The new patheffect to be installed in the
                        paint
        @return         effect
    */
    SkPathEffect* setPathEffect(SkPathEffect* effect);

    /** Get the paint's maskfilter object.
        <p />
      The maskfilter reference count is not affected.
        @return the paint's maskfilter (or NULL)
    */
    SkMaskFilter* getMaskFilter() const { return fMaskFilter; }

    /** Set or clear the maskfilter object.
        <p />
        Pass NULL to clear any previous maskfilter.
        As a convenience, the parameter passed is also returned.
        If a previous maskfilter exists, its reference count is decremented.
        If maskfilter is not NULL, its reference count is incremented.
        @param maskfilter   May be NULL. The new maskfilter to be installed in
                            the paint
        @return             maskfilter
    */
    SkMaskFilter* setMaskFilter(SkMaskFilter* maskfilter);

    // These attributes are for text/fonts

    /** Get the paint's typeface object.
        <p />
        The typeface object identifies which font to use when drawing or
        measuring text. The typeface reference count is not affected.
        @return the paint's typeface (or NULL)
    */
    SkTypeface* getTypeface() const { return fTypeface; }

    /** Set or clear the typeface object.
        <p />
        Pass NULL to clear any previous typeface.
        As a convenience, the parameter passed is also returned.
        If a previous typeface exists, its reference count is decremented.
        If typeface is not NULL, its reference count is incremented.
        @param typeface May be NULL. The new typeface to be installed in the
                        paint
        @return         typeface
    */
    SkTypeface* setTypeface(SkTypeface* typeface);

    /** Get the paint's rasterizer (or NULL).
        <p />
        The raster controls how paths/text are turned into alpha masks.
        @return the paint's rasterizer (or NULL)
    */
    SkRasterizer* getRasterizer() const { return fRasterizer; }

    /** Set or clear the rasterizer object.
        <p />
        Pass NULL to clear any previous rasterizer.
        As a convenience, the parameter passed is also returned.
        If a previous rasterizer exists in the paint, its reference count is
        decremented. If rasterizer is not NULL, its reference count is
        incremented.
        @param rasterizer May be NULL. The new rasterizer to be installed in
                          the paint.
        @return           rasterizer
    */
    SkRasterizer* setRasterizer(SkRasterizer* rasterizer);

    SkImageFilter* getImageFilter() const { return fImageFilter; }
    SkImageFilter* setImageFilter(SkImageFilter*);

    SkAnnotation* getAnnotation() const { return fAnnotation; }
    SkAnnotation* setAnnotation(SkAnnotation*);

    /**
     *  Returns true if there is an annotation installed on this paint, and
     *  the annotation specifics no-drawing.
     */
    SK_ATTR_DEPRECATED("use getAnnotation and check for non-null")
    bool isNoDrawAnnotation() const { return this->getAnnotation() != NULL; }

    /**
     *  Return the paint's SkDrawLooper (if any). Does not affect the looper's
     *  reference count.
     */
    SkDrawLooper* getLooper() const { return fLooper; }

    /**
     *  Set or clear the looper object.
     *  <p />
     *  Pass NULL to clear any previous looper.
     *  As a convenience, the parameter passed is also returned.
     *  If a previous looper exists in the paint, its reference count is
     *  decremented. If looper is not NULL, its reference count is
     *  incremented.
     *  @param looper May be NULL. The new looper to be installed in the paint.
     *  @return looper
     */
    SkDrawLooper* setLooper(SkDrawLooper* looper);

    enum Align {
        kLeft_Align,
        kCenter_Align,
        kRight_Align,
    };
    enum {
        kAlignCount = 3
    };

    /** Return the paint's Align value for drawing text.
        @return the paint's Align value for drawing text.
    */
    Align   getTextAlign() const { return (Align)fTextAlign; }

    /** Set the paint's text alignment.
        @param align set the paint's Align value for drawing text.
    */
    void    setTextAlign(Align align);

    /** Return the paint's text size.
        @return the paint's text size.
    */
    SkScalar getTextSize() const { return fTextSize; }

    /** Set the paint's text size. This value must be > 0
        @param textSize set the paint's text size.
    */
    void setTextSize(SkScalar textSize);

    /** Return the paint's horizontal scale factor for text. The default value
        is 1.0.
        @return the paint's scale factor in X for drawing/measuring text
    */
    SkScalar getTextScaleX() const { return fTextScaleX; }

    /** Set the paint's horizontal scale factor for text. The default value
        is 1.0. Values > 1.0 will stretch the text wider. Values < 1.0 will
        stretch the text narrower.
        @param scaleX   set the paint's scale factor in X for drawing/measuring
                        text.
    */
    void setTextScaleX(SkScalar scaleX);

    /** Return the paint's horizontal skew factor for text. The default value
        is 0.
        @return the paint's skew factor in X for drawing text.
    */
    SkScalar getTextSkewX() const { return fTextSkewX; }

    /** Set the paint's horizontal skew factor for text. The default value
        is 0. For approximating oblique text, use values around -0.25.
        @param skewX set the paint's skew factor in X for drawing text.
    */
    void setTextSkewX(SkScalar skewX);

    /** Describes how to interpret the text parameters that are passed to paint
        methods like measureText() and getTextWidths().
    */
    enum TextEncoding {
        kUTF8_TextEncoding,     //!< the text parameters are UTF8
        kUTF16_TextEncoding,    //!< the text parameters are UTF16
        kUTF32_TextEncoding,    //!< the text parameters are UTF32
        kGlyphID_TextEncoding   //!< the text parameters are glyph indices
    };

    TextEncoding getTextEncoding() const { return (TextEncoding)fTextEncoding; }

    void setTextEncoding(TextEncoding encoding);

    struct FontMetrics {
        /** Flags which indicate the confidence level of various metrics.
            A set flag indicates that the metric may be trusted.
        */
        enum FontMetricsFlags {
            kUnderlineThinknessIsValid_Flag = 1 << 0,
            kUnderlinePositionIsValid_Flag = 1 << 1,
        };

        uint32_t    fFlags;       //!< Bit field to identify which values are unknown
        SkScalar    fTop;       //!< The greatest distance above the baseline for any glyph (will be <= 0)
        SkScalar    fAscent;    //!< The recommended distance above the baseline (will be <= 0)
        SkScalar    fDescent;   //!< The recommended distance below the baseline (will be >= 0)
        SkScalar    fBottom;    //!< The greatest distance below the baseline for any glyph (will be >= 0)
        SkScalar    fLeading;   //!< The recommended distance to add between lines of text (will be >= 0)
        SkScalar    fAvgCharWidth;  //!< the average character width (>= 0)
        SkScalar    fMaxCharWidth;  //!< the max character width (>= 0)
        SkScalar    fXMin;      //!< The minimum bounding box x value for all glyphs
        SkScalar    fXMax;      //!< The maximum bounding box x value for all glyphs
        SkScalar    fXHeight;   //!< The height of an 'x' in px, or 0 if no 'x' in face
        SkScalar    fCapHeight;  //!< The cap height (> 0), or 0 if cannot be determined.
        SkScalar    fUnderlineThickness; //!< underline thickness, or 0 if cannot be determined

        /**  Underline Position - position of the top of the Underline stroke
                relative to the baseline, this can have following values
                - Negative - means underline should be drawn above baseline.
                - Positive - means below baseline.
                - Zero     - mean underline should be drawn on baseline.
         */
        SkScalar    fUnderlinePosition; //!< underline position, or 0 if cannot be determined

        /**  If the fontmetrics has a valid underlinethickness, return true, and set the
                thickness param to that value. If it doesn't return false and ignore the
                thickness param.
        */
        bool hasUnderlineThickness(SkScalar* thickness) const {
            if (SkToBool(fFlags & kUnderlineThinknessIsValid_Flag)) {
                *thickness = fUnderlineThickness;
                return true;
            }
            return false;
        }

        /**  If the fontmetrics has a valid underlineposition, return true, and set the
                thickness param to that value. If it doesn't return false and ignore the
                thickness param.
        */
        bool hasUnderlinePosition(SkScalar* position) const {
            if (SkToBool(fFlags & kUnderlinePositionIsValid_Flag)) {
                *position = fUnderlinePosition;
                return true;
            }
            return false;
        }

    };

    /** Return the recommend spacing between lines (which will be
        fDescent - fAscent + fLeading).
        If metrics is not null, return in it the font metrics for the
        typeface/pointsize/etc. currently set in the paint.
        @param metrics      If not null, returns the font metrics for the
                            current typeface/pointsize/etc setting in this
                            paint.
        @param scale        If not 0, return width as if the canvas were scaled
                            by this value
        @param return the recommended spacing between lines
    */
    SkScalar getFontMetrics(FontMetrics* metrics, SkScalar scale = 0) const;

    /** Return the recommend line spacing. This will be
        fDescent - fAscent + fLeading
    */
    SkScalar getFontSpacing() const { return this->getFontMetrics(NULL, 0); }

    /** Convert the specified text into glyph IDs, returning the number of
        glyphs ID written. If glyphs is NULL, it is ignore and only the count
        is returned.
    */
    int textToGlyphs(const void* text, size_t byteLength,
                     uint16_t glyphs[]) const;

    /** Return true if all of the specified text has a corresponding non-zero
        glyph ID. If any of the code-points in the text are not supported in
        the typeface (i.e. the glyph ID would be zero), then return false.

        If the text encoding for the paint is kGlyph_TextEncoding, then this
        returns true if all of the specified glyph IDs are non-zero.
     */
    bool containsText(const void* text, size_t byteLength) const;

    /** Convert the glyph array into Unichars. Unconvertable glyphs are mapped
        to zero. Note: this does not look at the text-encoding setting in the
        paint, only at the typeface.
    */
    void glyphsToUnichars(const uint16_t glyphs[], int count,
                          SkUnichar text[]) const;

    /** Return the number of drawable units in the specified text buffer.
        This looks at the current TextEncoding field of the paint. If you also
        want to have the text converted into glyph IDs, call textToGlyphs
        instead.
    */
    int countText(const void* text, size_t byteLength) const {
        return this->textToGlyphs(text, byteLength, NULL);
    }

    /** Return the width of the text. This will return the vertical measure
     *  if isVerticalText() is true, in which case the returned value should
     *  be treated has a height instead of a width.
     *
     *  @param text         The text to be measured
     *  @param length       Number of bytes of text to measure
     *  @param bounds       If not NULL, returns the bounds of the text,
     *                      relative to (0, 0).
     *  @param scale        If not 0, return width as if the canvas were scaled
     *                      by this value
     *  @return             The advance width of the text
     */
    SkScalar measureText(const void* text, size_t length,
                         SkRect* bounds, SkScalar scale = 0) const;

    /** Return the width of the text. This will return the vertical measure
     *  if isVerticalText() is true, in which case the returned value should
     *  be treated has a height instead of a width.
     *
     *  @param text     Address of the text
     *  @param length   Number of bytes of text to measure
     *  @return         The advance width of the text
     */
    SkScalar measureText(const void* text, size_t length) const {
        return this->measureText(text, length, NULL, 0);
    }

    /** Specify the direction the text buffer should be processed in breakText()
    */
    enum TextBufferDirection {
        /** When measuring text for breakText(), begin at the start of the text
            buffer and proceed forward through the data. This is the default.
        */
        kForward_TextBufferDirection,
        /** When measuring text for breakText(), begin at the end of the text
            buffer and proceed backwards through the data.
        */
        kBackward_TextBufferDirection
    };

    /** Return the number of bytes of text that were measured. If
     *  isVerticalText() is true, then the vertical advances are used for
     *  the measurement.
     *
     *  @param text     The text to be measured
     *  @param length   Number of bytes of text to measure
     *  @param maxWidth Maximum width. Only the subset of text whose accumulated
     *                  widths are <= maxWidth are measured.
     *  @param measuredWidth Optional. If non-null, this returns the actual
     *                  width of the measured text.
     *  @param tbd      Optional. The direction the text buffer should be
     *                  traversed during measuring.
     *  @return         The number of bytes of text that were measured. Will be
     *                  <= length.
     */
    size_t  breakText(const void* text, size_t length, SkScalar maxWidth,
                      SkScalar* measuredWidth = NULL,
                      TextBufferDirection tbd = kForward_TextBufferDirection)
                      const;

    /** Return the advances for the text. These will be vertical advances if
     *  isVerticalText() returns true.
     *
     *  @param text         the text
     *  @param byteLength   number of bytes to of text
     *  @param widths       If not null, returns the array of advances for
     *                      the glyphs. If not NULL, must be at least a large
     *                      as the number of unichars in the specified text.
     *  @param bounds       If not null, returns the bounds for each of
     *                      character, relative to (0, 0)
     *  @return the number of unichars in the specified text.
     */
    int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                      SkRect bounds[] = NULL) const;

    /** Return the path (outline) for the specified text.
        Note: just like SkCanvas::drawText, this will respect the Align setting
              in the paint.
    */
    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                     SkPath* path) const;

    void getPosTextPath(const void* text, size_t length,
                        const SkPoint pos[], SkPath* path) const;

#ifdef SK_BUILD_FOR_ANDROID
    uint32_t getGenerationID() const;
    void setGenerationID(uint32_t generationID);

    /** Returns the base glyph count for the strike associated with this paint
    */
    unsigned getBaseGlyphCount(SkUnichar text) const;

    const SkPaintOptionsAndroid& getPaintOptionsAndroid() const {
        return fPaintOptionsAndroid;
    }
    void setPaintOptionsAndroid(const SkPaintOptionsAndroid& options);
#endif

    // returns true if the paint's settings (e.g. xfermode + alpha) resolve to
    // mean that we need not draw at all (e.g. SrcOver + 0-alpha)
    bool nothingToDraw() const;

    ///////////////////////////////////////////////////////////////////////////
    // would prefer to make these private...

    /** Returns true if the current paint settings allow for fast computation of
     bounds (i.e. there is nothing complex like a patheffect that would make
     the bounds computation expensive.
     */
    bool canComputeFastBounds() const {
        if (this->getLooper()) {
            return this->getLooper()->canComputeFastBounds(*this);
        }
        return !this->getRasterizer();
    }

    /** Only call this if canComputeFastBounds() returned true. This takes a
     raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
     effects in the paint (e.g. stroking). If needed, it uses the storage
     rect parameter. It returns the adjusted bounds that can then be used
     for quickReject tests.

     The returned rect will either be orig or storage, thus the caller
     should not rely on storage being set to the result, but should always
     use the retured value. It is legal for orig and storage to be the same
     rect.

     e.g.
     if (paint.canComputeFastBounds()) {
     SkRect r, storage;
     path.computeBounds(&r, SkPath::kFast_BoundsType);
     const SkRect& fastR = paint.computeFastBounds(r, &storage);
     if (canvas->quickReject(fastR, ...)) {
     // don't draw the path
     }
     }
     */
    const SkRect& computeFastBounds(const SkRect& orig, SkRect* storage) const {
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

    const SkRect& computeFastStrokeBounds(const SkRect& orig,
                                          SkRect* storage) const {
        return this->doComputeFastBounds(orig, storage, kStroke_Style);
    }

    // Take the style explicitly, so the caller can force us to be stroked
    // without having to make a copy of the paint just to change that field.
    const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage,
                                      Style) const;

    /**
     *  Return a matrix that applies the paint's text values: size, scale, skew
     */
    static SkMatrix* SetTextMatrix(SkMatrix* matrix, SkScalar size,
                                   SkScalar scaleX, SkScalar skewX) {
        matrix->setScale(size * scaleX, size);
        if (skewX) {
            matrix->postSkew(skewX, 0);
        }
        return matrix;
    }

    SkMatrix* setTextMatrix(SkMatrix* matrix) const {
        return SetTextMatrix(matrix, fTextSize, fTextScaleX, fTextSkewX);
    }

    SK_TO_STRING_NONVIRT()

    struct FlatteningTraits {
        static void Flatten(SkWriteBuffer& buffer, const SkPaint& paint);
        static void Unflatten(SkReadBuffer& buffer, SkPaint* paint);
    };

private:
    SkTypeface*     fTypeface;
    SkScalar        fTextSize;
    SkScalar        fTextScaleX;
    SkScalar        fTextSkewX;

    SkPathEffect*   fPathEffect;
    SkShader*       fShader;
    SkXfermode*     fXfermode;
    SkMaskFilter*   fMaskFilter;
    SkColorFilter*  fColorFilter;
    SkRasterizer*   fRasterizer;
    SkDrawLooper*   fLooper;
    SkImageFilter*  fImageFilter;
    SkAnnotation*   fAnnotation;

    SkColor         fColor;
    SkScalar        fWidth;
    SkScalar        fMiterLimit;

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
            //unsigned      fFreeBits : 4;
        };
        uint32_t fBitfields;
    };
    uint32_t getBitfields() const { return fBitfields; }
    void setBitfields(uint32_t bitfields);

    uint32_t fDirtyBits;

    SkDrawCacheProc    getDrawCacheProc() const;
    SkMeasureCacheProc getMeasureCacheProc(TextBufferDirection dir,
                                           bool needFullMetrics) const;

    SkScalar measure_text(SkGlyphCache*, const char* text, size_t length,
                          int* count, SkRect* bounds) const;

    SkGlyphCache* detachCache(const SkDeviceProperties* deviceProperties, const SkMatrix*) const;

    void descriptorProc(const SkDeviceProperties* deviceProperties, const SkMatrix* deviceMatrix,
                        void (*proc)(SkTypeface*, const SkDescriptor*, void*),
                        void* context, bool ignoreGamma = false) const;

    static void Term();

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

        /*
         *  Above this size (taking into account CTM and textSize), we never use
         *  the cache for bits or metrics (we might overflow), so we just ask
         *  for a caononical size and post-transform that.
         */
        kMaxSizeForGlyphCache       = 256,
    };

    static bool TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM);

    bool tooBigToUseCache() const;
    bool tooBigToUseCache(const SkMatrix& ctm) const;

    // Set flags/hinting/textSize up to use for drawing text as paths.
    // Returns scale factor to restore the original textSize, since will will
    // have change it to kCanonicalTextSizeForPaths.
    SkScalar setupForAsPaths();

    static SkScalar MaxCacheSize2() {
        static const SkScalar kMaxSize = SkIntToScalar(kMaxSizeForGlyphCache);
        static const SkScalar kMag2Max = kMaxSize * kMaxSize;
        return kMag2Max;
    }

    friend class SkAutoGlyphCache;
    friend class SkCanvas;
    friend class SkDraw;
    friend class SkGraphics; // So Term() can be called.
    friend class SkPDFDevice;
    friend class GrBitmapTextContext;
    friend class GrDistanceFieldTextContext;
    friend class SkTextToPathIter;
    friend class SkCanonicalizePaint;

#ifdef SK_BUILD_FOR_ANDROID
    SkPaintOptionsAndroid fPaintOptionsAndroid;

    // In order for the == operator to work properly this must be the last field
    // in the struct so that we can do a memcmp to this field's offset.
    uint32_t        fGenerationID;
#endif
};

#endif
