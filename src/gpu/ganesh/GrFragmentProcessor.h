/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessor_DEFINED
#define GrFragmentProcessor_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkString.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>

class GrGLSLFPFragmentBuilder;
class GrGLSLProgramDataManager;
struct GrShaderCaps;
class GrTextureEffect;
struct SkPoint;
struct SkRect;

namespace skgpu {
class KeyBuilder;
class Swizzle;
}

/**
 * Some fragment-processor creation methods have preconditions that might not be satisfied by the
 * calling code. Those methods can return a `GrFPResult` from their factory methods. If creation
 * succeeds, the new fragment processor is created and `success` is true. If a precondition is not
 * met, `success` is set to false and the input FP is returned unchanged.
 */
class GrFragmentProcessor;  // IWYU pragma: keep
using GrFPResult = std::tuple<bool /*success*/, std::unique_ptr<GrFragmentProcessor>>;

/** Provides custom fragment shader code. Fragment processors receive an input position and
    produce an output color. They may contain uniforms and may have children fragment processors
    that are sampled.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    /**
     * Every GrFragmentProcessor must be capable of creating a subclass of ProgramImpl. The
     * ProgramImpl emits the fragment shader code that implements the GrFragmentProcessor, is
     * attached to the generated backend API pipeline/program and used to extract uniform data from
     * GrFragmentProcessor instances.
     */
    class ProgramImpl;

    /** Always returns 'color'. */
    static std::unique_ptr<GrFragmentProcessor> MakeColor(SkPMColor4f color);

    /**
     *  Returns the input color, modulated by the child's alpha.
     *
     *  output = input * child.a
     */
    static std::unique_ptr<GrFragmentProcessor> MulInputByChildAlpha(
            std::unique_ptr<GrFragmentProcessor> child);

    /**
     *  Invokes child with an opaque version of the input color, then applies the input alpha to
     *  the result. Used to incorporate paint alpha to the evaluation of an SkShader tree FP.
     */
    static std::unique_ptr<GrFragmentProcessor> ApplyPaintAlpha(
            std::unique_ptr<GrFragmentProcessor> child);

    /**
     *  Returns a fragment processor that generates the passed-in color, modulated by the child's
     *  RGBA color. The child's input color will be the parent's fInputColor. (Pass a null FP to use
     *  the color from fInputColor instead of a child FP.)
     */
    static std::unique_ptr<GrFragmentProcessor> ModulateRGBA(
            std::unique_ptr<GrFragmentProcessor> child, const SkPMColor4f& color);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will ignore its input color and instead feed the passed in color as input to the
     *  child.
     */
    static std::unique_ptr<GrFragmentProcessor> OverrideInput(std::unique_ptr<GrFragmentProcessor>,
                                                              const SkPMColor4f&);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will simply return the child's color, but disable the coverage-as-alpha
     *  optimization.
     */
    static std::unique_ptr<GrFragmentProcessor> DisableCoverageAsAlpha(
            std::unique_ptr<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor which returns `args.fDestColor`. This is only meaningful in
     *  contexts like blenders, which use a source and dest color.)
     */
    static std::unique_ptr<GrFragmentProcessor> DestColor();

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then swizzles
     *  the output.
     */
    static std::unique_ptr<GrFragmentProcessor> SwizzleOutput(std::unique_ptr<GrFragmentProcessor>,
                                                              const skgpu::Swizzle&);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then clamps
     *  the output to [0, 1].
     */
    static std::unique_ptr<GrFragmentProcessor> ClampOutput(std::unique_ptr<GrFragmentProcessor>);

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

    std::unique_ptr<ProgramImpl> makeProgramImpl() const;

    void addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
        this->onAddToKey(caps, b);
        for (const auto& child : fChildProcessors) {
            if (child) {
                child->addToKey(caps, b);
            }
        }
    }

    int numChildProcessors() const { return fChildProcessors.size(); }
    int numNonNullChildProcessors() const;

    GrFragmentProcessor* childProcessor(int index) { return fChildProcessors[index].get(); }
    const GrFragmentProcessor* childProcessor(int index) const {
        return fChildProcessors[index].get();
    }

    SkDEBUGCODE(bool isInstantiated() const;)

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
     * (e.g. uses EmitArgs::fSampleCoord in emitCode()). This is decided at FP-tree construction
     * time and is not affected by lifting coords to varyings.
     */
    bool usesSampleCoordsDirectly() const {
        return SkToBool(fFlags & kUsesSampleCoordsDirectly_Flag);
    }

    /**
     * True if this FP uses its input coordinates or if any descendant FP uses them through a chain
     * of non-explicit sample usages. (e.g. uses EmitArgs::fSampleCoord in emitCode()). This is
     * decided at FP-tree construction time and is not affected by lifting coords to varyings.
     */
    bool usesSampleCoords() const {
        return SkToBool(fFlags & (kUsesSampleCoordsDirectly_Flag |
                                  kUsesSampleCoordsIndirectly_Flag));
    }

    // The SampleUsage describing how this FP is invoked by its parent. This only reflects the
    // immediate sampling from parent to this FP.
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

    void clearConstantOutputForConstantInputFlag() {
        fFlags &= ~kConstantOutputForConstantInput_OptimizationFlag;
    }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use addToKey.
     */
    bool isEqual(const GrFragmentProcessor& that) const;

    void visitProxies(const GrVisitProxyFunc&) const;

    void visitTextureEffects(const std::function<void(const GrTextureEffect&)>&) const;

    void visitWithImpls(const std::function<void(const GrFragmentProcessor&, ProgramImpl&)>&,
                        ProgramImpl&) const;

    GrTextureEffect* asTextureEffect();
    const GrTextureEffect* asTextureEffect() const;

#if defined(GPU_TEST_UTILS)
    // Generates debug info for this processor tree by recursively calling dumpInfo() on this
    // processor and its children.
    SkString dumpTreeInfo() const;
#endif

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
    SK_DECL_BITFIELD_OPS_FRIENDS(OptimizationFlags)

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

    explicit GrFragmentProcessor(const GrFragmentProcessor& src)
            : INHERITED(src.classID()), fFlags(src.fFlags) {
        this->cloneAndRegisterAllChildProcessors(src);
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

    // FP implementations must call this function if their matching ProgramImpl's emitCode()
    // function uses the EmitArgs::fSampleCoord variable in generated SkSL.
    void setUsesSampleCoordsDirectly() {
        fFlags |= kUsesSampleCoordsDirectly_Flag;
    }

    // FP implementations must set this flag if their ProgramImpl's emitCode() function calls
    // dstColor() to read back the framebuffer.
    void setWillReadDstColor() {
        fFlags |= kWillReadDstColor_Flag;
    }

    // FP implementations must set this flag if their ProgramImpl's emitCode() function emits a
    // blend function (taking two color inputs instead of just one).
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

    /**
     * Returns a new instance of the appropriate ProgramImpl subclass for the given
     * GrFragmentProcessor. It will emit the appropriate code and live with the cached program
     * to setup uniform data for each draw that uses the program.
     */
    virtual std::unique_ptr<ProgramImpl> onMakeProgramImpl() const = 0;

    virtual void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const = 0;

    /**
     * Subclass implements this to support isEqual(). It will only be called if it is known that
     * the two processors are of the same subclass (i.e. have the same ClassID).
     */
    virtual bool onIsEqual(const GrFragmentProcessor&) const = 0;

    enum PrivateFlags {
        kFirstPrivateFlag = kAll_OptimizationFlags + 1,

        // Propagates up the FP tree to either root or first explicit sample usage.
        kUsesSampleCoordsIndirectly_Flag = kFirstPrivateFlag,

        // Does not propagate at all. It means this FP uses its input sample coords in some way.
        // Note passthrough and matrix sampling of children don't count as a usage of the coords.
        // Because indirect sampling stops at an explicit sample usage it is imperative that a FP
        // that calculates explicit coords for its children using its own sample coords sets this.
        kUsesSampleCoordsDirectly_Flag = kFirstPrivateFlag << 1,

        // Does not propagate at all.
        kIsBlendFunction_Flag = kFirstPrivateFlag << 2,

        // Propagates up the FP tree to the root.
        kWillReadDstColor_Flag = kFirstPrivateFlag << 3,
    };

    skia_private::STArray<1, std::unique_ptr<GrFragmentProcessor>, true> fChildProcessors;
    const GrFragmentProcessor* fParent = nullptr;
    uint32_t fFlags = 0;
    SkSL::SampleUsage fUsage;

    using INHERITED = GrProcessor;
};

//////////////////////////////////////////////////////////////////////////////

class GrFragmentProcessor::ProgramImpl {
public:
    ProgramImpl() = default;

    virtual ~ProgramImpl() = default;

    using UniformHandle = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle = GrGLSLUniformHandler::SamplerHandle;

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param fragBuilder       Interface used to emit code in the shaders.
        @param uniformHandler    Interface used for accessing information about our uniforms
        @param caps              The capabilities of the GPU which will render this FP
        @param fp                The processor that generated this program stage.
        @param inputColor        A half4 that holds the input color to the stage in the FS (or the
                                 source color, for blend processors). nullptr inputs are converted
                                 to "half4(1.0)" (solid white) during construction.
                                 TODO: Better system for communicating optimization info
                                 (e.g. input color is solid white, trans black, known to be opaque,
                                 etc.) that allows the processor to communicate back similar known
                                 info about its output.
        @param destColor         A half4 that holds the dest color to the stage. Only meaningful
                                 when the "is blend processor" FP flag is set.
        @param sampleCoord       The name of a local coord reference to a float2 variable. Only
                                 meaningful when the "references sample coords" FP flag is set.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* inputColor,
                 const char* destColor,
                 const char* sampleCoord)
                : fFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fFp(fp)
                , fInputColor(inputColor ? inputColor : "half4(1.0)")
                , fDestColor(destColor)
                , fSampleCoord(sampleCoord) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrFragmentProcessor& fFp;
        const char* fInputColor;
        const char* fDestColor;
        const char* fSampleCoord;
    };

    virtual void emitCode(EmitArgs&) = 0;

    // This does not recurse to any attached child processors. Recursing the entire processor tree
    // is the responsibility of the caller.
    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    int numChildProcessors() const { return fChildProcessors.size(); }

    ProgramImpl* childProcessor(int index) const { return fChildProcessors[index].get(); }

    void setFunctionName(SkString name) {
        SkASSERT(fFunctionName.isEmpty());
        fFunctionName = std::move(name);
    }

    const char* functionName() const {
        SkASSERT(!fFunctionName.isEmpty());
        return fFunctionName.c_str();
    }

    // Invoke the child with the default input and destination colors (solid white)
    inline SkString invokeChild(int childIndex,
                                EmitArgs& parentArgs,
                                std::string_view skslCoords = {}) {
        return this->invokeChild(childIndex,
                                 /*inputColor=*/nullptr,
                                 /*destColor=*/nullptr,
                                 parentArgs,
                                 skslCoords);
    }

    inline SkString invokeChildWithMatrix(int childIndex, EmitArgs& parentArgs) {
        return this->invokeChildWithMatrix(childIndex,
                                           /*inputColor=*/nullptr,
                                           /*destColor=*/nullptr,
                                           parentArgs);
    }

    // Invoke the child with the default destination color (solid white)
    inline SkString invokeChild(int childIndex,
                                const char* inputColor,
                                EmitArgs& parentArgs,
                                std::string_view skslCoords = {}) {
        return this->invokeChild(childIndex,
                                 inputColor,
                                 /*destColor=*/nullptr,
                                 parentArgs,
                                 skslCoords);
    }

    inline SkString invokeChildWithMatrix(int childIndex,
                                          const char* inputColor,
                                          EmitArgs& parentArgs) {
        return this->invokeChildWithMatrix(childIndex,
                                           inputColor,
                                           /*destColor=*/nullptr,
                                           parentArgs);
    }

    /** Invokes a child proc in its own scope. Pass in the parent's EmitArgs and invokeChild will
     *  automatically extract the coords and samplers of that child and pass them on to the child's
     *  emitCode(). Also, any uniforms or functions emitted by the child will have their names
     *  mangled to prevent redefinitions. The returned string contains the output color (as a call
     *  to the child's helper function). It is legal to pass nullptr as inputColor, since all
     *  fragment processors are required to work without an input color.
     *
     *  When skslCoords is empty, the child is invoked at the sample coordinates from parentArgs.
     *  When skslCoords is not empty, is must be an SkSL expression that evaluates to a float2.
     *  That expression is passed to the child's processor function as the "_coords" argument.
     */
    SkString invokeChild(int childIndex,
                         const char* inputColor,
                         const char* destColor,
                         EmitArgs& parentArgs,
                         std::string_view skslCoords = {});

    /**
     * As invokeChild, but transforms the coordinates according to the matrix expression attached
     * to the child's SampleUsage object. This is only valid if the child is sampled with a
     * const-uniform matrix.
     */
    SkString invokeChildWithMatrix(int childIndex,
                                   const char* inputColor,
                                   const char* destColor,
                                   EmitArgs& parentArgs);

    /**
     * Pre-order traversal of a GLSLFP hierarchy, or of multiple trees with roots in an array of
     * GLSLFPS. If initialized with an array color followed by coverage processors installed in a
     * program thenthe iteration order will agree with a GrFragmentProcessor::Iter initialized with
     * a GrPipeline that produces the same program key.
     */
    class Iter {
    public:
        Iter(std::unique_ptr<ProgramImpl> fps[], int cnt);
        Iter(ProgramImpl& fp) { fFPStack.push_back(&fp); }

        ProgramImpl& operator*() const;
        ProgramImpl* operator->() const;
        Iter& operator++();
        explicit operator bool() const { return !fFPStack.empty(); }

        // Because each iterator carries a stack we want to avoid copies.
        Iter(const Iter&) = delete;
        Iter& operator=(const Iter&) = delete;

    private:
        skia_private::STArray<4, ProgramImpl*, true> fFPStack;
    };

private:
    /**
     * A ProgramImpl instance can be reused with any GrFragmentProcessor that produces the same
     * the same key; this function reads data from a GrFragmentProcessor and uploads any
     * uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
     * parameter is guaranteed to be of the same type that created this ProgramImpl and
     * to have an identical key as the one that created this ProgramImpl.
     */
    virtual void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) {}

    // The (mangled) name of our entry-point function
    SkString fFunctionName;

    skia_private::TArray<std::unique_ptr<ProgramImpl>, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

//////////////////////////////////////////////////////////////////////////////

SK_MAKE_BITFIELD_OPS(GrFragmentProcessor::OptimizationFlags)

static inline GrFPResult GrFPFailure(std::unique_ptr<GrFragmentProcessor> fp) {
    return {false, std::move(fp)};
}
static inline GrFPResult GrFPSuccess(std::unique_ptr<GrFragmentProcessor> fp) {
    SkASSERT(fp);
    return {true, std::move(fp)};
}
// Equivalent to GrFPSuccess except it allows the returned fragment processor to be null.
static inline GrFPResult GrFPNullableSuccess(std::unique_ptr<GrFragmentProcessor> fp) {
    return {true, std::move(fp)};
}

#endif
