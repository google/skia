/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTextBlobPriv.h"
#include "tools/fonts/TestFontMgr.h"

void FuzzTextBlobDeserialize(SkReadBuffer& buf) {
    auto tb = SkTextBlobPriv::MakeFromBuffer(buf);
    if (!buf.isValid()) {
        return;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return;
    }
    s->getCanvas()->drawTextBlob(tb, 200, 200, SkPaint());
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    SkReadBuffer buf(data, size);
    FuzzTextBlobDeserialize(buf);
    return 0;
}
#endif
