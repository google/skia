/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageMode_DEFINED
#define SkCoverageMode_DEFINED

#include "SkTypes.h"

//A =          X*p0(As,Ad) +     Y*p1(As,Ad) +     Z*p2(As,Ad)

enum class SkCoverageOverlap {
                    // X                Y               Z
    kUncorrelated,  // S*D              S*(1-D)         D*(1-S)
    kConjoint,      // min(S,D)         max(S-D,0)      max(D-S,0)
    kDisjoint,      // max(S+D-1,0)     min(S,1-D)      min(D,1-S)
};

enum class SkCoverageMode {
    kUnion,             // 1,1,1    S+D-S*D     srcover, dstover
    kIntersect,         // 1,0,0    S*D         srcin, dstin
    kDifference,        // 0,1,0    S*(1-D)     srcout
    kReverseDifference, // 0,0,1    D*(1-S)     dstout
    kXor,               // 0,1,1    S+D-2*S*D   xor
};

#endif
