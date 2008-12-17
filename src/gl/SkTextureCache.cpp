#include "SkTextureCache.h"

//#define TRACE_HASH_HITS
//#define TRACE_TEXTURE_CACHE_PURGE

SkTextureCache::Entry::Entry(const SkBitmap& bitmap)
        : fName(0), fKey(bitmap), fPrev(NULL), fNext(NULL) {

    fMemSize = SkGL::ComputeTextureMemorySize(bitmap);
    fLockCount = 0;
}

SkTextureCache::Entry::~Entry() {
    if (fName != 0) {
        glDeleteTextures(1, &fName);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkTextureCache::SkTextureCache(size_t countMax, size_t sizeMax)
        : fHead(NULL), fTail(NULL),
          fTexCountMax(countMax), fTexSizeMax(sizeMax),
          fTexCount(0), fTexSize(0) {

    bzero(fHash, sizeof(fHash));
    this->validate();
}

SkTextureCache::~SkTextureCache() {
#ifdef SK_DEBUG
    Entry* entry = fHead;
    while (entry) {
        SkASSERT(entry->lockCount() == 0);
        entry = entry->fNext;
    }
#endif
    this->validate();
}

void SkTextureCache::deleteAllCaches(bool texturesAreValid) {
    this->validate();
    
    Entry* entry = fHead;
    while (entry) {
        Entry* next = entry->fNext;
        if (!texturesAreValid) {
            entry->abandonTexture();
        }
        SkDELETE(entry);
        entry = next;
    }
    
    fSorted.reset();
    bzero(fHash, sizeof(fHash));
    
    fTexCount = 0;
    fTexSize = 0;
    
    fTail = fHead = NULL;
    
    this->validate();
}

///////////////////////////////////////////////////////////////////////////////

int SkTextureCache::findInSorted(const Key& key) const {
    int count = fSorted.count();
    if (count == 0) {
        return ~0;
    }

    Entry** sorted = fSorted.begin();
    int lo = 0;
    int hi = count - 1;
    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        if (sorted[mid]->getKey() < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    
    // hi is now our best guess
    const Entry* entry = sorted[hi];
    if (entry->getKey() == key) {
        return hi;
    }
    
    // return where to insert it
    if (entry->getKey() < key) {
        hi += 1;
    }
    return ~hi; // we twiddle to indicate not-found
}

#ifdef TRACE_HASH_HITS
static int gHashHits;
static int gSortedHits;
#endif

SkTextureCache::Entry* SkTextureCache::find(const Key& key, int* insert) const {
    int count = fSorted.count();
    if (count == 0) {
        *insert = 0;
        return NULL;
    }

    // check the hash first
    int hashIndex = key.getHashIndex();
    Entry* entry = fHash[hashIndex];    
    if (NULL != entry && entry->getKey() == key) {
#ifdef TRACE_HASH_HITS
        gHashHits += 1;
#endif
        return entry;
    }
    
    int index = this->findInSorted(key);
    if (index >= 0) {
#ifdef TRACE_HASH_HITS
        gSortedHits += 1;
#endif
        entry = fSorted[index];
        fHash[hashIndex] = entry;
        return entry;
    }
    
    // ~index is where to insert the entry
    *insert = ~index;
    return NULL;
}

SkTextureCache::Entry* SkTextureCache::lock(const SkBitmap& bitmap) {
    this->validate();
    
    // call this before we call find(), so we don't reorder after find() and
    // invalidate our index
    this->purgeIfNecessary(SkGL::ComputeTextureMemorySize(bitmap));

    Key key(bitmap);
    int index;
    Entry* entry = this->find(key, &index);

    if (NULL == entry) {
        entry = SkNEW_ARGS(Entry, (bitmap));
        
        entry->fName = SkGL::BindNewTexture(bitmap, &entry->fTexSize);
        if (0 == entry->fName) {
            SkDELETE(entry);
            return NULL;
        }
        fHash[key.getHashIndex()] = entry;
        *fSorted.insert(index) = entry;

        fTexCount += 1;
        fTexSize += entry->memSize();
    } else {
        // detach from our llist
        Entry* prev = entry->fPrev;
        Entry* next = entry->fNext;
        if (prev) {
            prev->fNext = next;
        } else {
            SkASSERT(fHead == entry);
            fHead = next;
        }
        if (next) {
            next->fPrev = prev;
        } else {
            SkASSERT(fTail == entry);
            fTail = prev;
        }
        // now bind the texture
        glBindTexture(GL_TEXTURE_2D, entry->fName);
    }
    
    // add to head of llist for LRU
    entry->fPrev = NULL;
    entry->fNext = fHead;
    if (NULL != fHead) {
        SkASSERT(NULL == fHead->fPrev);
        fHead->fPrev = entry;
    }
    fHead = entry;
    if (NULL == fTail) {
        fTail = entry;
    }
    
    this->validate();
    entry->lock();
    
#ifdef TRACE_HASH_HITS
    SkDebugf("---- texture cache hash=%d sorted=%d\n", gHashHits, gSortedHits);
#endif
    return entry;
}

void SkTextureCache::unlock(Entry* entry) {
    this->validate();

#ifdef SK_DEBUG
    SkASSERT(entry);
    int index = this->findInSorted(entry->getKey());
    SkASSERT(fSorted[index] == entry);
#endif

    SkASSERT(entry->fLockCount > 0);
    entry->unlock();
}

void SkTextureCache::purgeIfNecessary(size_t extraSize) {
    this->validate();

    size_t countMax = fTexCountMax;
    size_t sizeMax = fTexSizeMax;
    
    // take extraSize into account, but watch for underflow of size_t
    if (extraSize > sizeMax) {
        sizeMax = 0;
    } else {
        sizeMax -= extraSize;
    }

    Entry* entry = fTail;
    while (entry) {
        if (fTexCount <= countMax && fTexSize <= sizeMax) {
            break;
        }

        Entry* prev = entry->fPrev;
        // don't purge an entry that is locked
        if (entry->isLocked()) {
            entry = prev;
            continue;
        }

        fTexCount -= 1;
        fTexSize -= entry->memSize();

        // remove from our sorted and hash arrays
        int index = this->findInSorted(entry->getKey());
        SkASSERT(index >= 0);
        fSorted.remove(index);
        index = entry->getKey().getHashIndex();
        if (entry == fHash[index]) {
            fHash[index] = NULL;
        }

        // now detach it from our llist
        Entry* next = entry->fNext;
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
        
        // now delete it
#ifdef TRACE_TEXTURE_CACHE_PURGE
        SkDebugf("---- purge texture cache %d size=%d\n",
                 entry->name(), entry->memSize());
#endif
        SkDELETE(entry);
        
        // keep going
        entry = prev;
    }

    this->validate();
}

void SkTextureCache::setMaxCount(size_t count) {
    if (fTexCountMax != count) {
        fTexCountMax = count;
        this->purgeIfNecessary(0);
    }
}

void SkTextureCache::setMaxSize(size_t size) {
    if (fTexSizeMax != size) {
        fTexSizeMax = size;
        this->purgeIfNecessary(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkTextureCache::validate() const {
    if (0 == fTexCount) {
        SkASSERT(0 == fTexSize);
        SkASSERT(NULL == fHead);
        SkASSERT(NULL == fTail);
        return;
    }

    SkASSERT(fTexSize); // do we allow a zero-sized texture?
    SkASSERT(fHead);
    SkASSERT(fTail);
    
    SkASSERT(NULL == fHead->fPrev);
    SkASSERT(NULL == fTail->fNext);
    if (1 == fTexCount) {
        SkASSERT(fHead == fTail);
    }

    const Entry* entry = fHead;
    size_t count = 0;
    size_t size = 0;
    size_t i;

    while (entry != NULL) {
        SkASSERT(count < fTexCount);
        SkASSERT(size < fTexSize);
        size += entry->memSize();
        count += 1;
        if (NULL == entry->fNext) {
            SkASSERT(fTail == entry);
        }
        entry = entry->fNext;
    }
    SkASSERT(count == fTexCount);
    SkASSERT(size == fTexSize);

    count = 0;
    size = 0;
    entry = fTail;
    while (entry != NULL) {
        SkASSERT(count < fTexCount);
        SkASSERT(size < fTexSize);
        size += entry->memSize();
        count += 1;
        if (NULL == entry->fPrev) {
            SkASSERT(fHead == entry);
        }
        entry = entry->fPrev;
    }
    SkASSERT(count == fTexCount);
    SkASSERT(size == fTexSize);
    
    SkASSERT(count == (size_t)fSorted.count());
    for (i = 1; i < count; i++) {
        SkASSERT(fSorted[i-1]->getKey() < fSorted[i]->getKey());
    }
    
    for (i = 0; i < kHashCount; i++) {
        if (fHash[i]) {
            size_t index = fHash[i]->getKey().getHashIndex();
            SkASSERT(index == i);
            index = fSorted.find(fHash[i]);
            SkASSERT((size_t)index < count);
        }
    }
}
#endif


