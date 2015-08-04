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
#include "gl/GrGLCaps.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/builders/GrGLProgramBuilder.h"

static const bool gUseUnpremul = false;

static void add_arithmetic_code(GrGLFragmentBuilder* fsBuilder,
                                const char* inputColor,
                                const char* dstColor,
                                const char* outputColor,
                                const char* kUni,
                                bool enforcePMColor) {
    // We don't try to optimize for this case at all
    if (NULL == inputColor) {
        fsBuilder->codeAppend("const vec4 src = vec4(1);");
    } else {
        fsBuilder->codeAppendf("vec4 src = %s;", inputColor);
        if (gUseUnpremul) {
            fsBuilder->codeAppend("src.rgb = clamp(src.rgb / src.a, 0.0, 1.0);");
        }
    }

    fsBuilder->codeAppendf("vec4 dst = %s;", dstColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppend("dst.rgb = clamp(dst.rgb / dst.a, 0.0, 1.0);");
    }

    fsBuilder->codeAppendf("%s = %s.x * src * dst + %s.y * src + %s.z * dst + %s.w;",
                           outputColor, kUni, kUni, kUni, kUni);
    fsBuilder->codeAppendf("%s = clamp(%s, 0.0, 1.0);\n", outputColor, outputColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppendf("%s.rgb *= %s.a;", outputColor, outputColor);
    } else if (enforcePMColor) {
        fsBuilder->codeAppendf("%s.rgb = min(%s.rgb, %s.a);",
                               outputColor, outputColor, outputColor);
    }
}

class GLArithmeticFP : public GrGLFragmentProcessor {
public:
    GLArithmeticFP(const GrProcessor&)
        : fEnforcePMColor(true) {
    }

    ~GLArithmeticFP() override {}

    void emitCode(EmitArgs& args) override {
        GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
        fsBuilder->codeAppend("vec4 bgColor = ");
        fsBuilder->appendTextureLookup(args.fSamplers[0], args.fCoords[0].c_str(),
                                       args.fCoords[0].getType());
        fsBuilder->codeAppendf(";");
        const char* dstColor = "bgColor";

        fKUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                    kVec4f_GrSLType, kDefault_GrSLPrecision,
                                    "k");
        const char* kUni = args.fBuilder->getUniformCStr(fKUni);

        add_arithmetic_code(fsBuilder, args.fInputColor, dstColor, args.fOutputColor, kUni,
                            fEnforcePMColor);
    }

    void setData(const GrGLProgramDataManager& pdman, const GrProcessor& proc) override {
        const GrArithmeticFP& arith = proc.cast<GrArithmeticFP>();
        pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
        fEnforcePMColor = arith.enforcePMColor();
    }

    static void GenKey(const GrProcessor& proc, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) {
        const GrArithmeticFP& arith = proc.cast<GrArithmeticFP>();
        uint32_t key = arith.enforcePMColor() ? 1 : 0;
        b->add32(key);
    }

private:
    GrGLProgramDataManager::UniformHandle fKUni;
    bool fEnforcePMColor;

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrArithmeticFP::GrArithmeticFP(GrProcessorDataManager*, float k1, float k2, float k3, float k4,
                               bool enforcePMColor, GrTexture* background)
  : fK1(k1), fK2(k2), fK3(k3), fK4(k4), fEnforcePMColor(enforcePMColor) {
    this->initClassID<GrArithmeticFP>();

    SkASSERT(background);

    fBackgroundTransform.reset(kLocal_GrCoordSet, background,
                               GrTextureParams::kNone_FilterMode);
    this->addCoordTransform(&fBackgroundTransform);
    fBackgroundAccess.reset(background);
    this->addTextureAccess(&fBackgroundAccess);
}

void GrArithmeticFP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLArithmeticFP::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* GrArithmeticFP::createGLInstance() const {
    return SkNEW_ARGS(GLArithmeticFP, (*this));
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

GrFragmentProcessor* GrArithmeticFP::TestCreate(GrProcessorTestData* d) {
    float k1 = d->fRandom->nextF();
    float k2 = d->fRandom->nextF();
    float k3 = d->fRandom->nextF();
    float k4 = d->fRandom->nextF();
    bool enforcePMColor = d->fRandom->nextBool();

    return SkNEW_ARGS(GrArithmeticFP, (d->fProcDataManager, k1, k2, k3, k4, enforcePMColor,
                                       d->fTextures[0]));
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

    GrGLXferProcessor* createGLInstance() const override;

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) override;

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

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

class GLArithmeticXP : public GrGLXferProcessor {
public:
    GLArithmeticXP(const GrProcessor&)
        : fEnforcePMColor(true) {
    }

    ~GLArithmeticXP() override {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const ArithmeticXP& arith = processor.cast<ArithmeticXP>();
        uint32_t key = arith.enforcePMColor() ? 1 : 0;
        b->add32(key);
    }

private:
    void emitBlendCodeForDstRead(GrGLXPBuilder* pb, const char* srcColor, const char* dstColor,
                                 const char* outColor, const GrXferProcessor& proc) override {
        GrGLXPFragmentBuilder* fsBuilder = pb->getFragmentShaderBuilder();

        fKUni = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                               kVec4f_GrSLType, kDefault_GrSLPrecision,
                               "k");
        const char* kUni = pb->getUniformCStr(fKUni);

        add_arithmetic_code(fsBuilder, srcColor, dstColor, outColor, kUni, fEnforcePMColor);
    }

    void onSetData(const GrGLProgramDataManager& pdman,
                   const GrXferProcessor& processor) override {
        const ArithmeticXP& arith = processor.cast<ArithmeticXP>();
        pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
        fEnforcePMColor = arith.enforcePMColor();
    };

    GrGLProgramDataManager::UniformHandle fKUni;
    bool fEnforcePMColor;

    typedef GrGLXferProcessor INHERITED;
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

void ArithmeticXP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLArithmeticXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* ArithmeticXP::createGLInstance() const {
    return SkNEW_ARGS(GLArithmeticXP, (*this));
}

GrXferProcessor::OptFlags ArithmeticXP::onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                           const GrProcOptInfo& coveragePOI,
                                                           bool doesStencilWrite,
                                                           GrColor* overrideColor,
                                                           const GrCaps& caps) {
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
                                             const GrProcOptInfo& colorPOI,
                                             const GrProcOptInfo& coveragePOI,
                                             bool hasMixedSamples,
                                             const DstTexture* dstTexture) const {
    return SkNEW_ARGS(ArithmeticXP, (dstTexture, hasMixedSamples, fK1, fK2, fK3, fK4,
                                     fEnforcePMColor));
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

GrXPFactory* GrArithmeticXPFactory::TestCreate(GrProcessorTestData* d) {
    float k1 = d->fRandom->nextF();
    float k2 = d->fRandom->nextF();
    float k3 = d->fRandom->nextF();
    float k4 = d->fRandom->nextF();
    bool enforcePMColor = d->fRandom->nextBool();

    return GrArithmeticXPFactory::Create(k1, k2, k3, k4, enforcePMColor);
}

#endif
