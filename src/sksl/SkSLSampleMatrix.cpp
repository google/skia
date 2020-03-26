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
    if (fKind == Kind::kVariable || other.fKind == Kind::kVariable) {
        *this = SampleMatrix(Kind::kVariable);
        return *this;
    }
    if (other.fKind == Kind::kConstantOrUniform) {
        if (fKind == other.fKind) {
            if (fExpression == other.fExpression) {
                return *this;
            }
            *this = SampleMatrix(Kind::kVariable);
            return *this;
        }
        SkASSERT(fKind == Kind::kNone);
        *this = other;
        return *this;
    }
    return *this;
}

} // namespace
