/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

void fuzz_PolyUtils(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    if (size > 4000) {
        return 0;
    }
    auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
    fuzz_PolyUtils(&fuzz);
#endif
    return 0;
}
