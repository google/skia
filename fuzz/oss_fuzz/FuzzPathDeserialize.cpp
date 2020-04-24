/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "src/core/SkReadBuffer.h"

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
    if (size < 4) {
        return 0;
    }
    uint32_t packed;
    memcpy(&packed, data, 4);
    unsigned version = packed & 0xFF;
    if (version != 4) {
        // Chrome only will produce version 4, so guide the fuzzer to
        // only focus on those branches.
        return 0;
    }
    SkReadBuffer buf(data, size);
    FuzzPathDeserialize(buf);
    return 0;
}
#endif
