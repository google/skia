/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "SkGradientBitmapCache.h"
#include "SkGradientShader.h"
#include "SkClampRange.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMallocPixelRef.h"
#include "SkUtils.h"
#include "SkTemplates.h"
#include "SkShader.h"
#include "SkOnce.h"

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
    struct Descriptor {
        Descriptor() {
            sk_bzero(this, sizeof(*this));
            fTileMode = SkShader::kClamp_TileMode;
        }

        const SkMatrix*     fLocalMatrix;
        const SkColor*      fColors;
        const SkScalar*     fPos;
        int                 fCount;
        SkShader::TileMode  fTileMode;
        uint32_t            fGradFlags;

        void flatten(SkWriteBuffer&) const;
    };

    class DescriptorScope : public Descriptor {
    public:
        DescriptorScope() {}
        
        bool unflatten(SkReadBuffer&);

        // fColors and fPos always point into local memory, so they can be safely mutated
        //
        SkColor* mutableColors() { return const_cast<SkColor*>(fColors); }
        SkScalar* mutablePos() { return const_cast<SkScalar*>(fPos); }

    private:
        enum {
            kStorageCount = 16
        };
        SkColor fColorStorage[kStorageCount];
        SkScalar fPosStorage[kStorageCount];
        SkMatrix fLocalMatrixStorage;
        SkAutoMalloc fDynamicStorage;
    };

public:
    SkGradientShaderBase(const Descriptor& desc);
    virtual ~SkGradientShaderBase();

    // The cache is initialized on-demand when getCache16/32 is called.
    class GradientShaderCache : public SkRefCnt {
    public:
        GradientShaderCache(U8CPU alpha, const SkGradientShaderBase& shader);
        ~GradientShaderCache();

        const uint16_t*     getCache16();
        const SkPMColor*    getCache32();

        SkMallocPixelRef* getCache32PixelRef() const { return fCache32PixelRef; }

        unsigned getAlpha() const { return fCacheAlpha; }

    private:
        // Working pointers. If either is NULL, we need to recompute the corresponding cache values.
        uint16_t*   fCache16;
        SkPMColor*  fCache32;

        uint16_t*         fCache16Storage;    // Storage for fCache16, allocated on demand.
        SkMallocPixelRef* fCache32PixelRef;
        const unsigned    fCacheAlpha;        // The alpha value we used when we computed the cache.
                                              // Larger than 8bits so we can store uninitialized
                                              // value.

        const SkGradientShaderBase& fShader;

        // Make sure we only initialize the caches once.
        bool    fCache16Inited, fCache32Inited;
        SkMutex fCache16Mutex, fCache32Mutex;

        static void initCache16(GradientShaderCache* cache);
        static void initCache32(GradientShaderCache* cache);

        static void Build16bitCache(uint16_t[], SkColor c0, SkColor c1, int count);
        static void Build32bitCache(SkPMColor[], SkColor c0, SkColor c1, int count,
                                    U8CPU alpha, uint32_t gradFlags);
    };

    class GradientShaderBaseContext : public SkShader::Context {
    public:
        GradientShaderBaseContext(const SkGradientShaderBase& shader, const ContextRec&);

        virtual uint32_t getFlags() const SK_OVERRIDE { return fFlags; }

    protected:
        SkMatrix    fDstToIndex;
        SkMatrix::MapXYProc fDstToIndexProc;
        uint8_t     fDstToIndexClass;
        uint8_t     fFlags;

        SkAutoTUnref<GradientShaderCache> fCache;

    private:
        typedef SkShader::Context INHERITED;
    };

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
        kCache32Count   = (1 << kCache32Bits),
        kCache32Shift   = 16 - kCache32Bits,
        kSqrt32Shift    = 8 - kCache32Bits,

        /// This value is used to *read* the dither cache; it may be 0
        /// if dithering is disabled.
        kDitherStride32 = kCache32Count,
        kDitherStride16 = kCache16Count,
    };

    enum GpuColorType {
        kTwo_GpuColorType,
        kThree_GpuColorType, // Symmetric three color
        kTexture_GpuColorType
    };

    // Determines and returns the gradient is a two color gradient, symmetric three color gradient
    // or other (texture gradient). If it is two or symmetric three color, the colors array will
    // also be filled with the gradient colors
    GpuColorType getGpuColorType(SkColor colors[3]) const;

    uint32_t getGradFlags() const { return fGradFlags; }

protected:
    SkGradientShaderBase(SkReadBuffer& );
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    SK_TO_STRING_OVERRIDE()

    SkMatrix    fPtsToUnit;     // set by subclass
    TileMode    fTileMode;
    TileProc    fTileProc;
    int         fColorCount;
    uint8_t     fGradFlags;

    struct Rec {
        SkFixed     fPos;   // 0...1
        uint32_t    fScale; // (1 << 24) / range
    };
    Rec*        fRecs;

    void commonAsAGradient(GradientInfo*, bool flipGrad = false) const;

    virtual bool onAsLuminanceColor(SkColor*) const SK_OVERRIDE;

    /*
     * Takes in pointers to gradient color and Rec info as colorSrc and recSrc respectively.
     * Count is the number of colors in the gradient
     * It will then flip all the color and rec information and return in their respective Dst
     * pointers. It is assumed that space has already been allocated for the Dst pointers.
     * The rec src and dst are only assumed to be valid if count > 2
     */
    static void FlipGradientColors(SkColor* colorDst, Rec* recDst,
                                   SkColor* colorSrc, Rec* recSrc,
                                   int count);

    // V23_COMPATIBILITY_CODE
    // Used for 2-pt conical gradients since we sort start/end cirlces by radius
    // Assumes space has already been allocated for fOrigColors
    void flipGradientColors();

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount * (sizeof(SkColor) + sizeof(SkScalar) + sizeof(Rec))
    };
    SkColor     fStorage[(kStorageSize + 3) >> 2];
    SkColor*    fOrigColors; // original colors, before modulation by paint in context.
    SkScalar*   fOrigPos;   // original positions
    bool        fColorsAreOpaque;

    GradientShaderCache* refCache(U8CPU alpha) const;
    mutable SkMutex                           fCacheMutex;
    mutable SkAutoTUnref<GradientShaderCache> fCache;

    void initCommon();

    typedef SkShader INHERITED;
};

static inline int init_dither_toggle(int x, int y) {
    x &= 1;
    y = (y & 1) << 1;
    return (x | y) * SkGradientShaderBase::kDitherStride32;
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

#include "GrCoordTransform.h"
#include "gl/GrGLProcessor.h"

class GrProcessorStage;
class GrBackendProcessorFactory;

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
class GrGradientEffect : public GrFragmentProcessor {
public:

    GrGradientEffect(GrContext* ctx,
                     const SkGradientShaderBase& shader,
                     const SkMatrix& matrix,
                     SkShader::TileMode tileMode);

    virtual ~GrGradientEffect();

    bool useAtlas() const { return SkToBool(-1 != fRow); }
    SkScalar getYCoord() const { return fYCoord; };

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    SkGradientShaderBase::GpuColorType getColorType() const { return fColorType; }

    enum PremulType {
        kBeforeInterp_PremulType,
        kAfterInterp_PremulType,
    };

    PremulType getPremulType() const { return fPremulType; }

    const SkColor* getColors(int pos) const {
        SkASSERT(fColorType != SkGradientShaderBase::kTexture_GpuColorType);
        SkASSERT((pos-1) <= fColorType);
        return &fColors[pos];
    }

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

    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

    const GrCoordTransform& getCoordTransform() const { return fCoordTransform; }

private:
    static const GrCoordSet kCoordSet = kLocal_GrCoordSet;

    GrCoordTransform fCoordTransform;
    GrTextureAccess fTextureAccess;
    SkScalar fYCoord;
    GrTextureStripAtlas* fAtlas;
    int fRow;
    bool fIsOpaque;
    SkGradientShaderBase::GpuColorType fColorType;
    SkColor fColors[3]; // More than 3 colors we use texture
    PremulType fPremulType; // This only changes behavior for two and three color special cases.
                            // It is already baked into to the table for texture gradients.
    typedef GrFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

// Base class for GL gradient effects
class GrGLGradientEffect : public GrGLFragmentProcessor {
public:
    GrGLGradientEffect(const GrBackendProcessorFactory& factory);
    virtual ~GrGLGradientEffect();

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

protected:
    /**
     * Subclasses must call this. It will return a key for the part of the shader code controlled
     * by the base class. The subclasses must stick it in their key and then pass it to the below
     * emit* functions from their emitCode function.
     */
    static uint32_t GenBaseGradientKey(const GrProcessor&);

    // Emits the uniform used as the y-coord to texture samples in derived classes. Subclasses
    // should call this method from their emitCode().
    void emitUniforms(GrGLProgramBuilder* builder, uint32_t baseKey);


    // emit code that gets a fragment's color from an expression for t; Has branches for 3 separate
    // control flows inside -- 2 color gradients, 3 color symmetric gradients (both using
    // native GLSL mix), and 4+ color gradients that use the traditional texture lookup.
    void emitColor(GrGLProgramBuilder* builder,
                   const char* gradientTValue,
                   uint32_t baseKey,
                   const char* outputColor,
                   const char* inputColor,
                   const TextureSamplerArray& samplers);

private:
    enum {
        kPremulTypeKeyBitCnt = 1,
        kPremulTypeMask = 1,
        kPremulBeforeInterpKey = kPremulTypeMask,

        kTwoColorKey = 2 << kPremulTypeKeyBitCnt,
        kThreeColorKey = 3 << kPremulTypeKeyBitCnt,
        kColorKeyMask = kTwoColorKey | kThreeColorKey,
        kColorKeyBitCnt = 2,

        // Subclasses must shift any key bits they produce up by this amount
        // and combine with the result of GenBaseGradientKey.
        kBaseKeyBitCnt = (kPremulTypeKeyBitCnt + kColorKeyBitCnt)
    };
    GR_STATIC_ASSERT(kBaseKeyBitCnt <= 32);

    static SkGradientShaderBase::GpuColorType ColorTypeFromKey(uint32_t baseKey){
        if (kTwoColorKey == (baseKey & kColorKeyMask)) {
            return SkGradientShaderBase::kTwo_GpuColorType;
        } else if (kThreeColorKey == (baseKey & kColorKeyMask)) {
            return SkGradientShaderBase::kThree_GpuColorType;
        } else {return SkGradientShaderBase::kTexture_GpuColorType;}
    }

    static GrGradientEffect::PremulType PremulTypeFromKey(uint32_t baseKey){
        if (kPremulBeforeInterpKey == (baseKey & kPremulTypeMask)) {
            return GrGradientEffect::kBeforeInterp_PremulType;
        } else {
            return GrGradientEffect::kAfterInterp_PremulType;
        }
    }

    SkScalar fCachedYCoord;
    GrGLProgramDataManager::UniformHandle fFSYUni;
    GrGLProgramDataManager::UniformHandle fColorStartUni;
    GrGLProgramDataManager::UniformHandle fColorMidUni;
    GrGLProgramDataManager::UniformHandle fColorEndUni;

    typedef GrGLFragmentProcessor INHERITED;
};

#endif

#endif
