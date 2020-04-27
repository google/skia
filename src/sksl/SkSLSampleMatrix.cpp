/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSampleMatrix.h"

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

SampleMatrix SampleMatrix::merge(const SampleMatrix& other) {
    if ((fFlags & kVariable_Flag) || (other.fFlags & kVariable_Flag)) {
        *this = SampleMatrix(kVariable_Flag);
    }
    else if (other.fFlags & (kVertex_Flag | kFragment_Flag)) {
        if (fFlags == other.fFlags) {
            if (fExpression != other.fExpression) {
                *this = SampleMatrix(kVariable_Flag);
            }
        } else {
            SkASSERT(fFlags == 0);
            *this = other;
        }
    }
    return *this;
}

} // namespace
