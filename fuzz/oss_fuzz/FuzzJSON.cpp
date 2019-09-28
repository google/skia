/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/utils/SkJSON.h"

void FuzzJSON(sk_sp<SkData> bytes) {
    skjson::DOM dom(static_cast<const char*>(bytes->data()), bytes->size());
    SkDynamicMemoryWStream wstream;
    dom.write(&wstream);
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzJSON(bytes);
    return 0;
}
#endif
