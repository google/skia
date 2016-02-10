/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"

#include "SkData.h"
#include "SkDiscardableMemoryPool.h"
#include "SkImageGeneratorPriv.h"
#include "SkForceLinking.h"

#include "SkCommandLineFlags.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_bool(useVolatileCache, false, "Use a volatile cache for deferred image decoding pixels. "
            "Only meaningful if --deferImageDecoding is set to true and the platform has an "
            "implementation.");

//  Fits SkPicture::InstallPixelRefProc call signature.
//  Used in SkPictureData::CreateFromStream
bool sk_tools::LazyDecodeBitmap(const void* src, size_t length, SkBitmap* dst) {
    SkAutoDataUnref data(SkData::NewWithCopy(src, length));
    if (nullptr == data.get()) {
        return false;
    }

    SkAutoTDelete<SkImageGenerator> gen(SkImageGenerator::NewFromEncoded(data));
    if (nullptr == gen.get()) {
        return false;
    }
    const SkImageInfo info = gen->getInfo();
    SkDiscardableMemory::Factory* pool = nullptr;
    if ((!FLAGS_useVolatileCache) || (info.width() * info.height() < 32 * 1024)) {
        // how to do switching with SkDiscardableMemory.
        pool = SkGetGlobalDiscardableMemoryPool();
        // Only meaningful if platform has a default discardable
        // memory implementation that differs from the global DM pool.
    }
    return SkDEPRECATED_InstallDiscardablePixelRef(gen.detach(), nullptr, dst, pool);
}
