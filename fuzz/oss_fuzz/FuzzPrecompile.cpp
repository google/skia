/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "tools/fonts/FontToolUtils.h"

void fuzz_Precompile(Fuzz* f);

extern "C" {

    // Set default LSAN options.
    const char *__lsan_default_options() {
        // Don't print the list of LSAN suppressions on every execution.
        return "print_suppressions=0";
    }

    int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        if (size > 4000) {
            return 0;
        }
        ToolUtils::UsePortableFontMgr();
        auto fuzz = Fuzz(data, size);
        fuzz_Precompile(&fuzz);
        return 0;
    }

}  // extern "C"
