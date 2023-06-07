/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRadialGradient_DEFINED
#define SkRadialGradient_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

class SkArenaAlloc;
class SkMatrix;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkRadialGradient final : public SkGradientBaseShader {
public:
    SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* matrix) const override;
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif
protected:
    SkRadialGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc,
                              SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;
#if defined(SK_ENABLE_SKVM)
    skvm::F32 transformT(skvm::Builder*,
                         skvm::Uniforms*,
                         skvm::Coord coord,
                         skvm::I32* mask) const final;
#endif

private:
    friend void ::SkRegisterRadialGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkRadialGradient)

    const SkPoint fCenter;
    const SkScalar fRadius;
};

#endif
