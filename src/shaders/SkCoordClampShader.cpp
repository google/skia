/*
 * Copyright 2023 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkCoordClampShader.h"

#include "include/core/SkFlattenable.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif // SK_GRAPHITE

#if defined(SK_ENABLE_SKVM)
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#endif

#include <optional>

sk_sp<SkFlattenable> SkCoordClampShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shader(buffer.readShader());
    SkRect subset = buffer.readRect();
    if (!buffer.validate(SkToBool(shader))) {
        return nullptr;
    }
    return SkShaders::CoordClamp(std::move(shader), subset);
}

void SkCoordClampShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    buffer.writeRect(fSubset);
}

bool SkCoordClampShader::appendStages(const SkStageRec& rec,
                                      const SkShaders::MatrixRec& mRec) const {
    std::optional<SkShaders::MatrixRec> childMRec = mRec.apply(rec);
    if (!childMRec.has_value()) {
        return false;
    }
    // Strictly speaking, childMRec's total matrix is not valid. It is only valid inside the subset
    // rectangle. However, we don't mark it as such because we want the "total matrix is valid"
    // behavior in SkImageShader for filtering.
    auto clampCtx = rec.fAlloc->make<SkRasterPipeline_CoordClampCtx>();
    *clampCtx = {fSubset.fLeft, fSubset.fTop, fSubset.fRight, fSubset.fBottom};
    rec.fPipeline->append(SkRasterPipelineOp::clamp_x_and_y, clampCtx);
    return as_SB(fShader)->appendStages(rec, *childMRec);
}

#if defined(SK_ENABLE_SKVM)
skvm::Color SkCoordClampShader::program(skvm::Builder* p,
                                        skvm::Coord device,
                                        skvm::Coord local,
                                        skvm::Color paint,
                                        const SkShaders::MatrixRec& mRec,
                                        const SkColorInfo& cinfo,
                                        skvm::Uniforms* uniforms,
                                        SkArenaAlloc* alloc) const {
    std::optional<SkShaders::MatrixRec> childMRec = mRec.apply(p, &local, uniforms);
    if (!childMRec.has_value()) {
        return {};
    }
    // See comment in appendStages about not marking childMRec with an invalid total matrix.

    auto l = uniforms->pushF(fSubset.left());
    auto t = uniforms->pushF(fSubset.top());
    auto r = uniforms->pushF(fSubset.right());
    auto b = uniforms->pushF(fSubset.bottom());

    local.x = p->clamp(local.x, p->uniformF(l), p->uniformF(r));
    local.y = p->clamp(local.y, p->uniformF(t), p->uniformF(b));

    return as_SB(fShader)->program(p, device, local, paint, *childMRec, cinfo, uniforms, alloc);
}
#endif

#if defined(SK_GRAPHITE)
void SkCoordClampShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                                  skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    CoordClampShaderBlock::CoordClampData data(fSubset);

    CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    builder->endBlock();
}
#endif // SK_GRAPHITE

void SkRegisterCoordClampShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkCoordClampShader);

    // Previous name
    SkFlattenable::Register("SkShader_CoordClamp", SkCoordClampShader::CreateProc);
}

sk_sp<SkShader> SkShaders::CoordClamp(sk_sp<SkShader> shader, const SkRect& subset) {
    if (!shader) {
        return nullptr;
    }
    if (!subset.isSorted()) {
        return nullptr;
    }
    return sk_make_sp<SkCoordClampShader>(std::move(shader), subset);
}
