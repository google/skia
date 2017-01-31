/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "effects/GrXfermodeFragmentProcessor.h"

#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "effects/GrConstColorProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGrPriv.h"

class ComposeTwoFragmentProcessor : public GrFragmentProcessor {
public:
    ComposeTwoFragmentProcessor(sk_sp<GrFragmentProcessor> src, sk_sp<GrFragmentProcessor> dst,
                                SkBlendMode mode)
            : INHERITED(OptFlags(src.get(), dst.get(), mode)), fMode(mode) {
        this->initClassID<ComposeTwoFragmentProcessor>();
        SkDEBUGCODE(int shaderAChildIndex = )this->registerChildProcessor(std::move(src));
        SkDEBUGCODE(int shaderBChildIndex = )this->registerChildProcessor(std::move(dst));
        SkASSERT(0 == shaderAChildIndex);
        SkASSERT(1 == shaderBChildIndex);
    }

    const char* name() const override { return "ComposeTwo"; }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32((int)fMode);
    }

    SkBlendMode getMode() const { return fMode; }

private:
    static OptimizationFlags OptFlags(const GrFragmentProcessor* src,
                                      const GrFragmentProcessor* dst, SkBlendMode mode) {
        // We only attempt the constant output optimization.
        // The CPU and GPU implementations differ significantly for the advanced modes.
        if (mode <= SkBlendMode::kLastSeparableMode && src->hasConstantOutputForConstantInput() &&
            dst->hasConstantOutputForConstantInput()) {
            return kConstantOutputForConstantInput_OptimizationFlag;
        }
        return kNone_OptimizationFlags;
    }

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ComposeTwoFragmentProcessor& cs = other.cast<ComposeTwoFragmentProcessor>();
        return fMode == cs.fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown();
    }

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        float alpha = input.fRGBA[3];
        input = input.opaque();
        GrColor4f srcColor = ConstantOutputForConstantInput(this->childProcessor(0), input);
        GrColor4f dstColor = ConstantOutputForConstantInput(this->childProcessor(1), input);
        SkPM4f src = GrColor4fToSkPM4f(srcColor);
        SkPM4f dst = GrColor4fToSkPM4f(dstColor);
        auto proc = SkXfermode::GetProc4f(fMode);
        return SkPM4fToGrColor4f(proc(src, dst)).mulByScalar(alpha);
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkBlendMode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

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

    SkBlendMode mode = static_cast<SkBlendMode>(
        d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
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
    fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkXfermode::ModeName(mode));
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
            return sk_sp<GrFragmentProcessor>(
                new ComposeTwoFragmentProcessor(std::move(src), std::move(dst), mode));
    }
}

//////////////////////////////////////////////////////////////////////////////

class ComposeOneFragmentProcessor : public GrFragmentProcessor {
public:
    enum Child {
        kDst_Child,
        kSrc_Child,
    };

    ComposeOneFragmentProcessor(sk_sp<GrFragmentProcessor> dst, SkBlendMode mode, Child child)
            : INHERITED(OptFlags(dst.get(), mode)), fMode(mode), fChild(child) {
        this->initClassID<ComposeOneFragmentProcessor>();
        SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(std::move(dst));
        SkASSERT(0 == dstIndex);
    }

    const char* name() const override { return "ComposeOne"; }

    SkString dumpInfo() const override {
        SkString str;

        for (int i = 0; i < this->numChildProcessors(); ++i) {
            str.append(this->childProcessor(i).dumpInfo());
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
    OptimizationFlags OptFlags(const GrFragmentProcessor* child, SkBlendMode mode) {
        // We only attempt the constant output optimization.
        // The CPU and GPU implementations differ significantly for the advanced modes.
        if (mode <= SkBlendMode::kLastSeparableMode && child->hasConstantOutputForConstantInput()) {
            return kConstantOutputForConstantInput_OptimizationFlag;
        }
        return kNone_OptimizationFlags;
    }

    bool onIsEqual(const GrFragmentProcessor& that) const override {
        return fMode == that.cast<ComposeOneFragmentProcessor>().fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        SkXfermode::Coeff skSrcCoeff, skDstCoeff;
        if (SkXfermode::ModeAsCoeff(fMode, &skSrcCoeff, &skDstCoeff)) {
            GrBlendCoeff srcCoeff = SkXfermodeCoeffToGrBlendCoeff(skSrcCoeff);
            GrBlendCoeff dstCoeff = SkXfermodeCoeffToGrBlendCoeff(skDstCoeff);
            GrInvariantOutput childOutput(0xFFFFFFFF, kRGBA_GrColorComponentFlags);
            this->childProcessor(0).computeInvariantOutput(&childOutput);
            GrColor blendColor;
            GrColorComponentFlags blendFlags;
            if (kDst_Child == fChild) {
                GrGetCoeffBlendKnownComponents(srcCoeff, dstCoeff,
                                               inout->color(), inout->validFlags(),
                                               childOutput.color(), childOutput.validFlags(),
                                               &blendColor, &blendFlags);
            } else {
                GrGetCoeffBlendKnownComponents(srcCoeff, dstCoeff,
                                               childOutput.color(), childOutput.validFlags(),
                                               inout->color(), inout->validFlags(),
                                               &blendColor, &blendFlags);
            }
            inout->setToOther(blendFlags, blendColor);
        } else {
            inout->setToUnknown();
        }
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
        auto proc = SkXfermode::GetProc4f(fMode);
        return SkPM4fToGrColor4f(proc(src, dst));
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkBlendMode fMode;
    Child       fChild;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

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
        this->emitChild(0, nullptr, &childColor, args);

        const char* inputColor = args.fInputColor;
        // We don't try to optimize for this case at all
        if (!inputColor) {
            fragBuilder->codeAppendf("const vec4 ones = vec4(1);");
            inputColor = "ones";
        }

        // emit blend code
        fragBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkXfermode::ModeName(mode));
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
    SkBlendMode mode = static_cast<SkBlendMode>(
        d->fRandom->nextRangeU(0, (int)SkBlendMode::kLastMode));
    ComposeOneFragmentProcessor::Child child = d->fRandom->nextBool() ?
        ComposeOneFragmentProcessor::kDst_Child :
        ComposeOneFragmentProcessor::kSrc_Child;
    return sk_sp<GrFragmentProcessor>(new ComposeOneFragmentProcessor(std::move(dst), mode, child));
}
#endif

GrGLSLFragmentProcessor* ComposeOneFragmentProcessor::onCreateGLSLInstance() const {
    return new GLComposeOneFragmentProcessor;
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrXfermodeFragmentProcessor::MakeFromDstProcessor(
    sk_sp<GrFragmentProcessor> dst, SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                                 GrConstColorProcessor::kIgnore_InputMode);
        case SkBlendMode::kSrc:
            return nullptr;
        default:
            return sk_sp<GrFragmentProcessor>(
                new ComposeOneFragmentProcessor(std::move(dst), mode,
                                                ComposeOneFragmentProcessor::kDst_Child));
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
            return sk_sp<GrFragmentProcessor>(
                new ComposeOneFragmentProcessor(src, mode,
                                                ComposeOneFragmentProcessor::kSrc_Child));
    }
}
