/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkDiscardableMemory.h"
#include "SkMutex.h"
#include "SkPixelRef.h"
#include "SkPixelResourceRec.h"
#include "SkResourceCache.h"

static void cacheablepixeldata_pixelref_release_proc(void* pixels, void* ctx) {
    static_cast<SkPixelResourceRec*>(ctx)->deinstall_from_pixelref();
}

bool SkPixelResourceRec::install(SkBitmap* bitmap) {
    SkASSERT(fExternalOwnerCount >= 0);

    if (0 == fExternalOwnerCount) {
        if (!this->onFirstReinstall()) {
            // we're a dead-object if this ever fails
            return false;
        }
    }

    if (!bitmap->installPixels(this->info(), this->addr(), this->rowBytes(), nullptr,
                               cacheablepixeldata_pixelref_release_proc, this)) {
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(fUniqueID);
    fExternalOwnerCount += 1;
    return true;
}

void SkPixelResourceRec::deinstall_from_pixelref() {
    SkAutoMutexAcquire ama(fMutex);
    SkASSERT(fExternalOwnerCount > 0);

    if (0 == --fExternalOwnerCount) {
        this->onNotifyNoExternalOwners();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkMalloc_CacheablePixelData : public SkPixelResourceRec {
public:
    SkMalloc_CacheablePixelData(SkMutex* mutex, const SkImageInfo& info, size_t rowBytes,
                                uint32_t uniqueID, void* mallocBlock)
        : INHERITED(mutex, info, rowBytes, uniqueID)
        , fMalloc(mallocBlock)
    {}

    ~SkMalloc_CacheablePixelData() override {
        sk_free(fMalloc);
    }

    SkDiscardableMemory* diagnostic_only_getDiscardable() override { return nullptr; }

protected:
    void* onGetAddr() override { return fMalloc; }
    bool onFirstReinstall() override { return true; }
    void onNotifyNoExternalOwners() override {}

private:
    void*   fMalloc;

    typedef SkPixelResourceRec INHERITED;
};

class SkDiscardable_CacheablePixelData : public SkPixelResourceRec {
public:
    SkDiscardable_CacheablePixelData(SkMutex* mutex, const SkImageInfo& info, size_t rowBytes,
                                     uint32_t uniqueID, std::unique_ptr<SkDiscardableMemory> dm)
        : INHERITED(mutex, info, rowBytes, uniqueID)
        , fDM(std::move(dm))
    {
        SkASSERT(fDM->data());  // we're already locked
    }

    SkDiscardableMemory* diagnostic_only_getDiscardable() override { return fDM.get(); }

protected:
    void* onGetAddr() override { return fDM ? fDM->data() : nullptr; }

    bool onFirstReinstall() override {
        // A previous call to onUnlock may have deleted our DM, so check for that
        if (nullptr == fDM) {
            return false;
        }

        if (!fDM->lock()) {
            // since it failed, we delete it now, to free-up the resource
            fDM = nullptr;
            return false;
        }
        return true;
    }

    void onNotifyNoExternalOwners() override { fDM->unlock(); }

private:
    std::unique_ptr<SkDiscardableMemory> fDM;

    typedef SkPixelResourceRec INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkPixelResourceRec_Allocator : public SkBitmap::Allocator {
public:
    SkPixelResourceRec_Allocator(SkMutex* mutex, SkResourceCache::DiscardableFactory factory) {
        fFactory = factory;
    }

    bool allocPixelRef(SkBitmap* bitmap, SkColorTable* ctable) override {
        size_t size = bitmap->getSize();
        uint64_t size64 = bitmap->computeSize64();
        if (0 == size || size64 > (uint64_t)size) {
            return false;
        }

        if (kIndex_8_SkColorType == bitmap->colorType()) {
            if (!ctable) {
                return false;
            }
        } else {
            ctable = nullptr;
        }

        const SkImageInfo& info = bitmap->info();
        const size_t rb = bitmap->rowBytes();
        const uint32_t uniqueID = bitmap->getGenerationID();
        SkPixelResourceRec* pd = nullptr;

        if (fFactory) {
            std::unique_ptr<SkDiscardableMemory> dm(fFactory(size));
            if (!dm) {
                return false;
            }
            pd = new SkDiscardable_CacheablePixelData(mutex, info, rb, uniqueID, std::move(dm));
        } else {
            void* block = sk_malloc_flags(size, 0);
            if (!block) {
                return false;
            }
            pd = new SkMalloc_CacheablePixelData(mutex, info, rb, uniqueID, block)
        }

        if (!bitmap->installPixels(pd->info(), pd->addr(), pd->rowBytes(), ctable,
                                   cacheablepixeldata_pixelref_release_proc, pd)) {
            delete pd;
            return false;
        }

        SkResourceCache::Add(pd);
        return true;
    }

private:
    SkResourceCache::DiscardableFactory fFactory;
};

SkResourceCache::SkResourceCache(DiscardableFactory factory) {
    this->init();
    fDiscardableFactory = factory;

    fAllocator = new SkPixelResourceRec_Allocator(factory);
}
