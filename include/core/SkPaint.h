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

    The SkPaint class holds the style and color information about how to draw
    geometries, text and bitmaps.
*/
class SK_API SkPaint {
public:
    SkPaint();
    SkPaint(const SkPaint& paint);
    SkPaint(SkPaint&& paint);
    ~SkPaint();

    SkPaint& operator=(const SkPaint& paint);
    SkPaint& operator=(SkPaint&& paint);

    /** operator== may give false negatives: two paints that draw equivalently
        may return false.  It will never give false positives: two paints that
        are not equivalent always return false.
    */
    SK_API friend bool operator==(const SkPaint& a, const SkPaint& b);
    friend bool operator!=(const SkPaint& a, const SkPaint& b) {
        return !(a == b);
    }

    /** getHash() is a shallow hash, with the same limitations as operator==.
     *  If operator== returns true for two paints, getHash() returns the same value for each.
     */
    uint32_t getHash() const;

    void flatten(SkWriteBuffer& buffer) const;
    void unflatten(SkReadBuffer& buffer);

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
        return static_cast<Hinting>(fBitfields.fHinting);
    }

    void setHinting(Hinting hintingLevel);

    /** Specifies the bit values that are stored in the paint's flags.
    */
    enum Flags {
        kAntiAlias_Flag       = 0x01,   //!< mask to enable antialiasing
        kDither_Flag          = 0x04,   //!< mask to enable dithering. see setDither()
        kFakeBoldText_Flag    = 0x20,   //!< mask to enable fake-bold text
        kLinearText_Flag      = 0x40,   //!< mask to enable linear-text
        kSubpixelText_Flag    = 0x80,   //!< mask to enable subpixel text positioning
        kDevKernText_Flag     = 0x100,  //!< mask to enable device kerning text
        kLCDRenderText_Flag   = 0x200,  //!< mask to enable subpixel glyph renderering
        kEmbeddedBitmapText_Flag = 0x400, //!< mask to enable embedded bitmap strikes
        kAutoHinting_Flag     = 0x800,  //!< mask to force Freetype's autohinter
        kVerticalText_Flag    = 0x1000,
        kGenA8FromLCD_Flag    = 0x2000, // hack for GDI -- do not use if you can help it
        // when adding extra flags, note that the fFlags member is specified
        // with a bit-width and you'll have to expand it.

        kAllFlags = 0xFFFF,
    };

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    enum ReserveFlags {
        // These are not used by paint, but the bits are reserved for private use by the
        // android framework.
        kUnderlineText_ReserveFlag   = 0x08,   //!< mask to enable underline text
        kStrikeThruText_ReserveFlag  = 0x10,   //!< mask to enable strike-thru text
    };
#endif

    /** Return the paint's flags. Use the Flag enum to test flag values.
        @return the paint's flags (see enums ending in _Flag for bit masks)
    */
    uint32_t getFlags() const { return fBitfields.fFlags; }

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

    /**
     *  Helper for setFlags(), setting or clearing the kDither_Flag bit
     *  @param dither   true to enable dithering, false to disable it
     *
     *  Note: gradients ignore this setting and always dither.
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
    void setVerticalText(bool verticalText);

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

    /**
     *  Return the filter level. This affects the quality (and performance) of
     *  drawing scaled images.
     */
    SkFilterQuality getFilterQuality() const {
        return (SkFilterQuality)fBitfields.fFilterQuality;
    }

    /**
     *  Set the filter quality. This affects the quality (and performance) of
     *  drawing scaled images.
     */
    void setFilterQuality(SkFilterQuality quality);

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
    Style getStyle() const { return (Style)fBitfields.fStyle; }

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

        If the cap is round or square, the caps are drawn when the contour has
        a zero length. Zero length contours can be created by following moveTo
        with a lineTo at the same point, or a moveTo followed by a close.

        A dash with an on interval of zero also creates a zero length contour.

        The zero length contour draws the square cap without rotation, since
        the no direction can be inferred.
    */
    enum Cap {
        kButt_Cap,      //!< begin/end contours with no extension
        kRound_Cap,     //!< begin/end contours with a semi-circle extension
        kSquare_Cap,    //!< begin/end contours with a half square extension

        kLast_Cap = kSquare_Cap,
        kDefault_Cap = kButt_Cap
    };
    static constexpr int kCapCount = kLast_Cap + 1;

    /** Join enum specifies the settings for the paint's strokejoin. This is
        the treatment that is applied to corners in paths and rectangles.
    */
    enum Join {
        kMiter_Join,    //!< connect path segments with a sharp join
        kRound_Join,    //!< connect path segments with a round join
        kBevel_Join,    //!< connect path segments with a flat bevel join

        kLast_Join = kBevel_Join,
        kDefault_Join = kMiter_Join
    };
    static constexpr int kJoinCount = kLast_Join + 1;

    /** Return the paint's stroke cap type, controlling how the start and end
        of stroked lines and paths are treated.
        @return the line cap style for the paint, used whenever the paint's
                style is Stroke or StrokeAndFill.
    */
    Cap getStrokeCap() const { return (Cap)fBitfields.fCapType; }

    /** Set the paint's stroke cap type.
        @param cap  set the paint's line cap style, used whenever the paint's
                    style is Stroke or StrokeAndFill.
    */
    void setStrokeCap(Cap cap);

    /** Return the paint's stroke join type.
        @return the paint's line join style, used whenever the paint's style is
                Stroke or StrokeAndFill.
    */
    Join getStrokeJoin() const { return (Join)fBitfields.fJoinType; }

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
     *  @param resScale If > 1, increase precision, else if (0 < res < 1) reduce precision
     *              in favor of speed/size.
     *  @return     true if the path should be filled, or false if it should be
     *              drawn with a hairline (width == 0)
     */
    bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                     SkScalar resScale = 1) const;

    bool getFillPath(const SkPath& src, SkPath* dst) const {
        return this->getFillPath(src, dst, NULL, 1);
    }

    /** Get the paint's shader object.
        <p />
      The shader's reference count is not affected.
        @return the paint's shader (or NULL)
    */
    SkShader* getShader() const { return fShader.get(); }
    sk_sp<SkShader> refShader() const;

    /** Set or clear the shader object.
     *  Shaders specify the source color(s) for what is being drawn. If a paint
     *  has no shader, then the paint's color is used. If the paint has a
     *  shader, then the shader's color(s) are use instead, but they are
     *  modulated by the paint's alpha. This makes it easy to create a shader
     *  once (e.g. bitmap tiling or gradient) and then change its transparency
     *  w/o having to modify the original shader... only the paint's alpha needs
     *  to be modified.
     *
     *  There is an exception to this only-respect-paint's-alpha rule: If the shader only generates
     *  alpha (e.g. SkShader::CreateBitmapShader(bitmap, ...) where bitmap's colortype is kAlpha_8)
     *  then the shader will use the paint's entire color to "colorize" its output (modulating the
     *  bitmap's alpha with the paint's color+alpha).
     *
     *  Pass NULL to clear any previous shader.
     *  As a convenience, the parameter passed is also returned.
     *  If a previous shader exists, its reference count is decremented.
     *  If shader is not NULL, its reference count is incremented.
     *  @param shader   May be NULL. The shader to be installed in the paint
     */
    void setShader(sk_sp<SkShader> shader);

    /** Get the paint's colorfilter. If there is a colorfilter, its reference
        count is not changed.
        @return the paint's colorfilter (or NULL)
    */
    SkColorFilter* getColorFilter() const { return fColorFilter.get(); }
    sk_sp<SkColorFilter> refColorFilter() const;

    /** Set or clear the paint's colorfilter.
        <p />
        If the paint already has a filter, its reference count is decremented.
        If filter is not NULL, its reference count is incremented.
        @param filter   May be NULL. The filter to be installed in the paint
    */
    void setColorFilter(sk_sp<SkColorFilter> colorFilter);

    SkBlendMode getBlendMode() const { return (SkBlendMode)fBlendMode; }
    bool isSrcOver() const { return (SkBlendMode)fBlendMode == SkBlendMode::kSrcOver; }
    void setBlendMode(SkBlendMode mode) { fBlendMode = (unsigned)mode; }

    /** Get the paint's patheffect object.
        <p />
      The patheffect reference count is not affected.
        @return the paint's patheffect (or NULL)
    */
    SkPathEffect* getPathEffect() const { return fPathEffect.get(); }
    sk_sp<SkPathEffect> refPathEffect() const;

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
    void setPathEffect(sk_sp<SkPathEffect> pathEffect);

    /** Get the paint's maskfilter object.
        <p />
      The maskfilter reference count is not affected.
        @return the paint's maskfilter (or NULL)
    */
    SkMaskFilter* getMaskFilter() const { return fMaskFilter.get(); }
    sk_sp<SkMaskFilter> refMaskFilter() const;

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
    void setMaskFilter(sk_sp<SkMaskFilter> maskFilter);

    // These attributes are for text/fonts

    /** Get the paint's typeface object.
        <p />
        The typeface object identifies which font to use when drawing or
        measuring text. The typeface reference count is not affected.
        @return the paint's typeface (or NULL)
    */
    SkTypeface* getTypeface() const { return fTypeface.get(); }
    sk_sp<SkTypeface> refTypeface() const;

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
    void setTypeface(sk_sp<SkTypeface> typeface);

    /** Get the paint's rasterizer (or NULL).
        <p />
        The raster controls how paths/text are turned into alpha masks.
        @return the paint's rasterizer (or NULL)
    */
    SkRasterizer* getRasterizer() const { return fRasterizer.get(); }
    sk_sp<SkRasterizer> refRasterizer() const;

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
    void setRasterizer(sk_sp<SkRasterizer> rasterizer);

    SkImageFilter* getImageFilter() const { return fImageFilter.get(); }
    sk_sp<SkImageFilter> refImageFilter() const;
    void setImageFilter(sk_sp<SkImageFilter> imageFilter);

    /**
     *  Return the paint's SkDrawLooper (if any). Does not affect the looper's
     *  reference count.
     */
    SkDrawLooper* getDrawLooper() const { return fDrawLooper.get(); }
    sk_sp<SkDrawLooper> refDrawLooper() const;

    SkDrawLooper* getLooper() const { return fDrawLooper.get(); }
    /**
     *  Set or clear the looper object.
     *  <p />
     *  Pass NULL to clear any previous looper.
     *  If a previous looper exists in the paint, its reference count is
     *  decremented. If looper is not NULL, its reference count is
     *  incremented.
     *  @param looper May be NULL. The new looper to be installed in the paint.
     */
    void setDrawLooper(sk_sp<SkDrawLooper> drawLooper);

    void setLooper(sk_sp<SkDrawLooper> drawLooper);

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
    Align   getTextAlign() const { return (Align)fBitfields.fTextAlign; }

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

    TextEncoding getTextEncoding() const {
      return (TextEncoding)fBitfields.fTextEncoding;
    }

    void setTextEncoding(TextEncoding encoding);

    struct FontMetrics {
        /** Flags which indicate the confidence level of various metrics.
            A set flag indicates that the metric may be trusted.
        */
        enum FontMetricsFlags {
            kUnderlineThicknessIsValid_Flag = 1 << 0,
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

        /**  If the fontmetrics has a valid underline thickness, return true, and set the
                thickness param to that value. If it doesn't return false and ignore the
                thickness param.
        */
        bool hasUnderlineThickness(SkScalar* thickness) const {
            if (SkToBool(fFlags & kUnderlineThicknessIsValid_Flag)) {
                *thickness = fUnderlineThickness;
                return true;
            }
            return false;
        }

        /**  If the fontmetrics has a valid underline position, return true, and set the
                position param to that value. If it doesn't return false and ignore the
                position param.
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
                     SkGlyphID glyphs[]) const;

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
    void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[]) const;

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
     *  @return             The advance width of the text
     */
    SkScalar measureText(const void* text, size_t length, SkRect* bounds) const;

    /** Return the width of the text. This will return the vertical measure
     *  if isVerticalText() is true, in which case the returned value should
     *  be treated has a height instead of a width.
     *
     *  @param text     Address of the text
     *  @param length   Number of bytes of text to measure
     *  @return         The advance width of the text
     */
    SkScalar measureText(const void* text, size_t length) const {
        return this->measureText(text, length, NULL);
    }

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
     *  @return         The number of bytes of text that were measured. Will be
     *                  <= length.
     */
    size_t  breakText(const void* text, size_t length, SkScalar maxWidth,
                      SkScalar* measuredWidth = NULL) const;

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
     *  Note: just like SkCanvas::drawText, this will respect the Align setting
     *        in the paint.
     *
     *  @param text         the text
     *  @param length       number of bytes of text
     *  @param x            The x-coordinate of the origin of the text.
     *  @param y            The y-coordinate of the origin of the text.
     *  @param path         The outline of the text.
     */
    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                     SkPath* path) const;

    /** Return the path (outline) for the specified text.
     *  Note: just like SkCanvas::drawText, this will respect the Align setting
     *        in the paint.
     *
     *  @param text         the text
     *  @param length       number of bytes of text
     *  @param pos          array of positions, used to position each character
     *  @param path         The outline of the text.
     */
    void getPosTextPath(const void* text, size_t length,
                        const SkPoint pos[], SkPath* path) const;

    /** Return the number of intervals that intersect the intercept along the axis of the advance.
     *  The return count is zero or a multiple of two, and is at most the number of glyphs * 2 in
     *  the string. The caller may pass nullptr for intervals to determine the size of the interval
     *  array, or may conservatively pre-allocate an array with length * 2 entries. The computed
     *  intervals are cached by glyph to improve performance for multiple calls.
     *  This permits constructing an underline that skips the descenders. 
     *
     *  @param text         the text
     *  @param length       number of bytes of text
     *  @param x            The x-coordinate of the origin of the text.
     *  @param y            The y-coordinate of the origin of the text.
     *  @param bounds       The lower and upper line parallel to the advance.
     *  @param array        If not null, the found intersections.
     *
     *  @return             The number of intersections, which may be zero.
     */
    int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,
                          const SkScalar bounds[2], SkScalar* intervals) const;

    /** Return the number of intervals that intersect the intercept along the axis of the advance.
     *  The return count is zero or a multiple of two, and is at most the number of glyphs * 2 in
     *  string. The caller may pass nullptr for intervals to determine the size of the interval
     *  array, or may conservatively pre-allocate an array with length * 2 entries. The computed
     *  intervals are cached by glyph to improve performance for multiple calls.
     *  This permits constructing an underline that skips the descenders. 
     *
     *  @param text         the text
     *  @param length       number of bytes of text
     *  @param pos          array of positions, used to position each character
     *  @param bounds       The lower and upper line parallel to the advance.
     *  @param array        If not null, the glyph bounds contained by the advance parallel lines.
     *
     *  @return             The number of intersections, which may be zero.
     */
    int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],
                             const SkScalar bounds[2], SkScalar* intervals) const;

    /** Return the number of intervals that intersect the intercept along the axis of the advance.
     *  The return count is zero or a multiple of two, and is at most the number of glyphs * 2 in
     *  string. The caller may pass nullptr for intervals to determine the size of the interval
     *  array, or may conservatively pre-allocate an array with length * 2 entries. The computed
     *  intervals are cached by glyph to improve performance for multiple calls.
     *  This permits constructing an underline that skips the descenders.
     *
     *  @param text         The text.
     *  @param length       Number of bytes of text.
     *  @param xpos         Array of x-positions, used to position each character.
     *  @param constY       The shared Y coordinate for all of the positions.
     *  @param bounds       The lower and upper line parallel to the advance.
     *  @param array        If not null, the glyph bounds contained by the advance parallel lines.
     *
     *  @return             The number of intersections, which may be zero.
     */
    int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],
                              SkScalar constY, const SkScalar bounds[2], SkScalar* intervals) const;

    /** Return the number of intervals that intersect the intercept along the axis of the advance.
     *  The return count is zero or a multiple of two, and is at most the number of glyphs * 2 in
     *  text blob. The caller may pass nullptr for intervals to determine the size of the interval
     *  array. The computed intervals are cached by glyph to improve performance for multiple calls.
     *  This permits constructing an underline that skips the descenders.
     *
     *  @param blob         The text blob.
     *  @param bounds       The lower and upper line parallel to the advance.
     *  @param array        If not null, the glyph bounds contained by the advance parallel lines.
     *
     *  @return             The number of intersections, which may be zero.
     */
    int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                              SkScalar* intervals) const;

    /**
     *  Return a rectangle that represents the union of the bounds of all
     *  of the glyphs, but each one positioned at (0,0). This may be conservatively large, and
     *  will not take into account any hinting, but will respect any text-scale-x or text-skew-x
     *  on this paint.
     */
    SkRect getFontBounds() const;

    // returns true if the paint's settings (e.g. xfermode + alpha) resolve to
    // mean that we need not draw at all (e.g. SrcOver + 0-alpha)
    bool nothingToDraw() const;

    ///////////////////////////////////////////////////////////////////////////
    // would prefer to make these private...

    /** Returns true if the current paint settings allow for fast computation of
     bounds (i.e. there is nothing complex like a patheffect that would make
     the bounds computation expensive.
     */
    bool canComputeFastBounds() const;

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

    const SkRect& computeFastStrokeBounds(const SkRect& orig,
                                          SkRect* storage) const {
        return this->doComputeFastBounds(orig, storage, kStroke_Style);
    }

    // Take the style explicitly, so the caller can force us to be stroked
    // without having to make a copy of the paint just to change that field.
    const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage,
                                      Style style) const;



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

        /*
         *  Above this size (taking into account CTM and textSize), we never use
         *  the cache for bits or metrics (we might overflow), so we just ask
         *  for a caononical size and post-transform that.
         */
        kMaxSizeForGlyphCache       = 256,
    };

    static bool TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM);

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
