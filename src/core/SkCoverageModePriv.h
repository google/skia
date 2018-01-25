/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageModePriv_DEFINED
#define SkCoverageModePriv_DEFINED

#include "SkBlendMode.h"
#include "SkCoverageMode.h"

const SkBlendMode gUncorrelatedCoverageToBlend[] = {
    SkBlendMode::kSrcOver,  // or DstOver
    SkBlendMode::kSrcIn,    // or kDstIn
    SkBlendMode::kSrcOut,
    SkBlendMode::kDstOut,
    SkBlendMode::kXor,
};

#endif
