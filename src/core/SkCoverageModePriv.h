/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageModePriv_DEFINED
#define SkCoverageModePriv_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkCoverageMode.h"

SkBlendMode SkUncorrelatedCoverageModeToBlendMode(SkCoverageMode);

#if 0
// Experimental idea to extend to overlap types

Master calculation =   X(S,D) + Y(S,D) + Z(S,D)

enum class SkCoverageOverlap {
                    // X                Y               Z
    kUncorrelated,  // S*D              S*(1-D)         D*(1-S)
    kConjoint,      // min(S,D)         max(S-D,0)      max(D-S,0)
    kDisjoint,      // max(S+D-1,0)     min(S,1-D)      min(D,1-S)

    kLast = kDisjoint
};

// The coverage modes each have a set of coefficients to be applied to the general equation (above)
//
//  e.g.
//     kXor+conjoint = max(S-D,0) + max(D-S,0) ==> abs(D-S)
//
kUnion,             // 1,1,1
kIntersect,         // 1,0,0
kDifference,        // 0,1,0
kReverseDifference, // 0,0,1
kXor,               // 0,1,1

#endif

#endif
