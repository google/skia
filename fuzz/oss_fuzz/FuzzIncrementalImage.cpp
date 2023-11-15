/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTemplates.h"

bool FuzzIncrementalImageDecode(const uint8_t *data, size_t size) {
    auto codec = SkCodec::MakeFromStream(SkMemoryStream::MakeDirect(data, size));
    if (!codec) {
        return false;
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(codec->getInfo())) {
        // May fail in memory-constrained fuzzing environments
        return false;
    }

    auto result = codec->startIncrementalDecode(bm.info(), bm.getPixels(), bm.rowBytes());
    if (result != SkCodec::kSuccess) {
        return false;
    }

    // Deliberately uninitialized to verify that incrementalDecode initializes it when it
    // returns kIncompleteInput or kErrorInInput.
    int rowsDecoded;
    result = codec->incrementalDecode(&rowsDecoded);
    switch (result) {
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            if (rowsDecoded < bm.height()) {
                void* dst = SkTAddOffset<void>(bm.getPixels(), rowsDecoded * bm.rowBytes());
                sk_bzero(dst, (bm.height() - rowsDecoded) * bm.rowBytes());
            }
            return true; // decoded a partial image
         case SkCodec::kSuccess:
            return true;
         default:
            return false;
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    FuzzIncrementalImageDecode(data, size);
    return 0;
}
#endif
