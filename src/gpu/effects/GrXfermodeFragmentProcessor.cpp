/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"

#include "src/core/SkXfermodePriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

// Some of the cpu implementations of blend modes differ too much from the GPU enough that
// we can't use the cpu implementation to implement constantOutputForConstantInput.
static inline bool does_cpu_blend_impl_match_gpu(SkBlendMode mode) {
    // The non-seperable modes differ too much. So does SoftLight. ColorBurn differs too much on our
    // test iOS device (but we just disable it across the aboard since it may happen on untested
    // GPUs).
    return mode <= SkBlendMode::kLastSeparableMode && mode != SkBlendMode::kSoftLight &&
           mode != SkBlendMode::kColorBurn;
}

//////////////////////////////////////////////////////////////////////////////

class ComposeTwoFragmentProcessor : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     std::unique_ptr<GrFragmentProcessor> src,
                                                     std::unique_ptr<GrFragmentProcessor> dst,
                                                     SkBlendMode mode) {
        return std::unique_ptr<GrFragmentProcessor>(new ComposeTwoFragmentProcessor(
                   std::move(inputFP), std::move(src), std::move(dst), mode));
    }

    const char* name() const override { return "ComposeTwo"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;

        str.appendf("Mode: %s", SkBlendMode_Name(fMode));

        for (int i = 0; i < this->numChildProcessors(); ++i) {
            str.appendf(" [%s %s]",
                        this->childProcessor(i).name(), this->childProcessor(i).dumpInfo().c_str());
        }
        return str;
    }
#endif

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    SkBlendMode getMode() const { return fMode; }

    bool hasInputFP() const {
        // The first two child processors will be the src and dest FPs.
        // The third, if it exists, is the input FP. (If not, the input will be sk_InColor.)
        return this->numChildProcessors() > 2;
    }

    static constexpr int kSrcFPIndex = 0;
    static constexpr int kDstFPIndex = 1;
    static constexpr int kInputFPIndex = 2;

private:
    ComposeTwoFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                std::unique_ptr<GrFragmentProcessor> src,
                                std::unique_ptr<GrFragmentProcessor> dst,
                                SkBlendMode mode)
            : INHERITED(kComposeTwoFragmentProcessor_ClassID,
                        OptFlags(inputFP.get(), src.get(), dst.get(), mode))
            , fMode(mode) {
        this->registerChild(std::move(src));
        this->registerChild(std::move(dst));
        if (inputFP) {
            // The input FP is optional, and will land at index 2 if provided.
            this->registerChild(std::move(inputFP));
        }
    }

    ComposeTwoFragmentProcessor(const ComposeTwoFragmentProcessor& that)
            : INHERITED(kComposeTwoFragmentProcessor_ClassID, that.optimizationFlags())
            , fMode(that.fMode) {
        this->cloneAndRegisterAllChildProcessors(that);
    }

    static OptimizationFlags OptFlags(const GrFragmentProcessor* inputFP,
                                      const GrFragmentProcessor* src,
                                      const GrFragmentProcessor* dst,
                                      SkBlendMode mode) {
        OptimizationFlags flags;
        switch (mode) {
            case SkBlendMode::kClear:
            case SkBlendMode::kSrc:
            case SkBlendMode::kDst:
                SK_ABORT("GrXfermodeFragmentProcessor::Make should have optimized this away.");
                flags = kNone_OptimizationFlags;
                break;

            // Produces opaque if both src and dst are opaque.
            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn:
            case SkBlendMode::kModulate:
                flags = src->preservesOpaqueInput() && dst->preservesOpaqueInput()
                                ? kPreservesOpaqueInput_OptimizationFlag
                                : kNone_OptimizationFlags;
                break;

            // Produces zero when both are opaque, indeterminate if one is opaque.
            case SkBlendMode::kSrcOut:
            case SkBlendMode::kDstOut:
            case SkBlendMode::kXor:
                flags = kNone_OptimizationFlags;
                break;

            // Is opaque if the dst is opaque.
            case SkBlendMode::kSrcATop:
                flags = dst->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                    : kNone_OptimizationFlags;
                break;

            // DstATop is the converse of kSrcATop. Screen is also opaque if the src is a opaque.
            case SkBlendMode::kDstATop:
            case SkBlendMode::kScreen:
                flags = src->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                    : kNone_OptimizationFlags;
                break;

            // These modes are all opaque if either src or dst is opaque. All the advanced modes
            // compute alpha as src-over.
            case SkBlendMode::kSrcOver:
            case SkBlendMode::kDstOver:
            case SkBlendMode::kPlus:
            case SkBlendMode::kOverlay:
            case SkBlendMode::kDarken:
            case SkBlendMode::kLighten:
            case SkBlendMode::kColorDodge:
            case SkBlendMode::kColorBurn:
            case SkBlendMode::kHardLight:
            case SkBlendMode::kSoftLight:
            case SkBlendMode::kDifference:
            case SkBlendMode::kExclusion:
            case SkBlendMode::kMultiply:
            case SkBlendMode::kHue:
            case SkBlendMode::kSaturation:
            case SkBlendMode::kColor:
            case SkBlendMode::kLuminosity:
                flags = src->preservesOpaqueInput() || dst->preservesOpaqueInput()
                                ? kPreservesOpaqueInput_OptimizationFlag
                                : kNone_OptimizationFlags;
                break;
        }
        if (does_cpu_blend_impl_match_gpu(mode) && src->hasConstantOutputForConstantInput() &&
            dst->hasConstantOutputForConstantInput()) {
            flags |= kConstantOutputForConstantInput_OptimizationFlag;
        }
        // Our ability to optimize is constrained by the input FP.
        OptimizationFlags inputFPFlags = inputFP ? ProcessorOptimizationFlags(inputFP)
                                                 : kAll_OptimizationFlags;
        return flags & inputFPFlags;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32((int)fMode);
    }

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ComposeTwoFragmentProcessor& cs = other.cast<ComposeTwoFragmentProcessor>();
        return fMode == cs.fMode;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& in) const override {
        SkPMColor4f inputColor = this->hasInputFP()
            ? ConstantOutputForConstantInput(this->childProcessor(kInputFPIndex), in)
            : in;
        SkPMColor4f opaqueInput = { inputColor.fR, inputColor.fG, inputColor.fB, 1 };
        SkPMColor4f src = ConstantOutputForConstantInput(this->childProcessor(kSrcFPIndex),
                                                         opaqueInput);
        SkPMColor4f dst = ConstantOutputForConstantInput(this->childProcessor(kDstFPIndex),
                                                         opaqueInput);
        SkPMColor4f res = SkBlendMode_Apply(fMode, src, dst);
        return res * inputColor.fA;
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkBlendMode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GLComposeTwoFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ComposeTwoFragmentProcessor);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> ComposeTwoFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    // Create two random frag procs.
    std::unique_ptr<GrFragmentProcessor> fpA(GrProcessorUnitTest::MakeChildFP(d));
    std::unique_ptr<GrFragmentProcessor> fpB(GrProcessorUnitTest::MakeChildFP(d));

    SkBlendMode mode;
    do {
        mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
    } while (SkBlendMode::kClear == mode || SkBlendMode::kSrc == mode || SkBlendMode::kDst == mode);
    return std::unique_ptr<GrFragmentProcessor>(new ComposeTwoFragmentProcessor(
               /*inputFP=*/nullptr, std::move(fpA), std::move(fpB), mode));
}
#endif

std::unique_ptr<GrFragmentProcessor> ComposeTwoFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new ComposeTwoFragmentProcessor(*this));
}

GrGLSLFragmentProcessor* ComposeTwoFragmentProcessor::onCreateGLSLInstance() const{
    return new GLComposeTwoFragmentProcessor;
}

/////////////////////////////////////////////////////////////////////

void GLComposeTwoFragmentProcessor::emitCode(EmitArgs& args) {

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const ComposeTwoFragmentProcessor& cs = args.fFp.cast<ComposeTwoFragmentProcessor>();
    fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkBlendMode_Name(cs.getMode()));

    // Assemble the input fragment from the input FP (or, in the absence of an FP, the input color).
    SkString inputFrag;
    if (cs.hasInputFP()) {
        inputFrag = this->invokeChild(cs.kInputFPIndex, args.fInputColor, args).c_str();
        fragBuilder->codeAppendf("half4 inputFrag = %s;\n", inputFrag.c_str());
    } else {
        inputFrag = args.fInputColor;
    }

    // Invoke each of the two children with the input fragment color, sans alpha.
    fragBuilder->codeAppendf("half4 inputSolid = %s.rgb1;\n", inputFrag.c_str());
    SkString srcColor = this->invokeChild(cs.kSrcFPIndex, "inputSolid", args);
    SkString dstColor = this->invokeChild(cs.kDstFPIndex, "inputSolid", args);

    // Blend the source and dest colors.
    GrGLSLBlend::AppendMode(fragBuilder,
                            srcColor.c_str(),
                            dstColor.c_str(),
                            args.fOutputColor,
                            cs.getMode());

    // Apply the input fragment's alpha to the output.
    fragBuilder->codeAppendf("%s *= %s.a;", args.fOutputColor, inputFrag.c_str());
}

//////////////////////////////////////////////////////////////////////////////

class ComposeOneFragmentProcessor : public GrFragmentProcessor {
public:
    enum Child {
        kDst_Child,
        kSrc_Child,
    };

    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     std::unique_ptr<GrFragmentProcessor> childFP,
                                                     SkBlendMode mode, Child child) {
        if (!childFP) {
            return nullptr;
        }
        return std::unique_ptr<GrFragmentProcessor>(new ComposeOneFragmentProcessor(
                   std::move(inputFP), std::move(childFP), mode, child));
    }

    const char* name() const override { return "ComposeOne"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;

        str.appendf("Mode: %s, Child: %s",
                    SkBlendMode_Name(fMode), kDst_Child == fChild ? "Dst" : "Src");

        for (int i = 0; i < this->numChildProcessors(); ++i) {
            str.appendf(" [%s %s]",
                        this->childProcessor(i).name(), this->childProcessor(i).dumpInfo().c_str());
        }
        return str;
    }
#endif

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    SkBlendMode mode() const { return fMode; }

    Child child() const { return fChild; }

    bool hasInputFP() const {
        // The first child processor will be the child (src or dest) FP.
        // The second, if it exists, is input FP. (If not, the input will be sk_InColor.)
        return this->numChildProcessors() > 1;
    }

    static constexpr int kChildFPIndex = 0;
    static constexpr int kInputFPIndex = 1;

private:
    OptimizationFlags OptFlags(const GrFragmentProcessor* inputFP,
                               const GrFragmentProcessor* childFP,
                               SkBlendMode mode, Child child) {
        OptimizationFlags flags;
        switch (mode) {
            case SkBlendMode::kClear:
            case SkBlendMode::kSrc:
            case SkBlendMode::kDst:
                SK_ABORT("GrXfermodeFragmentProcessor::Make should have optimized this away.");
                flags = kNone_OptimizationFlags;
                break;

            // Produces opaque if both src and dst are opaque. These also will modulate the child's
            // output by either the input color or alpha. However, if the child is not compatible
            // with the coverage as alpha then it may produce a color that is not valid premul.
            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn:
            case SkBlendMode::kModulate:
                flags = ProcessorOptimizationFlags(childFP) &
                        ~kConstantOutputForConstantInput_OptimizationFlag;
                break;

            // Produces zero when both are opaque, indeterminate if one is opaque.
            case SkBlendMode::kSrcOut:
            case SkBlendMode::kDstOut:
            case SkBlendMode::kXor:
                flags = kNone_OptimizationFlags;
                break;

            // Is opaque if the dst is opaque.
            case SkBlendMode::kSrcATop:
                if (child == kDst_Child) {
                    flags = childFP->preservesOpaqueInput()
                                 ? kPreservesOpaqueInput_OptimizationFlag
                                 : kNone_OptimizationFlags;
                } else {
                    flags = kPreservesOpaqueInput_OptimizationFlag;
                }
                break;

            // DstATop is the converse of kSrcATop. Screen is also opaque if the src is a opaque.
            case SkBlendMode::kDstATop:
            case SkBlendMode::kScreen:
                if (child == kSrc_Child) {
                    flags = childFP->preservesOpaqueInput()
                                 ? kPreservesOpaqueInput_OptimizationFlag
                                 : kNone_OptimizationFlags;
                } else {
                    flags = kPreservesOpaqueInput_OptimizationFlag;
                }
                break;

            // These modes are all opaque if either src or dst is opaque. All the advanced modes
            // compute alpha as src-over.
            case SkBlendMode::kSrcOver:
            case SkBlendMode::kDstOver:
            case SkBlendMode::kPlus:
            case SkBlendMode::kOverlay:
            case SkBlendMode::kDarken:
            case SkBlendMode::kLighten:
            case SkBlendMode::kColorDodge:
            case SkBlendMode::kColorBurn:
            case SkBlendMode::kHardLight:
            case SkBlendMode::kSoftLight:
            case SkBlendMode::kDifference:
            case SkBlendMode::kExclusion:
            case SkBlendMode::kMultiply:
            case SkBlendMode::kHue:
            case SkBlendMode::kSaturation:
            case SkBlendMode::kColor:
            case SkBlendMode::kLuminosity:
                flags = kPreservesOpaqueInput_OptimizationFlag;
                break;
        }
        if (does_cpu_blend_impl_match_gpu(mode) && childFP->hasConstantOutputForConstantInput()) {
            flags |= kConstantOutputForConstantInput_OptimizationFlag;
        }
        // Our ability to optimize is constrained by the input FP.
        OptimizationFlags inputFPFlags = inputFP ? ProcessorOptimizationFlags(inputFP)
                                                 : kAll_OptimizationFlags;
        return flags & inputFPFlags;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        static_assert(((int)SkBlendMode::kLastMode & UINT16_MAX) == (int)SkBlendMode::kLastMode);
        b->add32((int)fMode | (fChild << 16));
    }

    bool onIsEqual(const GrFragmentProcessor& that) const override {
        return fMode == that.cast<ComposeOneFragmentProcessor>().fMode;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& in) const override {
        SkPMColor4f inputColor = this->hasInputFP()
            ? ConstantOutputForConstantInput(this->childProcessor(kInputFPIndex), in)
            : in;
        SkPMColor4f childColor = ConstantOutputForConstantInput(
            this->childProcessor(kChildFPIndex), SK_PMColor4fWHITE);
        SkPMColor4f src, dst;
        if (kSrc_Child == fChild) {
            src = childColor;
            dst = inputColor;
        } else {
            src = inputColor;
            dst = childColor;
        }
        return SkBlendMode_Apply(fMode, src, dst);
    }

private:
    ComposeOneFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                std::unique_ptr<GrFragmentProcessor> childFP,
                                SkBlendMode mode, Child child)
            : INHERITED(kComposeOneFragmentProcessor_ClassID,
                        OptFlags(inputFP.get(), childFP.get(), mode, child))
            , fMode(mode)
            , fChild(child) {
        // The composed child FP should always land first, at index 0.
        this->registerChild(std::move(childFP));
        if (inputFP) {
            // The input FP is optional, and will land at index 1 if provided.
            this->registerChild(std::move(inputFP));
        }
    }

    ComposeOneFragmentProcessor(const ComposeOneFragmentProcessor& that)
            : INHERITED(kComposeOneFragmentProcessor_ClassID, that.optimizationFlags())
            , fMode(that.fMode)
            , fChild(that.fChild) {
        this->cloneAndRegisterAllChildProcessors(that);
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkBlendMode fMode;
    Child       fChild;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLComposeOneFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkBlendMode mode = args.fFp.cast<ComposeOneFragmentProcessor>().mode();
        const ComposeOneFragmentProcessor& processor = args.fFp.cast<ComposeOneFragmentProcessor>();
        ComposeOneFragmentProcessor::Child child = processor.child();

        SkString childColor = this->invokeChild(processor.kChildFPIndex, args);
        SkString inputColor = processor.hasInputFP()
            ? this->invokeChild(processor.kInputFPIndex, args.fInputColor, args)
            : SkString(args.fInputColor);

        if (ComposeOneFragmentProcessor::kDst_Child == child) {
            std::swap(childColor, inputColor);
        }

        // emit blend code
        fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkBlendMode_Name(mode));
        GrGLSLBlend::AppendMode(fragBuilder, childColor.c_str(), inputColor.c_str(),
                                args.fOutputColor, mode);
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ComposeOneFragmentProcessor);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> ComposeOneFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    // Create one random frag procs.
    // For now, we'll prevent either children from being a shader with children to prevent the
    // possibility of an arbitrarily large tree of procs.
    std::unique_ptr<GrFragmentProcessor> dst(GrProcessorUnitTest::MakeChildFP(d));
    SkBlendMode mode;
    ComposeOneFragmentProcessor::Child child;
    do {
        mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
        child = d->fRandom->nextBool() ? kDst_Child : kSrc_Child;
    } while (SkBlendMode::kClear == mode || (SkBlendMode::kDst == mode && child == kSrc_Child) ||
             (SkBlendMode::kSrc == mode && child == kDst_Child));
    return std::unique_ptr<GrFragmentProcessor>(
            new ComposeOneFragmentProcessor(/*inputFP=*/nullptr, std::move(dst), mode, child));
}
#endif

GrGLSLFragmentProcessor* ComposeOneFragmentProcessor::onCreateGLSLInstance() const {
    return new GLComposeOneFragmentProcessor;
}

std::unique_ptr<GrFragmentProcessor> ComposeOneFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new ComposeOneFragmentProcessor(*this));
}

//////////////////////////////////////////////////////////////////////////////

// It may seems as though when the mode is kDst that these factories could simply return the dest
// FP as-is (or same for src/kSrc). However, that doesn't have quite the same effect as the returned
// child FP will replace the FP's input with solid white and ignore the original input.

std::unique_ptr<GrFragmentProcessor> GrXfermodeFragmentProcessor::Make(
        std::unique_ptr<GrFragmentProcessor> inputFP,
        std::unique_ptr<GrFragmentProcessor> src,
        std::unique_ptr<GrFragmentProcessor> dst,
        SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:
            // The clear operation doesn't actually use the source or dest FPs.
            return GrConstColorProcessor::Make(std::move(inputFP), SK_PMColor4fTRANSPARENT,
                                               GrConstColorProcessor::InputMode::kIgnore);

        case SkBlendMode::kSrc:
            return GrFragmentProcessor::OverrideInput(std::move(src), SK_PMColor4fWHITE,
                                                      /*useUniform=*/false);

        case SkBlendMode::kDst:
            return GrFragmentProcessor::OverrideInput(std::move(dst), SK_PMColor4fWHITE,
                                                      /*useUniform=*/false);

        default:
            break;
    }

    if (dst == nullptr) {
        return ComposeOneFragmentProcessor::Make(std::move(inputFP), std::move(src), mode,
                                                 ComposeOneFragmentProcessor::kSrc_Child);
    }

    if (src == nullptr) {
        return ComposeOneFragmentProcessor::Make(std::move(inputFP), std::move(dst), mode,
                                                 ComposeOneFragmentProcessor::kDst_Child);
    }

    return ComposeTwoFragmentProcessor::Make(std::move(inputFP), std::move(src),
                                             std::move(dst), mode);
}
