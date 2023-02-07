/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixShader_DEFINED
#define SkLocalMatrixShader_DEFINED

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

class GrFragmentProcessor;
class SkArenaAlloc;

class SkLocalMatrixShader final : public SkShaderBase {
public:
    template <typename T, typename... Args>
    static std::enable_if_t<std::is_base_of_v<SkShader, T>, sk_sp<SkShader>>
    MakeWrapped(const SkMatrix* localMatrix, Args&&... args) {
        auto t = sk_make_sp<T>(std::forward<Args>(args)...);
        if (!localMatrix || localMatrix->isIdentity()) {
            return std::move(t);
        }
        return sk_make_sp<SkLocalMatrixShader>(sk_sp<SkShader>(std::move(t)), *localMatrix);
    }

    SkLocalMatrixShader(sk_sp<SkShader> wrapped, const SkMatrix& localMatrix)
            : fLocalMatrix(localMatrix), fWrappedShader(std::move(wrapped)) {}

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const override {
        if (localMatrix) {
            *localMatrix = fLocalMatrix;
        }
        return fWrappedShader;
    }

protected:
    void flatten(SkWriteBuffer&) const override;

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

    SkImage* onIsAImage(SkMatrix* matrix, SkTileMode* mode) const override;

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixShader)

    SkMatrix fLocalMatrix;
    sk_sp<SkShader> fWrappedShader;

    using INHERITED = SkShaderBase;
};

#endif
