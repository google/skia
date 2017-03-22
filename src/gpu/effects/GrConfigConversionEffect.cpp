/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConfigConversionEffect.h"
#include "../private/GrGLSL.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrGLConfigConversionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrConfigConversionEffect& cce = args.fFp.cast<GrConfigConversionEffect>();
        GrConfigConversionEffect::PMConversion pmConversion = cce.pmConversion();

        // Using highp for GLES here in order to avoid some precision issues on specific GPUs.
        GrShaderVar tmpVar("tmpColor", kVec4f_GrSLType, 0, kHigh_GrSLPrecision);
        SkString tmpDecl;

        args.fFragBuilder->codeAppendf("%s = %s;", tmpVar.c_str(), args.fImageStorages);

        SkString modulate;
        GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    }
};

GrGLSLFragmentProcessor* GrConfigConversionEffect::onCreateGLSLInstance() const {
    return new GrGLConfigConversionEffect();
}
