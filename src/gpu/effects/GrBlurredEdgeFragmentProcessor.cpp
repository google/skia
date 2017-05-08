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
        if (!args.fGpImplementsDistanceVector) {
            fragBuilder->codeAppendf("// assuming interpolant is set in vertex colors\n");
            fragBuilder->codeAppendf("float factor = color.a;");
        } else {
            fragBuilder->codeAppendf("// using distance to edge to compute interpolant\n");
            fragBuilder->codeAppend("float radius = color.r*256.0*64.0 + color.g*64.0;");
            fragBuilder->codeAppend("float pad = color.b*64.0;");

            fragBuilder->codeAppendf("float factor = clamp((%s.z - pad)/radius, 0.0, 1.0);",
                                     fragBuilder->distanceVectorName());
        }
        switch (mode) {
            case GrBlurredEdgeFP::kGaussian_Mode:
#if 0
                fragBuilder->codeAppend("factor = 1.0 - factor;");
                fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
#else
                fragBuilder->codeAppend("// use quartic approximation for guassian\n");
                fragBuilder->codeAppend("float c0 =  0.00030726194381713867;");
                fragBuilder->codeAppend("float c1 =  0.15489584207534790039;");
                fragBuilder->codeAppend("float c2 =  0.21345567703247070312;");
                fragBuilder->codeAppend("float c3 =  2.89795351028442382812;");
                fragBuilder->codeAppend("float c4 = -2.26661229133605957031;");
                fragBuilder->codeAppend("factor = c0 + factor*(c1 + factor*(c2 + factor*(c3 + factor*c4)));");
#endif
                break;
            case GrBlurredEdgeFP::kSmoothstep_Mode:
                fragBuilder->codeAppend("factor = smoothstep(1.0, 0.0, 1.0 - factor);");
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


