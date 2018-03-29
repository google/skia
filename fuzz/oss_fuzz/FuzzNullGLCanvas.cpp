/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../Fuzz.h"

void fuzz_NullGLCanvas(Fuzz* f);

// Don't print the list of LSAN suppressions on every execution.
static const char* kLsanDefaultOptions = "print_suppressions=0";

extern "C" {

    // Set default LSAN options.
    __attribute__((no_sanitize("address", "memory", "thread", "undefined"))) \
    __attribute__((visibility("default"))) \
    __attribute__((used)) const char *__lsan_default_options() {
        return kLsanDefaultOptions;
    }

    int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
        fuzz_NullGLCanvas(&fuzz);
        return 0;
    }
}
