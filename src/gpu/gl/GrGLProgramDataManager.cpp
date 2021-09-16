/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLProgramDataManager.h"

#include "include/core/SkMatrix.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

GrGLProgramDataManager::UniformManager::UniformManager(const GrUniformAggregator& uniformAggregator,
                                                       GrGLuint programID,
                                                       GrGLint firstUniformLocation,
                                                       const GrGLContext& ctx) {
    GrGLint location = firstUniformLocation;
    fUniforms.reserve(uniformAggregator.uniformCount());
    for (int p = 0; p < uniformAggregator.numProcessors(); ++p) {
        fUniforms.push_back({});
        for (const GrUniformAggregator::Record& record : uniformAggregator.processorRecords(p)) {
            const GrProcessor::Uniform& u = record.uniform();
            if (ctx.caps()->bindUniformLocationSupport() && firstUniformLocation >= 0) {
                GR_GL_CALL(ctx.glInterface(),
                           BindUniformLocation(programID, location, record.name.c_str()));
            } else {
                GR_GL_CALL_RET(ctx.glInterface(),
                               location,
                               GetUniformLocation(programID, record.name.c_str()));
            }
            fUniforms.back().push_back({
                    record.indexInProcessor,
                    u.type(),
                    u.count(),
                    location,
            });
            location++;
        }
    }
}

void GrGLProgramDataManager::UniformManager::setUniforms(const GrGLInterface* gl,
                                                         const GrProgramInfo& info) {
    auto set = [&, processorIndex = 0](const GrProcessor& p) mutable {
        const ProcessorUniforms& uniforms = fUniforms[processorIndex];
        for (const Uniform& u : uniforms) {
            if (u.location < 0) {
                // Presumably this got optimized out.
                continue;
            }
            size_t index = u.indexInProcessor;
            SkASSERT(u.count >= 0);
            static_assert(GrShaderVar::kNonArray == 0);
            int n = std::max(1, u.count);
            switch (u.type) {
                case kInt_GrSLType: {
                    const int32_t* values = p.uniformData<int32_t>(index);
                    GR_GL_CALL(gl, Uniform1iv(u.location, n, values));
                    break;
                }

                case kInt2_GrSLType: {
                    const int32_t* values = p.uniformData<int32_t>(index);
                    GR_GL_CALL(gl, Uniform2iv(u.location, n, values));
                    break;
                }

                case kInt3_GrSLType: {
                    const int32_t* values = p.uniformData<int32_t>(index);
                    GR_GL_CALL(gl, Uniform3iv(u.location, n, values));
                    break;
                }

                case kInt4_GrSLType: {
                    const int32_t* values = p.uniformData<int32_t>(index);
                    GR_GL_CALL(gl, Uniform4iv(u.location, n, values));
                    break;
                }

                case kHalf_GrSLType:
                case kFloat_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, Uniform1fv(u.location, n, values));
                    break;
                }

                case kHalf2_GrSLType:
                case kFloat2_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, Uniform2fv(u.location, n, values));
                    break;
                }

                case kHalf3_GrSLType:
                case kFloat3_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, Uniform3fv(u.location, n, values));
                    break;
                }

                case kHalf4_GrSLType:
                case kFloat4_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, Uniform4fv(u.location, n, values));
                    break;
                }

                case kHalf2x2_GrSLType:
                case kFloat2x2_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, UniformMatrix2fv(u.location, n, false, values));
                    break;
                }

                case kHalf3x3_GrSLType:
                case kFloat3x3_GrSLType: {
                    switch (p.uniforms()[index].ctype()) {
                        case GrProcessor::Uniform::CType::kDefault: {
                            const float* values = p.uniformData<float>(index);
                            GR_GL_CALL(gl, UniformMatrix3fv(u.location, n, false, values));
                            break;
                        }
                        case GrProcessor::Uniform::CType::kSkMatrix: {
                            const SkMatrix* matrix = p.uniformData<SkMatrix>(index);
                            int location = u.location;
                            for (int i = 0; i < n; ++i, ++matrix, ++location) {
                                float mt[] = {
                                        matrix->get(SkMatrix::kMScaleX),
                                        matrix->get(SkMatrix::kMSkewY),
                                        matrix->get(SkMatrix::kMPersp0),
                                        matrix->get(SkMatrix::kMSkewX),
                                        matrix->get(SkMatrix::kMScaleY),
                                        matrix->get(SkMatrix::kMPersp1),
                                        matrix->get(SkMatrix::kMTransX),
                                        matrix->get(SkMatrix::kMTransY),
                                        matrix->get(SkMatrix::kMPersp2),
                                };
                                GR_GL_CALL(gl, UniformMatrix3fv(location, 1, false, mt));
                            }
                            break;
                        }
                    }
                    break;
                }

                case kHalf4x4_GrSLType:
                case kFloat4x4_GrSLType: {
                    const float* values = p.uniformData<float>(index);
                    GR_GL_CALL(gl, UniformMatrix4fv(u.location, n, false, values));
                    break;
                }

                default:
                    SK_ABORT("Unexpect uniform type");
            }
        }
        ++processorIndex;
    };

    info.visitProcessors(set);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define ASSERT_ARRAY_UPLOAD_IN_BOUNDS(UNI, COUNT) \
         SkASSERT((COUNT) <= (UNI).fArrayCount || \
                  (1 == (COUNT) && GrShaderVar::kNonArray == (UNI).fArrayCount))

GrGLint get_first_unused_uniform_location(
        const GrGLProgramDataManager::UniformInfoArray& uniforms,
        const GrGLProgramDataManager::UniformInfoArray& samplers) {
    GrGLint id = -1;
    for (int i = 0; i < uniforms.count(); ++i) {
        id = std::max(id, uniforms.item(i).fLocation);
    }
    for (int i = 0; i < samplers.count(); ++i) {
        id = std::max(id, samplers.item(i).fLocation);
    }
    return id + 1;
}

GrGLProgramDataManager::GrGLProgramDataManager(GrGLGpu* gpu,
                                               const UniformInfoArray& uniforms,
                                               const UniformInfoArray& samplers,
                                               GrGLuint programID,
                                               bool usedProgramBinaries,
                                               const GrUniformAggregator& uniformAggregator)
        : fGpu(gpu)
        , fManager(uniformAggregator,
                   programID,
                   usedProgramBinaries ? -1 : get_first_unused_uniform_location(uniforms, samplers),
                   gpu->glContext()) {
    fUniforms.push_back_n(uniforms.count());
    int i = 0;
    for (const GLUniformInfo& builderUniform : uniforms.items()) {
        Uniform& uniform = fUniforms[i++];
        SkASSERT(GrShaderVar::kNonArray == builderUniform.fVariable.getArrayCount() ||
                 builderUniform.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = builderUniform.fVariable.getArrayCount();
            uniform.fType = builderUniform.fVariable.getType();
        )
        uniform.fLocation = builderUniform.fLocation;
    }
}

void GrGLProgramDataManager::setUniforms(const GrProgramInfo& info) {
    fManager.setUniforms(fGpu->glInterface(), info);
}

void GrGLProgramDataManager::setSamplerUniforms(const UniformInfoArray& samplers,
                                                int startUnit) const {
    int i = 0;
    for (const GLUniformInfo& sampler : samplers.items()) {
        SkASSERT(sampler.fVisibility);
        if (kUnusedUniform != sampler.fLocation) {
            GR_GL_CALL(fGpu->glInterface(), Uniform1i(sampler.fLocation, i + startUnit));
        }
        ++i;
    }
}

void GrGLProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fLocation, i));
    }
}

void GrGLProgramDataManager::set1iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fLocation, v0));
    }
}

void GrGLProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    // This assert fires in some instances of the two-pt gradient for its VSParams.
    // Once the uniform manager is responsible for inserting the duplicate uniform
    // arrays in VS and FS driver bug workaround, this can be enabled.
    // this->printUni(uni);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2i(uni.fLocation, i0, i1));
    }
}

void GrGLProgramDataManager::set2iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fLocation, v0, v1));
    }
}

void GrGLProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3i(UniformHandle u, int32_t i0, int32_t i1, int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3i(uni.fLocation, i0, i1, i2));
    }
}

void GrGLProgramDataManager::set3iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fLocation, v0, v1, v2));
    }
}

void GrGLProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4i(UniformHandle u,
                                   int32_t i0,
                                   int32_t i1,
                                   int32_t i2,
                                   int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4i(uni.fLocation, i0, i1, i2, i3));
    }
}

void GrGLProgramDataManager::set4iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4f(UniformHandle u,
                                   float v0,
                                   float v1,
                                   float v2,
                                   float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fLocation, v0, v1, v2, v3));
    }
}

void GrGLProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrGLProgramDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrGLProgramDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrGLProgramDataManager::setMatrices(UniformHandle u,
                                                                int arrayCount,
                                                                const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        set_uniform_matrix<N>::set(fGpu->glInterface(), uni.fLocation, arrayCount, matrices);
    }
}

template<> struct set_uniform_matrix<2> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix2fv(loc, cnt, false, m));
    }
};

template<> struct set_uniform_matrix<3> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix3fv(loc, cnt, false, m));
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix4fv(loc, cnt, false, m));
    }
};
