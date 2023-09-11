/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrMatrixEffect.h"

#include "include/core/SkString.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;

std::unique_ptr<GrFragmentProcessor> GrMatrixEffect::Make(
        const SkMatrix& matrix, std::unique_ptr<GrFragmentProcessor> child) {
    if (child->classID() == kGrMatrixEffect_ClassID) {
        auto me = static_cast<GrMatrixEffect*>(child.get());
        // registerChild's sample usage records whether the matrix used has perspective or not,
        // so we can't add perspective to 'me' if it doesn't already have it.
        if (me->fMatrix.hasPerspective() || !matrix.hasPerspective()) {
            me->fMatrix.preConcat(matrix);
            return child;
        }
    }
    return std::unique_ptr<GrFragmentProcessor>(new GrMatrixEffect(matrix, std::move(child)));
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrMatrixEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            fMatrixVar = args.fUniformHandler->addUniform(&args.fFp,
                                                          kFragment_GrShaderFlag,
                                                          SkSLType::kFloat3x3,
                                                          SkSL::SampleUsage::MatrixUniformName());
            args.fFragBuilder->codeAppendf("return %s;\n",
                                           this->invokeChildWithMatrix(0, args).c_str());
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const GrMatrixEffect& mtx = proc.cast<GrMatrixEffect>();
            if (auto te = mtx.childProcessor(0)->asTextureEffect()) {
                SkMatrix m = te->coordAdjustmentMatrix();
                m.preConcat(mtx.fMatrix);
                pdman.setSkMatrix(fMatrixVar, m);
            } else {
                pdman.setSkMatrix(fMatrixVar, mtx.fMatrix);
            }
        }

        UniformHandle fMatrixVar;
    };

    return std::make_unique<Impl>();
}

void GrMatrixEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {}

bool GrMatrixEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrMatrixEffect& that = other.cast<GrMatrixEffect>();
    if (fMatrix != that.fMatrix) return false;
    return true;
}

GrMatrixEffect::GrMatrixEffect(const GrMatrixEffect& src)
        : INHERITED(src)
        , fMatrix(src.fMatrix) {}

std::unique_ptr<GrFragmentProcessor> GrMatrixEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrMatrixEffect(*this));
}
