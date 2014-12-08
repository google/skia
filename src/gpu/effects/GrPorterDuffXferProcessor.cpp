/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrPorterDuffXferProcessor.h"

#include "GrDrawState.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GrGLPorterDuffXferProcessor : public GrGLXferProcessor {
public:
    GrGLPorterDuffXferProcessor(const GrProcessor&) {}

    virtual ~GrGLPorterDuffXferProcessor() {}

    virtual void emitCode(GrGLFPBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        GrGLFPFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = %s;", outputColor, inputColor);
    }

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {};

    static void GenKey(const GrProcessor&, const GrGLCaps& caps, GrProcessorKeyBuilder* b) {};

private:
    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrPorterDuffXferProcessor::GrPorterDuffXferProcessor(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend)
    : fSrcBlend(srcBlend), fDstBlend(dstBlend) {
    this->initClassID<GrPorterDuffXferProcessor>();
}

GrPorterDuffXferProcessor::~GrPorterDuffXferProcessor() {
}

void GrPorterDuffXferProcessor::getGLProcessorKey(const GrGLCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GrGLPorterDuffXferProcessor::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* GrPorterDuffXferProcessor::createGLInstance() const {
    return SkNEW_ARGS(GrGLPorterDuffXferProcessor, (*this));
}

void GrPorterDuffXferProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GrPorterDuffXPFactory::GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst)
    : fSrc(src), fDst(dst) {
    this->initClassID<GrPorterDuffXPFactory>();
}

GrXPFactory* GrPorterDuffXPFactory::Create(SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kClear_Mode: {
            static GrPorterDuffXPFactory gClearPDXPF(kZero_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gClearPDXPF);
            break;
        }
        case SkXfermode::kSrc_Mode: {
            static GrPorterDuffXPFactory gSrcPDXPF(kOne_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcPDXPF);
            break;
        }
        case SkXfermode::kDst_Mode: {
            static GrPorterDuffXPFactory gDstPDXPF(kZero_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gDstPDXPF);
            break;
        }
        case SkXfermode::kSrcOver_Mode: {
            static GrPorterDuffXPFactory gSrcOverPDXPF(kOne_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gSrcOverPDXPF);
            break;
        }
        case SkXfermode::kDstOver_Mode: {
            static GrPorterDuffXPFactory gDstOverPDXPF(kIDA_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gDstOverPDXPF);
            break;
        }
        case SkXfermode::kSrcIn_Mode: {
            static GrPorterDuffXPFactory gSrcInPDXPF(kDA_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcInPDXPF);
            break;
        }
        case SkXfermode::kDstIn_Mode: {
            static GrPorterDuffXPFactory gDstInPDXPF(kZero_GrBlendCoeff, kSA_GrBlendCoeff);
            return SkRef(&gDstInPDXPF);
            break;
        }
        case SkXfermode::kSrcOut_Mode: {
            static GrPorterDuffXPFactory gSrcOutPDXPF(kIDA_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcOutPDXPF);
            break;
        }
        case SkXfermode::kDstOut_Mode: {
            static GrPorterDuffXPFactory gDstOutPDXPF(kZero_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gDstOutPDXPF);
            break;
        }
        case SkXfermode::kSrcATop_Mode: {
            static GrPorterDuffXPFactory gSrcATopPDXPF(kDA_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gSrcATopPDXPF);
            break;
        }
        case SkXfermode::kDstATop_Mode: {
            static GrPorterDuffXPFactory gDstATopPDXPF(kIDA_GrBlendCoeff, kSA_GrBlendCoeff);
            return SkRef(&gDstATopPDXPF);
            break;
        }
        case SkXfermode::kXor_Mode: {
            static GrPorterDuffXPFactory gXorPDXPF(kIDA_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gXorPDXPF);
            break;
        }
        case SkXfermode::kPlus_Mode: {
            static GrPorterDuffXPFactory gPlusPDXPF(kOne_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gPlusPDXPF);
            break;
        }
        case SkXfermode::kModulate_Mode: {
            static GrPorterDuffXPFactory gModulatePDXPF(kZero_GrBlendCoeff, kSC_GrBlendCoeff);
            return SkRef(&gModulatePDXPF);
            break;
        }
        case SkXfermode::kScreen_Mode: {
            static GrPorterDuffXPFactory gScreenPDXPF(kOne_GrBlendCoeff, kISC_GrBlendCoeff);
            return SkRef(&gScreenPDXPF);
            break;
        }
        default:
            return NULL;
    }
}

const GrXferProcessor* GrPorterDuffXPFactory::createXferProcessor() const {
    return GrPorterDuffXferProcessor::Create(fSrc, fDst);
}

bool GrPorterDuffXPFactory::supportsRGBCoverage(GrColor /*knownColor*/,
                                                uint32_t knownColorFlags) const {
    if (kOne_GrBlendCoeff == fSrc && kISA_GrBlendCoeff == fDst &&
        kRGBA_GrColorComponentFlags == knownColorFlags) {
        return true;
    }
    return false;
}

