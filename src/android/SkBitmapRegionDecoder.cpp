/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#ifdef SK_ENABLE_ANDROID_UTILS
#include "client_utils/android/BitmapRegionDecoder.h"
#include "include/android/SkBitmapRegionDecoder.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"

// Note: This file is only temporary while we switch Android over to BitmapRegionDecoder
// directly.

namespace {
struct Context {
    std::unique_ptr<SkStreamRewindable> stream;
};
}

void release_proc(const void* ptr, void* context) {
    delete reinterpret_cast<Context*>(context);
}

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkStreamRewindable* stream, Strategy strategy) {
    std::unique_ptr<SkStreamRewindable> streamDeleter(stream);
    const void* memoryBase = streamDeleter->getMemoryBase();
    if (!memoryBase) {
        // All existing clients use an SkMemoryStream.
        SkASSERT(false);
        SkCodecPrintf("Error: Need an SkMemoryStream to create an SkBitmapRegionDecoder!");
        return nullptr;
    }
    Context* context = new Context;
    context->stream = std::move(streamDeleter);
    auto data = SkData::MakeWithProc(memoryBase, context->stream->getLength(), release_proc,
                                     context);
    return android::skia::BitmapRegionDecoder::Make(std::move(data)).release();
}
#endif // SK_ENABLE_ANDROID_UTILS
