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
#include "SkAutoMalloc.h"
#include "SkMatrix.h"
#include "SkShaderBase.h"
#include "SkTDArray.h"

class SkColorSpace;
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

    bool isOpaque() const override;

    enum class GradientBitmapType : uint8_t {
        kLegacy,
        kSRGB,
        kHalfFloat,
    };

    void getGradientTableBitmap(SkBitmap*, GradientBitmapType bitmapType) const;

    uint32_t getGradFlags() const { return fGradFlags; }

    SkColor4f getXformedColor(size_t index, SkColorSpace*) const;

protected:
    class GradientShaderBase4fContext;

    SkGradientShaderBase(SkReadBuffer& );
    void flatten(SkWriteBuffer&) const override;
    SK_TO_STRING_OVERRIDE()

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor*) const override;

    void initLinearBitmap(SkBitmap* bitmap, GradientBitmapType) const;

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

    const SkMatrix fPtsToUnit;
    TileMode       fTileMode;
    uint8_t        fGradFlags;

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount * (sizeof(SkColor) + sizeof(SkScalar) + sizeof(SkColor4f))
    };
    SkColor             fStorage[(kStorageSize + 3) >> 2];
public:
    SkScalar getPos(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigPos ? fOrigPos[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor*            fOrigColors;   // original colors, before modulation by paint in context.
    SkColor4f*          fOrigColors4f; // original colors, as linear floats
    SkScalar*           fOrigPos;      // original positions
    int                 fColorCount;
    sk_sp<SkColorSpace> fColorSpace; // color space of gradient stops

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    TileMode getTileMode() const { return fTileMode; }

private:
    bool                fColorsAreOpaque;

    void initCommon();

    typedef SkShaderBase INHERITED;
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
                   const SkColorSpace* dstColorSpace)
                : fContext(context)
                , fShader(shader)
                , fMatrix(matrix)
                , fDstColorSpace(dstColorSpace) {
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
            }
        }

        CreateArgs(GrContext* context,
                   const SkGradientShaderBase* shader,
                   const SkMatrix* matrix,
                   GrSamplerState::WrapMode wrapMode,
                   const SkColorSpace* dstColorSpace)
                : fContext(context)
                , fShader(shader)
                , fMatrix(matrix)
                , fWrapMode(wrapMode)
                , fDstColorSpace(dstColorSpace) {}

        GrContext*                  fContext;
        const SkGradientShaderBase* fShader;
        const SkMatrix*             fMatrix;
        GrSamplerState::WrapMode    fWrapMode;
        const SkColorSpace*         fDstColorSpace;
    };

    class GLSLProcessor;

    ~GrGradientEffect() override;

    bool useAtlas() const { return SkToBool(-1 != fRow); }
    SkScalar getYCoord() const { return fYCoord; }

    enum ColorType {
        kTwo_ColorType,
        kThree_ColorType,              // 0, t, 1
        kTexture_ColorType,
        kSingleHardStop_ColorType,     // 0, t, t, 1
        kHardStopLeftEdged_ColorType,  // 0, 0, 1
        kHardStopRightEdged_ColorType, // 0, 1, 1
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

    const GrColor4f* getColors4f(int pos) const {
        SkASSERT(fColorType != kTexture_ColorType);
        SkASSERT(pos < fColors4f.count());
        return &fColors4f[pos];
    }

protected:
    GrGradientEffect(ClassID classID, const CreateArgs&, bool isOpaque);
    explicit GrGradientEffect(const GrGradientEffect&);  // facilitates clone() implementations

    // Helper function used by derived class factories to handle color space transformation and
    // modulation by input alpha.
    static std::unique_ptr<GrFragmentProcessor> AdjustFP(
            std::unique_ptr<GrGradientEffect> gradientFP, const CreateArgs& args) {
        if (!gradientFP->isValid()) {
            return nullptr;
        }
        std::unique_ptr<GrFragmentProcessor> fp;
        // With analytic gradients, we pre-convert the stops to the destination color space, so no
        // xform is needed. With texture-based gradients, we leave the data in the source color
        // space (to avoid clamping if we can't use F16)... Add an extra FP to do the xform.
        if (kTexture_ColorType == gradientFP->getColorType()) {
            fp = GrColorSpaceXformEffect::Make(std::move(gradientFP),
                                               args.fShader->fColorSpace.get(),
                                               args.fDstColorSpace);
        } else {
            fp = std::move(gradientFP);
        }
        return GrFragmentProcessor::MulOutputByInputAlpha(std::move(fp));
    }

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

    /** Checks whether the constructor failed to fully initialize the processor. */
    bool isValid() const {
        return fColorType != kTexture_ColorType || fTextureSampler.isInitialized();
    }

private:
    static OptimizationFlags OptFlags(bool isOpaque);

    SkTDArray<GrColor4f> fColors4f;

    SkTDArray<SkScalar> fPositions;
    GrSamplerState::WrapMode fWrapMode;

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
    void emitAnalyticalColor(GrGLSLFPFragmentBuilder* fragBuilder,
                             GrGLSLUniformHandler* uniformHandler,
                             const GrShaderCaps* shaderCaps,
                             const GrGradientEffect&,
                             const char* gradientTValue,
                             const char* outputColor,
                             const char* inputColor);

    enum {
        // First bit for premul before/after interpolation
        kPremulBeforeInterpKey  =  1,

        // Next three bits for 2/3 color type or different special
        // hard stop cases ('none' means using texture atlas)
        kTwoColorKey            =  2,
        kThreeColorKey          =  4,

        kHardStopCenteredKey    =  6,
        kHardStopZeroZeroOneKey =  8,
        kHardStopZeroOneOneKey  = 10,

        // Next two bits for tile mode
        kClampTileMode          = 16,
        kRepeatTileMode         = 32,
        kMirrorTileMode         = 48,

        // Lower six bits for premul, 2/3 color type, and tile mode
        kReservedBits           = 6,
    };

    SkScalar fCachedYCoord;
    GrGLSLProgramDataManager::UniformHandle fColorsUni;
    GrGLSLProgramDataManager::UniformHandle fExtraStopT;
    GrGLSLProgramDataManager::UniformHandle fFSYUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

#endif

#endif
