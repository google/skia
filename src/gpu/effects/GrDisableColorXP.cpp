/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrDisableColorXP.h"
#include "GrShaderCaps.h"
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
    DisableColorXP() : INHERITED(kDisableColorXP_ClassID) {}

private:
    const char* name() const override { return "Disable Color"; }
    bool onIsEqual(const GrXferProcessor& xpBase) const override { return true; }
    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        return;  // No key.
    }
    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fWriteColor = false;
    }
    GrGLSLXferProcessor* createGLSLInstance() const override;

    typedef GrXferProcessor INHERITED;
};

class GLDisableColorXP : public GrGLSLXferProcessor {
private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        if (args.fShaderCaps->mustWriteToFragColor()) {
            // This emit code should be empty. However, on the nexus 6 there is a driver bug where
            // if you do not give gl_FragColor a value, the gl context is lost and we end up drawing
            // nothing. So this fix just sets the gl_FragColor arbitrarily to 0.
            // https://bugs.chromium.org/p/chromium/issues/detail?id=445377
            GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
            fragBuilder->codeAppendf("%s = half4(0);", args.fOutputPrimary);
        }
    }

    void emitOutputSwizzle(
            GrGLSLXPFragmentBuilder*, const GrSwizzle&, const char*, const char*) const override {
        // Don't write any swizzling. This makes sure the final shader does not output a color.
        return;
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

GrGLSLXferProcessor* DisableColorXP::createGLSLInstance() const {
    return new GLDisableColorXP();
}

GrDisableColorXPFactory::GrDisableColorXPFactory()
        : fXferProcessor(new DisableColorXP()) {
}

GR_DEFINE_XP_FACTORY_TEST(GrDisableColorXPFactory);

#if GR_TEST_UTILS
const GrXPFactory* GrDisableColorXPFactory::TestGet(GrProcessorTestData*) {
    return GrDisableColorXPFactory::Get();
}
#endif
