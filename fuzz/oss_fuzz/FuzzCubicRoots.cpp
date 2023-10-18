/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

void fuzz_CubicRoots(Fuzz* f);

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size > 4 * sizeof(double)) {
        return 0;
    }
    auto fuzz = Fuzz(data, size);
    fuzz_CubicRoots(&fuzz);
    return 0;
}
#endif
