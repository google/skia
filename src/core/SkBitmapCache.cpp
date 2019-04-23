/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkBitmapProvider.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkResourceCache.h"

/**
 *  Use this for bitmapcache and mipmapcache entries.
 */
uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID) {
    uint64_t sharedID = SkSetFourByteTag('b', 'm', 'a', 'p');
    return (sharedID << 32) | bitmapGenID;
}

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID) {
    SkResourceCache::PostPurgeSharedID(SkMakeResourceCacheSharedIDForBitmap(bitmapGenID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmapCacheDesc SkBitmapCacheDesc::Make(uint32_t imageID, const SkIRect& subset) {
    SkASSERT(imageID);
    SkASSERT(subset.width() > 0 && subset.height() > 0);
    return { imageID, subset };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image) {
    SkIRect bounds = SkIRect::MakeWH(image->width(), image->height());
    return Make(image->uniqueID(), bounds);
}

namespace {
static unsigned gBitmapKeyNamespaceLabel;

struct BitmapKey : public SkResourceCache::Key {
public:
    BitmapKey(const SkBitmapCacheDesc& desc) : fDesc(desc) {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fDesc.fImageID),
                   sizeof(fDesc));
    }

    const SkBitmapCacheDesc fDesc;
};
}

//////////////////////
#include "src/core/SkDiscardableMemory.h"
#include "src/core/SkNextID.h"

void SkBitmapCache_setImmutableWithID(SkPixelRef* pr, uint32_t id) {
    pr->setImmutableWithID(id);
}

class SkBitmapCache::Rec : public SkResourceCache::Rec {
public:
    Rec(const SkBitmapCacheDesc& desc, const SkImageInfo& info, size_t rowBytes,
        std::unique_ptr<SkDiscardableMemory> dm, void* block)
        : fKey(desc)
        , fDM(std::move(dm))
        , fMalloc(block)
        , fInfo(info)
        , fRowBytes(rowBytes)
    {
        SkASSERT(!(fDM && fMalloc));    // can't have both

        // We need an ID to return with the bitmap/pixelref. We can't necessarily use the key/desc
        // ID - lazy images cache the same ID with multiple keys (in different color types).
        fPrUniqueID = SkNextID::ImageID();
    }

    ~Rec() override {
        SkASSERT(0 == fExternalCounter);
        if (fDM && fDiscardableIsLocked) {
            SkASSERT(fDM->data());
            fDM->unlock();
        }
        sk_free(fMalloc);   // may be null
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        return sizeof(fKey) + fInfo.computeByteSize(fRowBytes);
    }
    bool canBePurged() override {
        SkAutoMutexAcquire ama(fMutex);
        return fExternalCounter == 0;
    }
    void postAddInstall(void* payload) override {
        SkAssertResult(this->install(static_cast<SkBitmap*>(payload)));
    }

    const char* getCategory() const override { return "bitmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fDM.get();
    }

    static void ReleaseProc(void* addr, void* ctx) {
        Rec* rec = static_cast<Rec*>(ctx);
        SkAutoMutexAcquire ama(rec->fMutex);

        SkASSERT(rec->fExternalCounter > 0);
        rec->fExternalCounter -= 1;
        if (rec->fDM) {
            SkASSERT(rec->fMalloc == nullptr);
            if (rec->fExternalCounter == 0) {
                rec->fDM->unlock();
                rec->fDiscardableIsLocked = false;
            }
        } else {
            SkASSERT(rec->fMalloc != nullptr);
        }
    }

    bool install(SkBitmap* bitmap) {
        SkAutoMutexAcquire ama(fMutex);

        if (!fDM && !fMalloc) {
            return false;
        }

        if (fDM) {
            if (!fDiscardableIsLocked) {
                SkASSERT(fExternalCounter == 0);
                if (!fDM->lock()) {
                    fDM.reset(nullptr);
                    return false;
                }
                fDiscardableIsLocked = true;
            }
            SkASSERT(fDM->data());
        }

        bitmap->installPixels(fInfo, fDM ? fDM->data() : fMalloc, fRowBytes, ReleaseProc, this);
        SkBitmapCache_setImmutableWithID(bitmap->pixelRef(), fPrUniqueID);
        fExternalCounter++;

        return true;
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextBitmap) {
        Rec* rec = (Rec*)&baseRec;
        SkBitmap* result = (SkBitmap*)contextBitmap;
        return rec->install(result);
    }

private:
    BitmapKey   fKey;

    SkMutex     fMutex;

    // either fDM or fMalloc can be non-null, but not both
    std::unique_ptr<SkDiscardableMemory> fDM;
    void*       fMalloc;

    SkImageInfo fInfo;
    size_t      fRowBytes;
    uint32_t    fPrUniqueID;

    // This field counts the number of external pixelrefs we have created.
    // They notify us when they are destroyed so we can decrement this.
    int  fExternalCounter     = 0;
    bool fDiscardableIsLocked = true;
};

void SkBitmapCache::PrivateDeleteRec(Rec* rec) { delete rec; }

SkBitmapCache::RecPtr SkBitmapCache::Alloc(const SkBitmapCacheDesc& desc, const SkImageInfo& info,
                                           SkPixmap* pmap) {
    // Ensure that the info matches the subset (i.e. the subset is the entire image)
    SkASSERT(info.width() == desc.fSubset.width());
    SkASSERT(info.height() == desc.fSubset.height());

    const size_t rb = info.minRowBytes();
    size_t size = info.computeByteSize(rb);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return nullptr;
    }

    std::unique_ptr<SkDiscardableMemory> dm;
    void* block = nullptr;

    auto factory = SkResourceCache::GetDiscardableFactory();
    if (factory) {
        dm.reset(factory(size));
    } else {
        block = sk_malloc_canfail(size);
    }
    if (!dm && !block) {
        return nullptr;
    }
    *pmap = SkPixmap(info, dm ? dm->data() : block, rb);
    return RecPtr(new Rec(desc, info, rb, std::move(dm), block));
}

void SkBitmapCache::Add(RecPtr rec, SkBitmap* bitmap) {
    SkResourceCache::Add(rec.release(), bitmap);
}

bool SkBitmapCache::Find(const SkBitmapCacheDesc& desc, SkBitmap* result) {
    desc.validate();
    return SkResourceCache::Find(BitmapKey(desc), SkBitmapCache::Rec::Finder, result);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

namespace {
static unsigned gMipMapKeyNamespaceLabel;

struct MipMapKey : public SkResourceCache::Key {
public:
    MipMapKey(const SkBitmapCacheDesc& desc) : fDesc(desc) {
        this->init(&gMipMapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fDesc.fImageID),
                   sizeof(fDesc));
    }

    const SkBitmapCacheDesc fDesc;
};

struct MipMapRec : public SkResourceCache::Rec {
    MipMapRec(const SkBitmapCacheDesc& desc, const SkMipMap* result)
        : fKey(desc)
        , fMipMap(result)
    {
        fMipMap->attachToCacheAndRef();
    }

    ~MipMapRec() override {
        fMipMap->detachFromCacheAndUnref();
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fMipMap->size(); }
    const char* getCategory() const override { return "mipmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fMipMap->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextMip) {
        const MipMapRec& rec = static_cast<const MipMapRec&>(baseRec);
        const SkMipMap* mm = SkRef(rec.fMipMap);
        // the call to ref() above triggers a "lock" in the case of discardable memory,
        // which means we can now check for null (in case the lock failed).
        if (nullptr == mm->data()) {
            mm->unref();    // balance our call to ref()
            return false;
        }
        // the call must call unref() when they are done.
        *(const SkMipMap**)contextMip = mm;
        return true;
    }

private:
    MipMapKey       fKey;
    const SkMipMap* fMipMap;
};
}

const SkMipMap* SkMipMapCache::FindAndRef(const SkBitmapCacheDesc& desc,
                                          SkResourceCache* localCache) {
    MipMapKey key(desc);
    const SkMipMap* result;

    if (!CHECK_LOCAL(localCache, find, Find, key, MipMapRec::Finder, &result)) {
        result = nullptr;
    }
    return result;
}

static SkResourceCache::DiscardableFactory get_fact(SkResourceCache* localCache) {
    return localCache ? localCache->GetDiscardableFactory()
                      : SkResourceCache::GetDiscardableFactory();
}

const SkMipMap* SkMipMapCache::AddAndRef(const SkBitmapProvider& provider,
                                         SkResourceCache* localCache) {
    SkBitmap src;
    if (!provider.asBitmap(&src)) {
        return nullptr;
    }

    SkMipMap* mipmap = SkMipMap::Build(src, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = new MipMapRec(provider.makeCacheDesc(), mipmap);
        CHECK_LOCAL(localCache, add, Add, rec);
        provider.notifyAddedToCache();
    }
    return mipmap;
}
