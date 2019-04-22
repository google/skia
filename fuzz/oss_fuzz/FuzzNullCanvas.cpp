/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "src/core/SkFontMgrPriv.h"
#include "tools/fonts/TestFontMgr.h"

void fuzz_NullCanvas(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
    fuzz_NullCanvas(&fuzz);
    return 0;
}
