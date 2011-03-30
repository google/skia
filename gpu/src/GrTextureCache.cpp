/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrTextureCache.h"
#include "GrTexture.h"

GrTextureEntry::GrTextureEntry(const GrTextureKey& key, GrTexture* texture)
        : fKey(key), fTexture(texture) {
    fLockCount = 0;
    fPrev = fNext = NULL;

    // we assume ownership of the texture, and will unref it when we die
    GrAssert(texture);
}

GrTextureEntry::~GrTextureEntry() {
    fTexture->unref();
}

#if GR_DEBUG
void GrTextureEntry::validate() const {
    GrAssert(fLockCount >= 0);
    GrAssert(fTexture);
    fTexture->validate();
}
#endif

///////////////////////////////////////////////////////////////////////////////

GrTextureCache::GrTextureCache(int maxCount, size_t maxBytes) :
        fMaxCount(maxCount),
        fMaxBytes(maxBytes) {
    fEntryCount          = 0;
    fEntryBytes          = 0;
    fClientDetachedCount = 0;
    fClientDetachedBytes = 0;

    fHead = fTail = NULL;
}

GrTextureCache::~GrTextureCache() {
    GrAutoTextureCacheValidate atcv(this);

    this->removeAll();
}

void GrTextureCache::getLimits(int* maxTextures, size_t* maxTextureBytes) const{
    if (maxTextures) {
        *maxTextures = fMaxCount;
    }
    if (maxTextureBytes) {
        *maxTextureBytes = fMaxBytes;
    }
}

void GrTextureCache::setLimits(int maxTextures, size_t maxTextureBytes) {
    bool smaller = (maxTextures < fMaxCount) || (maxTextureBytes < fMaxBytes);

    fMaxCount = maxTextures;
    fMaxBytes = maxTextureBytes;

    if (smaller) {
        this->purgeAsNeeded();
    }
}

void GrTextureCache::internalDetach(GrTextureEntry* entry,
                                    bool clientDetach) {
    GrTextureEntry* prev = entry->fPrev;
    GrTextureEntry* next = entry->fNext;

    if (prev) {
        prev->fNext = next;
    } else {
        fHead = next;
    }
    if (next) {
        next->fPrev = prev;
    } else {
        fTail = prev;
    }

    // update our stats
    if (clientDetach) {
        fClientDetachedCount += 1;
        fClientDetachedBytes += entry->texture()->sizeInBytes();
    } else {
        fEntryCount -= 1;
        fEntryBytes -= entry->texture()->sizeInBytes();
    }
}

void GrTextureCache::attachToHead(GrTextureEntry* entry,
                                  bool clientReattach) {
    entry->fPrev = NULL;
    entry->fNext = fHead;
    if (fHead) {
        fHead->fPrev = entry;
    }
    fHead = entry;
    if (NULL == fTail) {
        fTail = entry;
    }

    // update our stats
    if (clientReattach) {
        fClientDetachedCount -= 1;
        fClientDetachedBytes -= entry->texture()->sizeInBytes();
    } else {
        fEntryCount += 1;
        fEntryBytes += entry->texture()->sizeInBytes();
    }
}

class GrTextureCache::Key {
    typedef GrTextureEntry T;

    const GrTextureKey& fKey;
public:
    Key(const GrTextureKey& key) : fKey(key) {}

    uint32_t getHash() const { return fKey.hashIndex(); }

    static bool LT(const T& entry, const Key& key) {
        return entry.key() < key.fKey;
    }
    static bool EQ(const T& entry, const Key& key) {
        return entry.key() == key.fKey;
    }
#if GR_DEBUG
    static uint32_t GetHash(const T& entry) {
        return entry.key().hashIndex();
    }
    static bool LT(const T& a, const T& b) {
        return a.key() < b.key();
    }
    static bool EQ(const T& a, const T& b) {
        return a.key() == b.key();
    }
#endif
};

GrTextureEntry* GrTextureCache::findAndLock(const GrTextureKey& key) {
    GrAutoTextureCacheValidate atcv(this);

    GrTextureEntry* entry = fCache.find(key);
    if (entry) {
        this->internalDetach(entry, false);
        this->attachToHead(entry, false);
        // mark the entry as "busy" so it doesn't get purged
        entry->lock();
    }
    return entry;
}

GrTextureEntry* GrTextureCache::createAndLock(const GrTextureKey& key,
                                              GrTexture* texture) {
    GrAutoTextureCacheValidate atcv(this);

    GrTextureEntry* entry = new GrTextureEntry(key, texture);

    this->attachToHead(entry, false);
    fCache.insert(key, entry);

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("--- add texture to cache %p, count=%d bytes= %d %d\n",
             entry, fEntryCount, texture->sizeInBytes(), fEntryBytes);
#endif

    // mark the entry as "busy" so it doesn't get purged
    entry->lock();
    this->purgeAsNeeded();
    return entry;
}

void GrTextureCache::detach(GrTextureEntry* entry) {
    internalDetach(entry, true);
    fCache.remove(entry->fKey, entry);
}

void GrTextureCache::reattachAndUnlock(GrTextureEntry* entry) {
    attachToHead(entry, true);
    fCache.insert(entry->key(), entry);
    unlock(entry);
}

void GrTextureCache::unlock(GrTextureEntry* entry) {
    GrAutoTextureCacheValidate atcv(this);

    GrAssert(entry);
    GrAssert(entry->isLocked());
    GrAssert(fCache.find(entry->key()));

    entry->unlock();
    this->purgeAsNeeded();
}

void GrTextureCache::purgeAsNeeded() {
    GrAutoTextureCacheValidate atcv(this);

    GrTextureEntry* entry = fTail;
    while (entry) {
        if (fEntryCount <= fMaxCount && fEntryBytes <= fMaxBytes) {
            break;
        }

        GrTextureEntry* prev = entry->fPrev;
        if (!entry->isLocked()) {
            // remove from our cache
            fCache.remove(entry->fKey, entry);

            // remove from our llist
            this->internalDetach(entry, false);

#if GR_DUMP_TEXTURE_UPLOAD
            GrPrintf("--- ~texture from cache %p [%d %d]\n", entry->texture(),
                     entry->texture()->width(),
                     entry->texture()->height());
#endif
            delete entry;
        }
        entry = prev;
    }
}

void GrTextureCache::removeAll() {
    GrAssert(!fClientDetachedCount);
    GrAssert(!fClientDetachedBytes);

    GrTextureEntry* entry = fHead;
    while (entry) {
        GrAssert(!entry->isLocked());

        GrTextureEntry* next = entry->fNext;
        delete entry;
        entry = next;
    }

    fCache.removeAll();
    fHead = fTail = NULL;
    fEntryCount = 0;
    fEntryBytes = 0;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG
static int countMatches(const GrTextureEntry* head, const GrTextureEntry* target) {
    const GrTextureEntry* entry = head;
    int count = 0;
    while (entry) {
        if (target == entry) {
            count += 1;
        }
        entry = entry->next();
    }
    return count;
}

#if GR_DEBUG
static bool both_zero_or_nonzero(int count, size_t bytes) {
    return (count == 0 && bytes == 0) || (count > 0 && bytes > 0);
}
#endif

void GrTextureCache::validate() const {
    GrAssert(!fHead == !fTail);
    GrAssert(both_zero_or_nonzero(fEntryCount, fEntryBytes));
    GrAssert(both_zero_or_nonzero(fClientDetachedCount, fClientDetachedBytes));
    GrAssert(fClientDetachedBytes <= fEntryBytes);
    GrAssert(fClientDetachedCount <= fEntryCount);
    GrAssert((fEntryCount - fClientDetachedCount) == fCache.count());

    fCache.validate();

    GrTextureEntry* entry = fHead;
    int count = 0;
    size_t bytes = 0;
    while (entry) {
        entry->validate();
        GrAssert(fCache.find(entry->key()));
        count += 1;
        bytes += entry->texture()->sizeInBytes();
        entry = entry->fNext;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);
    GrAssert(bytes == fEntryBytes  - fClientDetachedBytes);

    count = 0;
    for (entry = fTail; entry; entry = entry->fPrev) {
        count += 1;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);

    for (int i = 0; i < count; i++) {
        int matches = countMatches(fHead, fCache.getArray()[i]);
        GrAssert(1 == matches);
    }
}
#endif


