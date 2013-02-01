
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "SkGradientShader.h"
#include "SkClampRange.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkMallocPixelRef.h"
#include "SkUnitMapper.h"
#include "SkUtils.h"
#include "SkTemplates.h"
#include "SkBitmapCache.h"
#include "SkShader.h"

#ifndef SK_DISABLE_DITHER_32BIT_GRADIENT
    #define USE_DITHER_32BIT_GRADIENT
#endif

static inline void sk_memset32_dither(uint32_t dst[], uint32_t v0, uint32_t v1,
                               int count) {
    if (count > 0) {
        if (v0 == v1) {
            sk_memset32(dst, v0, count);
        } else {
            int pairs = count >> 1;
            for (int i = 0; i < pairs; i++) {
                *dst++ = v0;
                *dst++ = v1;
            }
            if (count & 1) {
                *dst = v0;
            }
        }
    }
}

//  Clamp

static inline SkFixed clamp_tileproc(SkFixed x) {
    return SkClampMax(x, 0xFFFF);
}

// Repeat

static inline SkFixed repeat_tileproc(SkFixed x) {
    return x & 0xFFFF;
}

// Mirror

// Visual Studio 2010 (MSC_VER=1600) optimizes bit-shift code incorrectly.
// See http://code.google.com/p/skia/issues/detail?id=472
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", off)
#endif

static inline SkFixed mirror_tileproc(SkFixed x) {
    int s = x << 15 >> 31;
    return (x ^ s) & 0xFFFF;
}

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", on)
#endif

///////////////////////////////////////////////////////////////////////////////

typedef SkFixed (*TileProc)(SkFixed);

///////////////////////////////////////////////////////////////////////////////

static const TileProc gTileProcs[] = {
    clamp_tileproc,
    repeat_tileproc,
    mirror_tileproc
};

///////////////////////////////////////////////////////////////////////////////

class SkGradientShaderBase : public SkShader {
public:
    SkGradientShaderBase(const SkColor colors[], const SkScalar pos[],
                int colorCount, SkShader::TileMode mode, SkUnitMapper* mapper);
    virtual ~SkGradientShaderBase();

    virtual bool setContext(const SkBitmap&, const SkPaint&, const SkMatrix&) SK_OVERRIDE;
    virtual uint32_t getFlags() SK_OVERRIDE { return fFlags; }
    virtual bool isOpaque() const SK_OVERRIDE;

    void getGradientTableBitmap(SkBitmap*) const;

    enum {
        /// Seems like enough for visual accuracy. TODO: if pos[] deserves
        /// it, use a larger cache.
        kCache16Bits    = 8,
        kCache16Count = (1 << kCache16Bits),
        kCache16Shift   = 16 - kCache16Bits,
        kSqrt16Shift    = 8 - kCache16Bits,

        /// Seems like enough for visual accuracy. TODO: if pos[] deserves
        /// it, use a larger cache.
        kCache32Bits    = 8,
        kCache32Count = (1 << kCache32Bits),
        kCache32Shift   = 16 - kCache32Bits,
        kSqrt32Shift    = 8 - kCache32Bits,

        /// This value is used to *read* the dither cache; it may be 0
        /// if dithering is disabled.
#ifdef USE_DITHER_32BIT_GRADIENT
        kDitherStride32 = kCache32Count,
#else
        kDitherStride32 = 0,
#endif
        kDitherStride16 = kCache16Count,
    };


protected:
    SkGradientShaderBase(SkFlattenableReadBuffer& );
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    SK_DEVELOPER_TO_STRING()

    SkUnitMapper* fMapper;
    SkMatrix    fPtsToUnit;     // set by subclass
    SkMatrix    fDstToIndex;
    SkMatrix::MapXYProc fDstToIndexProc;
    TileMode    fTileMode;
    TileProc    fTileProc;
    int         fColorCount;
    uint8_t     fDstToIndexClass;
    uint8_t     fFlags;

    struct Rec {
        SkFixed     fPos;   // 0...1
        uint32_t    fScale; // (1 << 24) / range
    };
    Rec*        fRecs;

    const uint16_t*     getCache16() const;
    const SkPMColor*    getCache32() const;

    void commonAsAGradient(GradientInfo*) const;

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount * (sizeof(SkColor) + sizeof(Rec))
    };
    SkColor     fStorage[(kStorageSize + 3) >> 2];
    SkColor*    fOrigColors; // original colors, before modulation by paint in setContext
    bool        fColorsAreOpaque;

    mutable uint16_t*   fCache16;   // working ptr. If this is NULL, we need to recompute the cache values
    mutable SkPMColor*  fCache32;   // working ptr. If this is NULL, we need to recompute the cache values

    mutable uint16_t*   fCache16Storage;    // storage for fCache16, allocated on demand
    mutable SkMallocPixelRef* fCache32PixelRef;
    mutable unsigned    fCacheAlpha;        // the alpha value we used when we computed the cache. larger than 8bits so we can store uninitialized value

    static void Build16bitCache(uint16_t[], SkColor c0, SkColor c1, int count);
    static void Build32bitCache(SkPMColor[], SkColor c0, SkColor c1, int count,
                                U8CPU alpha);
    void setCacheAlpha(U8CPU alpha) const;
    void initCommon();

    typedef SkShader INHERITED;
};

static inline int init_dither_toggle(int x, int y) {
    return ((x ^ y) & 1) * SkGradientShaderBase::kDitherStride32;
}

static inline int next_dither_toggle(int toggle) {
    return toggle ^ SkGradientShaderBase::kDitherStride32;
}

static inline int init_dither_toggle16(int x, int y) {
    return ((x ^ y) & 1) * SkGradientShaderBase::kDitherStride16;
}

static inline int next_dither_toggle16(int toggle) {
    return toggle ^ SkGradientShaderBase::kDitherStride16;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"

class GrEffectStage;
class GrBackendEffectFactory;

/*
 * The interpretation of the texture matrix depends on the sample mode. The
 * texture matrix is applied both when the texture coordinates are explicit
 * and  when vertex positions are used as texture  coordinates. In the latter
 * case the texture matrix is applied to the pre-view-matrix position
 * values.
 *
 * Normal SampleMode
 *  The post-matrix texture coordinates are in normalize space with (0,0) at
 *  the top-left and (1,1) at the bottom right.
 * RadialGradient
 *  The matrix specifies the radial gradient parameters.
 *  (0,0) in the post-matrix space is center of the radial gradient.
 * Radial2Gradient
 *   Matrix transforms to space where first circle is centered at the
 *   origin. The second circle will be centered (x, 0) where x may be
 *   0 and is provided by setRadial2Params. The post-matrix space is
 *   normalized such that 1 is the second radius - first radius.
 * SweepGradient
 *  The angle from the origin of texture coordinates in post-matrix space
 *  determines the gradient value.
 */

 class GrTextureStripAtlas;

// Base class for Gr gradient effects
class GrGradientEffect : public GrEffect {
public:

    GrGradientEffect(GrContext* ctx,
                     const SkGradientShaderBase& shader,
                     const SkMatrix& matrix,
                     SkShader::TileMode tileMode);

    virtual ~GrGradientEffect();

    bool useAtlas() const { return SkToBool(-1 != fRow); }
    SkScalar getYCoord() const { return fYCoord; };
    const SkMatrix& getMatrix() const { return fMatrix;}

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

protected:

    /** Populates a pair of arrays with colors and stop info to construct a random gradient.
        The function decides whether stop values should be used or not. The return value indicates
        the number of colors, which will be capped by kMaxRandomGradientColors. colors should be
        sized to be at least kMaxRandomGradientColors. stops is a pointer to an array of at least
        size kMaxRandomGradientColors. It may be updated to NULL, indicating that NULL should be
        passed to the gradient factory rather than the array.
    */
    static const int kMaxRandomGradientColors = 4;
    static int RandomGradientParams(SkRandom* r,
                                    SkColor colors[kMaxRandomGradientColors],
                                    SkScalar** stops,
                                    SkShader::TileMode* tm);

    virtual bool onIsEqual(const GrEffect& effect) const SK_OVERRIDE;

private:

    GrTextureAccess fTextureAccess;
    SkScalar fYCoord;
    GrTextureStripAtlas* fAtlas;
    int fRow;
    SkMatrix fMatrix;
    bool fIsOpaque;

    typedef GrEffect INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

// Base class for GL gradient effects
class GrGLGradientEffect : public GrGLEffect {
public:
    GrGLGradientEffect(const GrBackendEffectFactory& factory);
    virtual ~GrGLGradientEffect();

    virtual void setData(const GrGLUniformManager&, const GrEffectStage&) SK_OVERRIDE;

protected:
    /**
     * Subclasses must reserve the lower kMatrixKeyBitCnt of their key for use by
     * GrGLGradientEffect.
     */
    enum {
        kMatrixKeyBitCnt = GrGLEffectMatrix::kKeyBits,
        kMatrixKeyMask = (1 << kMatrixKeyBitCnt) - 1,
    };

    /**
     * Subclasses must call this. It will return a value restricted to the lower kMatrixKeyBitCnt
     * bits.
     */
    static EffectKey GenMatrixKey(const GrEffectStage& s);

    /**
     * Inserts code to implement the GrGradientEffect's matrix. This should be called before a
     * subclass emits its own code. The name of the 2D coords is output via fsCoordName and already
     * incorporates any perspective division. The caller can also optionally retrieve the name of
     * the varying inserted in the VS and its type, which may be either vec2f or vec3f depending
     * upon whether the matrix has perspective or not. It is not necessary to mask the key before
     * calling.
     */
    void setupMatrix(GrGLShaderBuilder* builder,
                     EffectKey key,
                     const char* vertexCoords,
                     const char** fsCoordName,
                     const char** vsVaryingName = NULL,
                     GrSLType* vsVaryingType = NULL);

    // Emits the uniform used as the y-coord to texture samples in derived classes. Subclasses
    // should call this method from their emitCode().
    void emitYCoordUniform(GrGLShaderBuilder* builder);

    // emit code that gets a fragment's color from an expression for t; for now this always uses the
    // texture, but for simpler cases we'll be able to lerp. Subclasses should call this method from
    // their emitCode().
    void emitColorLookup(GrGLShaderBuilder* builder,
                         const char* gradientTValue,
                         const char* outputColor,
                         const char* inputColor,
                         const GrGLShaderBuilder::TextureSampler&);

private:
    SkScalar fCachedYCoord;
    GrGLUniformManager::UniformHandle fFSYUni;
    GrGLEffectMatrix fEffectMatrix;

    typedef GrGLEffect INHERITED;
};

#endif

#endif
