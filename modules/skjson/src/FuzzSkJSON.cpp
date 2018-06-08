/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkData.h"
#include "SkJSON.h"
#include "SkStream.h"

void FuzzSkJSON(sk_sp<SkData> bytes) {
    // TODO: add a size + len skjson::Dom factory?
    SkAutoMalloc c_str(bytes->size() + 1);
    auto* data = static_cast<char*>(c_str.get());

    memcpy(data, bytes->data(), bytes->size());
    data[bytes->size()] = '\0';

    skjson::DOM dom(data);
    SkDynamicMemoryWStream wstream;
    dom.write(&wstream);
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkJSON(bytes);
    return 0;
}
#endif
