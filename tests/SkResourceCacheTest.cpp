/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkBitmapProvider.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkResourceCache.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "tests/Test.h"

////////////////////////////////////////////////////////////////////////////////////////

enum LockedState {
    kNotLocked,
    kLocked,
};

enum CachedState {
    kNotInCache,
    kInCache,
};

static void check_data(skiatest::Reporter* reporter, const SkCachedData* data,
                       int refcnt, CachedState cacheState, LockedState lockedState) {
    REPORTER_ASSERT(reporter, data->testing_only_getRefCnt() == refcnt);
    REPORTER_ASSERT(reporter, data->testing_only_isInCache() == (kInCache == cacheState));
    bool isLocked = (data->data() != nullptr);
    REPORTER_ASSERT(reporter, isLocked == (lockedState == kLocked));
}

static void test_mipmapcache(skiatest::Reporter* reporter, SkResourceCache* cache) {
    cache->purgeAll();

    SkBitmap src;
    src.allocN32Pixels(5, 5);
    src.setImmutable();
    sk_sp<SkImage> img = SkImage::MakeFromBitmap(src);
    SkBitmapProvider provider(img.get());
    const auto desc = provider.makeCacheDesc();

    const SkMipMap* mipmap = SkMipMapCache::FindAndRef(desc, cache);
    REPORTER_ASSERT(reporter, nullptr == mipmap);

    mipmap = SkMipMapCache::AddAndRef(provider, cache);
    REPORTER_ASSERT(reporter, mipmap);

    {
        const SkMipMap* mm = SkMipMapCache::FindAndRef(desc, cache);
        REPORTER_ASSERT(reporter, mm);
        REPORTER_ASSERT(reporter, mm == mipmap);
        mm->unref();
    }

    check_data(reporter, mipmap, 2, kInCache, kLocked);

    mipmap->unref();
    // tricky, since technically after this I'm no longer an owner, but since the cache is
    // local, I know it won't get purged behind my back
    check_data(reporter, mipmap, 1, kInCache, kNotLocked);

    // find us again
    mipmap = SkMipMapCache::FindAndRef(desc, cache);
    check_data(reporter, mipmap, 2, kInCache, kLocked);

    cache->purgeAll();
    check_data(reporter, mipmap, 1, kNotInCache, kLocked);

    mipmap->unref();
}

static void test_mipmap_notify(skiatest::Reporter* reporter, SkResourceCache* cache) {
    const int N = 3;

    SkBitmap src[N];
    sk_sp<SkImage> img[N];
    SkBitmapCacheDesc desc[N];
    for (int i = 0; i < N; ++i) {
        src[i].allocN32Pixels(5, 5);
        src[i].setImmutable();
        img[i] = SkImage::MakeFromBitmap(src[i]);
        SkBitmapProvider provider(img[i].get());
        SkMipMapCache::AddAndRef(provider, cache)->unref();
        desc[i] = provider.makeCacheDesc();
    }

    for (int i = 0; i < N; ++i) {
        const SkMipMap* mipmap = SkMipMapCache::FindAndRef(desc[i], cache);
        // We're always using a local cache, so we know we won't be purged by other threads
        REPORTER_ASSERT(reporter, mipmap);
        SkSafeUnref(mipmap);

        img[i].reset(); // delete the image, which *should not* remove us from the cache
        mipmap = SkMipMapCache::FindAndRef(desc[i], cache);
        REPORTER_ASSERT(reporter, mipmap);
        SkSafeUnref(mipmap);

        src[i].reset(); // delete the underlying pixelref, which *should* remove us from the cache
        mipmap = SkMipMapCache::FindAndRef(desc[i], cache);
        REPORTER_ASSERT(reporter, !mipmap);
    }
}

#include "src/lazy/SkDiscardableMemoryPool.h"

static SkDiscardableMemoryPool* gPool = nullptr;
static SkDiscardableMemory* pool_factory(size_t bytes) {
    SkASSERT(gPool);
    return gPool->create(bytes);
}

static void testBitmapCache_discarded_bitmap(skiatest::Reporter* reporter, SkResourceCache* cache,
                                             SkResourceCache::DiscardableFactory factory) {
    test_mipmapcache(reporter, cache);
    test_mipmap_notify(reporter, cache);
}

DEF_TEST(BitmapCache_discarded_bitmap, reporter) {
    const size_t byteLimit = 100 * 1024;
    {
        SkResourceCache cache(byteLimit);
        testBitmapCache_discarded_bitmap(reporter, &cache, nullptr);
    }
    {
        sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Make(byteLimit));
        gPool = pool.get();
        SkResourceCache::DiscardableFactory factory = pool_factory;
        SkResourceCache cache(factory);
        testBitmapCache_discarded_bitmap(reporter, &cache, factory);
    }
}

static void test_discarded_image(skiatest::Reporter* reporter, const SkMatrix& transform,
                                 sk_sp<SkImage> (*buildImage)()) {
    auto surface(SkSurface::MakeRasterN32Premul(10, 10));
    SkCanvas* canvas = surface->getCanvas();

    // SkBitmapCache is global, so other threads could be evicting our bitmaps.  Loop a few times
    // to mitigate this risk.
    const unsigned kRepeatCount = 42;
    for (unsigned i = 0; i < kRepeatCount; ++i) {
        SkAutoCanvasRestore acr(canvas, true);

        sk_sp<SkImage> image(buildImage());

        // always use high quality to ensure caching when scaled
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);

        // draw the image (with a transform, to tickle different code paths) to ensure
        // any associated resources get cached
        canvas->concat(transform);
        canvas->drawImage(image, 0, 0, &paint);

        const auto desc = SkBitmapCacheDesc::Make(image.get());

        // delete the image
        image.reset(nullptr);

        // all resources should have been purged
        SkBitmap result;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc, &result));
    }
}


// Verify that associated bitmap cache entries are purged on SkImage destruction.
DEF_TEST(BitmapCache_discarded_image, reporter) {
    // Cache entries associated with SkImages fall into two categories:
    //
    // 1) generated image bitmaps (managed by the image cacherator)
    // 2) scaled/resampled bitmaps (cached when HQ filters are used)
    //
    // To exercise the first cache type, we use generated/picture-backed SkImages.
    // To exercise the latter, we draw scaled bitmap images using HQ filters.

    const SkMatrix xforms[] = {
        SkMatrix::MakeScale(1, 1),
        SkMatrix::MakeScale(1.7f, 0.5f),
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(xforms); ++i) {
        test_discarded_image(reporter, xforms[i], []() {
            auto surface(SkSurface::MakeRasterN32Premul(10, 10));
            surface->getCanvas()->clear(SK_ColorCYAN);
            return surface->makeImageSnapshot();
        });

        test_discarded_image(reporter, xforms[i], []() {
            SkPictureRecorder recorder;
            SkCanvas* canvas = recorder.beginRecording(10, 10);
            canvas->clear(SK_ColorCYAN);
            return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                            SkISize::Make(10, 10), nullptr, nullptr,
                                            SkImage::BitDepth::kU8,
                                            SkColorSpace::MakeSRGB());
        });
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void* gTestNamespace;

struct TestKey : SkResourceCache::Key {
    int32_t fData;

    TestKey(int sharedID, int32_t data) : fData(data) {
        this->init(&gTestNamespace, sharedID, sizeof(fData));
    }
};

struct TestRec : SkResourceCache::Rec {
    enum {
        kDidInstall = 1 << 0,
    };

    TestKey fKey;
    int*    fFlags;
    bool    fCanBePurged;

    TestRec(int sharedID, int32_t data, int* flagPtr) : fKey(sharedID, data), fFlags(flagPtr) {
        fCanBePurged = false;
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return 1024; /* just need a value */ }
    bool canBePurged() override { return fCanBePurged; }
    void postAddInstall(void*) override {
        *fFlags |= kDidInstall;
    }
    const char* getCategory() const override { return "test-category"; }
};

static void test_duplicate_add(SkResourceCache* cache, skiatest::Reporter* reporter,
                               bool purgable) {
    int sharedID = 1;
    int data = 0;

    int flags0 = 0, flags1 = 0;

    auto rec0 = skstd::make_unique<TestRec>(sharedID, data, &flags0);
    auto rec1 = skstd::make_unique<TestRec>(sharedID, data, &flags1);
    SkASSERT(rec0->getKey() == rec1->getKey());

    TestRec* r0 = rec0.get();   // save the bare-pointer since we will release rec0
    r0->fCanBePurged = purgable;

    REPORTER_ASSERT(reporter, !(flags0 & TestRec::kDidInstall));
    REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));

    cache->add(rec0.release(), nullptr);
    REPORTER_ASSERT(reporter, flags0 & TestRec::kDidInstall);
    REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));
    flags0 = 0; // reset the flag

    cache->add(rec1.release(), nullptr);
    if (purgable) {
        // we purged rec0, and did install rec1
        REPORTER_ASSERT(reporter, !(flags0 & TestRec::kDidInstall));
        REPORTER_ASSERT(reporter, flags1 & TestRec::kDidInstall);
    } else {
        // we re-used rec0 and did not install rec1
        REPORTER_ASSERT(reporter, flags0 & TestRec::kDidInstall);
        REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));
        r0->fCanBePurged = true;  // so we can cleanup the cache
    }
}

/*
 *  Test behavior when the same key is added more than once.
 */
DEF_TEST(ResourceCache_purge, reporter) {
    for (bool purgable : { false, true }) {
        {
            SkResourceCache cache(1024 * 1024);
            test_duplicate_add(&cache, reporter, purgable);
        }
        {
            SkResourceCache cache(SkDiscardableMemory::Create);
            test_duplicate_add(&cache, reporter, purgable);
        }
    }
}
