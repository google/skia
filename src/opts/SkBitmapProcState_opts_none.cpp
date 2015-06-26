/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapScaler.h"
#include "SkBitmapProcState.h"

/*  A platform may optionally overwrite any of these with accelerated
    versions. On input, these will already have valid function pointers,
    so a platform need only overwrite the ones it chooses, based on the
    current state (e.g. fBitmap, fInvMatrix, etc.)

    fShaderProc32
    fShaderProc16
    fMatrixProc
    fSampleProc32
    fSampleProc32
 */

// empty implementation just uses default supplied function pointers
void SkBitmapProcState::platformProcs() {}

// empty implementation just uses default supplied function pointers
void SkBitmapScaler::PlatformConvolutionProcs(SkConvolutionProcs*) {}
