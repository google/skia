/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkData.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"


void FuzzRegionSetPath(Fuzz* fuzz) {
    SkPath p;
    FuzzNicePath(fuzz, &p, 1000);
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
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    sk_sp<SkData> bytes(SkData::MakeWithoutCopy(data, size));
    Fuzz fuzz(bytes);
    FuzzRegionSetPath(&fuzz);
    return 0;
}
#endif
