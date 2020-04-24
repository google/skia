/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

DEF_FUZZ(RegionOp, fuzz) {  // `fuzz -t api -n RegionOp`
    SkRegion regionA, regionB, regionC;
    FuzzNiceRegion(fuzz, &regionA, 2000);
    FuzzNiceRegion(fuzz, &regionB, 2000);
    SkRegion::Op op;
    fuzz->nextRange(&op, 0, SkRegion::kLastOp);
    regionC.op(regionA, regionB, op);
}
