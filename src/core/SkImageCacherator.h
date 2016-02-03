/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageCacherator_DEFINED
#define SkImageCacherator_DEFINED

#include "SkImageGenerator.h"
#include "SkMutex.h"
#include "SkTemplates.h"

class GrContext;
class GrTextureParams;
class GrUniqueKey;
class SkBitmap;
class SkImage;

/*
 *  Internal class to manage caching the output of an ImageGenerator.
 */
class SkImageCacherator {
public:
    // Takes ownership of the generator
    static SkImageCacherator* NewFromGenerator(SkImageGenerator*, const SkIRect* subset = nullptr);

    const SkImageInfo& info() const { return fInfo; }
    uint32_t uniqueID() const { return fUniqueID; }

    /**
     *  On success (true), bitmap will point to the pixels for this generator. If this returns
     *  false, the bitmap will be reset to empty.
     *
     *  If not NULL, the client will be notified (->notifyAddedToCache()) when resources are
     *  added to the cache on its behalf.
     */
    bool lockAsBitmap(SkBitmap*, const SkImage* client,
                      SkImage::CachingHint = SkImage::kAllow_CachingHint);

    /**
     *  Returns a ref() on the texture produced by this generator. The caller must call unref()
     *  when it is done. Will return nullptr on failure.
     *
     *  If not NULL, the client will be notified (->notifyAddedToCache()) when resources are
     *  added to the cache on its behalf.
     *
     *  The caller is responsible for calling texture->unref() when they are done.
     */
    GrTexture* lockAsTexture(GrContext*, const GrTextureParams&, const SkImage* client,
                             SkImage::CachingHint = SkImage::kAllow_CachingHint);

    /**
     *  If the underlying src naturally is represented by an encoded blob (in SkData), this returns
     *  a ref to that data. If not, it returns null.
     *
     *  If a GrContext is specified, then the caller is only interested in gpu-specific encoded
     *  formats, so others (e.g. PNG) can just return nullptr.
     */
    SkData* refEncoded(GrContext*);

    // Only return true if the generate has already been cached.
    bool lockAsBitmapOnlyIfAlreadyCached(SkBitmap*);
    // Call the underlying generator directly
    bool directGeneratePixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                              int srcX, int srcY);

private:
    SkImageCacherator(SkImageGenerator*, const SkImageInfo&, const SkIPoint&, uint32_t uniqueID);

    bool generateBitmap(SkBitmap*);
    bool tryLockAsBitmap(SkBitmap*, const SkImage*, SkImage::CachingHint);
#if SK_SUPPORT_GPU
    // Returns the texture. If the cacherator is generating the texture and wants to cache it,
    // it should use the passed in key (if the key is valid).
    GrTexture* lockTexture(GrContext*, const GrUniqueKey& key, const SkImage* client,
                           SkImage::CachingHint);
#endif

    class ScopedGenerator {
        SkImageCacherator* fCacher;
    public:
        ScopedGenerator(SkImageCacherator* cacher) : fCacher(cacher) {
            fCacher->fMutexForGenerator.acquire();
        }
        ~ScopedGenerator() {
            fCacher->fMutexForGenerator.release();
        }
        SkImageGenerator* operator->() const { return fCacher->fNotThreadSafeGenerator; }
        operator SkImageGenerator*() const { return fCacher->fNotThreadSafeGenerator; }
    };

    SkMutex                         fMutexForGenerator;
    SkAutoTDelete<SkImageGenerator> fNotThreadSafeGenerator;

    const SkImageInfo   fInfo;
    const SkIPoint      fOrigin;
    const uint32_t      fUniqueID;

    friend class GrImageTextureMaker;
};

#endif
