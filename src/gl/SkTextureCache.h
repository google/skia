#ifndef SkTextureCache_DEFINED
#define SkTextureCache_DEFINED

#include "SkBitmap.h"
#include "SkPoint.h"
#include "SkGL.h"
#include "SkTDArray.h"

class SkTextureCache {
public:
    SkTextureCache(size_t maxCount, size_t maxSize);
    ~SkTextureCache();
    
    size_t getMaxCount() { return fTexCountMax; }
    size_t getMaxSize() { return fTexSizeMax; }

    void setMaxCount(size_t count);
    void setMaxSize(size_t size);
    
    /** Deletes all the caches. Pass true if the texture IDs are still valid,
        and if so, it will call glDeleteTextures. Pass false if the texture IDs
        are invalid (e.g. the gl-context has changed), in which case they will
        just be abandoned.
    */
    void deleteAllCaches(bool texturesAreValid);
    
    static int HashMask() { return kHashMask; }
    
    class Key {
    public:
        Key(const SkBitmap& bm) {
            fGenID = bm.getGenerationID();
            fOffset = bm.pixelRefOffset();
            fWH = (bm.width() << 16) | bm.height();
            this->computeHash();
        }
        
        int getHashIndex() const { return fHashIndex; }
        
        friend bool operator==(const Key& a, const Key& b) {
            return  a.fHash ==   b.fHash &&
                    a.fGenID ==  b.fGenID &&
                    a.fOffset == b.fOffset &&
                    a.fWH ==     b.fWH;
        }
        
        friend bool operator<(const Key& a, const Key& b) {
            if (a.fHash < b.fHash) {
                return true;
            } else if (a.fHash > b.fHash) {
                return false;
            }
            
            if (a.fGenID < b.fGenID) {
                return true;
            } else if (a.fGenID > b.fGenID) {
                return false;
            }
            
            if (a.fOffset < b.fOffset) {
                return true;
            } else if (a.fOffset > b.fOffset) {
                return false;
            }
            
            return a.fWH < b.fWH;
        }
        
    private:
        void computeHash() {
            uint32_t hash = fGenID ^ fOffset ^ fWH;
            fHash = hash;
            hash ^= hash >> 16;
            fHashIndex = hash & SkTextureCache::HashMask();
        }
        
        uint32_t    fHash;  // computed from the other fields
        uint32_t    fGenID;
        size_t      fOffset;
        uint32_t    fWH;
        // for indexing into the texturecache's fHash
        int fHashIndex;
    };

    class Entry {
    public:
        GLuint name() const { return fName; }
        SkPoint texSize() const { return fTexSize; }
        size_t memSize() const { return fMemSize; }
        const Key& getKey() const { return fKey; }

        // call this to clear the texture name, in case the context has changed
        // in which case we should't reference or delete this texture in GL
        void abandonTexture() { fName = 0; }

    private:
        Entry(const SkBitmap& bitmap);
        ~Entry();

        int lockCount() const { return fLockCount; }
        bool isLocked() const { return fLockCount > 0; }

        void lock() { fLockCount += 1; }
        void unlock() {
            SkASSERT(fLockCount > 0);
            fLockCount -= 1;
        }

    private:
        GLuint  fName;
        SkPoint fTexSize;
        Key     fKey;
        size_t  fMemSize;
        int     fLockCount;
        
        Entry*  fPrev;
        Entry*  fNext;
        
        friend class SkTextureCache;
    };
    
    Entry* lock(const SkBitmap&);
    void unlock(Entry*);
    
private:
    void purgeIfNecessary(size_t extraSize);
    
#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    Entry* fHead;
    Entry* fTail;
    
    // limits for the cache
    size_t  fTexCountMax;
    size_t  fTexSizeMax;
    
    // current values for the cache
    size_t  fTexCount;
    size_t  fTexSize;
    
    enum {
        kHashBits = 6,
        kHashCount = 1 << kHashBits,
        kHashMask = kHashCount - 1
    };
    mutable Entry* fHash[kHashCount];
    SkTDArray<Entry*> fSorted;
    
    /*  If we find the key, return the entry and ignore index. If we don't,
        return NULL and set index to the place to insert the entry in fSorted
    */
    Entry* find(const Key&, int* index) const;
    // returns index or <0 if not found. Does NOT update hash
    int findInSorted(const Key& key) const;
};

#endif
