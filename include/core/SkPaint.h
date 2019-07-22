/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaint_DEFINED
#define SkPaint_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTo.h"

class SkColorFilter;
class SkColorSpace;
class SkDrawLooper;
struct SkRect;
class SkImageFilter;
class SkMaskFilter;
class SkPath;
class SkPathEffect;
class SkShader;

/** \class SkPaint
    SkPaint controls options applied when drawing. SkPaint collects all
    options outside of the SkCanvas clip and SkCanvas matrix.

    Various options apply to strokes and fills, and images.

    SkPaint collects effects and filters that describe single-pass and multiple-pass
    algorithms that alter the drawing geometry, color, and transparency. For instance,
    SkPaint does not directly implement dashing or blur, but contains the objects that do so.
*/
class SK_API SkPaint {
public:

    /** Constructs SkPaint with default values.

        @return  default initialized SkPaint
    */
    SkPaint();

    /** Constructs SkPaint with default values and the given color.

        Sets alpha and RGB used when stroking and filling. The color is four floating
        point values, unpremultiplied. The color values are interpreted as being in
        the colorSpace. If colorSpace is nullptr, then color is assumed to be in the
        sRGB color space.

        @param color       unpremultiplied RGBA
        @param colorSpace  SkColorSpace describing the encoding of color
        @return            SkPaint with the given color
    */
    explicit SkPaint(const SkColor4f& color, SkColorSpace* colorSpace = nullptr);

    /** Makes a shallow copy of SkPaint. SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkDrawLooper, and SkImageFilter are shared
        between the original paint and the copy. Objects containing SkRefCnt increment
        their references by one.

        The referenced objects SkPathEffect, SkShader, SkMaskFilter, SkColorFilter,
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

    /** Decreases SkPaint SkRefCnt of owned objects: SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkDrawLooper, and SkImageFilter. If the
        objects containing SkRefCnt go to zero, they are deleted.
    */
    ~SkPaint();

    /** Makes a shallow copy of SkPaint. SkPathEffect, SkShader,
        SkMaskFilter, SkColorFilter, SkDrawLooper, and SkImageFilter are shared
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
        if SkPathEffect, SkShader, SkMaskFilter, SkColorFilter,
        SkDrawLooper, or SkImageFilter have identical contents but different pointers.

        @param a  SkPaint to compare
        @param b  SkPaint to compare
        @return   true if SkPaint pair are equivalent
    */
    SK_API friend bool operator==(const SkPaint& a, const SkPaint& b);

    /** Compares a and b, and returns true if a and b are not equivalent. May return true
        if SkPathEffect, SkShader, SkMaskFilter, SkColorFilter,
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

    /** Sets all SkPaint contents to their initial values. This is equivalent to replacing
        SkPaint with the result of SkPaint().
    */
    void reset();

    /** Returns true if pixels on the active edges of SkPath may be drawn with partial transparency.
        @return  antialiasing state
    */
    bool isAntiAlias() const {
        return SkToBool(fBitfields.fAntiAlias);
    }

    /** Requests, but does not require, that edge pixels draw opaque or with
        partial transparency.
        @param aa  setting for antialiasing
    */
    void setAntiAlias(bool aa) { fBitfields.fAntiAlias = static_cast<unsigned>(aa); }

    /** Returns true if color error may be distributed to smooth color transition.
        @return  dithering state
    */
    bool isDither() const {
        return SkToBool(fBitfields.fDither);
    }

    /** Requests, but does not require, to distribute color error.
        @param dither  setting for ditering
    */
    void setDither(bool dither) { fBitfields.fDither = static_cast<unsigned>(dither); }

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
    enum Style : uint8_t {
        kFill_Style,          //!< set to fill geometry
        kStroke_Style,        //!< set to stroke geometry
        kStrokeAndFill_Style, //!< sets to stroke and fill geometry
    };

    /** May be used to verify that SkPaint::Style is a legal value.
    */
    static constexpr int kStyleCount = kStrokeAndFill_Style + 1;

    /** Returns whether the geometry is filled, stroked, or filled and stroked.

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
    SkColor getColor() const { return fColor4f.toSkColor(); }

    /** Retrieves alpha and RGB, unpremultiplied, as four floating point values. RGB are
        are extended sRGB values (sRGB gamut, and encoded with the sRGB transfer function).

        @return  unpremultiplied RGBA
    */
    SkColor4f getColor4f() const { return fColor4f; }

    /** Sets alpha and RGB used when stroking and filling. The color is a 32-bit value,
        unpremultiplied, packing 8-bit components for alpha, red, blue, and green.

        @param color  unpremultiplied ARGB
    */
    void setColor(SkColor color);

    /** Sets alpha and RGB used when stroking and filling. The color is four floating
        point values, unpremultiplied. The color values are interpreted as being in
        the colorSpace. If colorSpace is nullptr, then color is assumed to be in the
        sRGB color space.

        @param color       unpremultiplied RGBA
        @param colorSpace  SkColorSpace describing the encoding of color
    */
    void setColor(const SkColor4f& color, SkColorSpace* colorSpace = nullptr);

    void setColor4f(const SkColor4f& color, SkColorSpace* colorSpace = nullptr) {
        this->setColor(color, colorSpace);
    }

    /** Retrieves alpha from the color used when stroking and filling.

        @return  alpha ranging from zero, fully transparent, to 255, fully opaque
    */
    float getAlphaf() const { return fColor4f.fA; }

    // Helper that scales the alpha by 255.
    uint8_t getAlpha() const { return sk_float_round2int(this->getAlphaf() * 255); }

    /** Replaces alpha, leaving RGB
        unchanged. An out of range value triggers an assert in the debug
        build. a is a value from 0.0 to 1.0.
        a set to zero makes color fully transparent; a set to 1.0 makes color
        fully opaque.

        @param a  alpha component of color
    */
    void setAlphaf(float a);

    // Helper that accepts an int between 0 and 255, and divides it by 255.0
    void setAlpha(U8CPU a) {
        this->setAlphaf(a * (1.0f / 255));
    }

    /** Sets color used when drawing solid fills. The color components range from 0 to 255.
        The color is unpremultiplied; alpha sets the transparency independent of RGB.

        @param a  amount of alpha, from fully transparent (0) to fully opaque (255)
        @param r  amount of red, from no red (0) to full red (255)
        @param g  amount of green, from no green (0) to full green (255)
        @param b  amount of blue, from no blue (0) to full blue (255)
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

    /** Returns the limit at which a sharp corner is drawn beveled.

        @return  zero and greater miter limit
    */
    SkScalar getStrokeMiter() const { return fMiterLimit; }

    /** Sets the limit at which a sharp corner is drawn beveled.
        Valid values are zero and greater.
        Has no effect if miter is less than zero.

        @param miter  zero and greater miter limit
    */
    void setStrokeMiter(SkScalar miter);

    /** \enum SkPaint::Cap
        Cap draws at the beginning and end of an open path contour.
    */
    enum Cap {
        kButt_Cap,                  //!< no stroke extension
        kRound_Cap,                 //!< adds circle
        kSquare_Cap,                //!< adds square
        kLast_Cap    = kSquare_Cap, //!< largest Cap value
        kDefault_Cap = kButt_Cap,   //!< equivalent to kButt_Cap
    };

    /** May be used to verify that SkPaint::Cap is a legal value.
    */
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
    enum Join : uint8_t {
        kMiter_Join,                 //!< extends to miter limit
        kRound_Join,                 //!< adds circle
        kBevel_Join,                 //!< connects outside edges
        kLast_Join    = kBevel_Join, //!< equivalent to the largest value for Join
        kDefault_Join = kMiter_Join, //!< equivalent to kMiter_Join
    };

    /** May be used to verify that SkPaint::Join is a legal value.
    */
    static constexpr int kJoinCount = kLast_Join + 1;

    /** Returns the geometry drawn at the beginning and end of strokes.

        @return  one of: kButt_Cap, kRound_Cap, kSquare_Cap
    */
    Cap getStrokeCap() const { return (Cap)fBitfields.fCapType; }

    /** Sets the geometry drawn at the beginning and end of strokes.

        @param cap  one of: kButt_Cap, kRound_Cap, kSquare_Cap;
                    has no effect if cap is not valid
    */
    void setStrokeCap(Cap cap);

    /** Returns the geometry drawn at the corners of strokes.

        @return  one of: kMiter_Join, kRound_Join, kBevel_Join
    */
    Join getStrokeJoin() const { return (Join)fBitfields.fJoinType; }

    /** Sets the geometry drawn at the corners of strokes.

        @param join  one of: kMiter_Join, kRound_Join, kBevel_Join;
                     otherwise, has no effect
    */
    void setStrokeJoin(Join join);

    /** Returns the filled equivalent of the stroked path.

        @param src       SkPath read to create a filled version
        @param dst       resulting SkPath; may be the same as src, but may not be nullptr
        @param cullRect  optional limit passed to SkPathEffect
        @param resScale  if > 1, increase precision, else if (0 < resScale < 1) reduce precision
                         to favor speed and size
        @return          true if the path represents style fill, or false if it represents hairline
    */
    bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                     SkScalar resScale = 1) const;

    /** Returns the filled equivalent of the stroked path.

        Replaces dst with the src path modified by SkPathEffect and style stroke.
        SkPathEffect, if any, is not culled. stroke width is created with default precision.

        @param src  SkPath read to create a filled version
        @param dst  resulting SkPath dst may be the same as src, but may not be nullptr
        @return     true if the path represents style fill, or false if it represents hairline
    */
    bool getFillPath(const SkPath& src, SkPath* dst) const {
        return this->getFillPath(src, dst, nullptr, 1);
    }

    /** Returns optional colors used when filling a path, such as a gradient.

        Does not alter SkShader SkRefCnt.

        @return  SkShader if previously set, nullptr otherwise
    */
    SkShader* getShader() const { return fShader.get(); }

    /** Returns optional colors used when filling a path, such as a gradient.

        Increases SkShader SkRefCnt by one.

        @return  SkShader if previously set, nullptr otherwise
    */
    sk_sp<SkShader> refShader() const;

    /** Sets optional colors used when filling a path, such as a gradient.

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
    SkBlendMode getBlendMode() const { return (SkBlendMode)fBitfields.fBlendMode; }

    /** Returns true if SkBlendMode is SkBlendMode::kSrcOver, the default.

        @return  true if SkBlendMode is SkBlendMode::kSrcOver
    */
    bool isSrcOver() const { return (SkBlendMode)fBitfields.fBlendMode == SkBlendMode::kSrcOver; }

    /** Sets SkBlendMode to mode.
        Does not check for valid input.

        @param mode  SkBlendMode used to combine source color and destination
    */
    void setBlendMode(SkBlendMode mode) { fBitfields.fBlendMode = (unsigned)mode; }

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

        Increments maskFilter SkRefCnt by one.

        @param maskFilter  modifies clipping mask generated from drawn geometry
    */
    void setMaskFilter(sk_sp<SkMaskFilter> maskFilter);

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

        Increments imageFilter SkRefCnt by one.

        @param imageFilter  how SkImage is sampled when transformed
    */
    void setImageFilter(sk_sp<SkImageFilter> imageFilter);

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
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
        (see skbug.com/6259)
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
        (see skbug.com/6259)
    */
    void setLooper(sk_sp<SkDrawLooper> drawLooper);
#endif

    /** Returns true if SkPaint prevents all drawing;
        otherwise, the SkPaint may or may not allow drawing.

        Returns true if, for example, SkBlendMode combined with alpha computes a
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
            For example:
            if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
                SkRect storage;
                if (canvas->quickReject(paint.computeFastBounds(path.getBounds(), &storage))) {
                    return; // do not draw the path
                }
            }
            // draw the path

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
            uintptr_t effects = 0;
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
            effects |= reinterpret_cast<uintptr_t>(this->getLooper());
#endif
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

private:
    sk_sp<SkPathEffect>   fPathEffect;
    sk_sp<SkShader>       fShader;
    sk_sp<SkMaskFilter>   fMaskFilter;
    sk_sp<SkColorFilter>  fColorFilter;
    sk_sp<SkDrawLooper>   fDrawLooper;
    sk_sp<SkImageFilter>  fImageFilter;

    SkColor4f       fColor4f;
    SkScalar        fWidth;
    SkScalar        fMiterLimit;
    union {
        struct {
            unsigned    fAntiAlias : 1;
            unsigned    fDither : 1;
            unsigned    fCapType : 2;
            unsigned    fJoinType : 2;
            unsigned    fStyle : 2;
            unsigned    fFilterQuality : 2;
            unsigned    fBlendMode : 8; // only need 5-6?
            unsigned    fPadding : 14;  // 14==32-1-1-2-2-2-2-8
        } fBitfields;
        uint32_t fBitfieldsUInt;
    };
};

#endif
