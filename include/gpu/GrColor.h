
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrColor_DEFINED
#define GrColor_DEFINED

#include "GrTypes.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"

/**
 * GrColor is 4 bytes for R, G, B, A, in a specific order defined below. Whether the color is
 * premultiplied or not depends on the context in which it is being used.
 */
typedef uint32_t GrColor;

// shift amount to assign a component to a GrColor int
// These shift values are chosen for compatibility with GL attrib arrays
// ES doesn't allow BGRA vertex attrib order so if they were not in this order
// we'd have to swizzle in shaders.
#ifdef SK_CPU_BENDIAN
    #define GrColor_SHIFT_R     24
    #define GrColor_SHIFT_G     16
    #define GrColor_SHIFT_B     8
    #define GrColor_SHIFT_A     0
#else
    #define GrColor_SHIFT_R     0
    #define GrColor_SHIFT_G     8
    #define GrColor_SHIFT_B     16
    #define GrColor_SHIFT_A     24
#endif

/**
 *  Pack 4 components (RGBA) into a GrColor int
 */
static inline GrColor GrColorPackRGBA(unsigned r, unsigned g, unsigned b, unsigned a) {
    SkASSERT((uint8_t)r == r);
    SkASSERT((uint8_t)g == g);
    SkASSERT((uint8_t)b == b);
    SkASSERT((uint8_t)a == a);
    return  (r << GrColor_SHIFT_R) |
            (g << GrColor_SHIFT_G) |
            (b << GrColor_SHIFT_B) |
            (a << GrColor_SHIFT_A);
}

/**
 *  Packs a color with an alpha channel replicated across all four channels.
 */
static inline GrColor GrColorPackA4(unsigned a) {
    SkASSERT((uint8_t)a == a);
    return  (a << GrColor_SHIFT_R) |
            (a << GrColor_SHIFT_G) |
            (a << GrColor_SHIFT_B) |
            (a << GrColor_SHIFT_A);
}

// extract a component (byte) from a GrColor int

#define GrColorUnpackR(color)   (((color) >> GrColor_SHIFT_R) & 0xFF)
#define GrColorUnpackG(color)   (((color) >> GrColor_SHIFT_G) & 0xFF)
#define GrColorUnpackB(color)   (((color) >> GrColor_SHIFT_B) & 0xFF)
#define GrColorUnpackA(color)   (((color) >> GrColor_SHIFT_A) & 0xFF)

/**
 *  Since premultiplied means that alpha >= color, we construct a color with
 *  each component==255 and alpha == 0 to be "illegal"
 */
#define GrColor_ILLEGAL     (~(0xFF << GrColor_SHIFT_A))

#define GrColor_WHITE 0xFFFFFFFF
#define GrColor_TRANSPARENT_BLACK 0x0

/**
 * Assert in debug builds that a GrColor is premultiplied.
 */
static inline void GrColorIsPMAssert(GrColor SkDEBUGCODE(c)) {
#ifdef SK_DEBUG
    unsigned a = GrColorUnpackA(c);
    unsigned r = GrColorUnpackR(c);
    unsigned g = GrColorUnpackG(c);
    unsigned b = GrColorUnpackB(c);

    SkASSERT(r <= a);
    SkASSERT(g <= a);
    SkASSERT(b <= a);
#endif
}

/** Inverts each color channel. */
static inline GrColor GrInvertColor(GrColor c) {
    U8CPU a = GrColorUnpackA(c);
    U8CPU r = GrColorUnpackR(c);
    U8CPU g = GrColorUnpackG(c);
    U8CPU b = GrColorUnpackB(c);
    return GrColorPackRGBA(0xff - r, 0xff - g, 0xff - b, 0xff - a);
}

static inline GrColor GrColorMul(GrColor c0, GrColor c1) {
    U8CPU r = SkMulDiv255Round(GrColorUnpackR(c0), GrColorUnpackR(c1));
    U8CPU g = SkMulDiv255Round(GrColorUnpackG(c0), GrColorUnpackG(c1));
    U8CPU b = SkMulDiv255Round(GrColorUnpackB(c0), GrColorUnpackB(c1));
    U8CPU a = SkMulDiv255Round(GrColorUnpackA(c0), GrColorUnpackA(c1));
    return GrColorPackRGBA(r, g, b, a);
}

static inline GrColor GrColorSatAdd(GrColor c0, GrColor c1) {
    unsigned r = SkTMin<unsigned>(GrColorUnpackR(c0) + GrColorUnpackR(c1), 0xff);
    unsigned g = SkTMin<unsigned>(GrColorUnpackG(c0) + GrColorUnpackG(c1), 0xff);
    unsigned b = SkTMin<unsigned>(GrColorUnpackB(c0) + GrColorUnpackB(c1), 0xff);
    unsigned a = SkTMin<unsigned>(GrColorUnpackA(c0) + GrColorUnpackA(c1), 0xff);
    return GrColorPackRGBA(r, g, b, a);
}

/** Converts a GrColor to an rgba array of GrGLfloat */
static inline void GrColorToRGBAFloat(GrColor color, float rgba[4]) {
    static const float ONE_OVER_255 = 1.f / 255.f;
    rgba[0] = GrColorUnpackR(color) * ONE_OVER_255;
    rgba[1] = GrColorUnpackG(color) * ONE_OVER_255;
    rgba[2] = GrColorUnpackB(color) * ONE_OVER_255;
    rgba[3] = GrColorUnpackA(color) * ONE_OVER_255;
}

/** Normalizes and coverts an uint8_t to a float. [0, 255] -> [0.0, 1.0] */
static inline float GrNormalizeByteToFloat(uint8_t value) {
    static const float ONE_OVER_255 = 1.f / 255.f;
    return value * ONE_OVER_255;
}

/** Determines whether the color is opaque or not. */
static inline bool GrColorIsOpaque(GrColor color) {
    return (color & (0xFFU << GrColor_SHIFT_A)) == (0xFFU << GrColor_SHIFT_A);
}

static inline GrColor GrPremulColor(GrColor color) {
    unsigned r = GrColorUnpackR(color);
    unsigned g = GrColorUnpackG(color);
    unsigned b = GrColorUnpackB(color);
    unsigned a = GrColorUnpackA(color);
    return GrColorPackRGBA(SkMulDiv255Round(r, a),
                           SkMulDiv255Round(g, a),
                           SkMulDiv255Round(b, a),
                           a);
}

/** Returns an unpremuled version of the GrColor. */
static inline GrColor GrUnpremulColor(GrColor color) {
    GrColorIsPMAssert(color);
    unsigned r = GrColorUnpackR(color);
    unsigned g = GrColorUnpackG(color); 
    unsigned b = GrColorUnpackB(color);
    unsigned a = GrColorUnpackA(color);
    SkPMColor colorPM = SkPackARGB32(a, r, g, b);
    SkColor colorUPM = SkUnPreMultiply::PMColorToColor(colorPM);

    r = SkColorGetR(colorUPM);
    g = SkColorGetG(colorUPM);
    b = SkColorGetB(colorUPM);
    a = SkColorGetA(colorUPM);

    return GrColorPackRGBA(r, g, b, a);
}


/**
* Similarly, GrColor4f is 4 floats for R, G, B, A, in that order. And like GrColor, whether
* the color is premultiplied or not depends on the context.
*/
struct GrColor4f {
    float fRGBA[4];

    GrColor4f() {}
    GrColor4f(float r, float g, float b, float a) {
        fRGBA[0] = r;
        fRGBA[1] = g;
        fRGBA[2] = b;
        fRGBA[3] = a;
    }

    enum Illegal_Constructor {
        kIllegalConstructor
    };
    GrColor4f(Illegal_Constructor) {
        fRGBA[0] = SK_FloatNaN;
        fRGBA[1] = SK_FloatNaN;
        fRGBA[2] = SK_FloatNaN;
        fRGBA[3] = SK_FloatNaN;
    }

    static GrColor4f OpaqueWhite() {
        return GrColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static GrColor4f TransparentBlack() {
        return GrColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    }

    static GrColor4f FromGrColor(GrColor color) {
        GrColor4f result;
        GrColorToRGBAFloat(color, result.fRGBA);
        return result;
    }

    static GrColor4f FromSkColor4f(const SkColor4f& color) {
        return GrColor4f(color.fR, color.fG, color.fB, color.fA);
    }

    GrColor4f modulate(const GrColor4f& x) const {
        return GrColor4f(fRGBA[0] * x.fRGBA[0],
                         fRGBA[1] * x.fRGBA[1],
                         fRGBA[2] * x.fRGBA[2],
                         fRGBA[3] * x.fRGBA[3]);
    }

    GrColor4f mulByScalar(float x) const {
        return GrColor4f(fRGBA[0] * x, fRGBA[1] * x, fRGBA[2] * x, fRGBA[3] * x);
    }

    bool operator==(const GrColor4f& other) const {
        return
            fRGBA[0] == other.fRGBA[0] &&
            fRGBA[1] == other.fRGBA[1] &&
            fRGBA[2] == other.fRGBA[2] &&
            fRGBA[3] == other.fRGBA[3];
    }
    bool operator!=(const GrColor4f& other) const {
        return !(*this == other);
    }

    GrColor toGrColor() const {
        return GrColorPackRGBA(
            SkTPin<unsigned>(static_cast<unsigned>(fRGBA[0] * 255.0f + 0.5f), 0, 255),
            SkTPin<unsigned>(static_cast<unsigned>(fRGBA[1] * 255.0f + 0.5f), 0, 255),
            SkTPin<unsigned>(static_cast<unsigned>(fRGBA[2] * 255.0f + 0.5f), 0, 255),
            SkTPin<unsigned>(static_cast<unsigned>(fRGBA[3] * 255.0f + 0.5f), 0, 255));
    }

    SkColor4f toSkColor4f() const {
        return SkColor4f { fRGBA[0], fRGBA[1], fRGBA[2], fRGBA[3] };
    }

    GrColor4f opaque() const {
        return GrColor4f(fRGBA[0], fRGBA[1], fRGBA[2], 1.0f);
    }

    bool isOpaque() const {
        return fRGBA[3] >= 1.f;  // just in case precision causes a superopaque value.
    }

    GrColor4f premul() const {
        float a = fRGBA[3];
        return GrColor4f(fRGBA[0] * a, fRGBA[1] * a, fRGBA[2] * a, a);
    }

    GrColor4f unpremul() const {
        float a = fRGBA[3];
        if (a <= 0.0f) {
            return GrColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        }
        float invAlpha = 1.0f / a;
        return GrColor4f(fRGBA[0] * invAlpha, fRGBA[1] * invAlpha, fRGBA[2] * invAlpha, a);
    }
};

/**
 * Flags used for bitfields of color components. They are defined so that the bit order reflects the
 * GrColor shift order.
 */
enum GrColorComponentFlags {
    kR_GrColorComponentFlag = 1 << (GrColor_SHIFT_R / 8),
    kG_GrColorComponentFlag = 1 << (GrColor_SHIFT_G / 8),
    kB_GrColorComponentFlag = 1 << (GrColor_SHIFT_B / 8),
    kA_GrColorComponentFlag = 1 << (GrColor_SHIFT_A / 8),

    kNone_GrColorComponentFlags = 0,

    kRGB_GrColorComponentFlags = (kR_GrColorComponentFlag | kG_GrColorComponentFlag |
                                  kB_GrColorComponentFlag),

    kRGBA_GrColorComponentFlags = (kR_GrColorComponentFlag | kG_GrColorComponentFlag |
                                   kB_GrColorComponentFlag | kA_GrColorComponentFlag)
};

GR_MAKE_BITFIELD_OPS(GrColorComponentFlags)

#endif
