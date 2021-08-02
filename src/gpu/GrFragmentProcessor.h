/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessor_DEFINED
#define GrFragmentProcessor_DEFINED

#include <tuple>

#include "include/private/SkSLSampleUsage.h"
#include "src/gpu/GrProcessor.h"

class GrGLSLFragmentProcessor;
class GrPaint;
class GrPipeline;
class GrProcessorKeyBuilder;
class GrShaderCaps;
class GrSwizzle;
class GrTextureEffect;

/**
 * Some fragment-processor creation methods have preconditions that might not be satisfied by the
 * calling code. Those methods can return a `GrFPResult` from their factory methods. If creation
 * succeeds, the new fragment processor is created and `success` is true. If a precondition is not
 * met, `success` is set to false and the input FP is returned unchanged.
 */
class GrFragmentProcessor;
using GrFPResult = std::tuple<bool /*success*/, std::unique_ptr<GrFragmentProcessor>>;

/** Provides custom fragment shader code. Fragment processors receive an input position and
    produce an output color. They may contain uniforms and may have children fragment processors
    that are sampled.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    /** Always returns 'color'. */
    static std::unique_ptr<GrFragmentProcessor> MakeColor(SkPMColor4f color);

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
     *  Returns a fragment processor that generates the passed-in color, modulated by the child's
     *  alpha channel. The child's input color will be the parent's fInputColor. (Pass a null FP to
     *  use the alpha from fInputColor instead of a child FP.)
     */
    static std::unique_ptr<GrFragmentProcessor> ModulateAlpha(
            std::unique_ptr<GrFragmentProcessor> child, const SkPMColor4f& color);

    /**
     *  Returns a fragment processor that generates the passed-in color, modulated by the child's
     *  RGBA color. The child's input color will be the parent's fInputColor. (Pass a null FP to use
     *  the color from fInputColor instead of a child FP.)
     */
    static std::unique_ptr<GrFragmentProcessor> ModulateRGBA(
            std::unique_ptr<GrFragmentProcessor> child, const SkPMColor4f& color);

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
     *  Returns a fragment processor which samples the passed-in fragment processor using
     *  `args.fDestColor` as its input color. Pass a null FP to access `args.fDestColor` directly.
     *  (This is only meaningful in contexts like blenders, which use a source and dest color.)
     */
    static std::unique_ptr<GrFragmentProcessor> UseDestColorAsInput(
            std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will unpremul its input color, make it opaque, and pass that as the input to
     *  the child. Then the original input alpha is applied to the result of the child.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeInputOpaqueAndPostApplyAlpha(
            std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then swizzles
     *  the output.
     */
    static std::unique_ptr<GrFragmentProcessor> SwizzleOutput(std::unique_ptr<GrFragmentProcessor>,
                                                              const GrSwizzle&);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then clamps
     *  the output to [0, 1].
     */
    static std::unique_ptr<GrFragmentProcessor> ClampOutput(std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then ensures
     *  the output is a valid premul color by clamping RGB to [0, A].
     */
    static std::unique_ptr<GrFragmentProcessor> ClampPremulOutput(
            std::unique_ptr<GrFragmentProcessor>);

    /**
     * Returns a fragment processor that composes two fragment processors `f` and `g` into f(g(x)).
     * This is equivalent to running them in series (`g`, then `f`). This is not the same as
     * transfer-mode composition; there is no blending step.
     */
    static std::unique_ptr<GrFragmentProcessor> Compose(std::unique_ptr<GrFragmentProcessor> f,
                                                        std::unique_ptr<GrFragmentProcessor> g);

    /*
     * Returns a fragment processor that calls the passed in fragment processor, then runs the
     * resulting color through the supplied color matrix.
     */
    static std::unique_ptr<GrFragmentProcessor> ColorMatrix(
            std::unique_ptr<GrFragmentProcessor> child,
            const float matrix[20],
            bool unpremulInput,
            bool clampRGBOutput,
            bool premulOutput);

    /**
     * Returns a fragment processor that reads back the color on the surface being painted; that is,
     * sampling this will return the color of the pixel that is currently being painted over.
     */
    static std::unique_ptr<GrFragmentProcessor> SurfaceColor();

    /**
     * Returns a fragment processor that calls the passed in fragment processor, but evaluates it
     * in device-space (rather than local space).
     */
    static std::unique_ptr<GrFragmentProcessor> DeviceSpace(std::unique_ptr<GrFragmentProcessor>);

    /**
     * "Shape" FPs, often used for clipping. Each one evaluates a particular kind of shape (rect,
     * circle, ellipse), and modulates the coverage of that shape against the results of the input
     * FP. GrClipEdgeType is used to select inverse/normal fill, and AA or non-AA edges.
     */
    static std::unique_ptr<GrFragmentProcessor> Rect(std::unique_ptr<GrFragmentProcessor>,
                                                     GrClipEdgeType,
                                                     SkRect);

    static GrFPResult Circle(std::unique_ptr<GrFragmentProcessor>,
                             GrClipEdgeType,
                             SkPoint center,
                             float radius);

    static GrFPResult Ellipse(std::unique_ptr<GrFragmentProcessor>,
                              GrClipEdgeType,
                              SkPoint center,
                              SkPoint radii,
                              const GrShaderCaps&);

    /**
     * Returns a fragment processor that calls the passed in fragment processor, but ensures the
     * entire program is compiled with high-precision types.
     */
    static std::unique_ptr<GrFragmentProcessor> HighPrecision(std::unique_ptr<GrFragmentProcessor>);

    /**
     * Makes a copy of this fragment processor that draws equivalently to the original.
     * If the processor has child processors they are cloned as well.
     */
    virtual std::unique_ptr<GrFragmentProcessor> clone() const = 0;

    // The FP this was registered with as a child function. This will be null if this is a root.
    const GrFragmentProcessor* parent() const { return fParent; }

    std::unique_ptr<GrGLSLFragmentProcessor> makeProgramImpl() const;

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
        this->onGetGLSLProcessorKey(caps, b);
        for (const auto& child : fChildProcessors) {
            if (child) {
                child->getGLSLProcessorKey(caps, b);
            }
        }
    }

    int numChildProcessors() const { return fChildProcessors.count(); }
    int numNonNullChildProcessors() const;

    GrFragmentProcessor* childProcessor(int index) { return fChildProcessors[index].get(); }
    const GrFragmentProcessor* childProcessor(int index) const {
        return fChildProcessors[index].get();
    }

    SkDEBUGCODE(bool isInstantiated() const;)

    /**
     * Does this FP require local coordinates to be produced by the primitive processor? This only
     * returns true if this FP will directly read those local coordinates. FPs that are sampled
     * explicitly do not require primitive-generated local coordinates (because the sample
     * coordinates are supplied by the parent FP).
     *
     * If the root of an FP tree does not provide explicit coordinates, the geometry processor
     * provides the original local coordinates to start. This may be implicit as part of vertex
     * shader-lifted varyings, or by providing the base local coordinate to the fragment shader.
     */
    bool usesVaryingCoordsDirectly() const {
        return SkToBool(fFlags & kUsesSampleCoordsDirectly_Flag) &&
               !SkToBool(fFlags & kSampledWithExplicitCoords_Flag);
    }

    /**
     * Do any of the FPs in this tree require local coordinates to be produced by the primitive
     * processor? This can return true even if this FP does not refer to sample coordinates, but
     * true if a descendant FP uses them.
     */
    bool usesVaryingCoords() const {
        return (SkToBool(fFlags & kUsesSampleCoordsDirectly_Flag) ||
                SkToBool(fFlags & kUsesSampleCoordsIndirectly_Flag)) &&
               !SkToBool(fFlags & kSampledWithExplicitCoords_Flag);
    }

    /** Do any of the FPs in this tree read back the color from the destination surface? */
    bool willReadDstColor() const {
        return SkToBool(fFlags & kWillReadDstColor_Flag);
    }

    /** Does the SkSL for this FP take two colors as its input arguments? */
    bool isBlendFunction() const {
        return SkToBool(fFlags & kIsBlendFunction_Flag);
    }

   /**
     * True if this FP refers directly to the sample coordinate parameter of its function
     * (e.g. uses EmitArgs::fSampleCoord in emitCode()). This also returns true if the
     * coordinate reference comes from autogenerated code invoking 'sample(matrix)' expressions.
     *
     * Unlike usesVaryingCoords(), this can return true whether or not the FP is explicitly
     * sampled, and does not change based on how the FP is composed. This property is specific to
     * the FP's function and not the entire program.
     */
    bool referencesSampleCoords() const {
        return SkToBool(fFlags & kUsesSampleCoordsDirectly_Flag);
    }

    // True if this FP's parent invokes it with 'sample(float2)'
    bool isSampledWithExplicitCoords() const {
        return SkToBool(fFlags & kSampledWithExplicitCoords_Flag);
    }

    // The SampleUsage describing how this FP is invoked by its parent using 'sample(matrix)'
    // This only reflects the immediate sampling from parent to this FP
    const SkSL::SampleUsage& sampleUsage() const {
        return fUsage;
    }

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

    void visitProxies(const GrVisitProxyFunc&) const;

    void visitTextureEffects(const std::function<void(const GrTextureEffect&)>&) const;

    GrTextureEffect* asTextureEffect();
    const GrTextureEffect* asTextureEffect() const;

#if GR_TEST_UTILS
    // Generates debug info for this processor tree by recursively calling dumpInfo() on this
    // processor and its children.
    SkString dumpTreeInfo() const;
#endif

    // A pre-order traversal iterator over a hierarchy of FPs. It can also iterate over all the FP
    // hierarchies rooted in a GrPaint, GrProcessorSet, or GrPipeline. For these collections it
    // iterates the tree rooted at each color FP and then each coverage FP.
    //
    // An iterator is constructed from one of the srcs and used like this:
    //   for (GrFragmentProcessor::Iter iter(pipeline); iter; ++iter) {
    //       GrFragmentProcessor& fp = *iter;
    //   }
    // The exit test for the loop is using CIter's operator bool().
    // To use a range-for loop instead see CIterRange below.
    class CIter;

    // Used to implement a range-for loop using CIter. Src is one of GrFragmentProcessor,
    // GrPaint, GrProcessorSet, or GrPipeline. Type aliases for these defined below.
    // Example usage:
    //   for (const auto& fp : GrFragmentProcessor::PaintRange(paint)) {
    //       if (fp.usesLocalCoords()) {
    //       ...
    //       }
    //   }
    template <typename Src> class CIterRange;

    // We would use template deduction guides for CIter but for:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501
    // Instead we use these specialized type aliases to make it prettier
    // to construct CIters for particular sources of FPs.
    using FPRange = CIterRange<GrFragmentProcessor>;
    using PaintRange = CIterRange<GrPaint>;

    // Sentinel type for range-for using CIter.
    class EndCIter {};

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
     * value read from a texture of the passed color type and that the texture contains
     * premultiplied color or alpha values that are in range.
     *
     * Since there are multiple ways in which a sampler may have its coordinates clamped or wrapped,
     * callers must determine on their own if the sampling uses a decal strategy in any way, in
     * which case the texture may become transparent regardless of the color type.
     */
    static OptimizationFlags ModulateForSamplerOptFlags(SkAlphaType alphaType, bool samplingDecal) {
        if (samplingDecal) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        } else {
            return ModulateForClampedSamplerOptFlags(alphaType);
        }
    }

    // As above, but callers should somehow ensure or assert their sampler still uses clamping
    static OptimizationFlags ModulateForClampedSamplerOptFlags(SkAlphaType alphaType) {
        if (alphaType == kOpaque_SkAlphaType) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                   kPreservesOpaqueInput_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }

    GrFragmentProcessor(ClassID classID, OptimizationFlags optimizationFlags)
            : INHERITED(classID), fFlags(optimizationFlags) {
        SkASSERT((optimizationFlags & ~kAll_OptimizationFlags) == 0);
    }

    OptimizationFlags optimizationFlags() const {
        return static_cast<OptimizationFlags>(kAll_OptimizationFlags & fFlags);
    }

    /** Useful when you can't call fp->optimizationFlags() on a base class object from a subclass.*/
    static OptimizationFlags ProcessorOptimizationFlags(const GrFragmentProcessor* fp) {
        return fp ? fp->optimizationFlags() : kAll_OptimizationFlags;
    }

    /**
     * This allows one subclass to access another subclass's implementation of
     * constantOutputForConstantInput. It must only be called when
     * hasConstantOutputForConstantInput() is known to be true.
     */
    static SkPMColor4f ConstantOutputForConstantInput(const GrFragmentProcessor* fp,
                                                      const SkPMColor4f& input) {
        if (fp) {
            SkASSERT(fp->hasConstantOutputForConstantInput());
            return fp->constantOutputForConstantInput(input);
        } else {
            return input;
        }
    }

    /**
     * FragmentProcessor subclasses call this from their constructor to register any child
     * FragmentProcessors they have. This must be called AFTER all texture accesses and coord
     * transforms have been added.
     * This is for processors whose shader code will be composed of nested processors whose output
     * colors will be combined somehow to produce its output color. Registering these child
     * processors will allow the ProgramBuilder to automatically handle their transformed coords and
     * texture accesses and mangle their uniform and output color names.
     *
     * The SampleUsage parameter describes all of the ways that the child is sampled by the parent.
     */
    void registerChild(std::unique_ptr<GrFragmentProcessor> child,
                       SkSL::SampleUsage sampleUsage = SkSL::SampleUsage::PassThrough());

    /**
     * This method takes an existing fragment processor, clones all of its children, and registers
     * the clones as children of this fragment processor.
     */
    void cloneAndRegisterAllChildProcessors(const GrFragmentProcessor& src);

    // FP implementations must call this function if their matching GrGLSLFragmentProcessor's
    // emitCode() function uses the EmitArgs::fSampleCoord variable in generated SkSL.
    void setUsesSampleCoordsDirectly() {
        fFlags |= kUsesSampleCoordsDirectly_Flag;
    }

    // FP implementations must set this flag if their GrGLSLFragmentProcessor's emitCode() function
    // calls dstColor() to read back the framebuffer.
    void setWillReadDstColor() {
        fFlags |= kWillReadDstColor_Flag;
    }

    // FP implementations must set this flag if their GrGLSLFragmentProcessor's emitCode() function
    // emits a blend function (taking two color inputs instead of just one).
    void setIsBlendFunction() {
        fFlags |= kIsBlendFunction_Flag;
    }

    void mergeOptimizationFlags(OptimizationFlags flags) {
        SkASSERT((flags & ~kAll_OptimizationFlags) == 0);
        fFlags &= (flags | ~kAll_OptimizationFlags);
    }

private:
    virtual SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& /* inputColor */) const {
        SK_ABORT("Subclass must override this if advertising this optimization.");
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrFragmentProcessor; caller is responsible for deleting
        the object. */
    virtual std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const = 0;

    /** Implemented using GLFragmentProcessor::GenKey as described in this class's comment. */
    virtual void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;

    /**
     * Subclass implements this to support isEqual(). It will only be called if it is known that
     * the two processors are of the same subclass (i.e. they return the same object from
     * getFactory()).
     */
    virtual bool onIsEqual(const GrFragmentProcessor&) const = 0;

    enum PrivateFlags {
        kFirstPrivateFlag = kAll_OptimizationFlags + 1,

        // Propagates up the FP tree to the root
        kUsesSampleCoordsIndirectly_Flag = kFirstPrivateFlag,

        // Does not propagate at all
        kUsesSampleCoordsDirectly_Flag = kFirstPrivateFlag << 1,
        kIsBlendFunction_Flag = kFirstPrivateFlag << 2,

        // Propagates down the FP to all its leaves
        kSampledWithExplicitCoords_Flag = kFirstPrivateFlag << 3,

        // Propagates up the FP tree to the root
        kWillReadDstColor_Flag = kFirstPrivateFlag << 5,
    };
    void addAndPushFlagToChildren(PrivateFlags flag);

    SkSTArray<1, std::unique_ptr<GrFragmentProcessor>, true> fChildProcessors;
    const GrFragmentProcessor* fParent = nullptr;
    uint32_t fFlags = 0;
    SkSL::SampleUsage fUsage;

    using INHERITED = GrProcessor;
};

//////////////////////////////////////////////////////////////////////////////

GR_MAKE_BITFIELD_OPS(GrFragmentProcessor::OptimizationFlags)

//////////////////////////////////////////////////////////////////////////////

class GrFragmentProcessor::CIter {
public:
    explicit CIter(const GrFragmentProcessor& fp) { fFPStack.push_back(&fp); }
    explicit CIter(const GrPaint&);
    explicit CIter(const GrPipeline&);

    const GrFragmentProcessor& operator*() const  { return *fFPStack.back(); }
    const GrFragmentProcessor* operator->() const { return fFPStack.back(); }

    CIter& operator++();

    operator bool() const { return !fFPStack.empty(); }

    bool operator!=(const EndCIter&) { return (bool)*this; }

    // Hopefully this does not actually get called because of RVO.
    CIter(const CIter&) = default;

    CIter(CIter&&) = default;

    // Because each iterator carries a stack we want to avoid copies.
    CIter& operator=(const CIter&) = delete;

    CIter& operator=(CIter&&) = default;

protected:
    CIter() = delete;

    SkSTArray<4, const GrFragmentProcessor*, true> fFPStack;
};

//////////////////////////////////////////////////////////////////////////////

template <typename Src> class GrFragmentProcessor::CIterRange {
public:
    explicit CIterRange(const Src& t) : fT(t) {}
    CIter begin() const { return CIter(fT); }
    EndCIter end() const { return EndCIter(); }

private:
    const Src& fT;
};

static inline GrFPResult GrFPFailure(std::unique_ptr<GrFragmentProcessor> fp) {
    return {false, std::move(fp)};
}
static inline GrFPResult GrFPSuccess(std::unique_ptr<GrFragmentProcessor> fp) {
    SkASSERT(fp);
    return {true, std::move(fp)};
}

#endif
