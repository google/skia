/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkTransformShader.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <optional>

SkTransformShader::SkTransformShader(const SkShaderBase& shader, bool allowPerspective)
        : fShader{shader}, fAllowPerspective{allowPerspective} {
    SkMatrix::I().get9(fMatrixStorage);
}

bool SkTransformShader::update(const SkMatrix& matrix) {
    if (!fAllowPerspective && matrix.hasPerspective()) {
        return false;
    }

    matrix.get9(fMatrixStorage);
    return true;
}

bool SkTransformShader::appendStages(const SkStageRec& rec,
                                     const SkShaders::MatrixRec& mRec) const {
    // We have to seed and apply any constant matrices before appending our matrix that may
    // mutate. We could try to add one matrix stage and then incorporate the parent matrix
    // with the variable matrix in each call to update(). However, in practice our callers
    // fold the CTM into the update() matrix and don't wrap the transform shader in local matrix
    // shaders so the call to apply below should just seed the coordinates. If this assert fires
    // it just indicates an optimization opportunity, not a correctness bug.
    SkASSERT(!mRec.hasPendingMatrix());
    std::optional<SkShaders::MatrixRec> childMRec = mRec.apply(rec);
    if (!childMRec.has_value()) {
        return false;
    }
    // The matrix we're about to insert gets updated between uses of the pipeline so our children
    // can't know the total transform when they add their stages. We don't even incorporate this
    // matrix into the SkShaders::MatrixRec at all.
    childMRec->markTotalMatrixInvalid();

    auto type = fAllowPerspective ? SkRasterPipelineOp::matrix_perspective
                                  : SkRasterPipelineOp::matrix_2x3;
    rec.fPipeline->append(type, fMatrixStorage);

    fShader.appendStages(rec, *childMRec);
    return true;
}
