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
                   const GrFragmentProcessor& proc,
                   SkIPoint viewportOffset) override {
        const GrMatrixEffect& mtx = proc.cast<GrMatrixEffect>();

//        SkDebugf("GrGLSLMatrixEffect::onSetData\n");
//        SkDebugf("%.2f %.2f %.2f\n", mtx.matrix()[0], mtx.matrix()[1], mtx.matrix()[2]);
//        SkDebugf("%.2f %.2f %.2f\n", mtx.matrix()[3], mtx.matrix()[4], mtx.matrix()[5]);
//        SkDebugf("%.2f %.2f %.2f\n", mtx.matrix()[6], mtx.matrix()[7], mtx.matrix()[8]);

//        if (mtx.matrix()[0] == 1) {
//            pdman.setSkMatrix(fMatrixVar, SkMatrix::I()); //mtx.matrix());
//        } else {
            pdman.setSkMatrix(fMatrixVar, mtx.matrix());
//        }
    }

    UniformHandle fMatrixVar;
};

GrGLSLFragmentProcessor* GrMatrixEffect::onCreateGLSLInstance() const {
    return new GrGLSLMatrixEffect();
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
