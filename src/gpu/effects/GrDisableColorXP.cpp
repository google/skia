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

    ~DisableColorXP() override {};

    const char* name() const override { return "Disable Color"; }

    GrGLXferProcessor* createGLInstance() const override;

private:
    DisableColorXP();

    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* color,
                                                 const GrCaps& caps) override {
        return GrXferProcessor::kIgnoreColor_OptFlag | GrXferProcessor::kIgnoreCoverage_OptFlag;
    }

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        return true;
    }

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLDisableColorXP : public GrGLXferProcessor {
public:
    GLDisableColorXP(const GrProcessor&) {}

    ~GLDisableColorXP() override {}

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        // This emit code should be empty. However, on the nexus 6 there is a driver bug where if
        // you do not give gl_FragColor a value, the gl context is lost and we end up drawing
        // nothing. So this fix just sets the gl_FragColor arbitrarily to 0.
        GrGLXPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = vec4(0);", args.fOutputPrimary);
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DisableColorXP::DisableColorXP() {
    this->initClassID<DisableColorXP>();
}

void DisableColorXP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLDisableColorXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* DisableColorXP::createGLInstance() const {
    return SkNEW_ARGS(GLDisableColorXP, (*this));
}

void DisableColorXP::onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
    blendInfo->fWriteColor = false;
}

///////////////////////////////////////////////////////////////////////////////

GrDisableColorXPFactory::GrDisableColorXPFactory() {
    this->initClassID<GrDisableColorXPFactory>();
}

GrXferProcessor*
GrDisableColorXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                               const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& covPOI,
                                               bool hasMixedSamples,
                                               const DstTexture* dst) const {
    return DisableColorXP::Create();
}

GR_DEFINE_XP_FACTORY_TEST(GrDisableColorXPFactory);

GrXPFactory* GrDisableColorXPFactory::TestCreate(GrProcessorTestData*) {
    return GrDisableColorXPFactory::Create();
}

