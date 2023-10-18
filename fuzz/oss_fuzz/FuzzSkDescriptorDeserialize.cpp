/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDescriptor.h"
#include "src/core/SkReadBuffer.h"

void FuzzSkDescriptorDeserialize(const uint8_t *data, size_t size) {
    SkReadBuffer buffer{data, size};
    auto sut = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!sut.has_value()) {
        return;
    }

    auto desc = sut->getDesc();

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
    FuzzSkDescriptorDeserialize(data, size);
    return 0;
}
#endif
