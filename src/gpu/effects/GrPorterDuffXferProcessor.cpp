/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPorterDuffXferProcessor.h"

#include "GrBackendProcessorFactory.h"
#include "GrDrawState.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrTBackendProcessorFactory.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GrGLPorterDuffXferProcessor : public GrGLXferProcessor {
public:
    GrGLPorterDuffXferProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED(factory) {}

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
    : fSrcBlend(srcBlend), fDstBlend(dstBlend) {}

GrPorterDuffXferProcessor::~GrPorterDuffXferProcessor() {
}

const GrBackendFragmentProcessorFactory& GrPorterDuffXferProcessor::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrPorterDuffXferProcessor>::getInstance();
}

void GrPorterDuffXferProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

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

