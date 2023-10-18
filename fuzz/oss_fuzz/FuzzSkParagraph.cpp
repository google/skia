/*
 * Copyright 2021 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

#if defined(SK_ENABLE_PARAGRAPH)

void fuzz_SkParagraph(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4000) {
        return 0;
    }
    auto fuzz = Fuzz(data, size);
    fuzz_SkParagraph(&fuzz);
    return 0;
}

#endif
