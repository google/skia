/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkBitmapCache.h"
#include "SkImage.h"
#include "SkResourceCache.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkRect.h"

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

SkBitmapCacheDesc SkBitmapCacheDesc::Make(uint32_t imageID, SkColorType colorType,
                                          SkColorSpace* colorSpace, const SkIRect& subset) {
    SkASSERT(imageID);
    SkASSERT(subset.width() > 0 && subset.height() > 0);
    return { imageID,
             colorType,
             colorSpace ? colorSpace->toXYZD50Hash() : 0,
             colorSpace ? colorSpace->transferFnHash() : 0,
             subset };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkIPoint origin = bm.pixelRefOrigin();
    SkIRect bounds = SkIRect::MakeXYWH(origin.fX, origin.fY, bm.width(), bm.height());
    return Make(bm.getGenerationID(), bm.colorType(), bm.colorSpace(), bounds);
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image) {
    SkIRect bounds = SkIRect::MakeWH(image->width(), image->height());
    return Make(image->uniqueID(), image->colorType(), image->colorSpace(), bounds);
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
#include "SkDiscardableMemory.h"
#include "SkNextID.h"

void SkBitmapCache_setImmutableWithID(SkPixelRef* pr, uint32_t id) {
    pr->setImmutableWithID(id);
}

//#define REC_TRACE   SkDebugf
static void REC_TRACE(const char format[], ...) {}

// for diagnostics
static int32_t gRecCounter;

class SkBitmapCache::Rec : public SkResourceCache::Rec {
public:
    Rec(const SkBitmapCacheDesc& desc, const SkImageInfo& info, size_t rowBytes,
        std::unique_ptr<SkDiscardableMemory> dm, void* block)
        : fKey(desc)
        , fDM(std::move(dm))
        , fMalloc(block)
        , fInfo(info)
        , fRowBytes(rowBytes)
        , fExternalCounter(kBeforeFirstInstall_ExternalCounter)
    {
        SkASSERT(!(fDM && fMalloc));    // can't have both

        // We need an ID to return with the bitmap/pixelref. We can't necessarily use the key/desc
        // ID - lazy images cache the same ID with multiple keys (in different color types).
        fPrUniqueID = SkNextID::ImageID();
        REC_TRACE(" Rec(%d): [%d %d] %d\n",
                  sk_atomic_inc(&gRecCounter), fInfo.width(), fInfo.height(), fPrUniqueID);
    }

    ~Rec() override {
        SkASSERT(0 == fExternalCounter || kBeforeFirstInstall_ExternalCounter == fExternalCounter);
        if (fDM && kBeforeFirstInstall_ExternalCounter == fExternalCounter) {
            // we never installed, so we need to unlock before we destroy the DM
            SkASSERT(fDM->data());
            fDM->unlock();
        }
        REC_TRACE("~Rec(%d): [%d %d] %d\n",
                  sk_atomic_dec(&gRecCounter) - 1, fInfo.width(), fInfo.height(), fPrUniqueID);
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

        REC_TRACE(" Rec: [%d] releaseproc\n", rec->fPrUniqueID);

        SkASSERT(rec->fExternalCounter > 0);
        rec->fExternalCounter -= 1;
        if (rec->fDM) {
            SkASSERT(rec->fMalloc == nullptr);
            if (rec->fExternalCounter == 0) {
                REC_TRACE(" Rec [%d] unlock\n", rec->fPrUniqueID);
                rec->fDM->unlock();
            }
        } else {
            SkASSERT(rec->fMalloc != nullptr);
        }
    }

    bool install(SkBitmap* bitmap) {
        SkAutoMutexAcquire ama(fMutex);

        // are we still valid
        if (!fDM && !fMalloc) {
            REC_TRACE(" Rec: [%d] invalid\n", fPrUniqueID);
            return false;
        }

        /*
            constructor      fExternalCount < 0     fDM->data()
            after install    fExternalCount > 0     fDM->data()
            after Release    fExternalCount == 0    !fDM->data()
        */
        if (fDM) {
            if (kBeforeFirstInstall_ExternalCounter == fExternalCounter) {
                SkASSERT(fDM->data());
            } else if (fExternalCounter > 0) {
                SkASSERT(fDM->data());
            } else {
                SkASSERT(fExternalCounter == 0);
                if (!fDM->lock()) {
                    REC_TRACE(" Rec [%d] re-lock failed\n", fPrUniqueID);
                    fDM.reset(nullptr);
                    return false;
                }
                REC_TRACE(" Rec [%d] re-lock succeeded\n", fPrUniqueID);
            }
            SkASSERT(fDM->data());
        }

        bitmap->installPixels(fInfo, fDM ? fDM->data() : fMalloc, fRowBytes, ReleaseProc, this);
        SkBitmapCache_setImmutableWithID(bitmap->pixelRef(), fPrUniqueID);

        REC_TRACE(" Rec: [%d] install new pr\n", fPrUniqueID);

        if (kBeforeFirstInstall_ExternalCounter == fExternalCounter) {
            fExternalCounter = 1;
        } else {
            fExternalCounter += 1;
        }
        SkASSERT(fExternalCounter > 0);
        return true;
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextBitmap) {
        Rec* rec = (Rec*)&baseRec;
        SkBitmap* result = (SkBitmap*)contextBitmap;
        REC_TRACE(" Rec: [%d] found\n", rec->fPrUniqueID);
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

    // This field counts the number of external pixelrefs we have created. They notify us when
    // they are destroyed so we can decrement this.
    //
    //  > 0     we have outstanding pixelrefs
    // == 0     we have no outstanding pixelrefs, and can be safely purged
    //  < 0     we have been created, but not yet "installed" the first time.
    //
    int         fExternalCounter;

    enum {
        kBeforeFirstInstall_ExternalCounter = -1
    };
};

void SkBitmapCache::PrivateDeleteRec(Rec* rec) { delete rec; }

SkBitmapCache::RecPtr SkBitmapCache::Alloc(const SkBitmapCacheDesc& desc, const SkImageInfo& info,
                                           SkPixmap* pmap) {
    // Ensure that the info matches the subset (i.e. the subset is the entire image)
    SkASSERT(info.width() == desc.fSubset.width());
    SkASSERT(info.height() == desc.fSubset.height());
    SkASSERT(info.colorType() == desc.fColorType);
    SkASSERT((info.colorSpace() ? info.colorSpace()->toXYZD50Hash() : 0) == desc.fCSXYZHash);
    SkASSERT((info.colorSpace() ? info.colorSpace()->transferFnHash() : 0) ==
             desc.fCSTransferFnHash);

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

const SkMipMap* SkMipMapCache::AddAndRef(const SkBitmap& src, SkResourceCache* localCache) {
    SkMipMap* mipmap = SkMipMap::Build(src, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = new MipMapRec(SkBitmapCacheDesc::Make(src), mipmap);
        CHECK_LOCAL(localCache, add, Add, rec);
        src.pixelRef()->notifyAddedToCache();
    }
    return mipmap;
}
