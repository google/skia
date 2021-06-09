/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomBlend_DEFINED
#define SkCustomBlend_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkVM.h"

/**
 * Encapsulates a custom blend function for Runtime Effects. These combine a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SkCustomBlend : public SkRefCnt {
public:
    SK_WARN_UNUSED_RESULT
    virtual skvm::Color program(skvm::Builder*, skvm::Color srcColor, skvm::Color dstColor,
                                skvm::Uniforms*, SkArenaAlloc*) const = 0;
};

#endif  // SkCustomBlend_DEFINED
