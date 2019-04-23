/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessor_DEFINED
#define GrFragmentProcessor_DEFINED

#include "GrProcessor.h"
#include "GrProxyRef.h"

class GrCoordTransform;
class GrGLSLFragmentProcessor;
class GrPaint;
class GrPipeline;
class GrProcessorKeyBuilder;
class GrShaderCaps;
class GrSwizzle;

/** Provides custom fragment shader code. Fragment processors receive an input color (half4) and
    produce an output color. They may reference textures and uniforms. They may use
    GrCoordTransforms to receive a transformation of the local coordinates that map from local space
    to the fragment being processed.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    class TextureSampler;

    /**
    *  In many instances (e.g. SkShader::asFragmentProcessor() implementations) it is desirable to
    *  only consider the input color's alpha. However, there is a competing desire to have reusable
    *  GrFragmentProcessor subclasses that can be used in other scenarios where the entire input
    *  color is considered. This function exists to filter the input color and pass it to a FP. It
    *  does so by returning a parent FP that multiplies the passed in FPs output by the parent's
    *  input alpha. The passed in FP will not receive an input color.
    */
    static std::unique_ptr<GrFragmentProcessor> MulChildByInputAlpha(
            std::unique_ptr<GrFragmentProcessor> child);

    /**
     *  Like MulChildByInputAlpha(), but reverses the sense of src and dst. In this case, return
     *  the input modulated by the child's alpha. The passed in FP will not receive an input color.
     *
     *  output = input * child.a
     */
    static std::unique_ptr<GrFragmentProcessor> MulInputByChildAlpha(
            std::unique_ptr<GrFragmentProcessor> child);

    /**
     *  This assumes that the input color to the returned processor will be unpremul and that the
     *  passed processor (which becomes the returned processor's child) produces a premul output.
     *  The result of the returned processor is a premul of its input color modulated by the child
     *  processor's premul output.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeInputPremulAndMulByOutput(
            std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will ignore its input color and instead feed the passed in color as input to the
     *  child.
     */
    static std::unique_ptr<GrFragmentProcessor> OverrideInput(std::unique_ptr<GrFragmentProcessor>,
                                                              const SkPMColor4f&,
                                                              bool useUniform = true);

    /**
     *  Returns a fragment processor that premuls the input before calling the passed in fragment
     *  processor.
     */
    static std::unique_ptr<GrFragmentProcessor> PremulInput(std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then swizzles
     *  the output.
     */
    static std::unique_ptr<GrFragmentProcessor> SwizzleOutput(std::unique_ptr<GrFragmentProcessor>,
                                                              const GrSwizzle&);

    /**
     * Returns a fragment processor that runs the passed in array of fragment processors in a
     * series. The original input is passed to the first, the first's output is passed to the
     * second, etc. The output of the returned processor is the output of the last processor of the
     * series.
     *
     * The array elements with be moved.
     */
    static std::unique_ptr<GrFragmentProcessor> RunInSeries(std::unique_ptr<GrFragmentProcessor>*,
                                                            int cnt);

    /**
     * Makes a copy of this fragment processor that draws equivalently to the original.
     * If the processor has child processors they are cloned as well.
     */
    virtual std::unique_ptr<GrFragmentProcessor> clone() const = 0;

    GrGLSLFragmentProcessor* createGLSLInstance() const;

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
        this->onGetGLSLProcessorKey(caps, b);
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            fChildProcessors[i]->getGLSLProcessorKey(caps, b);
        }
    }

    int numTextureSamplers() const { return fTextureSamplerCnt; }
    const TextureSampler& textureSampler(int i) const;

    int numCoordTransforms() const { return fCoordTransforms.count(); }

    /** Returns the coordinate transformation at index. index must be valid according to
        numTransforms(). */
    const GrCoordTransform& coordTransform(int index) const { return *fCoordTransforms[index]; }

    const SkTArray<const GrCoordTransform*, true>& coordTransforms() const {
        return fCoordTransforms;
    }

    int numChildProcessors() const { return fChildProcessors.count(); }

    const GrFragmentProcessor& childProcessor(int index) const { return *fChildProcessors[index]; }

    bool instantiate(GrResourceProvider*) const;

    void markPendingExecution() const;

    /** Do any of the coordtransforms for this processor require local coords? */
    bool usesLocalCoords() const { return SkToBool(fFlags & kUsesLocalCoords_Flag); }

    /**
     * A GrDrawOp may premultiply its antialiasing coverage into its GrGeometryProcessor's color
     * output under the following scenario:
     *   * all the color fragment processors report true to this query,
     *   * all the coverage fragment processors report true to this query,
     *   * the blend mode arithmetic allows for it it.
     * To be compatible a fragment processor's output must be a modulation of its input color or
     * alpha with a computed premultiplied color or alpha that is in 0..1 range. The computed color
     * or alpha that is modulated against the input cannot depend on the input's alpha. The computed
     * value cannot depend on the input's color channels unless it unpremultiplies the input color
     * channels by the input alpha.
     */
    bool compatibleWithCoverageAsAlpha() const {
        return SkToBool(fFlags & kCompatibleWithCoverageAsAlpha_OptimizationFlag);
    }

    /**
     * If this is true then all opaque input colors to the processor produce opaque output colors.
     */
    bool preservesOpaqueInput() const {
        return SkToBool(fFlags & kPreservesOpaqueInput_OptimizationFlag);
    }

    /**
     * Tests whether given a constant input color the processor produces a constant output color
     * (for all fragments). If true outputColor will contain the constant color produces for
     * inputColor.
     */
    bool hasConstantOutputForConstantInput(SkPMColor4f inputColor, SkPMColor4f* outputColor) const {
        if (fFlags & kConstantOutputForConstantInput_OptimizationFlag) {
            *outputColor = this->constantOutputForConstantInput(inputColor);
            return true;
        }
        return false;
    }
    bool hasConstantOutputForConstantInput() const {
        return SkToBool(fFlags & kConstantOutputForConstantInput_OptimizationFlag);
    }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use getGLSLProcessorKey
     */
    bool isEqual(const GrFragmentProcessor& that) const;

    /**
     * Pre-order traversal of a FP hierarchy, or of the forest of FPs in a GrPipeline. In the latter
     * case the tree rooted at each FP in the GrPipeline is visited successively.
     */
    class Iter : public SkNoncopyable {
    public:
        explicit Iter(const GrFragmentProcessor* fp) { fFPStack.push_back(fp); }
        explicit Iter(const GrPipeline& pipeline);
        explicit Iter(const GrPaint&);
        const GrFragmentProcessor* next();

    private:
        SkSTArray<4, const GrFragmentProcessor*, true> fFPStack;
    };

    /**
     * Iterates over all the Ts owned by a GrFragmentProcessor and its children or over all the Ts
     * owned by the forest of GrFragmentProcessors in a GrPipeline. FPs are visited in the same
     * order as Iter and each of an FP's Ts are visited in order.
     */
    template <typename T, int (GrFragmentProcessor::*COUNT)() const,
              const T& (GrFragmentProcessor::*GET)(int)const>
    class FPItemIter : public SkNoncopyable {
    public:
        explicit FPItemIter(const GrFragmentProcessor* fp)
                : fCurrFP(nullptr)
                , fCTIdx(0)
                , fFPIter(fp) {
            fCurrFP = fFPIter.next();
        }
        explicit FPItemIter(const GrPipeline& pipeline)
                : fCurrFP(nullptr)
                , fCTIdx(0)
                , fFPIter(pipeline) {
            fCurrFP = fFPIter.next();
        }

        const T* next() {
            if (!fCurrFP) {
                return nullptr;
            }
            while (fCTIdx == (fCurrFP->*COUNT)()) {
                fCTIdx = 0;
                fCurrFP = fFPIter.next();
                if (!fCurrFP) {
                    return nullptr;
                }
            }
            return &(fCurrFP->*GET)(fCTIdx++);
        }

    private:
        const GrFragmentProcessor*  fCurrFP;
        int                         fCTIdx;
        GrFragmentProcessor::Iter   fFPIter;
    };

    using CoordTransformIter = FPItemIter<GrCoordTransform,
                                          &GrFragmentProcessor::numCoordTransforms,
                                          &GrFragmentProcessor::coordTransform>;

    using TextureAccessIter = FPItemIter<TextureSampler,
                                         &GrFragmentProcessor::numTextureSamplers,
                                         &GrFragmentProcessor::textureSampler>;

    void visitProxies(const std::function<void(GrSurfaceProxy*)>& func);

protected:
    enum OptimizationFlags : uint32_t {
        kNone_OptimizationFlags,
        kCompatibleWithCoverageAsAlpha_OptimizationFlag = 0x1,
        kPreservesOpaqueInput_OptimizationFlag = 0x2,
        kConstantOutputForConstantInput_OptimizationFlag = 0x4,
        kAll_OptimizationFlags = kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                                 kPreservesOpaqueInput_OptimizationFlag |
                                 kConstantOutputForConstantInput_OptimizationFlag
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(OptimizationFlags)

    /**
     * Can be used as a helper to decide which fragment processor OptimizationFlags should be set.
     * This assumes that the subclass output color will be a modulation of the input color with a
     * value read from a texture of the passed config and that the texture contains premultiplied
     * color or alpha values that are in range.
     *
     * Since there are multiple ways in which a sampler may have its coordinates clamped or wrapped,
     * callers must determine on their own if the sampling uses a decal strategy in any way, in
     * which case the texture may become transparent regardless of the pixel config.
     */
    static OptimizationFlags ModulateForSamplerOptFlags(GrPixelConfig config, bool samplingDecal) {
        if (samplingDecal) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        } else {
            return ModulateForClampedSamplerOptFlags(config);
        }
    }

    // As above, but callers should somehow ensure or assert their sampler still uses clamping
    static OptimizationFlags ModulateForClampedSamplerOptFlags(GrPixelConfig config) {
        if (GrPixelConfigIsOpaque(config)) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                   kPreservesOpaqueInput_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }

    GrFragmentProcessor(ClassID classID, OptimizationFlags optimizationFlags)
            : INHERITED(classID)
            , fFlags(optimizationFlags) {
        SkASSERT((fFlags & ~kAll_OptimizationFlags) == 0);
    }

    OptimizationFlags optimizationFlags() const {
        return static_cast<OptimizationFlags>(kAll_OptimizationFlags & fFlags);
    }

    /** Useful when you can't call fp->optimizationFlags() on a base class object from a subclass.*/
    static OptimizationFlags ProcessorOptimizationFlags(const GrFragmentProcessor* fp) {
        return fp->optimizationFlags();
    }

    /**
     * This allows one subclass to access another subclass's implementation of
     * constantOutputForConstantInput. It must only be called when
     * hasConstantOutputForConstantInput() is known to be true.
     */
    static SkPMColor4f ConstantOutputForConstantInput(const GrFragmentProcessor& fp,
                                                      const SkPMColor4f& input) {
        SkASSERT(fp.hasConstantOutputForConstantInput());
        return fp.constantOutputForConstantInput(input);
    }

    /**
     * Fragment Processor subclasses call this from their constructor to register coordinate
     * transformations. Coord transforms provide a mechanism for a processor to receive coordinates
     * in their FS code. The matrix expresses a transformation from local space. For a given
     * fragment the matrix will be applied to the local coordinate that maps to the fragment.
     *
     * When the transformation has perspective, the transformed coordinates will have
     * 3 components. Otherwise they'll have 2.
     *
     * This must only be called from the constructor because GrProcessors are immutable. The
     * processor subclass manages the lifetime of the transformations (this function only stores a
     * pointer). The GrCoordTransform is typically a member field of the GrProcessor subclass.
     *
     * A processor subclass that has multiple methods of construction should always add its coord
     * transforms in a consistent order. The non-virtual implementation of isEqual() automatically
     * compares transforms and will assume they line up across the two processor instances.
     */
    void addCoordTransform(const GrCoordTransform*);

    /**
     * FragmentProcessor subclasses call this from their constructor to register any child
     * FragmentProcessors they have. This must be called AFTER all texture accesses and coord
     * transforms have been added.
     * This is for processors whose shader code will be composed of nested processors whose output
     * colors will be combined somehow to produce its output color.  Registering these child
     * processors will allow the ProgramBuilder to automatically handle their transformed coords and
     * texture accesses and mangle their uniform and output color names.
     */
    int registerChildProcessor(std::unique_ptr<GrFragmentProcessor> child);

    void setTextureSamplerCnt(int cnt) {
        SkASSERT(cnt >= 0);
        fTextureSamplerCnt = cnt;
    }

    /**
     * Helper for implementing onTextureSampler(). E.g.:
     * return IthTexureSampler(i, fMyFirstSampler, fMySecondSampler, fMyThirdSampler);
     */
    template <typename... Args>
    static const TextureSampler& IthTextureSampler(int i, const TextureSampler& samp0,
                                                   const Args&... samps) {
        return (0 == i) ? samp0 : IthTextureSampler(i - 1, samps...);
    }
    inline static const TextureSampler& IthTextureSampler(int i);

private:
    virtual SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& /* inputColor */) const {
        SK_ABORT("Subclass must override this if advertising this optimization.");
        return SK_PMColor4fTRANSPARENT;
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrFragmentProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLFragmentProcessor* onCreateGLSLInstance() const = 0;

    /** Implemented using GLFragmentProcessor::GenKey as described in this class's comment. */
    virtual void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;

    /**
     * Subclass implements this to support isEqual(). It will only be called if it is known that
     * the two processors are of the same subclass (i.e. they return the same object from
     * getFactory()). The processor subclass should not compare its coord transforms as that will
     * be performed automatically in the non-virtual isEqual().
     */
    virtual bool onIsEqual(const GrFragmentProcessor&) const = 0;

    virtual const TextureSampler& onTextureSampler(int) const { return IthTextureSampler(0); }

    bool hasSameTransforms(const GrFragmentProcessor&) const;

    enum PrivateFlags {
        kFirstPrivateFlag = kAll_OptimizationFlags + 1,
        kUsesLocalCoords_Flag = kFirstPrivateFlag,
    };

    mutable uint32_t fFlags = 0;

    int fTextureSamplerCnt = 0;

    SkSTArray<4, const GrCoordTransform*, true> fCoordTransforms;

    SkSTArray<1, std::unique_ptr<GrFragmentProcessor>, true> fChildProcessors;

    typedef GrProcessor INHERITED;
};

/**
 * Used to represent a texture that is required by a GrFragmentProcessor. It holds a GrTextureProxy
 * along with an associated GrSamplerState. TextureSamplers don't perform any coord manipulation to
 * account for texture origin.
 */
class GrFragmentProcessor::TextureSampler {
public:
    TextureSampler() = default;

    /**
     * This copy constructor is used by GrFragmentProcessor::clone() implementations. The copy
     * always takes a new ref on the texture proxy as the new fragment processor will not yet be
     * in pending execution state.
     */
    explicit TextureSampler(const TextureSampler& that)
            : fProxyRef(sk_ref_sp(that.fProxyRef.get()), that.fProxyRef.ioType())
            , fSamplerState(that.fSamplerState) {}

    TextureSampler(sk_sp<GrTextureProxy>, const GrSamplerState&);

    explicit TextureSampler(sk_sp<GrTextureProxy>,
                            GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
                            GrSamplerState::WrapMode wrapXAndY = GrSamplerState::WrapMode::kClamp);

    TextureSampler& operator=(const TextureSampler&) = delete;

    void reset(sk_sp<GrTextureProxy>, const GrSamplerState&);
    void reset(sk_sp<GrTextureProxy>,
               GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
               GrSamplerState::WrapMode wrapXAndY = GrSamplerState::WrapMode::kClamp);

    bool operator==(const TextureSampler& that) const {
        return this->proxy()->underlyingUniqueID() == that.proxy()->underlyingUniqueID() &&
               fSamplerState == that.fSamplerState;
    }

    bool operator!=(const TextureSampler& other) const { return !(*this == other); }

    // 'instantiate' should only ever be called at flush time.
    // TODO: this can go away once explicit allocation has stuck
    bool instantiate(GrResourceProvider* resourceProvider) const {
        return fProxyRef.get()->isInstantiated();
    }

    // 'peekTexture' should only ever be called after a successful 'instantiate' call
    GrTexture* peekTexture() const {
        SkASSERT(fProxyRef.get()->peekTexture());
        return fProxyRef.get()->peekTexture();
    }

    GrTextureProxy* proxy() const { return fProxyRef.get(); }
    const GrSamplerState& samplerState() const { return fSamplerState; }

    bool isInitialized() const { return SkToBool(fProxyRef.get()); }
    /**
     * For internal use by GrFragmentProcessor.
     */
    const GrTextureProxyRef* proxyRef() const { return &fProxyRef; }

private:
    GrTextureProxyRef fProxyRef;
    GrSamplerState fSamplerState;
};

//////////////////////////////////////////////////////////////////////////////

const GrFragmentProcessor::TextureSampler& GrFragmentProcessor::IthTextureSampler(int i) {
    SK_ABORT("Illegal texture sampler index");
    static const TextureSampler kBogus;
    return kBogus;
}

GR_MAKE_BITFIELD_OPS(GrFragmentProcessor::OptimizationFlags)

#endif
