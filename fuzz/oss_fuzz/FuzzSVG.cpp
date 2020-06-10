/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "experimental/svg/model/SkSVGDOM.h"
void FuzzSVG(sk_sp<SkData> bytes) {
    SkMemoryStream stream(bytes);
    auto dom = SkSVGDOM::MakeFromStream(stream);
    if (!dom) {
        return;
    }
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSVG(bytes);
    return 0;
}
#endif