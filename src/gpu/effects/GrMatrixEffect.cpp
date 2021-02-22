/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrMatrixEffect.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

class GrGLSLMatrixEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLMatrixEffect() {}

    void emitCode(EmitArgs& args) override {
        fMatrixVar = args.fUniformHandler->addUniform(&args.fFp, kFragment_GrShaderFlag,
                                                      kFloat3x3_GrSLType, "matrix");
        args.fFragBuilder->codeAppendf("return %s;\n",
                                       this->invokeChildWithMatrix(0, args).c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& proc) override {
        const GrMatrixEffect& mtx = proc.cast<GrMatrixEffect>();
        pdman.setSkMatrix(fMatrixVar, mtx.matrix());
    }

    UniformHandle fMatrixVar;
};

std::unique_ptr<GrGLSLFragmentProcessor> GrMatrixEffect::onMakeProgramImpl() const {
    return std::make_unique<GrGLSLMatrixEffect>();
}

void GrMatrixEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                           GrProcessorKeyBuilder* b) const {}

bool GrMatrixEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrMatrixEffect& that = other.cast<GrMatrixEffect>();
    if (fMatrix != that.fMatrix) return false;
    return true;
}

GrMatrixEffect::GrMatrixEffect(const GrMatrixEffect& src)
        : INHERITED(kGrMatrixEffect_ClassID, src.optimizationFlags())
        , fMatrix(src.fMatrix) {
    this->cloneAndRegisterAllChildProcessors(src);
}

std::unique_ptr<GrFragmentProcessor> GrMatrixEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrMatrixEffect(*this));
}
