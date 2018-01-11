/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRegion.h"
#include "SkSurface.h"

bool FuzzRegionDeserialize(sk_sp<SkData> bytes) {
    SkRegion region;
    if (!region.readFromMemory(bytes->data(), bytes->size())) {
        return false;
    }
    region.computeRegionComplexity();
    region.isComplex();
    SkRegion r2;
    if (region == r2) {
        region.contains(0,0);
    } else {
        region.contains(1,1);
    }
    auto s = SkSurface::MakeRasterN32Premul(1024, 1024);
    s->getCanvas()->drawRegion(region, SkPaint());
    SkDEBUGCODE(region.validate());
    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzRegionDeserialize(bytes);
    return 0;
}
#endif
