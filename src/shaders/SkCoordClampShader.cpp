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
    auto clampCtx = rec.fAlloc->make<SkRasterPipelineContexts::CoordClampCtx>();
    *clampCtx = {fSubset.fLeft, fSubset.fTop, fSubset.fRight, fSubset.fBottom};
    rec.fPipeline->append(SkRasterPipelineOp::clamp_x_and_y, clampCtx);
    return as_SB(fShader)->appendStages(rec, *childMRec);
}

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
