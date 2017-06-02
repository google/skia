/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrBlurredEdgeFragmentProcessor.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GLSLBlurredEdgeFP : public GrGLSLFragmentProcessor {
public:
    GLSLBlurredEdgeFP() {}

    void emitCode(EmitArgs& args) override {

        GrBlurredEdgeFP::Mode mode = args.fFp.cast<GrBlurredEdgeFP>().mode();

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
        fragBuilder->codeAppendf("// assuming interpolant is set in vertex colors\n");
        fragBuilder->codeAppendf("float factor = 1.0 - color.a;");
        switch (mode) {
            case GrBlurredEdgeFP::kGaussian_Mode:
                fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
                break;
            case GrBlurredEdgeFP::kSmoothstep_Mode:
                fragBuilder->codeAppend("factor = smoothstep(1.0, 0.0, factor);");
                break;
        }
        fragBuilder->codeAppendf("%s = vec4(factor);", args.fOutputColor);
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& proc) override {}

    GrBlurredEdgeFP::Mode fMode;
};

GrGLSLFragmentProcessor* GrBlurredEdgeFP::onCreateGLSLInstance() const {
    return new GLSLBlurredEdgeFP();
}

void GrBlurredEdgeFP::onGetGLSLProcessorKey(const GrShaderCaps& caps, 
                                            GrProcessorKeyBuilder* b) const {
    b->add32(fMode);
}

bool GrBlurredEdgeFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrBlurredEdgeFP& that = other.cast<GrBlurredEdgeFP>();
    return that.fMode == fMode;
}


