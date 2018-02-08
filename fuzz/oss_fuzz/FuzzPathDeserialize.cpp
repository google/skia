/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkReadBuffer.h"
#include "SkSurface.h"

void FuzzPathDeserialize(SkReadBuffer& buf) {
    SkPath path;
    buf.readPath(&path);
    if (!buf.isValid()) {
        return;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return;
    }
    s->getCanvas()->drawPath(path, SkPaint());
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SkReadBuffer buf(data, size);
    FuzzPathDeserialize(buf);
    return 0;
}
#endif
