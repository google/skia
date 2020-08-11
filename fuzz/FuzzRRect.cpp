/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// To use this fuzzer follow the libfuzzer directions in
// site/dev/testing/fuzz.md.

#include "include/core/SkRRect.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* buf, size_t size) {
    SkRRect rrect;
    (void)rrect.readFromMemory(buf, size);
    return 0;
}
