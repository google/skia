/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelResourceRec_DEFINED
#define SkPixelResourceRec_DEFINED

#include "SkResourceCache.h"

/**
 *  Pixel memory block, managed by the resource cache. Keeps track of how many external
 *  references (bitmaps) there are.
 *
 *  resourcecache_allocator::allocPixelRef(...)
 *      based on discardable_factory, create either
 *          discardable_pixeldata_subclass
 *      else
 *          malloc_pixeldata_subclass
 *      install pixelref with release_proc to deinstall() pixeldata
 *      add pixeldata to the cache
 *
 *  SkBitmap <-- resourcecache::find(...)
 *      resourcecache_mutex_acquire
 *      find entry
 *      increment fInstallCount
 *      if fInstallCount == 1 && fNeedsReinstall
 *          notify subclass it must re-install (useful for discardable memory)
 *          if reinstall fails
 *              delete entry (like a purge)
 *              return failure
 *      install the pixel_data into the bitmap, with a custom callback when destroyed
 *      resourcecache_mutex_release
 *      return bitmap
 *
 *  resourcecache::purge(...)
 *      resourcecache_mutex_acquire
 *      find entry
 *      if fInstallCount == 0
 *          purge entry
 *      resourcecache_mutex_release
 *
 *  ~SkBitmap()
 *      resourcecache_mutex_acquire
 *      decrement fInstallCount // it will have been > 0 before this
 *      if fInstallCount == 0
 *          notify subclass that it is now purgable (useful for discardable memory
 *          fNeedsReinstall = true
 *      resourcecache_mutex_release
 *
 */
class SkPixelResourceRec : public SkResourceCache::Rec {
public:
    SkPixelResourceRec(SkMutex* mutex, const SkImageInfo& info, size_t rowBytes, uint32_t uniqueID)
        : fMutex(mutex)
        , fInfo(info)
        , fRowBytes(rowBytes)
        , fUniqueID(uniqueID)
    {}

    ~SkPixelResourceRec() override {
        SkASSERT(0 == fExternalOwnerCount);
    }

    const SkImageInfo& info() const { return fInfo; }
    void*  addr() { return this->onGetAddr(); }
    size_t rowBytes() const { return fRowBytes; }

    /**
     *  If returns true, this installs the pixels into the bitmap. The bitmap's (pixelref's)
     *  destructor will manage "unrefing/unlocking" the shared pixel memory in this object.
     *
     *  If this returns false, the entry holding this data should be purged from the cache.
     */
    bool install(SkBitmap* bitmap);

    void deinstall_from_pixelref();

    virtual SkDiscardableMemory* diagnostic_only_getDiscardable() = 0;

protected:
    virtual void* onGetAddr() = 0;
    virtual bool onFirstReinstall() = 0;
    virtual void onNotifyNoExternalOwners() = 0;

private:
    SkMutex*    fMutex;
    SkImageInfo fInfo;
    size_t      fRowBytes;
    uint32_t    fUniqueID;
    int32_t     fExternalOwnerCount = 0;
};

#endif
