/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/shaders/SkTransformShader.h"

SkTransformShader::SkTransformShader(const SkShaderBase& shader) : fShader{shader} {}

skvm::Color SkTransformShader::onProgram(skvm::Builder* b,
                      skvm::Coord device, skvm::Coord local, skvm::Color color,
                      const SkMatrixProvider& matrices, const SkMatrix* localM,
                      const SkColorInfo& dst,
                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Coord newLocal = this->applyMatrix(b, matrices.localToDevice(), local, uniforms);
    SkSimpleMatrixProvider matrixProvider{SkMatrix::I()};
    return fShader.program(
            b, device, newLocal, color, matrixProvider, localM, dst, uniforms, alloc);
}

skvm::Coord SkTransformShader::applyMatrix(
        skvm::Builder* b, const SkMatrix& matrix, skvm::Coord local,
        skvm::Uniforms* uniforms) const {
    fMatrix = uniforms->pushPtr(&fMatrixStorage);

    skvm::F32 x = local.x,
              y = local.y;

    auto dot = [&,x,y](int row) {
        return b->mad(x, b->arrayF(fMatrix, 3*row+0),
                      b->mad(y, b->arrayF(fMatrix, 3*row+1),
                             b->arrayF(fMatrix, 3*row+2)));
    };

    x = dot(0);
    y = dot(1);
    fProcessingAsPerspective = matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective();
    if (fProcessingAsPerspective) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    return {x, y};
}

void SkTransformShader::appendMatrix(const SkMatrix& matrix, SkRasterPipeline* p) const {
    fProcessingAsPerspective = matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective();
    if (fProcessingAsPerspective) {
        p->append(SkRasterPipeline::matrix_perspective, fMatrixStorage);
    } else {
        p->append(SkRasterPipeline::matrix_2x3, fMatrixStorage);
    }
}

bool SkTransformShader::update(const SkMatrix& ctm) const {
    if (SkMatrix matrix; this->computeTotalInverse(ctm, nullptr, &matrix)) {
        if (!fProcessingAsPerspective) {
            SkASSERT(!matrix.hasPerspective());
            if (matrix.hasPerspective()) {
                return false;
            }
        }

        matrix.get9(fMatrixStorage);
        return true;
    }
    return false;
}

bool SkTransformShader::onAppendStages(const SkStageRec& rec) const {
    // TODO
    return false;
}
