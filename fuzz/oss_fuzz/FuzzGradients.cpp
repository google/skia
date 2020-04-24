/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

void fuzz_Gradients(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
    fuzz_Gradients(&fuzz);
    return 0;
}
