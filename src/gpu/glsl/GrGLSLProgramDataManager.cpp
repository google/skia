/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLProgramDataManager.h"

#include "SkMatrix.h"
#include "SkMatrix44.h"

void GrGLSLProgramDataManager::setSkMatrix(UniformHandle u, const SkMatrix& matrix) const {
    float mt[] = {
        matrix.get(SkMatrix::kMScaleX),
        matrix.get(SkMatrix::kMSkewY),
        matrix.get(SkMatrix::kMPersp0),
        matrix.get(SkMatrix::kMSkewX),
        matrix.get(SkMatrix::kMScaleY),
        matrix.get(SkMatrix::kMPersp1),
        matrix.get(SkMatrix::kMTransX),
        matrix.get(SkMatrix::kMTransY),
        matrix.get(SkMatrix::kMPersp2),
    };
    this->setMatrix3f(u, mt);
}

void GrGLSLProgramDataManager::setSkMatrix44(UniformHandle u, const SkMatrix44& matrix) const {
    // TODO: We could skip this temporary buffer if we had direct access to the matrix storage
    float m[16];
    matrix.asColMajorf(m);
    this->setMatrix4f(u, m);
}
