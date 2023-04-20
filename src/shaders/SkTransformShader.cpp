/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/shaders/SkTransformShader.h"

SkTransformShader::SkTransformShader(const SkShaderBase& shader, bool allowPerspective)
        : fShader{shader}, fAllowPerspective{allowPerspective} {
    SkMatrix::I().get9(fMatrixStorage);
}

skvm::Color SkTransformShader::program(skvm::Builder* b,
                                       skvm::Coord device,
                                       skvm::Coord local,
                                       skvm::Color color,
                                       const MatrixRec& mRec,
                                       const SkColorInfo& dst,
                                       skvm::Uniforms* uniforms,
                                       SkArenaAlloc* alloc) const {
    // We have to seed and apply any constant matrices before appending our matrix that may
    // mutate. We could try to apply one matrix stage and then incorporate the parent matrix
    // with the variable matrix in each call to update(). However, in practice our callers
    // fold the CTM into the update() matrix and don't wrap the transform shader in local matrix
    // shaders so the call to apply below should be no-op. If this assert fires it just indicates an
    // optimization opportunity, not a correctness bug.
    SkASSERT(!mRec.hasPendingMatrix());

    std::optional<MatrixRec> childMRec = mRec.apply(b, &local, uniforms);
    if (!childMRec.has_value()) {
        return {};
    }
    // The matrix we're about to insert gets updated between uses of the VM so our children can't
    // know the total transform when they add their stages. We don't incorporate this shader's
    // matrix into the MatrixRec at all.
    childMRec->markTotalMatrixInvalid();

    auto matrix = uniforms->pushPtr(&fMatrixStorage);

    skvm::F32 x = local.x,
              y = local.y;

    auto dot = [&, x, y](int row) {
        return b->mad(x,
                      b->arrayF(matrix, 3 * row + 0),
                      b->mad(y, b->arrayF(matrix, 3 * row + 1), b->arrayF(matrix, 3 * row + 2)));
    };

    x = dot(0);
    y = dot(1);
    if (fAllowPerspective) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    skvm::Coord newLocal = {x, y};
    return fShader.program(b, device, newLocal, color, *childMRec, dst, uniforms, alloc);
}

bool SkTransformShader::update(const SkMatrix& matrix) {
    if (SkMatrix inv; matrix.invert(&inv)) {
        if (!fAllowPerspective && inv.hasPerspective()) {
            return false;
        }

        inv.get9(fMatrixStorage);
        return true;
    }
    return false;
}

bool SkTransformShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    // We have to seed and apply any constant matrices before appending our matrix that may
    // mutate. We could try to add one matrix stage and then incorporate the parent matrix
    // with the variable matrix in each call to update(). However, in practice our callers
    // fold the CTM into the update() matrix and don't wrap the transform shader in local matrix
    // shaders so the call to apply below should just seed the coordinates. If this assert fires
    // it just indicates an optimization opportunity, not a correctness bug.
    SkASSERT(!mRec.hasPendingMatrix());
    std::optional<MatrixRec> childMRec = mRec.apply(rec);
    if (!childMRec.has_value()) {
        return false;
    }
    // The matrix we're about to insert gets updated between uses of the pipeline so our children
    // can't know the total transform when they add their stages. We don't even incorporate this
    // matrix into the MatrixRec at all.
    childMRec->markTotalMatrixInvalid();

    auto type = fAllowPerspective ? SkRasterPipelineOp::matrix_perspective
                                  : SkRasterPipelineOp::matrix_2x3;
    rec.fPipeline->append(type, fMatrixStorage);

    fShader.appendStages(rec, *childMRec);
    return true;
}
