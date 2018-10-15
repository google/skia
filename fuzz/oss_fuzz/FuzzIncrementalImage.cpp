/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"

void FuzzIncrementalImage(sk_sp<SkData> bytes) {
    auto codec = SkCodec::MakeFromData(bytes);
    if (!codec) {
        return;
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(codec->getInfo())) {
        // May fail in memory-constrained fuzzing environments
        return;
    }

    auto result = codec->startIncrementalDecode(bm.info(), bm.getPixels(), bm.rowBytes());
    if (result != SkCodec::kSuccess) {
        return;
    }

    // Deliberately uninitialized to verify that incrementalDecode initializes it.
    int rowsDecoded;
    result = codec->incrementalDecode(&rowsDecoded);
    if (result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput) {
        if (rowsDecoded < bm.height()) {
            void* dst = SkTAddOffset<void>(bm.getPixels(), rowsDecoded * bm.rowBytes());
            sk_bzero(dst, (bm.height() - rowsDecoded) * bm.rowBytes());
        }
    }
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzIncrementalImage(bytes);
    return 0;
}
#endif
