/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessor_DEFINED
#define GrFragmentProcessor_DEFINED

#include "GrProcessor.h"

class GrCoordTransform;
class GrGLSLFragmentProcessor;
class GrInvariantOutput;
class GrPipeline;
class GrProcessorKeyBuilder;
class GrShaderCaps;
class GrSwizzle;

/** Provides custom fragment shader code. Fragment processors receive an input color (vec4f) and
    produce an output color. They may reference textures and uniforms. They may use
    GrCoordTransforms to receive a transformation of the local coordinates that map from local space
    to the fragment being processed.
 */
class GrFragmentProcessor : public GrResourceIOProcessor, public GrProgramElement {
public:
    /**
    *  In many instances (e.g. SkShader::asFragmentProcessor() implementations) it is desirable to
    *  only consider the input color's alpha. However, there is a competing desire to have reusable
    *  GrFragmentProcessor subclasses that can be used in other scenarios where the entire input
    *  color is considered. This function exists to filter the input color and pass it to a FP. It
    *  does so by returning a parent FP that multiplies the passed in FPs output by the parent's
    *  input alpha. The passed in FP will not receive an input color.
    */
    static sk_sp<GrFragmentProcessor> MulOutputByInputAlpha(sk_sp<GrFragmentProcessor>);

    /**
     *  This assumes that the input color to the returned processor will be unpremul and that the
     *  passed processor (which becomes the returned processor's child) produces a premul output.
     *  The result of the returned processor is a premul of its input color modulated by the child
     *  processor's premul output.
     */
    static sk_sp<GrFragmentProcessor> MakeInputPremulAndMulByOutput(sk_sp<GrFragmentProcessor>);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will ignore its input color and instead feed the passed in color as input to the
     *  child.
     */
    static sk_sp<GrFragmentProcessor> OverrideInput(sk_sp<GrFragmentProcessor>, GrColor4f);

    /**
     *  Returns a fragment processor that premuls the input before calling the passed in fragment
     *  processor.
     */
    static sk_sp<GrFragmentProcessor> PremulInput(sk_sp<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then premuls
     *  the output.
     */
    static sk_sp<GrFragmentProcessor> PremulOutput(sk_sp<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then unpremuls
     *  the output.
     */
    static sk_sp<GrFragmentProcessor> UnpremulOutput(sk_sp<GrFragmentProcessor>);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then swizzles
     *  the output.
     */
    static sk_sp<GrFragmentProcessor> SwizzleOutput(sk_sp<GrFragmentProcessor>, const GrSwizzle&);

    /**
     * Returns a fragment processor that runs the passed in array of fragment processors in a
     * series. The original input is passed to the first, the first's output is passed to the
     * second, etc. The output of the returned processor is the output of the last processor of the
     * series.
     *
     * The array elements with be moved.
     */
    static sk_sp<GrFragmentProcessor> RunInSeries(sk_sp<GrFragmentProcessor>*, int cnt);

    ~GrFragmentProcessor() override;

    GrGLSLFragmentProcessor* createGLSLInstance() const;

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
        this->onGetGLSLProcessorKey(caps, b);
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            fChildProcessors[i]->getGLSLProcessorKey(caps, b);
        }
    }

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
    bool hasConstantOutputForConstantInput(GrColor4f inputColor, GrColor4f* outputColor) const {
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
        const GrFragmentProcessor* next();

    private:
        SkSTArray<4, const GrFragmentProcessor*, true> fFPStack;
    };

    /**
     * Iterates over all the Ts owned by a GrFragmentProcessor and its children or over all the Ts
     * owned by the forest of GrFragmentProcessors in a GrPipeline. FPs are visited in the same
     * order as Iter and each of an FP's Ts are visited in order.
     */
    template <typename T, typename BASE,
              int (BASE::*COUNT)() const,
              const T& (BASE::*GET)(int) const>
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
                                          GrFragmentProcessor,
                                          &GrFragmentProcessor::numCoordTransforms,
                                          &GrFragmentProcessor::coordTransform>;

    using TextureAccessIter = FPItemIter<TextureSampler,
                                         GrResourceIOProcessor,
                                         &GrResourceIOProcessor::numTextureSamplers,
                                         &GrResourceIOProcessor::textureSampler>;

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

    GrFragmentProcessor(OptimizationFlags optimizationFlags) : fFlags(optimizationFlags) {
        SkASSERT((fFlags & ~kAll_OptimizationFlags) == 0);
    }

    OptimizationFlags optimizationFlags() const {
        return static_cast<OptimizationFlags>(kAll_OptimizationFlags & fFlags);
    }

    /**
     * This allows one subclass to access another subclass's implementation of
     * constantOutputForConstantInput. It must only be called when
     * hasConstantOutputForConstantInput() is known to be true.
     */
    static GrColor4f ConstantOutputForConstantInput(const GrFragmentProcessor& fp,
                                                    GrColor4f input) {
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
    int registerChildProcessor(sk_sp<GrFragmentProcessor> child);

private:
    void addPendingIOs() const override { GrResourceIOProcessor::addPendingIOs(); }
    void removeRefs() const override { GrResourceIOProcessor::removeRefs(); }
    void pendingIOComplete() const override { GrResourceIOProcessor::pendingIOComplete(); }

    void notifyRefCntIsZero() const final;

    virtual GrColor4f constantOutputForConstantInput(GrColor4f /* inputColor */) const {
        SkFAIL("Subclass must override this if advertising this optimization.");
        return GrColor4f::TransparentBlack();
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

    bool hasSameTransforms(const GrFragmentProcessor&) const;

    enum PrivateFlags {
        kFirstPrivateFlag = kAll_OptimizationFlags + 1,
        kUsesLocalCoords_Flag = kFirstPrivateFlag,
    };

    mutable uint32_t fFlags = 0;

    SkSTArray<4, const GrCoordTransform*, true> fCoordTransforms;

    /**
     * This is not SkSTArray<1, sk_sp<GrFragmentProcessor>> because this class holds strong
     * references until notifyRefCntIsZero and then it holds pending executions.
     */
    SkSTArray<1, GrFragmentProcessor*, true> fChildProcessors;

    typedef GrResourceIOProcessor INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrFragmentProcessor::OptimizationFlags)

#endif
