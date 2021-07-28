/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
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
    if (matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective()) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    return {x, y};
}

bool SkTransformShader::update(const SkMatrix& ctm) const {
    SkMatrix matrix;
    if (this->computeTotalInverse(ctm, nullptr, &matrix)) {
        for (int i = 0; i < 9; ++i) {
            fMatrixStorage[i] = matrix[i];
        }
        return true;
    }
    return false;
}

bool SkTransformShader::onAppendStages(const SkStageRec& rec) const {
    // TODO
    return false;
}
