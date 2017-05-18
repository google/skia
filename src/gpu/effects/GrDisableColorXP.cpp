/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrDisableColorXP.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLXferProcessor.h"

/**
 * This xfer processor disables color writing. Thus color and coverage and ignored and no blending
 * occurs. This XP is usful for things like stenciling.
 */
class DisableColorXP : public GrXferProcessor {
public:
    DisableColorXP() { this->initClassID<DisableColorXP>(); }

    const char* name() const override { return "Disable Color"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

private:

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        return true;
    }

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLDisableColorXP : public GrGLSLXferProcessor {
public:
    GLDisableColorXP(const GrProcessor&) {}

    ~GLDisableColorXP() override {}

    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*) {}

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        // This emit code should be empty. However, on the nexus 6 there is a driver bug where if
        // you do not give gl_FragColor a value, the gl context is lost and we end up drawing
        // nothing. So this fix just sets the gl_FragColor arbitrarily to 0.
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
        fragBuilder->codeAppendf("%s = vec4(0);", args.fOutputPrimary);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void DisableColorXP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    GLDisableColorXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* DisableColorXP::createGLSLInstance() const { return new GLDisableColorXP(*this); }

void DisableColorXP::onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
    blendInfo->fWriteColor = false;
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<const GrXferProcessor> GrDisableColorXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor&,
        GrProcessorAnalysisCoverage,
        bool hasMixedSamples,
        const GrCaps& caps) const {
    return sk_sp<const GrXferProcessor>(new DisableColorXP);
}

GR_DEFINE_XP_FACTORY_TEST(GrDisableColorXPFactory);

#if GR_TEST_UTILS
const GrXPFactory* GrDisableColorXPFactory::TestGet(GrProcessorTestData*) {
    return GrDisableColorXPFactory::Get();
}
#endif
