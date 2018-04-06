/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"

DEF_FUZZ(RegionOp, fuzz) {  // `fuzz -t api -n RegionOp`
    SkRegion regionA, regionB, regionC;
    fuzz_region(fuzz, &regionA, 2000);
    fuzz_region(fuzz, &regionB, 2000);
    SkRegion::Op op;
    fuzz_enum_range(fuzz, &op, (SkRegion::Op)0, SkRegion::kLastOp);
    regionC.op(regionA, regionB, op);
}
