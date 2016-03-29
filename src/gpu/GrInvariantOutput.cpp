/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInvariantOutput.h"

#ifdef SK_DEBUG

void GrInvariantOutput::validate() const {
    if (fIsSingleComponent) {
        SkASSERT(0 == fValidFlags || kRGBA_GrColorComponentFlags == fValidFlags);
        if (kRGBA_GrColorComponentFlags == fValidFlags) {
            SkASSERT(this->colorComponentsAllEqual());
        }
    }

    // If we claim that we are not using the input color we must not be modulating the input.
    SkASSERT(fNonMulStageFound || fWillUseInputColor);
}

bool GrInvariantOutput::colorComponentsAllEqual() const {
    unsigned colorA = GrColorUnpackA(fColor);
    return(GrColorUnpackR(fColor) == colorA &&
           GrColorUnpackG(fColor) == colorA &&
           GrColorUnpackB(fColor) == colorA);
}

#endif // end DEBUG
