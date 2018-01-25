/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageModes_DEFINED
#define SkCoverageModes_DEFINED

#include "SkTypes.h"

//A =          X*p0(As,Ad) +     Y*p1(As,Ad) +     Z*p2(As,Ad)

enum class SkCoverageOverlap {
                    // X                Y               Z
    kUncorrelated,  // S*D              S*(1-D)         D*(1-S)
    kConjoint,      // min(S,D)         max(S-D,0)      max(D-S,0)
    kDisjoint,      // max(S+D-1,0)     min(S,1-D)      min(D,1-S)
};

enum class SkCoverageMode {
    kZero,      // 0,0,0
    kSrc,       // 1,1,0
    kDst,       // 1,0,1
    kOver,      // 1,1,1    union
    kIn,        // 1,0,0    intersect
    kSrcOut,    // 0,1,0    difference
    kDstOut,    // 0,0,1    reverse difference
    kXor,       // 0,1,1    xor
};

#endif
