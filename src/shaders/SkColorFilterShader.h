/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "src/core/SkColorFilterBase.h"
#include "src/shaders/SkShaderBase.h"

class SkArenaAlloc;

class SkColorFilterShader : public SkShaderBase {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, float alpha, sk_sp<SkColorFilter> filter);

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    bool isOpaque() const override;
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>          fShader;
    sk_sp<SkColorFilterBase> fFilter;
    float                    fAlpha;

    using INHERITED = SkShaderBase;
};

#endif
