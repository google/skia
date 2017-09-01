/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../Fuzz.h"
#include "../FuzzCommon.cpp"
#include "SkData.h"
#include "SkPath.h"
#include "SkRegion.h"


int fuzzRegionSetPath(Fuzz* fuzz) {
    SkPath p;
    fuzz_path(fuzz, &p, 1000);
    SkRegion r1;
    bool initR1;
    fuzz->next(&initR1);
    if (initR1) {
        fuzz->next(&r1);
    }
    SkRegion r2;
    fuzz->next(&r2);

    r1.setPath(p, r2);

    // Do some follow on computations to make sure region is well-formed.
    r1.computeRegionComplexity();
    r1.isComplex();
    if (r1 == r2) {
        r1.contains(0,0);
    } else {
        r1.contains(1,1);
    }

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    sk_sp<SkData> bytes(SkData::MakeWithoutCopy(data, size));
    Fuzz fuzz(bytes);
    return fuzzRegionSetPath(&fuzz);
}

