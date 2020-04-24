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
    if (fExpression == "") {
        fExpression = other.fExpression;
    } else {
        SkASSERT(other.fExpression == "");
    }
    fFlags |= other.fFlags;
    if (fFlags & kFragment_Flag) {
    	fFlags &= ~kVertex_Flag;
    }
    if (!fOwner) {
    	fOwner = other.fOwner;
    } else {
    	SkASSERT(other.fOwner == nullptr || other.fOwner == fOwner);
    }
    return *this;
}

} // namespace
