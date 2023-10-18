/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "src/core/SkRegionPriv.h"

bool FuzzRegionDeserialize(const uint8_t *data, size_t size) {
    SkRegion region;
    if (!region.readFromMemory(data, size)) {
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
    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }
    s->getCanvas()->drawRegion(region, SkPaint());
    SkDEBUGCODE(SkRegionPriv::Validate(region));
    return true;
}

// TODO(kjlubick): remove IS_FUZZING... after https://crrev.com/c/2410304 lands
#if defined(SK_BUILD_FOR_LIBFUZZER) || defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 512) {
        return 0;
    }
    FuzzRegionDeserialize(data, size);
    return 0;
}
#endif
