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
    // TODO: add a size + len skjson::DOM factory?
    SkAutoMalloc data(bytes->size() + 1);
    auto* c_str = static_cast<char*>(data.get());

    memcpy(c_str, bytes->data(), bytes->size());
    c_str[bytes->size()] = '\0';

    skjson::DOM dom(c_str);
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
