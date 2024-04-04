/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"

#include "include/core/SkMatrix.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkMatrixPriv.h"

#include <cstddef>

void GrGLSLProgramDataManager::setSkMatrix(UniformHandle u, const SkMatrix& matrix) const {
    float mt[] = {
        matrix.get(SkMatrix::kMScaleX),
        matrix.get(SkMatrix::kMSkewY),
        matrix.get(SkMatrix::kMPersp0),
        matrix.get(SkMatrix::kMSkewX),
        matrix.get(SkMatrix::kMScaleY),
        matrix.get(SkMatrix::kMPersp1),
        matrix.get(SkMatrix::kMTransX),
        matrix.get(SkMatrix::kMTransY),
        matrix.get(SkMatrix::kMPersp2),
    };
    this->setMatrix3f(u, mt);
}

void GrGLSLProgramDataManager::setSkM44(UniformHandle u, const SkM44& matrix) const {
    this->setMatrix4f(u, SkMatrixPriv::M44ColMajor(matrix));
}

void GrGLSLProgramDataManager::setRuntimeEffectUniforms(
        SkSpan<const SkRuntimeEffect::Uniform> uniforms,
        SkSpan<const UniformHandle> handles,
        SkSpan<const Specialized> specialized,
        const void* src) const {
    SkASSERT(uniforms.empty() || src);
    SkASSERT(specialized.empty() || specialized.size() == uniforms.size());
    SkASSERT(!specialized.empty() || handles.size() == uniforms.size());

    using Type = SkRuntimeEffect::Uniform::Type;
    size_t handleIdx = 0;
    for (size_t uniformIdx = 0; uniformIdx < uniforms.size(); ++uniformIdx) {
        const auto& u = uniforms[uniformIdx];
        auto floatData = [=] { return SkTAddOffset<const float>(src, u.offset); };
        auto intData   = [=] { return SkTAddOffset<const int  >(src, u.offset); };
        if (!specialized.empty() && specialized[uniformIdx] == Specialized::kYes) {
            continue;
        }
        const auto h = handles[handleIdx++];
        switch (u.type) {
            case Type::kFloat:  this->set1fv(h, u.count, floatData()); break;
            case Type::kFloat2: this->set2fv(h, u.count, floatData()); break;
            case Type::kFloat3: this->set3fv(h, u.count, floatData()); break;
            case Type::kFloat4: this->set4fv(h, u.count, floatData()); break;

            case Type::kFloat2x2: this->setMatrix2fv(h, u.count, floatData()); break;
            case Type::kFloat3x3: this->setMatrix3fv(h, u.count, floatData()); break;
            case Type::kFloat4x4: this->setMatrix4fv(h, u.count, floatData()); break;

            case Type::kInt:  this->set1iv(h, u.count, intData()); break;
            case Type::kInt2: this->set2iv(h, u.count, intData()); break;
            case Type::kInt3: this->set3iv(h, u.count, intData()); break;
            case Type::kInt4: this->set4iv(h, u.count, intData()); break;

            default:
                SkDEBUGFAIL("Unsupported uniform type");
                break;
        }
    }
    SkASSERT(handleIdx == handles.size());
}
