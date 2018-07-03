/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "SkGradientShader.h"

#include "SkArenaAlloc.h"
#include "SkMatrix.h"
#include "SkPM4fPriv.h"
#include "SkShaderBase.h"
#include "SkTArray.h"
#include "SkTemplates.h"

class SkColorSpace;
class SkColorSpaceXformer;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkGradientShaderBase : public SkShaderBase {
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
        SkSTArray<16, SkColor4f, true> fColorStorage;
        SkSTArray<16, SkScalar , true> fPosStorage;
        SkMatrix                       fLocalMatrixStorage;
    };

    SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit);
    ~SkGradientShaderBase() override;

    bool isOpaque() const override;

    void getGradientTableBitmap(const SkColor4f* colors, SkBitmap*, SkColorType) const;

    uint32_t getGradFlags() const { return fGradFlags; }

protected:
    class GradientShaderBase4fContext;

    SkGradientShaderBase(SkReadBuffer& );
    void flatten(SkWriteBuffer&) const override;

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor*) const override;

    void initLinearBitmap(const SkColor4f* colors, SkBitmap* bitmap, SkColorType colorType) const;

    bool onAppendStages(const StageRec&) const override;

    virtual void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                                      SkRasterPipeline* postPipeline) const = 0;

    template <typename T, typename... Args>
    static Context* CheckedMakeContext(SkArenaAlloc* alloc, Args&&... args) {
        auto* ctx = alloc->make<T>(std::forward<Args>(args)...);
        if (!ctx->isValid()) {
            return nullptr;
        }
        return ctx;
    }

    struct AutoXformColors {
        AutoXformColors(const SkGradientShaderBase&, SkColorSpaceXformer*);

        SkAutoSTMalloc<8, SkColor> fColors;
    };

    const SkMatrix fPtsToUnit;
    TileMode       fTileMode;
    uint8_t        fGradFlags;

public:
    SkScalar getPos(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigPos ? fOrigPos[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor getLegacyColor(int i) const {
        SkASSERT(i < fColorCount);
        return Sk4f_toL32(swizzle_rb(Sk4f::Load(fOrigColors4f[i].vec())));
    }

    SkColor4f*          fOrigColors4f; // original colors, as linear floats
    SkScalar*           fOrigPos;      // original positions
    int                 fColorCount;
    sk_sp<SkColorSpace> fColorSpace;   // color space of gradient stops

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    TileMode getTileMode() const { return fTileMode; }

private:
    // Reserve inline space for up to 4 stops.
    static constexpr size_t kInlineStopCount   = 4;
    static constexpr size_t kInlineStorageSize = (sizeof(SkColor4f) + sizeof(SkScalar))
                                               * kInlineStopCount;
    SkAutoSTMalloc<kInlineStorageSize, uint8_t> fStorage;

    bool                                        fColorsAreOpaque;

    typedef SkShaderBase INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

struct SkColor4fXformer {
    SkColor4fXformer(const SkColor4f* colors, int colorCount, SkColorSpace* src, SkColorSpace* dst);

    const SkColor4f*              fColors;
    SkSTArray<4, SkColor4f, true> fStorage;
};

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrColorSpaceInfo.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
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
                   const GrColorSpaceInfo* dstColorSpaceInfo)
                : fContext(context)
                , fShader(shader)
                , fMatrix(matrix)
                , fDstColorSpaceInfo(dstColorSpaceInfo) {
            switch (tileMode) {
                case SkShader::kClamp_TileMode:
                    fWrapMode = GrSamplerState::WrapMode::kClamp;
                    break;
                case SkShader::kRepeat_TileMode:
                    fWrapMode = GrSamplerState::WrapMode::kRepeat;
                    break;
                case SkShader::kMirror_TileMode:
                    fWrapMode = GrSamplerState::WrapMode::kMirrorRepeat;
                    break;
                case SkShader::kDecal_TileMode:
                    // TODO: actually support decal
                    fWrapMode = GrSamplerState::WrapMode::kClamp;
                    break;
            }
        }

        CreateArgs(GrContext* context,
                   const SkGradientShaderBase* shader,
                   const SkMatrix* matrix,
                   GrSamplerState::WrapMode wrapMode,
                   const GrColorSpaceInfo* dstColorSpaceInfo)
                : fContext(context)
                , fShader(shader)
                , fMatrix(matrix)
                , fWrapMode(wrapMode)
                , fDstColorSpaceInfo(dstColorSpaceInfo) {}

        GrContext*                  fContext;
        const SkGradientShaderBase* fShader;
        const SkMatrix*             fMatrix;
        GrSamplerState::WrapMode    fWrapMode;
        const GrColorSpaceInfo*     fDstColorSpaceInfo;
    };

    class GLSLProcessor;

    ~GrGradientEffect() override;

    bool useAtlas() const { return SkToBool(-1 != fRow); }

    // Controls the implementation strategy for this effect.
    // NB: all entries need to be reflected in the key.
    enum class InterpolationStrategy : uint8_t {
        kSingle,          // interpolation in a single domain [0,1]
        kThreshold,       // interpolation in two domains [0,T) [T,1], with normal clamping
        kThresholdClamp0, // same as kThreshold, but clamped only on the left edge
        kThresholdClamp1, // same as kThreshold, but clamped only on the right edge
        kTexture,         // texture-based fallback
    };

    enum PremulType {
        kBeforeInterp_PremulType,
        kAfterInterp_PremulType,
    };

protected:
    GrGradientEffect(ClassID classID, const CreateArgs&, bool isOpaque);
    explicit GrGradientEffect(const GrGradientEffect&);  // facilitates clone() implementations

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    // Helper function used by derived class factories to handle modulation by input alpha.
    static std::unique_ptr<GrFragmentProcessor> AdjustFP(
            std::unique_ptr<GrGradientEffect> gradientFP, const CreateArgs& args) {
        if (!gradientFP->isValid()) {
            return nullptr;
        }
        return GrFragmentProcessor::MulChildByInputAlpha(std::move(gradientFP));
    }

#if GR_TEST_UTILS
    /** Helper struct that stores (and populates) parameters to construct a random gradient.
        If fUseColors4f is true, then the SkColor4f factory should be called, with fColors4f and
        fColorSpace. Otherwise, the SkColor factory should be called, with fColors. fColorCount
        will be the number of color stops in either case, and fColors and fStops can be passed to
        the gradient factory. (The constructor may decide not to use stops, in which case fStops
        will be nullptr). */
    struct RandomGradientParams {
        static constexpr int kMaxRandomGradientColors = 5;

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

    /** Checks whether the constructor failed to fully initialize the processor. */
    bool isValid() const {
        return fStrategy != InterpolationStrategy::kTexture || fTextureSampler.isInitialized();
    }

private:
    void addInterval(const SkGradientShaderBase&, const SkColor4f* colors,
                     size_t idx0, size_t idx1);

    static OptimizationFlags OptFlags(bool isOpaque);

    // Interpolation intervals, encoded as 4f tuples of (scale, bias)
    // such that color(t) = t * scale + bias.
    SkSTArray<4, GrColor4f, true> fIntervals;

    GrSamplerState::WrapMode fWrapMode;

    GrCoordTransform fCoordTransform;
    TextureSampler fTextureSampler;
    SkScalar fYCoord;
    sk_sp<GrTextureStripAtlas> fAtlas;
    int fRow;
    bool fIsOpaque;

    InterpolationStrategy fStrategy;
    SkScalar              fThreshold;  // used for InterpolationStrategy::kThreshold
    PremulType            fPremulType; // This is already baked into the table for texture
                                       // gradients, and only changes behavior for gradients
                                       // that don't use a texture.

    typedef GrFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

// Base class for GL gradient effects
class GrGradientEffect::GLSLProcessor : public GrGLSLFragmentProcessor {
public:
    GLSLProcessor() {
        fCachedYCoord = SK_ScalarMax;
    }

    static uint32_t GenBaseGradientKey(const GrProcessor&);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

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
    void emitAnalyticalColor(GrGLSLFPFragmentBuilder* fragBuilder,
                             GrGLSLUniformHandler* uniformHandler,
                             const GrShaderCaps* shaderCaps,
                             const GrGradientEffect&,
                             const char* gradientTValue,
                             const char* outputColor,
                             const char* inputColor);

    SkScalar fCachedYCoord;
    GrGLSLProgramDataManager::UniformHandle fIntervalsUni;
    GrGLSLProgramDataManager::UniformHandle fThresholdUni;
    GrGLSLProgramDataManager::UniformHandle fFSYUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

#endif

#endif
