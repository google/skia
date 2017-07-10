/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrXfermodeFragmentProcessor.h"

#include "GrConstColorProcessor.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"
#include "SkXfermodePriv.h"

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
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> src,
                                           sk_sp<GrFragmentProcessor> dst,
                                           SkBlendMode mode) {
        return sk_sp<GrFragmentProcessor>(new ComposeTwoFragmentProcessor(std::move(src),
                                                                          std::move(dst), mode));
    }

    const char* name() const override { return "ComposeTwo"; }

    SkString dumpInfo() const override {
        SkString str;

        str.appendf("Mode: %s", SkBlendMode_Name(fMode));

        for (int i = 0; i < this->numChildProcessors(); ++i) {
            str.appendf(" [%s %s]",
                        this->childProcessor(i).name(), this->childProcessor(i).dumpInfo().c_str());
        }
        return str;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32((int)fMode);
    }

    SkBlendMode getMode() const { return fMode; }

private:
    ComposeTwoFragmentProcessor(sk_sp<GrFragmentProcessor> src,
                                sk_sp<GrFragmentProcessor> dst,
                                SkBlendMode mode)
            : INHERITED(OptFlags(src.get(), dst.get(), mode))
            , fMode(mode) {
        this->initClassID<ComposeTwoFragmentProcessor>();
        SkDEBUGCODE(int shaderAChildIndex = )this->registerChildProcessor(std::move(src));
        SkDEBUGCODE(int shaderBChildIndex = )this->registerChildProcessor(std::move(dst));
        SkASSERT(0 == shaderAChildIndex);
        SkASSERT(1 == shaderBChildIndex);
    }

    static OptimizationFlags OptFlags(const GrFragmentProcessor* src,
                                      const GrFragmentProcessor* dst, SkBlendMode mode) {
        OptimizationFlags flags;
        switch (mode) {
            case SkBlendMode::kClear:
            case SkBlendMode::kSrc:
            case SkBlendMode::kDst:
                SkFAIL("Should never create clear, src, or dst compose two FP.");
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
        return flags;
    }

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ComposeTwoFragmentProcessor& cs = other.cast<ComposeTwoFragmentProcessor>();
        return fMode == cs.fMode;
    }

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        float alpha = input.fRGBA[3];
        input = input.opaque();
        GrColor4f srcColor = ConstantOutputForConstantInput(this->childProcessor(0), input);
        GrColor4f dstColor = ConstantOutputForConstantInput(this->childProcessor(1), input);
        SkPM4f src = GrColor4fToSkPM4f(srcColor);
        SkPM4f dst = GrColor4fToSkPM4f(dstColor);
        SkPM4f res = SkBlendMode_Apply(fMode, src, dst);
        return SkPM4fToGrColor4f(res).mulByScalar(alpha);
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
sk_sp<GrFragmentProcessor> ComposeTwoFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create two random frag procs.
    sk_sp<GrFragmentProcessor> fpA(GrProcessorUnitTest::MakeChildFP(d));
    sk_sp<GrFragmentProcessor> fpB(GrProcessorUnitTest::MakeChildFP(d));

    SkBlendMode mode;
    do {
        mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
    } while (SkBlendMode::kClear == mode || SkBlendMode::kSrc == mode || SkBlendMode::kDst == mode);
    return sk_sp<GrFragmentProcessor>(
        new ComposeTwoFragmentProcessor(std::move(fpA), std::move(fpB), mode));
}
#endif

GrGLSLFragmentProcessor* ComposeTwoFragmentProcessor::onCreateGLSLInstance() const{
    return new GLComposeTwoFragmentProcessor;
}

/////////////////////////////////////////////////////////////////////

void GLComposeTwoFragmentProcessor::emitCode(EmitArgs& args) {

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const ComposeTwoFragmentProcessor& cs = args.fFp.cast<ComposeTwoFragmentProcessor>();

    const char* inputColor = nullptr;
    if (args.fInputColor) {
        inputColor = "inputColor";
        fragBuilder->codeAppendf("vec4 inputColor = vec4(%s.rgb, 1.0);", args.fInputColor);
    }

    // declare outputColor and emit the code for each of the two children
    SkString srcColor("xfer_src");
    this->emitChild(0, inputColor, &srcColor, args);

    SkString dstColor("xfer_dst");
    this->emitChild(1, inputColor, &dstColor, args);

    // emit blend code
    SkBlendMode mode = cs.getMode();
    fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkBlendMode_Name(mode));
    GrGLSLBlend::AppendMode(fragBuilder,
                            srcColor.c_str(),
                            dstColor.c_str(),
                            args.fOutputColor,
                            mode);

    // re-multiply the output color by the input color's alpha
    if (args.fInputColor) {
        fragBuilder->codeAppendf("%s *= %s.a;", args.fOutputColor, args.fInputColor);
    }
}

sk_sp<GrFragmentProcessor> GrXfermodeFragmentProcessor::MakeFromTwoProcessors(
         sk_sp<GrFragmentProcessor> src, sk_sp<GrFragmentProcessor> dst, SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
        case SkBlendMode::kSrc:
            return src;
        case SkBlendMode::kDst:
            return dst;
        default:
            return ComposeTwoFragmentProcessor::Make(std::move(src), std::move(dst), mode);
    }
}

//////////////////////////////////////////////////////////////////////////////

class ComposeOneFragmentProcessor : public GrFragmentProcessor {
public:
    enum Child {
        kDst_Child,
        kSrc_Child,
    };

    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> fp, SkBlendMode mode,
                                           Child child) {
        if (!fp) {
            return nullptr;
        }
        return sk_sp<GrFragmentProcessor>(new ComposeOneFragmentProcessor(std::move(fp),
                                                                          mode, child));
    }

    const char* name() const override { return "ComposeOne"; }

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

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GR_STATIC_ASSERT(((int)SkBlendMode::kLastMode & SK_MaxU16) == (int)SkBlendMode::kLastMode);
        b->add32((int)fMode | (fChild << 16));
    }

    SkBlendMode mode() const { return fMode; }

    Child child() const { return fChild; }

private:
    OptimizationFlags OptFlags(const GrFragmentProcessor* fp, SkBlendMode mode, Child child) {
        OptimizationFlags flags;
        switch (mode) {
            case SkBlendMode::kClear:
                SkFAIL("Should never create clear compose one FP.");
                flags = kNone_OptimizationFlags;
                break;

            case SkBlendMode::kSrc:
                SkASSERT(child == kSrc_Child);
                flags = fp->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                   : kNone_OptimizationFlags;
                break;

            case SkBlendMode::kDst:
                SkASSERT(child == kDst_Child);
                flags = fp->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                   : kNone_OptimizationFlags;
                break;

            // Produces opaque if both src and dst are opaque. These also will modulate the child's
            // output by either the input color or alpha. However, if the child is not compatible
            // with the coverage as alpha then it may produce a color that is not valid premul.
            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn:
            case SkBlendMode::kModulate:
                if (fp->compatibleWithCoverageAsAlpha()) {
                    if (fp->preservesOpaqueInput()) {
                        flags = kPreservesOpaqueInput_OptimizationFlag |
                                kCompatibleWithCoverageAsAlpha_OptimizationFlag;
                    } else {
                        flags = kCompatibleWithCoverageAsAlpha_OptimizationFlag;
                    }
                } else {
                    flags = fp->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                       : kNone_OptimizationFlags;
                }
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
                    flags = fp->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                       : kNone_OptimizationFlags;
                } else {
                    flags = kPreservesOpaqueInput_OptimizationFlag;
                }
                break;

            // DstATop is the converse of kSrcATop. Screen is also opaque if the src is a opaque.
            case SkBlendMode::kDstATop:
            case SkBlendMode::kScreen:
                if (child == kSrc_Child) {
                    flags = fp->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
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
        if (does_cpu_blend_impl_match_gpu(mode) && fp->hasConstantOutputForConstantInput()) {
            flags |= kConstantOutputForConstantInput_OptimizationFlag;
        }
        return flags;
    }

    bool onIsEqual(const GrFragmentProcessor& that) const override {
        return fMode == that.cast<ComposeOneFragmentProcessor>().fMode;
    }

    GrColor4f constantOutputForConstantInput(GrColor4f inputColor) const override {
        GrColor4f childColor =
                ConstantOutputForConstantInput(this->childProcessor(0), GrColor4f::OpaqueWhite());
        SkPM4f src, dst;
        if (kSrc_Child == fChild) {
            src = GrColor4fToSkPM4f(childColor);
            dst = GrColor4fToSkPM4f(inputColor);
        } else {
            src = GrColor4fToSkPM4f(inputColor);
            dst = GrColor4fToSkPM4f(childColor);
        }
        SkPM4f res = SkBlendMode_Apply(fMode, src, dst);
        return SkPM4fToGrColor4f(res);
    }

private:
    ComposeOneFragmentProcessor(sk_sp<GrFragmentProcessor> fp, SkBlendMode mode, Child child)
            : INHERITED(OptFlags(fp.get(), mode, child))
            , fMode(mode)
            , fChild(child) {
        this->initClassID<ComposeOneFragmentProcessor>();
        SkDEBUGCODE(int dstIndex =) this->registerChildProcessor(std::move(fp));
        SkASSERT(0 == dstIndex);
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
        ComposeOneFragmentProcessor::Child child =
            args.fFp.cast<ComposeOneFragmentProcessor>().child();
        SkString childColor("child");
        this->emitChild(0, &childColor, args);

        const char* inputColor = args.fInputColor;
        // We don't try to optimize for this case at all
        if (!inputColor) {
            fragBuilder->codeAppendf("const vec4 ones = vec4(1);");
            inputColor = "ones";
        }

        // emit blend code
        fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkBlendMode_Name(mode));
        const char* childStr = childColor.c_str();
        if (ComposeOneFragmentProcessor::kDst_Child == child) {
            GrGLSLBlend::AppendMode(fragBuilder, inputColor, childStr, args.fOutputColor, mode);
        } else {
            GrGLSLBlend::AppendMode(fragBuilder, childStr, inputColor, args.fOutputColor, mode);
        }
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ComposeOneFragmentProcessor);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> ComposeOneFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create one random frag procs.
    // For now, we'll prevent either children from being a shader with children to prevent the
    // possibility of an arbitrarily large tree of procs.
    sk_sp<GrFragmentProcessor> dst(GrProcessorUnitTest::MakeChildFP(d));
    SkBlendMode mode;
    ComposeOneFragmentProcessor::Child child;
    do {
        mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
        child = d->fRandom->nextBool() ? kDst_Child : kSrc_Child;
    } while (SkBlendMode::kClear == mode || (SkBlendMode::kDst == mode && child == kSrc_Child) ||
             (SkBlendMode::kSrc == mode && child == kDst_Child));
    return sk_sp<GrFragmentProcessor>(new ComposeOneFragmentProcessor(std::move(dst), mode, child));
}
#endif

GrGLSLFragmentProcessor* ComposeOneFragmentProcessor::onCreateGLSLInstance() const {
    return new GLComposeOneFragmentProcessor;
}

//////////////////////////////////////////////////////////////////////////////

// It may seems as though when the input FP is the dst and the mode is kDst (or same for src/kSrc)
// that these factories could simply return the input FP. However, that doesn't have quite
// the same effect as the returned compose FP will replace the FP's input with solid white and
// ignore the original input. This could be implemented as:
// RunInSeries(ConstColor(GrColor_WHITE, kIgnoreInput), inputFP).

sk_sp<GrFragmentProcessor> GrXfermodeFragmentProcessor::MakeFromDstProcessor(
    sk_sp<GrFragmentProcessor> dst, SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
        case SkBlendMode::kSrc:
            return nullptr;
        default:
            return ComposeOneFragmentProcessor::Make(std::move(dst), mode,
                                                     ComposeOneFragmentProcessor::kDst_Child);
    }
}

sk_sp<GrFragmentProcessor> GrXfermodeFragmentProcessor::MakeFromSrcProcessor(
    sk_sp<GrFragmentProcessor> src, SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
        case SkBlendMode::kDst:
            return nullptr;
        default:
            return ComposeOneFragmentProcessor::Make(std::move(src), mode,
                                                     ComposeOneFragmentProcessor::kSrc_Child);
    }
}
