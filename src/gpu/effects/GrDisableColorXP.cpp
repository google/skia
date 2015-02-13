/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrDisableColorXP.h"
#include "GrProcessor.h"
#include "gl/GrGLXferProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

/**
 * This xfer processor disables color writing. Thus color and coverage and ignored and no blending
 * occurs. This XP is usful for things like stenciling.
 */
class DisableColorXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create() {
        return SkNEW(DisableColorXP);
    }

    ~DisableColorXP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Disable Color"; }

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* color,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE {
        return GrXferProcessor::kIgnoreColor_OptFlag | GrXferProcessor::kIgnoreCoverage_OptFlag;
    }

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE;

private:
    DisableColorXP();

    void onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
        return true;
    }

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLDisableColorXP : public GrGLXferProcessor {
public:
    GLDisableColorXP(const GrProcessor&) {}

    ~GLDisableColorXP() SK_OVERRIDE {}

    static void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*) {}

private:
    void onEmitCode(const EmitArgs& args) SK_OVERRIDE {
        // This emit code should be empty. However, on the nexus 6 there is a driver bug where if
        // you do not give gl_FragColor a value, the gl context is lost and we end up drawing
        // nothing. So this fix just sets the gl_FragColor arbitrarily to 0.
        GrGLFPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = vec4(0);", args.fOutputPrimary);
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) SK_OVERRIDE {}

    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DisableColorXP::DisableColorXP() {
    this->initClassID<DisableColorXP>();
}

void DisableColorXP::onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLDisableColorXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* DisableColorXP::createGLInstance() const {
    return SkNEW_ARGS(GLDisableColorXP, (*this));
}

void DisableColorXP::getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
    blendInfo->fWriteColor = false;
}

///////////////////////////////////////////////////////////////////////////////

GrDisableColorXPFactory::GrDisableColorXPFactory() {
    this->initClassID<GrDisableColorXPFactory>();
}

GrXferProcessor*
GrDisableColorXPFactory::onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                               const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& covPOI,
                                               const GrDeviceCoordTexture* dstCopy) const {
    return DisableColorXP::Create();
}

GR_DEFINE_XP_FACTORY_TEST(GrDisableColorXPFactory);

GrXPFactory* GrDisableColorXPFactory::TestCreate(SkRandom* random,
                                                  GrContext*,
                                                  const GrDrawTargetCaps&,
                                                  GrTexture*[]) {
    return GrDisableColorXPFactory::Create();
}

