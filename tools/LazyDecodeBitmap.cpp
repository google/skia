/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"

#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkDiscardableMemoryPool.h"
#include "SkImageGenerator.h"
#include "SkForceLinking.h"

#include "SkCommandLineFlags.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_bool(useVolatileCache, false, "Use a volatile cache for deferred image decoding pixels. "
            "Only meaningful if --deferImageDecoding is set to true and the platform has an "
            "implementation.");

//  Fits SkPicture::InstallPixelRefProc call signature.
//  Used in SkPicturePlayback::CreateFromStream
bool sk_tools::LazyDecodeBitmap(const void* src,
                                size_t length,
                                SkBitmap* dst) {
    SkAutoDataUnref data(SkData::NewWithCopy(src, length));
    if (NULL == data.get()) {
        return false;
    }

    SkAutoTDelete<SkImageGenerator> gen(SkNEW_ARGS(SkDecodingImageGenerator,
                                                   (data)));
    SkImageInfo info;
    if (!gen->getInfo(&info)) {
        return false;
    }
    SkDiscardableMemory::Factory* pool = NULL;
    if ((!FLAGS_useVolatileCache) || (info.fWidth * info.fHeight < 32 * 1024)) {
        // how to do switching with SkDiscardableMemory.
        pool = SkGetGlobalDiscardableMemoryPool();
        // Only meaningful if platform has a default discardable
        // memory implementation that differs from the global DM pool.
    }
    return SkInstallDiscardablePixelRef(gen.detach(), dst, pool);
}
