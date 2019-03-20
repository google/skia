/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkFontMgrPriv.h"
#include "SkStream.h"
#include "Skottie.h"
#include "TestFontMgr.h"

void FuzzSkottieJSON(sk_sp<SkData> bytes) {
    SkMemoryStream stream(bytes);
    auto animation = skottie::Animation::Make(&stream);
    if (!animation) {
        return;
    }
    animation->seek(0.1337f); // A "nothing up my sleeve" number
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkottieJSON(bytes);
    return 0;
}
#endif
