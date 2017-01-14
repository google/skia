/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInvariantOutput.h"

#ifdef SK_DEBUG

void GrInvariantOutput::validate() const {
    // If we claim that we are not using the input color we must not be modulating the input.
    SkASSERT(fNonMulStageFound || fWillUseInputColor);
}

#endif // end DEBUG
