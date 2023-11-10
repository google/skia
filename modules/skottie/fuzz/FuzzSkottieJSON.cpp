/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "modules/skottie/include/Skottie.h"
#include "src/core/SkFontMgrPriv.h"
#include "tools/fonts/TestFontMgr.h"

void FuzzSkottieJSON(const uint8_t *data, size_t size) {
    SkMemoryStream stream(data, size);
    auto animation = skottie::Animation::Make(&stream);
    if (!animation) {
        return;
    }
    animation->seek(0.1337f); // A "nothing up my sleeve" number
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    FuzzSkottieJSON(data, size);
    return 0;
}
#endif
