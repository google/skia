/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

DEF_FUZZ(RegionOp, fuzz) {  // `fuzz -t api -n RegionOp`
    SkRegion regionA;
    int8_t numOps;
    fuzz->next(&numOps);
    for (int8_t i = 0; i < numOps; i++) {
        SkRegion regionB;
        FuzzNiceRegion(fuzz, &regionB, 100);
        SkRegion::Op op;
        fuzz->nextEnum(&op, SkRegion::kLastOp);
        regionA.op(regionB, op);
    }
}
