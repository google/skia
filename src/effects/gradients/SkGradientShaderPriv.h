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

#include "SkArenaAlloc.h"
#include "SkAutoMalloc.h"
#include "SkClampRange.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkOnce.h"
#include "SkReadBuffer.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
    #define GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS 1
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

static inline SkFixed mirror_tileproc(SkFixed x) {
    int s = SkLeftShift(x, 15) >> 31;
    return (x ^ s) & 0xFFFF;
}

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
        const SkColor4f*    fColors;
        sk_sp<SkColorSpace> fColorSpace;
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
        SkColor4f* mutableColors() { return const_cast<SkColor4f*>(fColors); }
        SkScalar* mutablePos() { return const_cast<SkScalar*>(fPos); }

    private:
        enum {
            kStorageCount = 16
        };
        SkColor4f fColorStorage[kStorageCount];
        SkScalar fPosStorage[kStorageCount];
        SkMatrix fLocalMatrixStorage;
        SkAutoMalloc fDynamicStorage;
    };

    SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit);
    ~SkGradientShaderBase() override;

    // The cache is initialized on-demand when getCache32 is called.
    class GradientShaderCache : public SkRefCnt {
    public:
        GradientShaderCache(U8CPU alpha, bool dither, const SkGradientShaderBase& shader);
        ~GradientShaderCache();

        const SkPMColor*    getCache32();

        SkPixelRef* getCache32PixelRef() const { return fCache32PixelRef.get(); }

        unsigned getAlpha() const { return fCacheAlpha; }
        bool getDither() const { return fCacheDither; }

    private:
        // Working pointer. If it's nullptr, we need to recompute the cache values.
        SkPMColor*  fCache32;

        sk_sp<SkPixelRef> fCache32PixelRef;
        const unsigned    fCacheAlpha;        // The alpha value we used when we computed the cache.
                                              // Larger than 8bits so we can store uninitialized
                                              // value.
        const bool        fCacheDither;       // The dither flag used when we computed the cache.

        const SkGradientShaderBase& fShader;

        // Make sure we only initialize the cache once.
        SkOnce fCache32InitOnce;

        static void initCache32(GradientShaderCache* cache);

        static void Build32bitCache(SkPMColor[], SkColor c0, SkColor c1, int count,
                                    U8CPU alpha, uint32_t gradFlags, bool dither);
    };

    class GradientShaderBaseContext : public SkShader::Context {
    public:
        GradientShaderBaseContext(const SkGradientShaderBase& shader, const ContextRec&);

        uint32_t getFlags() const override { return fFlags; }

        bool isValid() const;

    protected:
        SkMatrix    fDstToIndex;
        SkMatrix::MapXYProc fDstToIndexProc;
        uint8_t     fDstToIndexClass;
        uint8_t     fFlags;
        bool        fDither;

        sk_sp<GradientShaderCache> fCache;

    private:
        typedef SkShader::Context INHERITED;
    };

    bool isOpaque() const override;

    enum class GradientBitmapType : uint8_t {
        kLegacy,
        kSRGB,
        kHalfFloat,
    };

    void getGradientTableBitmap(SkBitmap*, GradientBitmapType bitmapType) const;

    enum {
        /// Seems like enough for visual accuracy. TODO: if pos[] deserves
        /// it, use a larger cache.
        kCache32Bits    = 8,
        kCache32Count   = (1 << kCache32Bits),
        kCache32Shift   = 16 - kCache32Bits,
        kSqrt32Shift    = 8 - kCache32Bits,

        /// This value is used to *read* the dither cache; it may be 0
        /// if dithering is disabled.
        kDitherStride32 = kCache32Count,
    };

    uint32_t getGradFlags() const { return fGradFlags; }

protected:
    class GradientShaderBase4fContext;

    SkGradientShaderBase(SkReadBuffer& );
    void flatten(SkWriteBuffer&) const override;
    SK_TO_STRING_OVERRIDE()

    const SkMatrix fPtsToUnit;
    TileMode    fTileMode;
    TileProc    fTileProc;
    uint8_t     fGradFlags;

    struct Rec {
        SkFixed     fPos;   // 0...1
        uint32_t    fScale; // (1 << 24) / range
    };
    Rec*        fRecs;

    void commonAsAGradient(GradientInfo*, bool flipGrad = false) const;

    bool onAsLuminanceColor(SkColor*) const override;


    void initLinearBitmap(SkBitmap* bitmap) const;

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

    template <typename T, typename... Args>
    static Context* CheckedMakeContext(SkArenaAlloc* alloc, Args&&... args) {
        auto* ctx = alloc->make<T>(std::forward<Args>(args)...);
        if (!ctx->isValid()) {
            return nullptr;
        }
        return ctx;
    }

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount *
                       (sizeof(SkColor) + sizeof(SkScalar) + sizeof(Rec) + sizeof(SkColor4f))
    };
    SkColor             fStorage[(kStorageSize + 3) >> 2];
public:
    SkColor*            fOrigColors;   // original colors, before modulation by paint in context.
    SkColor4f*          fOrigColors4f; // original colors, as linear floats
    SkScalar*           fOrigPos;      // original positions
    int                 fColorCount;
    sk_sp<SkColorSpace> fColorSpace; // color space of gradient stops

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    TileMode getTileMode() const { return fTileMode; }
    Rec* getRecs() const { return fRecs; }

private:
    bool                fColorsAreOpaque;

    sk_sp<GradientShaderCache> refCache(U8CPU alpha, bool dither) const;
    mutable SkMutex                    fCacheMutex;
    mutable sk_sp<GradientShaderCache> fCache;

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

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrColorSpaceXform.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrInvariantOutput;

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
    struct CreateArgs {
        CreateArgs(GrContext* context,
                   const SkGradientShaderBase* shader,
                   const SkMatrix* matrix,
                   SkShader::TileMode tileMode,
                   sk_sp<GrColorSpaceXform> colorSpaceXform,
                   bool gammaCorrect)
            : fContext(context)
            , fShader(shader)
            , fMatrix(matrix)
            , fTileMode(tileMode)
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fGammaCorrect(gammaCorrect) {}

        GrContext*                  fContext;
        const SkGradientShaderBase* fShader;
        const SkMatrix*             fMatrix;
        SkShader::TileMode          fTileMode;
        sk_sp<GrColorSpaceXform>    fColorSpaceXform;
        bool                        fGammaCorrect;
    };

    class GLSLProcessor;

    ~GrGradientEffect() override;

    bool useAtlas() const { return SkToBool(-1 != fRow); }
    SkScalar getYCoord() const { return fYCoord; }

    enum ColorType {
        kTwo_ColorType,
        kThree_ColorType, // Symmetric three color
        kTexture_ColorType,

#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        kSingleHardStop_ColorType,     // 0, t, t, 1
        kHardStopLeftEdged_ColorType,  // 0, 0, 1
        kHardStopRightEdged_ColorType, // 0, 1, 1
#endif
    };

    ColorType getColorType() const { return fColorType; }

    // Determines the type of gradient, one of:
    //    - Two-color
    //    - Symmetric three-color
    //    - Texture
    //    - Centered hard stop
    //    - Left-edged hard stop
    //    - Right-edged hard stop
    ColorType determineColorType(const SkGradientShaderBase& shader);

    enum PremulType {
        kBeforeInterp_PremulType,
        kAfterInterp_PremulType,
    };

    PremulType getPremulType() const { return fPremulType; }

    const SkColor* getColors(int pos) const {
        SkASSERT(fColorType != kTexture_ColorType);
        SkASSERT(pos < fColors.count());
        return &fColors[pos];
    }

    const SkColor4f* getColors4f(int pos) const {
        SkASSERT(fColorType != kTexture_ColorType);
        SkASSERT(pos < fColors4f.count());
        return &fColors4f[pos];
    }

protected:
    GrGradientEffect(const CreateArgs&, bool isOpaque);

    #if GR_TEST_UTILS
    /** Helper struct that stores (and populates) parameters to construct a random gradient.
        If fUseColors4f is true, then the SkColor4f factory should be called, with fColors4f and
        fColorSpace. Otherwise, the SkColor factory should be called, with fColors. fColorCount
        will be the number of color stops in either case, and fColors and fStops can be passed to
        the gradient factory. (The constructor may decide not to use stops, in which case fStops
        will be nullptr). */
    struct RandomGradientParams {
        static const int kMaxRandomGradientColors = 5;

        RandomGradientParams(SkRandom* r);

        bool fUseColors4f;
        SkColor fColors[kMaxRandomGradientColors];
        SkColor4f fColors4f[kMaxRandomGradientColors];
        sk_sp<SkColorSpace> fColorSpace;
        SkScalar fStopStorage[kMaxRandomGradientColors];
        SkShader::TileMode fTileMode;
        int fColorCount;
        SkScalar* fStops;
    };
    #endif

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const GrCoordTransform& getCoordTransform() const { return fCoordTransform; }

private:
    static OptimizationFlags OptFlags(bool isOpaque);

    // If we're in legacy mode, then fColors will be populated. If we're gamma-correct, then
    // fColors4f and fColorSpaceXform will be populated.
    SkTDArray<SkColor>       fColors;

    SkTDArray<SkColor4f>     fColors4f;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    SkTDArray<SkScalar>      fPositions;
    SkShader::TileMode       fTileMode;

    GrCoordTransform fCoordTransform;
    TextureSampler fTextureSampler;
    SkScalar fYCoord;
    GrTextureStripAtlas* fAtlas;
    int fRow;
    bool fIsOpaque;
    ColorType fColorType;
    PremulType fPremulType; // This is already baked into the table for texture gradients, and
                            // only changes behavior for gradients that don't use a texture.
    typedef GrFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

// Base class for GL gradient effects
class GrGradientEffect::GLSLProcessor : public GrGLSLFragmentProcessor {
public:
    GLSLProcessor() {
        fCachedYCoord = SK_ScalarMax;
    }

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

protected:
    /**
     * Subclasses must call this. It will return a key for the part of the shader code controlled
     * by the base class. The subclasses must stick it in their key and then pass it to the below
     * emit* functions from their emitCode function.
     */
    static uint32_t GenBaseGradientKey(const GrProcessor&);

    // Emits the uniform used as the y-coord to texture samples in derived classes. Subclasses
    // should call this method from their emitCode().
    void emitUniforms(GrGLSLUniformHandler*, const GrGradientEffect&);

    // Emit code that gets a fragment's color from an expression for t; has branches for
    // several control flows inside -- 2-color gradients, 3-color symmetric gradients, 4+
    // color gradients that use the traditional texture lookup, as well as several varieties
    // of hard stop gradients
    void emitColor(GrGLSLFPFragmentBuilder* fragBuilder,
                   GrGLSLUniformHandler* uniformHandler,
                   const GrShaderCaps* shaderCaps,
                   const GrGradientEffect&,
                   const char* gradientTValue,
                   const char* outputColor,
                   const char* inputColor,
                   const TextureSamplers&);

private:
    enum {
        // First bit for premul before/after interp
        kPremulBeforeInterpKey  =  1,

        // Next three bits for 2/3 color type or different special
        // hard stop cases (neither means using texture atlas)
        kTwoColorKey            =  2,
        kThreeColorKey          =  4,
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        kHardStopCenteredKey    =  6,
        kHardStopZeroZeroOneKey =  8,
        kHardStopZeroOneOneKey  = 10,

        // Next two bits for tile mode
        kClampTileMode          = 16,
        kRepeatTileMode         = 32,
        kMirrorTileMode         = 48,

        // Lower six bits for premul, 2/3 color type, and tile mode
        kReservedBits           = 6,
#endif
    };

    SkScalar fCachedYCoord;
    GrGLSLProgramDataManager::UniformHandle fColorsUni;
    GrGLSLProgramDataManager::UniformHandle fHardStopT;
    GrGLSLProgramDataManager::UniformHandle fFSYUni;
    GrGLSLColorSpaceXformHelper             fColorSpaceHelper;

    typedef GrGLSLFragmentProcessor INHERITED;
};

#endif

#endif
