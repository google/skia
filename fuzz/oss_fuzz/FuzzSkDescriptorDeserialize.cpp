/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkDescriptor.h"

void FuzzSkDescriptorDeserialize(sk_sp<SkData> bytes) {
    SkAutoDescriptor aDesc;
    bool ok = SkFuzzDeserializeSkDescriptor(bytes, &aDesc);
    if (!ok) {
         return;
    }

    auto desc = aDesc.getDesc();

    desc->computeChecksum();
    desc->isValid();

    // An arbitrary number
    uint32_t tagToFind = 117;

    uint32_t ignore;
    desc->findEntry(tagToFind, &ignore);
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 1024) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkDescriptorDeserialize(bytes);
    return 0;
}
#endif
