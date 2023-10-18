/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "modules/skcms/skcms.h"

void FuzzColorspace(const uint8_t *data, size_t size) {
    sk_sp<SkColorSpace> space(SkColorSpace::Deserialize(data, size));
    if (!space) {
        return;
    }
    // Call some arbitrary methods on the colorspace, using a throw-away
    // variable to prevent the compiler from optimizing things away.
    int i = 0;
    if (space->gammaCloseToSRGB()) {
        i += 1;
    }
    if (space->gammaIsLinear()) {
        i += 1;
    }
    if (space->isSRGB()) {
        i += 1;
    }
    skcms_ICCProfile profile;
    space->toProfile(&profile);
    sk_sp<SkColorSpace> space2 = space->makeLinearGamma()->makeSRGBGamma()->makeColorSpin();
    sk_sp<SkData> data1 = space->serialize();
    if (SkColorSpace::Equals(space.get(), space2.get()) && i > 5) {
        SkDebugf("Should never happen %d", (int)data1->size());
        space2->writeToMemory(nullptr);
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4000) {
        return 0;
    }
    FuzzColorspace(data, size);
    return 0;
}
#endif
