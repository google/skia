/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDescriptor.h"
#include "src/core/SkRemoteGlyphCache.h"

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

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkDescriptorDeserialize(&fuzz);
    return 0;
}
#endif
