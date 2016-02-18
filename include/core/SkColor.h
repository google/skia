/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColor_DEFINED
#define SkColor_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

/** \file SkColor.h

    Types and macros for colors
*/

/** 8-bit type for an alpha value. 0xFF is 100% opaque, 0x00 is 100% transparent.
*/
typedef uint8_t SkAlpha;
/** 32 bit ARGB color value, not premultiplied. The color components are always in
    a known order. This is different from SkPMColor, which has its bytes in a configuration
    dependent order, to match the format of kARGB32 bitmaps. SkColor is the type used to
    specify colors in SkPaint and in gradients.
*/
typedef uint32_t SkColor;

/** Return a SkColor value from 8 bit component values
*/
static inline SkColor SkColorSetARGBInline(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
    SkASSERT(a <= 255 && r <= 255 && g <= 255 && b <= 255);

    return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

#define SkColorSetARGBMacro(a, r, g, b) \
    static_cast<SkColor>( \
        (static_cast<U8CPU>(a) << 24) | \
        (static_cast<U8CPU>(r) << 16) | \
        (static_cast<U8CPU>(g) << 8) | \
        (static_cast<U8CPU>(b) << 0))

/** gcc will generate static initializers for code of this form:
 * static const SkColor kMyColor = SkColorSetARGB(0xFF, 0x01, 0x02, 0x03)
 * if SkColorSetARGB() is a static inline, but not if it's a macro.
 */
#if defined(NDEBUG)
#define SkColorSetARGB(a, r, g, b) SkColorSetARGBMacro(a, r, g, b)
#else
#define SkColorSetARGB(a, r, g, b) SkColorSetARGBInline(a, r, g, b)
#endif

/** Return a SkColor value from 8 bit component values, with an implied value
    of 0xFF for alpha (fully opaque)
*/
#define SkColorSetRGB(r, g, b)  SkColorSetARGB(0xFF, r, g, b)

/** return the alpha byte from a SkColor value */
#define SkColorGetA(color)      (((color) >> 24) & 0xFF)
/** return the red byte from a SkColor value */
#define SkColorGetR(color)      (((color) >> 16) & 0xFF)
/** return the green byte from a SkColor value */
#define SkColorGetG(color)      (((color) >>  8) & 0xFF)
/** return the blue byte from a SkColor value */
#define SkColorGetB(color)      (((color) >>  0) & 0xFF)

static inline SkColor SkColorSetA(SkColor c, U8CPU a) {
    return (c & 0x00FFFFFF) | (a << 24);
}

// common colors

#define SK_AlphaTRANSPARENT 0x00        //!< transparent SkAlpha value
#define SK_AlphaOPAQUE      0xFF        //!< opaque SkAlpha value

#define SK_ColorTRANSPARENT 0x00000000  //!< transparent SkColor value

#define SK_ColorBLACK       0xFF000000  //!< black SkColor value
#define SK_ColorDKGRAY      0xFF444444  //!< dark gray SkColor value
#define SK_ColorGRAY        0xFF888888  //!< gray SkColor value
#define SK_ColorLTGRAY      0xFFCCCCCC  //!< light gray SkColor value
#define SK_ColorWHITE       0xFFFFFFFF  //!< white SkColor value

#define SK_ColorRED         0xFFFF0000  //!< red SkColor value
#define SK_ColorGREEN       0xFF00FF00  //!< green SkColor value
#define SK_ColorBLUE        0xFF0000FF  //!< blue SkColor value
#define SK_ColorYELLOW      0xFFFFFF00  //!< yellow SkColor value
#define SK_ColorCYAN        0xFF00FFFF  //!< cyan SkColor value
#define SK_ColorMAGENTA     0xFFFF00FF  //!< magenta SkColor value

////////////////////////////////////////////////////////////////////////

/** Convert RGB components to HSV.
        hsv[0] is Hue [0 .. 360)
        hsv[1] is Saturation [0...1]
        hsv[2] is Value [0...1]
    @param red  red component value [0..255]
    @param green  green component value [0..255]
    @param blue  blue component value [0..255]
    @param hsv  3 element array which holds the resulting HSV components.
*/
SK_API void SkRGBToHSV(U8CPU red, U8CPU green, U8CPU blue, SkScalar hsv[3]);

/** Convert the argb color to its HSV components.
        hsv[0] is Hue [0 .. 360)
        hsv[1] is Saturation [0...1]
        hsv[2] is Value [0...1]
    @param color the argb color to convert. Note: the alpha component is ignored.
    @param hsv  3 element array which holds the resulting HSV components.
*/
static inline void SkColorToHSV(SkColor color, SkScalar hsv[3]) {
    SkRGBToHSV(SkColorGetR(color), SkColorGetG(color), SkColorGetB(color), hsv);
}

/** Convert HSV components to an ARGB color. The alpha component is passed through unchanged.
        hsv[0] is Hue [0 .. 360)
        hsv[1] is Saturation [0...1]
        hsv[2] is Value [0...1]
    If hsv values are out of range, they are pinned.
    @param alpha the alpha component of the returned argb color.
    @param hsv  3 element array which holds the input HSV components.
    @return the resulting argb color
*/
SK_API SkColor SkHSVToColor(U8CPU alpha, const SkScalar hsv[3]);

/** Convert HSV components to an ARGB color. The alpha component set to 0xFF.
        hsv[0] is Hue [0 .. 360)
        hsv[1] is Saturation [0...1]
        hsv[2] is Value [0...1]
    If hsv values are out of range, they are pinned.
    @param hsv  3 element array which holds the input HSV components.
    @return the resulting argb color
*/
static inline SkColor SkHSVToColor(const SkScalar hsv[3]) {
    return SkHSVToColor(0xFF, hsv);
}

////////////////////////////////////////////////////////////////////////

/** 32 bit ARGB color value, premultiplied. The byte order for this value is
    configuration dependent, matching the format of kARGB32 bitmaps. This is different
    from SkColor, which is nonpremultiplied, and is always in the same byte order.
*/
typedef uint32_t SkPMColor;

/** Return a SkPMColor value from unpremultiplied 8 bit component values
*/
SK_API SkPMColor SkPreMultiplyARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);
/** Return a SkPMColor value from a SkColor value. This is done by multiplying the color
    components by the color's alpha, and by arranging the bytes in a configuration
    dependent order, to match the format of kARGB32 bitmaps.
*/
SK_API SkPMColor SkPreMultiplyColor(SkColor c);

/** Define a function pointer type for combining two premultiplied colors
*/
typedef SkPMColor (*SkXfermodeProc)(SkPMColor src, SkPMColor dst);

///////////////////////////////////////////////////////////////////////////////////////////////////

struct SkPM4f;

/*
 *  The float values are 0...1 unpremultiplied
 */
struct SkColor4f {
    float fA;
    float fR;
    float fG;
    float fB;

    bool operator==(const SkColor4f& other) const {
        return fA == other.fA && fR == other.fR && fG == other.fG && fB == other.fB;
    }
    bool operator!=(const SkColor4f& other) const {
        return !(*this == other);
    }

    const float* vec() const { return &fA; }
    float* vec() { return &fA; }

    static SkColor4f Pin(float a, float r, float g, float b);
    static SkColor4f FromColor(SkColor);

    SkColor4f pin() const {
        return Pin(fA, fR, fG, fB);
    }

    SkPM4f premul() const;
};

#endif
