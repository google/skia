/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticMode_gpu.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

static void add_arithmetic_code(GrGLSLFragmentBuilder* fragBuilder,
                                const char* srcColor,
                                const char* dstColor,
                                const char* outputColor,
                                const char* kUni,
                                bool enforcePMColor) {
    // We don't try to optimize for this case at all
    if (nullptr == srcColor) {
        fragBuilder->codeAppend("const vec4 src = vec4(1);");
    } else {
        fragBuilder->codeAppendf("vec4 src = %s;", srcColor);
    }

    fragBuilder->codeAppendf("vec4 dst = %s;", dstColor);
    fragBuilder->codeAppendf("%s = %s.x * src * dst + %s.y * src + %s.z * dst + %s.w;",
                             outputColor, kUni, kUni, kUni, kUni);
    fragBuilder->codeAppendf("%s = clamp(%s, 0.0, 1.0);\n", outputColor, outputColor);
    if (enforcePMColor) {
        fragBuilder->codeAppendf("%s.rgb = min(%s.rgb, %s.a);",
                                 outputColor, outputColor, outputColor);
    }
}

class GLArithmeticFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrArithmeticFP& arith = args.fFp.cast<GrArithmeticFP>();

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString dstColor("dstColor");
        this->emitChild(0, nullptr, &dstColor, args);

        fKUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                 kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                 "k");
        const char* kUni = args.fUniformHandler->getUniformCStr(fKUni);

        add_arithmetic_code(fragBuilder,
                            args.fInputColor,
                            dstColor.c_str(),
                            args.fOutputColor,
                            kUni,
                            arith.enforcePMColor());
    }

    static void GenKey(const GrProcessor& proc, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        const GrArithmeticFP& arith = proc.cast<GrArithmeticFP>();
        uint32_t key = arith.enforcePMColor() ? 1 : 0;
        b->add32(key);
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
        const GrArithmeticFP& arith = proc.cast<GrArithmeticFP>();
        pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
    }

private:
    GrGLSLProgramDataManager::UniformHandle fKUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrArithmeticFP::GrArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                               sk_sp<GrFragmentProcessor> dst)
  : fK1(k1), fK2(k2), fK3(k3), fK4(k4), fEnforcePMColor(enforcePMColor) {
    this->initClassID<GrArithmeticFP>();

    SkASSERT(dst);
    SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(std::move(dst));
    SkASSERT(0 == dstIndex);
}

void GrArithmeticFP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLArithmeticFP::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrArithmeticFP::onCreateGLSLInstance() const {
    return new GLArithmeticFP;
}

bool GrArithmeticFP::onIsEqual(const GrFragmentProcessor& fpBase) const {
    const GrArithmeticFP& fp = fpBase.cast<GrArithmeticFP>();
    return fK1 == fp.fK1 &&
           fK2 == fp.fK2 &&
           fK3 == fp.fK3 &&
           fK4 == fp.fK4 &&
           fEnforcePMColor == fp.fEnforcePMColor;
}

void GrArithmeticFP::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    // TODO: optimize this
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrArithmeticFP::TestCreate(GrProcessorTestData* d) {
    float k1 = d->fRandom->nextF();
    float k2 = d->fRandom->nextF();
    float k3 = d->fRandom->nextF();
    float k4 = d->fRandom->nextF();
    bool enforcePMColor = d->fRandom->nextBool();

    sk_sp<GrFragmentProcessor> dst(GrProcessorUnitTest::MakeChildFP(d));
    return GrArithmeticFP::Make(k1, k2, k3, k4, enforcePMColor, std::move(dst));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrArithmeticFP);

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class ArithmeticXP : public GrXferProcessor {
public:
    ArithmeticXP(const DstTexture*, bool hasMixedSamples,
                 float k1, float k2, float k3, float k4, bool enforcePMColor);

    const char* name() const override { return "Arithmetic"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ArithmeticXP& xp = xpBase.cast<ArithmeticXP>();
        if (fK1 != xp.fK1 ||
            fK2 != xp.fK2 ||
            fK3 != xp.fK3 ||
            fK4 != xp.fK4 ||
            fEnforcePMColor != xp.fEnforcePMColor) {
            return false;
        }
        return true;
    }

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLArithmeticXP : public GrGLSLXferProcessor {
public:
    GLArithmeticXP(const ArithmeticXP& arithmeticXP)
        : fEnforcePMColor(arithmeticXP.enforcePMColor()) {
    }

    ~GLArithmeticXP() override {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const ArithmeticXP& arith = processor.cast<ArithmeticXP>();
        uint32_t key = arith.enforcePMColor() ? 1 : 0;
        b->add32(key);
    }

private:
    void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                 GrGLSLUniformHandler* uniformHandler,
                                 const char* srcColor,
                                 const char* srcCoverage,
                                 const char* dstColor,
                                 const char* outColor,
                                 const char* outColorSecondary,
                                 const GrXferProcessor& proc) override {
        fKUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                           kVec4f_GrSLType, kDefault_GrSLPrecision,
                                           "k");
        const char* kUni = uniformHandler->getUniformCStr(fKUni);

        add_arithmetic_code(fragBuilder, srcColor, dstColor, outColor, kUni, fEnforcePMColor);

        // Apply coverage.
        INHERITED::DefaultCoverageModulation(fragBuilder, srcCoverage, dstColor, outColor,
                                             outColorSecondary, proc);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrXferProcessor& processor) override {
        const ArithmeticXP& arith = processor.cast<ArithmeticXP>();
        pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
        fEnforcePMColor = arith.enforcePMColor();
    };

    GrGLSLProgramDataManager::UniformHandle fKUni;
    bool fEnforcePMColor;

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

ArithmeticXP::ArithmeticXP(const DstTexture* dstTexture, bool hasMixedSamples,
                           float k1, float k2, float k3, float k4, bool enforcePMColor)
    : INHERITED(dstTexture, true, hasMixedSamples)
    , fK1(k1)
    , fK2(k2)
    , fK3(k3)
    , fK4(k4)
    , fEnforcePMColor(enforcePMColor) {
    this->initClassID<ArithmeticXP>();
}

void ArithmeticXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLArithmeticXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* ArithmeticXP::createGLSLInstance() const { return new GLArithmeticXP(*this); }

GrXferProcessor::OptFlags ArithmeticXP::onGetOptimizations(
                                                       const GrPipelineOptimizations& optimizations,
                                                       bool doesStencilWrite,
                                                       GrColor* overrideColor,
                                                       const GrCaps& caps) const {
   return GrXferProcessor::kNone_OptFlags;
}

///////////////////////////////////////////////////////////////////////////////

GrArithmeticXPFactory::GrArithmeticXPFactory(float k1, float k2, float k3, float k4,
                                             bool enforcePMColor)
    : fK1(k1), fK2(k2), fK3(k3), fK4(k4), fEnforcePMColor(enforcePMColor) {
    this->initClassID<GrArithmeticXPFactory>();
}

GrXferProcessor*
GrArithmeticXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                             const GrPipelineOptimizations& optimizations,
                                             bool hasMixedSamples,
                                             const DstTexture* dstTexture) const {
    return new ArithmeticXP(dstTexture, hasMixedSamples, fK1, fK2, fK3, fK4, fEnforcePMColor);
}


void GrArithmeticXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                                     InvariantBlendedColor* blendedColor) const {
    blendedColor->fWillBlendWithDst = true;

    // TODO: We could try to optimize this more. For example if fK1 and fK3 are zero, then we won't
    // be blending the color with dst at all so we can know what the output color is (up to the
    // valid color components passed in).
    blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
}

GR_DEFINE_XP_FACTORY_TEST(GrArithmeticXPFactory);

sk_sp<GrXPFactory> GrArithmeticXPFactory::TestCreate(GrProcessorTestData* d) {
    float k1 = d->fRandom->nextF();
    float k2 = d->fRandom->nextF();
    float k3 = d->fRandom->nextF();
    float k4 = d->fRandom->nextF();
    bool enforcePMColor = d->fRandom->nextBool();

    return GrArithmeticXPFactory::Make(k1, k2, k3, k4, enforcePMColor);
}

#endif
