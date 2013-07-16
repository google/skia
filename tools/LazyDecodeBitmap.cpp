/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"

#include "PictureRenderingFlags.h"  // --deferImageDecoding is defined here.
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkForceLinking.h"
#include "SkLruImageCache.h"
#include "SkPurgeableImageCache.h"
#include "SkCommandLineFlags.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_bool(useVolatileCache, false, "Use a volatile cache for deferred image decoding pixels. "
            "Only meaningful if --deferImageDecoding is set to true and the platform has an "
            "implementation.");

SkLruImageCache gLruImageCache(1024 * 1024);

namespace sk_tools {

// Simple cache selector to choose between a purgeable cache for large images and the standard one
// for smaller images.
//
class CacheSelector : public SkBitmapFactory::CacheSelector {

public:
    CacheSelector() {
        fPurgeableImageCache = SkPurgeableImageCache::Create();
    }

    ~CacheSelector() {
        SkSafeUnref(fPurgeableImageCache);
    }

    virtual SkImageCache* selectCache(const SkImage::Info& info) SK_OVERRIDE {
        if (info.fWidth * info.fHeight > 32 * 1024 && fPurgeableImageCache != NULL) {
            return fPurgeableImageCache;
        }
        return &gLruImageCache;
    }
private:
    SkImageCache* fPurgeableImageCache;
};

static CacheSelector gCacheSelector;
static SkBitmapFactory gFactory(&SkImageDecoder::DecodeMemoryToTarget);

bool LazyDecodeBitmap(const void* buffer, size_t size, SkBitmap* bitmap) {
    void* copiedBuffer = sk_malloc_throw(size);
    memcpy(copiedBuffer, buffer, size);
    SkAutoDataUnref data(SkData::NewFromMalloc(copiedBuffer, size));

    static bool gOnce;
    if (!gOnce) {
        // Only use the cache selector if there is a purgeable image cache to use for large
        // images.
        if (FLAGS_useVolatileCache && SkAutoTUnref<SkImageCache>(
                SkPurgeableImageCache::Create()).get() != NULL) {
            gFactory.setCacheSelector(&gCacheSelector);
        } else {
            gFactory.setImageCache(&gLruImageCache);
        }
        gOnce = true;
    }
    return gFactory.installPixelRef(data, bitmap);
}

}  // namespace sk_tools
