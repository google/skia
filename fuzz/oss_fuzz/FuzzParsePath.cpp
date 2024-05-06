/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkParsePath.h"

void FuzzParsePath(const uint8_t* data, size_t size) {
    SkPath path;
    // Put into a SkString first because we are not sure if the input
    // data is null-terminated or not.
    SkString input((const char*) data, size);
    if (SkParsePath::FromSVGString(input.c_str(), &path)) {
        SkString output1 = SkParsePath::ToSVGString(path, SkParsePath::PathEncoding::Absolute);
        SkString output2 = SkParsePath::ToSVGString(path, SkParsePath::PathEncoding::Relative);
        // Do something with the output so it is not optimized away.
        if (output1.startsWith("Impossible") || output2.startsWith("Impossible")) {
            SK_ABORT("invalid SVG created");
        }
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size > 1000) {
        return 0;
    }
    FuzzParsePath(data, size);
    return 0;
}
#endif
