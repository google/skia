/* include/graphics/SkPaint.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkPaint_DEFINED
#define SkPaint_DEFINED

#include "SkColor.h"
#include "SkMath.h"
#include "SkPorterDuff.h"

class SkColorFilter;
struct SkGlyph;
class SkGlyphCache;
class SkMaskFilter;
class SkMatrix;
class SkPath;
class SkPathEffect;
class SkRasterizer;
class SkShader;
class SkTypeface;
class SkXfermode;

typedef const SkGlyph& (*SkGlyphCacheProc)(SkGlyphCache*, const char**);

/** \class SkPaint

    The SkPaint class holds the style and color information about how to draw geometries, text and bitmaps.
*/
class SkPaint {
public:
    SkPaint();
    SkPaint(const SkPaint& paint);
    ~SkPaint();

    SkPaint& operator=(const SkPaint&);

    friend int operator==(const SkPaint& a, const SkPaint& b);
    friend int operator!=(const SkPaint& a, const SkPaint& b) { return !(a == b); }

    /** Restores the paint to its initial settings.
    */
    void    reset();

    /** FlagShift enum specifies the amount to bit-shift for a given flag setting.
        Can be used to slide a boolean value into the correct position (e.g.
        flags |= isAntiAlias << kAntiAlias_Shift;
    */
    enum FlagShift {
        kAntiAlias_Shift,       //!< bit position for the flag enabling antialiasing
        kLinearText_Shift   ,   //!< bit position for the flag enabling linear-text (no gridding)
        kUnderlineText_Shift,   //!< bit position for the flag enabling underline text
        kStrikeThruText_Shift,  //!< bit position for the flag enabling strike-thru text
        kFakeBoldText_Shift,    //!< bit position for the flag enabling fake-bold text
        kLCDText_Shift,
        kNativeHintsText_Shift,

        kFlagShiftCount
    };
    
    /** Flag enum specifies the bit values that are stored in the paint's flags.
    */
    enum Flag {
        kAntiAlias_Flag     = 1 << kAntiAlias_Shift,        //!< bit mask for the flag enabling antialiasing
        kLinearText_Flag    = 1 << kLinearText_Shift,       //!< bit mask for the flag enabling linear-text (no gridding)
        kUnderlineText_Flag = 1 << kUnderlineText_Shift,    //!< bit mask for the flag enabling underline text
        kStrikeThruText_Flag= 1 << kStrikeThruText_Shift,   //!< bit mask for the flag enabling strike-thru text
        kFakeBoldText_Flag  = 1 << kFakeBoldText_Shift,     //!< bit mask for the flag enabling fake-bold text
        kLCDText_Flag       = 1 << kLCDText_Shift,
        kNativeHintsText_Flag = 1 << kNativeHintsText_Shift,

        kAllFlags = (1 << kFlagShiftCount) - 1
    };

    /** Return the paint's flags. Use the Flag enum to test flag values.
        @return the paint's flags (see enums ending in _Flag for bit masks)
    */
    uint32_t    getFlags() const { return fFlags; }
    /** Set the paint's flags. Use the Flag enum to specific flag values.
        @param flags    The new flag bits for the paint (see enums ending in _Flag for bit masks)
    */
    void    setFlags(uint32_t flags);

    /** Helper for getFlags(), returning true if kAntiAlias_Flag bit is set
        @return true if the antialias bit is set in the paint's flags.
    */
    bool    isAntiAliasOn() const { return SkToBool(this->getFlags() & kAntiAlias_Flag); }
    /** Helper for setFlags(), setting or clearing the kAntiAlias_Flag bit
        @param aa   true to set the antialias bit in the flags, false to clear it
    */
    void    setAntiAliasOn(bool aa);
    /** Helper for getFlags(), returning true if kLinearText_Flag bit is set
        @return true if the lineartext bit is set in the paint's flags
    */
    bool    isLinearTextOn() const { return SkToBool(this->getFlags() & kLinearText_Flag); }
    /** Helper for setFlags(), setting or clearing the kLinearText_Flag bit
        @param linearText true to set the linearText bit in the paint's flags, false to clear it.
    */
    void    setLinearTextOn(bool linearText);
    /** Helper for getFlags(), returning true if kUnderlineText_Flag bit is set
        @return true if the underlineText bit is set in the paint's flags.
    */
    bool    isUnderlineTextOn() const { return SkToBool(this->getFlags() & kUnderlineText_Flag); }
    /** Helper for setFlags(), setting or clearing the kUnderlineText_Flag bit
        @param underlineText true to set the underlineText bit in the paint's flags, false to clear it.
    */
    void    setUnderlineTextOn(bool underlineText);
    /** Helper for getFlags(), returning true if kStrikeThruText_Flag bit is set
        @return true if the strikeThruText bit is set in the paint's flags.
    */
    bool    isStrikeThruTextOn() const { return SkToBool(this->getFlags() & kStrikeThruText_Flag); }
    /** Helper for setFlags(), setting or clearing the kStrikeThruText_Flag bit
        @param strikeThruText   true to set the strikeThruText bit in the paint's flags, false to clear it.
    */
    void    setStrikeThruTextOn(bool strikeThruText);
    /** Helper for getFlags(), returning true if kFakeBoldText_Flag bit is set
        @return true if the fakeBoldText bit is set in the paint's flags.
    */
    bool    isFakeBoldTextOn() const { return SkToBool(this->getFlags() & kFakeBoldText_Flag); }
    /** Helper for setFlags(), setting or clearing the kStrikeThruText_Flag bit
        @param fakeBoldText true to set the fakeBoldText bit in the paint's flags, false to clear it.
    */
    void    setFakeBoldTextOn(bool fakeBoldText);

    /** Styles apply to rect, oval, path, and text.
        Bitmaps are always drawn in "fill", and lines are always drawn in "stroke"
    */
    enum Style {
        kFill_Style,            //!< fill with the paint's color
        kStroke_Style,          //!< stroke with the paint's color
        kStrokeAndFill_Style,   //!< fill and stroke with the paint's color

        kStyleCount,
        kDefault_Style = kFill_Style,   //!< the default style setting in the paint
    };
    /** Return the paint's style, used for controlling how primitives'
        geometries are interpreted (except for drawBitmap, which always assumes
        kFill_Style).
        @return the paint's style setting (Fill, Stroke, StrokeAndFill)
    */
    Style   getStyle() const { return (Style)fStyle; }
    /** Set the paint's style, used for controlling how primitives'
        geometries are interpreted (except for drawBitmap, which always assumes
        Fill).
        @param style    The new style to set in the paint (Fill, Stroke, StrokeAndFill)
    */
    void    setStyle(Style style);

    /** Return the paint's color. Note that the color is a 32bit value containing alpha
        as well as r,g,b. This 32bit value is not premultiplied, meaning that
        its alpha can be any value, regardless of the values of r,g,b.
        @return the paint's color (and alpha).
    */
    SkColor getColor() const { return fColor; }
    /** Helper to getColor() that just returns the color's alpha value.
        @return the alpha component of the paint's color.
    */
    uint8_t getAlpha() const { return SkToU8(SkColorGetA(fColor)); }
    /** Set the paint's color. Note that the color is a 32bit value containing alpha
        as well as r,g,b. This 32bit value is not premultiplied, meaning that
        its alpha can be any value, regardless of the values of r,g,b.
        @param color    The new color (including alpha) to set in the paint.
    */
    void    setColor(SkColor color);
    /** Helper to setColor(), that only assigns the color's alpha value, leaving its
        r,g,b values unchanged.
        @param a    set the alpha component (0..255) of the paint's color.
    */
    void    setAlpha(U8CPU a);
    /** Helper to setColor(), that takes a,r,g,b and constructs the color value using SkColorSetARGB()
        @param a    The new alpha component (0..255) of the paint's color.
        @param r    The new red component (0..255) of the paint's color.
        @param g    The new green component (0..255) of the paint's color.
        @param b    The new blue component (0..255) of the paint's color.
    */
    void    setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

    /** Return the width for stroking. 
        <p />
        A value of 0 strokes in hairline mode.
        Hairlines always draws a single pixel independent of the canva's matrix.
        @return the paint's stroke width, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    SkScalar    getStrokeWidth() const { return fWidth; }
    /** Set the width for stroking. 
        Pass 0 to stroke in hairline mode.
        Hairlines always draws a single pixel independent of the canva's matrix.
        @param width set the paint's stroke width, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    void        setStrokeWidth(SkScalar width);

    /** Return the paint's stroke miter value. This is used to control the behavior
        of miter joins when the joins angle is sharp.
        @return the paint's miter limit, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    SkScalar    getStrokeMiter() const { return fMiterLimit; }
    /** Set the paint's stroke miter value. This is used to control the behavior
        of miter joins when the joins angle is sharp. This value must be >= 0.
        @param miter    set the miter limit on the paint, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    void        setStrokeMiter(SkScalar miter);

    /** Cap enum specifies the settings for the paint's strokecap. This is the treatment
        that is applied to the beginning and end of each non-closed contour (e.g. lines).
    */
    enum Cap {
        kButt_Cap,      //!< begin and end a contour with no extension
        kRound_Cap,     //!< begin and end a contour with a semi-circle extension
        kSquare_Cap,    //!< begin and end a contour with a half square extension

        kCapCount,
        kDefault_Cap = kButt_Cap
    };

    /** Join enum specifies the settings for the paint's strokejoin. This is the treatment
        that is applied to corners in paths and rectangles.
    */
    enum Join {
        kMiter_Join,    //!< connect path segments with a sharp join (respects miter-limit)
        kRound_Join,    //!< connect path segments with a round join
        kBevel_Join,    //!< connect path segments with a flat bevel join

        kJoinCount,
        kDefault_Join = kMiter_Join
    };

    /** Return the paint's stroke cap type, controlling how the start and end of stroked lines and paths
        are treated.
        @return the line cap style for the paint, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    Cap     getStrokeCap() const { return (Cap)fCapType; }
    /** Set the paint's stroke cap type.
        @param cap  set the paint's line cap style, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    void    setStrokeCap(Cap cap);

    /** Return the paint's stroke join type.
        @return the paint's line join style, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    Join    getStrokeJoin() const { return (Join)fJoinType; }
    /** Set the paint's stroke join type.
        @param join set the paint's line join style, used whenever the paint's style is Stroke or StrokeAndFill.
    */
    void    setStrokeJoin(Join join);

    /** Applies any/all effects (patheffect, stroking) to src, returning the result in dst.
        The result is that drawing src with this paint will be the same as drawing dst
        with a default paint (at least from the geometric perspective).
        @param src  input path
        @param dst  output path (may be the same as src)
        @return true if the path should be filled, or false if it should be drawn with a hairline (width == 0)
    */
    bool    getFillPath(const SkPath& src, SkPath* dst) const;

    enum FilterType {
        kNo_FilterType,         //!< draw bitmaps using nearest-neighbor sampling
        kBilinear_FilterType,   //!< draw bitmaps using bilinear sampling

        kFilterTypeCount
    };
    /** Return the paint's bitmap filter type. This setting affects drawBitmap() and bitmaps
        that appear inside a bitmap shader.
        @return the paint's filter type, used when drawing bitmaps.
    */
    FilterType  getFilterType() const { return (FilterType)fFilterType; }
    /** Set the paint's bitmap filter type. This setting affects drawBitmap() and bitmaps
        that appear inside a bitmap shader.
        @param filterType   set the new filter type on the paint, used when drawing a bitmap
    */
    void        setFilterType(FilterType filterType);

    /** Get the paint's shader object.
        <p />
      The shader's reference count is not affected.
        @return the paint's shader (or NULL)
    */
    SkShader*   getShader() const { return fShader; }
    /** Set or clear the shader object.
        <p />
        Pass NULL to clear any previous shader.
        As a convenience, the parameter passed is also returned.
        If a previous shader exists, its reference count is decremented.
        If shader is not NULL, its reference count is incremented.
        @param shader   May be NULL. the new shader to be installed in the paint
        @return         shader
    */
    SkShader*   setShader(SkShader* shader);
    
    /** Get the paint's colorfilter (or NULL). If there is a colorfilter, its reference
        count is not changed.
        @return the paint's colorfilter (or NULL)
    */
    SkColorFilter*  getColorFilter() const { return fColorFilter; }
    /** Set or clear the paint's colorfilter, returning the parameter.
        <p />
        If the paint already has a filter, its reference count is decremented.
        If filter is not NULL, its reference count is incremented.
        @param filter   May be NULL. The new filter to be installed in the paint
        @return         filter
    */
    SkColorFilter*  setColorFilter(SkColorFilter* filter);

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
        @param xfermode May be NULL. The new xfermode to be installed in the paint
        @return         xfermode
    */
    SkXfermode* setXfermode(SkXfermode* xfermode);
    
    /** Helper for setXfermode, passing the corresponding xfermode object returned from the
        PorterDuff factory.
        @param mode The porter-duff mode used to create an xfermode for the paint.
        @return the resulting xfermode object (or NULL if the mode is SrcOver)
    */
    SkXfermode* setPorterDuffXfermode(SkPorterDuff::Mode mode);

    /** Get the paint's patheffect object.
        <p />
      The patheffect reference count is not affected.
        @return the paint's patheffect (or NULL)
    */
    SkPathEffect*   getPathEffect() const { return fPathEffect; }
    /** Set or clear the patheffect object.
        <p />
        Pass NULL to clear any previous patheffect.
        As a convenience, the parameter passed is also returned.
        If a previous patheffect exists, its reference count is decremented.
        If patheffect is not NULL, its reference count is incremented.
        @param effect   May be NULL. The new patheffect to be installed in the paint
        @return         effect
    */
    SkPathEffect*   setPathEffect(SkPathEffect* effect);

    /** Get the paint's maskfilter object.
        <p />
      The maskfilter reference count is not affected.
        @return the paint's maskfilter (or NULL)
    */
    SkMaskFilter*   getMaskFilter() const { return fMaskFilter; }
    /** Set or clear the maskfilter object.
        <p />
        Pass NULL to clear any previous maskfilter.
        As a convenience, the parameter passed is also returned.
        If a previous maskfilter exists, its reference count is decremented.
        If maskfilter is not NULL, its reference count is incremented.
        @param maskfilter   May be NULL. The new maskfilter to be installed in the paint
        @return             maskfilter
    */
    SkMaskFilter*   setMaskFilter(SkMaskFilter* maskfilter);

    // These attributes are for text/fonts

    /** Get the paint's typeface object.
        <p />
        The typeface object identifies which font to use when drawing or measuring text.
        The typeface reference count is not affected.
        @return the paint's typeface (or NULL)
    */
    SkTypeface* getTypeface() const { return fTypeface; }
    /** Set or clear the typeface object.
        <p />
        Pass NULL to clear any previous typeface.
        As a convenience, the parameter passed is also returned.
        If a previous typeface exists, its reference count is decremented.
        If typeface is not NULL, its reference count is incremented.
        @param typeface May be NULL. The new typeface to be installed in the paint
        @return         typeface
    */
    SkTypeface* setTypeface(SkTypeface* typeface);

    /** Get the paint's rasterizer (or NULL).
        <p />
        The raster controls/modifies how paths/text are turned into alpha masks.
        @return the paint's rasterizer (or NULL)
    */
    SkRasterizer* getRasterizer() const { return fRasterizer; }
    /** Set or clear the rasterizer object.
        <p />
        Pass NULL to clear any previous rasterizer.
        As a convenience, the parameter passed is also returned.
        If a previous rasterizer exists in the paint, its reference count is decremented.
        If r is not NULL, its reference count is incremented.
        @param rasterizer May be NULL. The new rasterizer to be installed in the paint.
        @return rasterizer
    */
    SkRasterizer* setRasterizer(SkRasterizer* rasterizer);

    enum Align {
        kLeft_Align,
        kCenter_Align,
        kRight_Align,

        kAlignCount
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
    SkScalar    getTextSize() const { return fTextSize; }
    /** Set the paint's text size. This value must be > 0
        @param textSize set the paint's text size.
    */
    void        setTextSize(SkScalar textSize);

    /** Return the paint's horizontal scale factor for text. The default value
        is 1.0.
        @return the paint's scale factor in X for drawing/measuring text
    */
    SkScalar    getTextScaleX() const { return fTextScaleX; }
    /** Set the paint's horizontal scale factor for text. The default value
        is 1.0. Values > 1.0 will stretch the text wider. Values < 1.0 will
        stretch the text narrower.
        @param scaleX   set the paint's scale factor in X for drawing/measuring text.
    */
    void        setTextScaleX(SkScalar scaleX);

    /** Return the paint's horizontal skew factor for text. The default value
        is 0.
        @return the paint's skew factor in X for drawing text.
    */
    SkScalar    getTextSkewX() const { return fTextSkewX; }
    /** Set the paint's horizontal skew factor for text. The default value
        is 0. For approximating oblique text, use values around -0.25.
        @param skewX set the paint's skew factor in X for drawing text.
    */
    void        setTextSkewX(SkScalar skewX);

    enum TextEncoding {
        kUTF8_TextEncoding,
        kUTF16_TextEncoding,
        kGlyphID_TextEncoding
    };
    
    TextEncoding    getTextEncoding() const { return (TextEncoding)fTextEncoding; }
    void            setTextEncoding(TextEncoding encoding);

    /** Convert the specified text into glyph IDs, returning the number of glyphs ID written.
        If glyphs is NULL, it is ignore and only the count is returned.
    */
    int textToGlyphs(const void* text, size_t byteLength, uint16_t glyphs[]) const;

    int countText(const void* text, size_t byteLength) const
    {
        return this->textToGlyphs(text, byteLength, NULL);
    }

    /** Return the distance above (negative) the baseline (ascent) based on the current typeface and text size.
        @return the distance above (negative) the baseline (ascent) based on the current typeface and text size.
    */
    SkScalar ascent() const;
    /** Return the distance below (positive) the baseline (descent) based on the current typeface and text size.
        @return the distance below (positive) the baseline (descent) based on the current typeface and text size.
    */
    SkScalar descent() const;
    
    /** Return the width of the text.
        @param text         Address of the text
        @param byteLength   Number of bytes of text to measure
        @param above        If not NULL, returns the distance above the baseline (ascent)
        @param below        If not NULL, returns the distance below the baseline (descent)
        @return The width of the text
    */
    SkScalar    measureText(const void* text, size_t byteLength,
                            SkScalar* above, SkScalar* below) const;

    /** Return the width of the text.
        @param text         Address of the text
        @param byteLength   Number of bytes of text to measure
        @return The width of the text
    */
    SkScalar measureText(const void* text, size_t byteLength) const
    {
        return this->measureText(text, byteLength, NULL, NULL);
    }
    
    /** Return the advance widths for the characters in the string.
        @param text UTF8 text
        @param byteLength   number of bytes to read from the UTF8 text parameter
        @param widths   array of SkScalars to receive the advance widths of the characters.
                        May be NULL. If not NULL, must be at least a large as the number
                        of unichars in the specified text.
        @return the number of unichars in the specified text.
    */
    int getTextWidths(const void* text, size_t byteLength, SkScalar widths[]) const;

    /** Return the path (outline) for the specified text.
        Note: just like SkCanvas::drawText, this will respect the Align setting in the paint.
    */
    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y, SkPath* path) const;

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

    SkColor         fColor;
    SkScalar        fWidth;
    SkScalar        fMiterLimit;
    unsigned        fFlags : kFlagShiftCount;
    unsigned        fFilterType : 1;
    unsigned        fTextAlign : 2;
    unsigned        fCapType : 2;
    unsigned        fJoinType : 2;
    unsigned        fStyle : 2;
    unsigned        fTextEncoding : 2;  // 3 values

    SkGlyphCacheProc getGlyphCacheProc() const;

    SkScalar measure_text(SkGlyphCache*, const char* text, size_t length, int* count) const;

    SkGlyphCache*   detachCache(const SkMatrix*) const;

    enum {
        kCanonicalTextSizeForPaths = 64
    };
    friend class SkCanvas;
    friend class SkDraw;
    friend class SkGlyphCache;
    friend class SkTextToPathIter;
};

class SkAutoRestorePaintFlags {
public:
    SkAutoRestorePaintFlags(const SkPaint& paint, uint32_t newFlags)
    {
        SkASSERT(&paint);
        fPaint = (SkPaint*)&paint;  // remove constness
        fOldFlags = paint.getFlags();
        fPaint->setFlags(newFlags);
    }
    ~SkAutoRestorePaintFlags()
    {
        fPaint->setFlags(fOldFlags);
    }
private:
    SkPaint* fPaint;
    uint32_t fOldFlags;
};

//////////////////////////////////////////////////////////////////////////

#include "SkPathEffect.h"

/** \class SkStrokePathEffect

    SkStrokePathEffect simulates stroking inside a patheffect, allowing the caller to have explicit
    control of when to stroke a path. Typically this is used if the caller wants to stroke before
    another patheffect is applied (using SkComposePathEffect or SkSumPathEffect).
*/
class SkStrokePathEffect : public SkPathEffect {
public:
    SkStrokePathEffect(const SkPaint&);
    SkStrokePathEffect(SkScalar width, SkPaint::Style, SkPaint::Join, SkPaint::Cap, SkScalar miterLimit = -1);

    // overrides
    // This method is not exported to java.
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

    // overrides for SkFlattenable
    // This method is not exported to java.
    virtual void flatten(SkWBuffer&);
    // This method is not exported to java.
    virtual Factory getFactory();

private:
    SkScalar    fWidth, fMiter;
    uint8_t     fStyle, fJoin, fCap;

    static SkFlattenable* CreateProc(SkRBuffer&);
    SkStrokePathEffect(SkRBuffer&);

    typedef SkPathEffect INHERITED;

    // illegal
    SkStrokePathEffect(const SkStrokePathEffect&);
    SkStrokePathEffect& operator=(const SkStrokePathEffect&);
};

#endif

