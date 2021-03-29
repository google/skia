/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

DEF_FUZZ(RegionOp, fuzz) {  // `fuzz -t api -n RegionOp`
    SkRegion region;
    // FuzzNiceRegion generates a random region by joining a random amount of regions
    // together. This fuzzer simply targets that directly. 300 was picked arbitrarily as
    // a number over 2^8.
    FuzzNiceRegion(fuzz, &region, 300);
    // Do a computation to make sure region is not optimized out.
    region.computeRegionComplexity();
}
