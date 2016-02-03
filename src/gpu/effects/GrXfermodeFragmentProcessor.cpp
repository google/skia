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
    ComposeTwoFragmentProcessor(const GrFragmentProcessor* src, const GrFragmentProcessor* dst,
                    SkXfermode::Mode mode)
        : fMode(mode) {
        this->initClassID<ComposeTwoFragmentProcessor>();
        SkDEBUGCODE(int shaderAChildIndex = )this->registerChildProcessor(src);
        SkDEBUGCODE(int shaderBChildIndex = )this->registerChildProcessor(dst);
        SkASSERT(0 == shaderAChildIndex);
        SkASSERT(1 == shaderBChildIndex);
    }

    const char* name() const override { return "ComposeTwo"; }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(fMode);
    }

    SkXfermode::Mode getMode() const { return fMode; }

protected:
    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ComposeTwoFragmentProcessor& cs = other.cast<ComposeTwoFragmentProcessor>();
        return fMode == cs.fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkXfermode::Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GLComposeTwoFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GLComposeTwoFragmentProcessor(const GrProcessor& processor) {}

    void emitCode(EmitArgs&) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ComposeTwoFragmentProcessor);

const GrFragmentProcessor* ComposeTwoFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create two random frag procs.
    SkAutoTUnref<const GrFragmentProcessor> fpA(GrProcessorUnitTest::CreateChildFP(d));
    SkAutoTUnref<const GrFragmentProcessor> fpB(GrProcessorUnitTest::CreateChildFP(d));

    SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(
        d->fRandom->nextRangeU(0, SkXfermode::kLastMode));
    return new ComposeTwoFragmentProcessor(fpA, fpB, mode);
}

GrGLSLFragmentProcessor* ComposeTwoFragmentProcessor::onCreateGLSLInstance() const{
    return new GLComposeTwoFragmentProcessor(*this);
}

/////////////////////////////////////////////////////////////////////

void GLComposeTwoFragmentProcessor::emitCode(EmitArgs& args) {

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    const ComposeTwoFragmentProcessor& cs = args.fFp.cast<ComposeTwoFragmentProcessor>();

    const char* inputColor = nullptr;
    if (args.fInputColor) {
        inputColor = "inputColor";
        fragBuilder->codeAppendf("vec4 inputColor = vec4(%s.rgb, 1.0);", args.fInputColor);
    }

    // declare outputColor and emit the code for each of the two children
    SkString srcColor("src");
    this->emitChild(0, inputColor, &srcColor, args);

    SkString dstColor("dst");
    this->emitChild(1, inputColor, &dstColor, args);

    // emit blend code
    SkXfermode::Mode mode = cs.getMode();
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

const GrFragmentProcessor* GrXfermodeFragmentProcessor::CreateFromTwoProcessors(
         const GrFragmentProcessor* src, const GrFragmentProcessor* dst, SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANSPARENT_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
        case SkXfermode::kSrc_Mode:
            return SkRef(src);
        case SkXfermode::kDst_Mode:
            return SkRef(dst);
        default:
            return new ComposeTwoFragmentProcessor(src, dst, mode);
    }
}

//////////////////////////////////////////////////////////////////////////////

class ComposeOneFragmentProcessor : public GrFragmentProcessor {
public:
    enum Child {
        kDst_Child,
        kSrc_Child,
    };

    ComposeOneFragmentProcessor(const GrFragmentProcessor* dst, SkXfermode::Mode mode, Child child)
        : fMode(mode)
        , fChild(child) {
        this->initClassID<ComposeOneFragmentProcessor>();
        SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(dst);
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

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GR_STATIC_ASSERT((SkXfermode::kLastMode & SK_MaxU16) == SkXfermode::kLastMode);
        b->add32(fMode | (fChild << 16));
    }

    SkXfermode::Mode mode() const { return fMode; }

    Child child() const { return fChild; }

protected:
    bool onIsEqual(const GrFragmentProcessor& that) const override {
        return fMode == that.cast<ComposeOneFragmentProcessor>().fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        SkXfermode::Coeff skSrcCoeff, skDstCoeff;
        if (SkXfermode::ModeAsCoeff(fMode, &skSrcCoeff, &skDstCoeff)) {
            GrBlendCoeff srcCoeff = SkXfermodeCoeffToGrBlendCoeff(skSrcCoeff);
            GrBlendCoeff dstCoeff = SkXfermodeCoeffToGrBlendCoeff(skDstCoeff);
            GrInvariantOutput childOutput(0xFFFFFFFF, kRGBA_GrColorComponentFlags, false);
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
            // will the shader code reference the input color?
            GrInvariantOutput::ReadInput readsInput = GrInvariantOutput::kWillNot_ReadInput;
            if (kDst_Child == fChild) {
                if (kZero_GrBlendCoeff != srcCoeff || GrBlendCoeffRefsSrc(dstCoeff)) {
                    readsInput = GrInvariantOutput::kWill_ReadInput;
                }
            } else {
                if (kZero_GrBlendCoeff != dstCoeff || GrBlendCoeffRefsDst(srcCoeff)) {
                    readsInput = GrInvariantOutput::kWill_ReadInput;
                }
            }
            inout->setToOther(blendFlags, blendColor, readsInput);
        } else {
            inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
        }
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkXfermode::Mode    fMode;
    Child               fChild;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLComposeOneFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GLComposeOneFragmentProcessor(const GrProcessor& processor) {}

    void emitCode(EmitArgs& args) override {
        GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkXfermode::Mode mode = args.fFp.cast<ComposeOneFragmentProcessor>().mode();
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

const GrFragmentProcessor* ComposeOneFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create one random frag procs.
    // For now, we'll prevent either children from being a shader with children to prevent the
    // possibility of an arbitrarily large tree of procs.
    SkAutoTUnref<const GrFragmentProcessor> dst(GrProcessorUnitTest::CreateChildFP(d));
    SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(
        d->fRandom->nextRangeU(0, SkXfermode::kLastMode));
    ComposeOneFragmentProcessor::Child child = d->fRandom->nextBool() ?
        ComposeOneFragmentProcessor::kDst_Child :
        ComposeOneFragmentProcessor::kSrc_Child;
    return new ComposeOneFragmentProcessor(dst, mode, child);
}

GrGLSLFragmentProcessor* ComposeOneFragmentProcessor::onCreateGLSLInstance() const {
    return new GLComposeOneFragmentProcessor(*this);
}

//////////////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* GrXfermodeFragmentProcessor::CreateFromDstProcessor(
    const GrFragmentProcessor* dst, SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANSPARENT_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
        case SkXfermode::kSrc_Mode:
            return nullptr;
        default:
            return new ComposeOneFragmentProcessor(dst, mode,
                                                   ComposeOneFragmentProcessor::kDst_Child);
    }
}

const GrFragmentProcessor* GrXfermodeFragmentProcessor::CreateFromSrcProcessor(
    const GrFragmentProcessor* src, SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANSPARENT_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
        case SkXfermode::kDst_Mode:
            return nullptr;
        default:
            return new ComposeOneFragmentProcessor(src, mode,
                                                   ComposeOneFragmentProcessor::kSrc_Child);
    }
}
