/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/private/SkHdrMetadata.h"

#include <cstddef>
#include <cstdint>

void FuzzHdrAgtm(const uint8_t* data, size_t size) {
    sk_sp<SkData> skdata = SkData::MakeWithoutCopy(data, size);
    skhdr::AdaptiveGlobalToneMap agtm;
    if (agtm.parse(skdata.get())) {
        auto serialized = agtm.serialize();
        (void)serialized;
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // The SMPTE ST 2094-50 bitstream is very compact (max 4 alternate images, max 32 control
    // points per curve). Valid data is always well under 1KB. Limiting the size improves
    // fuzzer efficiency by reducing the search space.
    if (size > 1024) {
        return 0;
    }
    FuzzHdrAgtm(data, size);
    return 0;
}
#endif
