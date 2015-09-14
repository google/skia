/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "effects/GrXfermodeFragmentProcessor.h"

#include "GrFragmentProcessor.h"
#include "effects/GrConstColorProcessor.h"
#include "gl/GrGLBlend.h"
#include "gl/builders/GrGLProgramBuilder.h"


class GrComposeTwoFragmentProcessor : public GrFragmentProcessor {
public:
    GrComposeTwoFragmentProcessor(const GrFragmentProcessor* src, const GrFragmentProcessor* dst,
                    SkXfermode::Mode mode)
        : fMode(mode) {
        // Only coefficient xfer modes are supported
        SkASSERT(SkXfermode::kLastCoeffMode >= mode);
        this->initClassID<GrComposeTwoFragmentProcessor>();
        SkDEBUGCODE(int shaderAChildIndex = )this->registerChildProcessor(src);
        SkDEBUGCODE(int shaderBChildIndex = )this->registerChildProcessor(dst);
        SkASSERT(0 == shaderAChildIndex);
        SkASSERT(1 == shaderBChildIndex);
    }

    const char* name() const override { return "ComposeShader"; }

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(fMode);
    }

    SkXfermode::Mode getMode() const { return fMode; }

protected:
    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const GrComposeTwoFragmentProcessor& cs = other.cast<GrComposeTwoFragmentProcessor>();
        return fMode == cs.fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

private:
    GrGLFragmentProcessor* onCreateGLInstance() const override;

    SkXfermode::Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrGLComposeTwoFragmentProcessor : public GrGLFragmentProcessor {
public:
    GrGLComposeTwoFragmentProcessor(const GrProcessor& processor) {}

    void emitCode(EmitArgs&) override;

private:
    typedef GrGLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrComposeTwoFragmentProcessor);

const GrFragmentProcessor* GrComposeTwoFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    // Create two random frag procs.
    SkAutoTUnref<const GrFragmentProcessor> fpA(GrProcessorUnitTest::CreateChildFP(d));
    SkAutoTUnref<const GrFragmentProcessor> fpB(GrProcessorUnitTest::CreateChildFP(d));

    SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(
            d->fRandom->nextRangeU(0, SkXfermode::kLastCoeffMode));
    return SkNEW_ARGS(GrComposeTwoFragmentProcessor, (fpA, fpB, mode));
}

GrGLFragmentProcessor* GrComposeTwoFragmentProcessor::onCreateGLInstance() const{
    return SkNEW_ARGS(GrGLComposeTwoFragmentProcessor, (*this));
}

/////////////////////////////////////////////////////////////////////

void GrGLComposeTwoFragmentProcessor::emitCode(EmitArgs& args) {

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
    const GrComposeTwoFragmentProcessor& cs = args.fFp.cast<GrComposeTwoFragmentProcessor>();

    // Store alpha of input color and un-premultiply the input color by its alpha. We will
    // re-multiply by this alpha after blending the output colors of the two child procs.
    // This is because we don't want the paint's alpha to affect either child proc's output
    // before the blend; we want to apply the paint's alpha AFTER the blend. This mirrors the
    // software implementation of SkComposeShader.
    const char* opaqueInput = nullptr;
    const char* inputAlpha = nullptr;
    if (args.fInputColor) {
        inputAlpha = "inputAlpha";
        opaqueInput = "opaqueInput";
        fsBuilder->codeAppendf("float inputAlpha = %s.a;", args.fInputColor);
        fsBuilder->codeAppendf("vec4 opaqueInput = vec4(%s.rgb / inputAlpha, 1);",
                               args.fInputColor);
    }

    // declare outputColor and emit the code for each of the two children
    SkString outputColorSrc(args.fOutputColor);
    outputColorSrc.append("_src");
    fsBuilder->codeAppendf("vec4 %s;\n", outputColorSrc.c_str());
    this->emitChild(0, opaqueInput, outputColorSrc.c_str(), args);

    SkString outputColorDst(args.fOutputColor);
    outputColorDst.append("_dst");
    fsBuilder->codeAppendf("vec4 %s;\n", outputColorDst.c_str());
    this->emitChild(1, opaqueInput, outputColorDst.c_str(), args);

    // emit blend code
    SkXfermode::Mode mode = cs.getMode();
    fsBuilder->codeAppend("{");
    fsBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkXfermode::ModeName(mode));
    GrGLBlend::AppendPorterDuffBlend(fsBuilder, outputColorSrc.c_str(),
                                     outputColorDst.c_str(), args.fOutputColor, mode);
    fsBuilder->codeAppend("}");

    // re-multiply the output color by the input color's alpha
    if (inputAlpha) {
        fsBuilder->codeAppendf("%s *= %s;", args.fOutputColor, inputAlpha);
    }
}

const GrFragmentProcessor* GrXfermodeFragmentProcessor::CreateFromTwoProcessors(
         const GrFragmentProcessor* src, const GrFragmentProcessor* dst, SkXfermode::Mode mode) {
    if (SkXfermode::kLastCoeffMode < mode) {
        return nullptr;
    }
    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANS_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkXfermode::kSrc_Mode:
            return SkRef(src);
            break;
        case SkXfermode::kDst_Mode:
            return SkRef(dst);
            break;
        default:
            return new GrComposeTwoFragmentProcessor(src, dst, mode);
    }
}
