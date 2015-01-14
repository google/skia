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
#include "gl/GrGLProcessor.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/builders/GrGLProgramBuilder.h"

static const bool gUseUnpremul = false;

class GLArithmeticFP : public GrGLFragmentProcessor {
public:
    GLArithmeticFP(const GrProcessor&);
    virtual ~GLArithmeticFP();

    virtual void emitCode(GrGLFPBuilder*,
                          const GrFragmentProcessor&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

    static void GenKey(const GrProcessor&, const GrGLCaps& caps, GrProcessorKeyBuilder* b);

private:
    GrGLProgramDataManager::UniformHandle fKUni;
    bool fEnforcePMColor;

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrArithmeticFP::GrArithmeticFP(float k1, float k2, float k3, float k4,
                               bool enforcePMColor, GrTexture* background)
  : fK1(k1), fK2(k2), fK3(k3), fK4(k4), fEnforcePMColor(enforcePMColor) {
    this->initClassID<GrArithmeticFP>();
    if (background) {
        fBackgroundTransform.reset(kLocal_GrCoordSet, background,
                                   GrTextureParams::kNone_FilterMode);
        this->addCoordTransform(&fBackgroundTransform);
        fBackgroundAccess.reset(background);
        this->addTextureAccess(&fBackgroundAccess);
    } else {
        this->setWillReadDstColor();
    }
}

void GrArithmeticFP::getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
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

GLArithmeticFP::GLArithmeticFP(const GrProcessor&)
   : fEnforcePMColor(true) {
}

GLArithmeticFP::~GLArithmeticFP() {
}

void GLArithmeticFP::emitCode(GrGLFPBuilder* builder,
                              const GrFragmentProcessor& fp,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray& coords,
                              const TextureSamplerArray& samplers) {

    GrTexture* backgroundTex = fp.cast<GrArithmeticFP>().backgroundTexture();
    GrGLFPFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    const char* dstColor;
    if (backgroundTex) {
        fsBuilder->codeAppend("\t\tvec4 bgColor = ");
        fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
        fsBuilder->codeAppendf(";\n");
        dstColor = "bgColor";
    } else {
        dstColor = fsBuilder->dstColor();
    }

    SkASSERT(dstColor);
    fKUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                kVec4f_GrSLType, kDefault_GrSLPrecision,
                                "k");
    const char* kUni = builder->getUniformCStr(fKUni);

    // We don't try to optimize for this case at all
    if (NULL == inputColor) {
        fsBuilder->codeAppendf("\t\tconst vec4 src = vec4(1);\n");
    } else {
        fsBuilder->codeAppendf("\t\tvec4 src = %s;\n", inputColor);
        if (gUseUnpremul) {
            fsBuilder->codeAppendf("\t\tsrc.rgb = clamp(src.rgb / src.a, 0.0, 1.0);\n");
        }
    }

    fsBuilder->codeAppendf("\t\tvec4 dst = %s;\n", dstColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppendf("\t\tdst.rgb = clamp(dst.rgb / dst.a, 0.0, 1.0);\n");
    }

    fsBuilder->codeAppendf("\t\t%s = %s.x * src * dst + %s.y * src + %s.z * dst + %s.w;\n", outputColor, kUni, kUni, kUni, kUni);
    fsBuilder->codeAppendf("\t\t%s = clamp(%s, 0.0, 1.0);\n", outputColor, outputColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
    } else if (fEnforcePMColor) {
        fsBuilder->codeAppendf("\t\t%s.rgb = min(%s.rgb, %s.a);\n", outputColor, outputColor, outputColor);
    }
}

void GLArithmeticFP::setData(const GrGLProgramDataManager& pdman, const GrProcessor& processor) {
    const GrArithmeticFP& arith = processor.cast<GrArithmeticFP>();
    pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
    fEnforcePMColor = arith.enforcePMColor();
}

void GLArithmeticFP::GenKey(const GrProcessor& processor, const GrGLCaps&,
                            GrProcessorKeyBuilder* b) {
    const GrArithmeticFP& arith = processor.cast<GrArithmeticFP>();
    uint32_t key = arith.enforcePMColor() ? 1 : 0;
    if (arith.backgroundTexture()) {
        key |= 2;
    }
    b->add32(key);
}

GrFragmentProcessor* GrArithmeticFP::TestCreate(SkRandom* rand,
                                                    GrContext*,
                                                    const GrDrawTargetCaps&,
                                                    GrTexture*[]) {
    float k1 = rand->nextF();
    float k2 = rand->nextF();
    float k3 = rand->nextF();
    float k4 = rand->nextF();
    bool enforcePMColor = rand->nextBool();

    return SkNEW_ARGS(GrArithmeticFP, (k1, k2, k3, k4, enforcePMColor, NULL));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrArithmeticFP);

#endif
