/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../Fuzz.h"
#include "SkFontMgrPriv.h"
#include "TestFontMgr.h"

void fuzz_RasterN32Canvas(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
    auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
    fuzz_RasterN32Canvas(&fuzz);
    return 0;
}
