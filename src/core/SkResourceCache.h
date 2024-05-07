/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkResourceCache_DEFINED
#define SkResourceCache_DEFINED

#include "include/private/base/SkDebug.h"
#include "src/core/SkMessageBus.h"

#include <cstddef>
#include <cstdint>

class SkCachedData;
class SkDiscardableMemory;
class SkTraceMemoryDump;

/**
 *  Cache object for bitmaps (with possible scale in X Y as part of the key).
 *
 *  Multiple caches can be instantiated, but each instance is not implicitly
 *  thread-safe, so if a given instance is to be shared across threads, the
 *  caller must manage the access itself (e.g. via a mutex).
 *
 *  As a convenience, a global instance is also defined, which can be safely
 *  access across threads via the static methods (e.g. FindAndLock, etc.).
 */
class SkResourceCache {
public:
    struct Key {
        /** Key subclasses must call this after their own fields and data are initialized.
         *  All fields and data must be tightly packed.
         *  @param nameSpace must be unique per Key subclass.
         *  @param sharedID == 0 means ignore this field, does not support group purging.
         *  @param dataSize is size of fields and data of the subclass, must be a multiple of 4.
         */
        void init(void* nameSpace, uint64_t sharedID, size_t dataSize);

        /** Returns the size of this key. */
        size_t size() const {
            return fCount32 << 2;
        }

        void* getNamespace() const { return fNamespace; }
        uint64_t getSharedID() const { return ((uint64_t)fSharedID_hi << 32) | fSharedID_lo; }

        // This is only valid after having called init().
        uint32_t hash() const { return fHash; }

        bool operator==(const Key& other) const {
            const uint32_t* a = this->as32();
            const uint32_t* b = other.as32();
            for (int i = 0; i < fCount32; ++i) {  // (This checks fCount == other.fCount first.)
                if (a[i] != b[i]) {
                    return false;
                }
            }
            return true;
        }

    private:
        int32_t  fCount32;   // local + user contents count32
        uint32_t fHash;
        // split uint64_t into hi and lo so we don't force ourselves to pad on 32bit machines.
        uint32_t fSharedID_lo;
        uint32_t fSharedID_hi;
        void*    fNamespace; // A unique namespace tag. This is hashed.
        /* uint32_t fContents32[] */

        const uint32_t* as32() const { return (const uint32_t*)this; }
    };

    struct Rec {
        typedef SkResourceCache::Key Key;

        Rec() {}
        virtual ~Rec() {}

        uint32_t getHash() const { return this->getKey().hash(); }

        virtual const Key& getKey() const = 0;
        virtual size_t bytesUsed() const = 0;

        // Called if the cache needs to purge/remove/delete the Rec. Default returns true.
        // Subclass may return false if there are outstanding references to it (e.g. bitmaps).
        // Will only be deleted/removed-from-the-cache when this returns true.
        virtual bool canBePurged() { return true; }

        // A rec is first created/initialized, and then added to the cache. As part of the add(),
        // the cache will callback into the rec with postAddInstall, passing in whatever payload
        // was passed to add/Add.
        //
        // This late-install callback exists because the process of add-ing might end up deleting
        // the new rec (if an existing rec in the cache has the same key and cannot be purged).
        // If the new rec will be deleted during add, the pre-existing one (with the same key)
        // will have postAddInstall() called on it instead, so that either way an "install" will
        // happen during the add.
        virtual void postAddInstall(void*) {}

        // for memory usage diagnostics
        virtual const char* getCategory() const = 0;
        virtual SkDiscardableMemory* diagnostic_only_getDiscardable() const { return nullptr; }

    private:
        Rec*    fNext;
        Rec*    fPrev;

        friend class SkResourceCache;
    };

    // Used with SkMessageBus
    struct PurgeSharedIDMessage {
        PurgeSharedIDMessage(uint64_t sharedID) : fSharedID(sharedID) {}
        uint64_t fSharedID;
    };

    typedef const Rec* ID;

    /**
     *  Callback function for find(). If called, the cache will have found a match for the
     *  specified Key, and will pass in the corresponding Rec, along with a caller-specified
     *  context. The function can read the data in Rec, and copy whatever it likes into context
     *  (casting context to whatever it really is).
     *
     *  The return value determines what the cache will do with the Rec. If the function returns
     *  true, then the Rec is considered "valid". If false is returned, the Rec will be considered
     *  "stale" and will be purged from the cache.
     */
    typedef bool (*FindVisitor)(const Rec&, void* context);

    /**
     *  Returns a locked/pinned SkDiscardableMemory instance for the specified
     *  number of bytes, or nullptr on failure.
     */
    typedef SkDiscardableMemory* (*DiscardableFactory)(size_t bytes);

    /*
     *  The following static methods are thread-safe wrappers around a global
     *  instance of this cache.
     */

    /**
     *  Returns true if the visitor was called on a matching Key, and the visitor returned true.
     *
     *  Find() will search the cache for the specified Key. If no match is found, return false and
     *  do not call the FindVisitor. If a match is found, return whatever the visitor returns.
     *  Its return value is interpreted to mean:
     *      true  : Rec is valid
     *      false : Rec is "stale" -- the cache will purge it.
     */
    static bool Find(const Key& key, FindVisitor, void* context);
    static void Add(Rec*, void* payload = nullptr);

    typedef void (*Visitor)(const Rec&, void* context);
    // Call the visitor for every Rec in the cache.
    static void VisitAll(Visitor, void* context);

    static size_t GetTotalBytesUsed();
    static size_t GetTotalByteLimit();
    static size_t SetTotalByteLimit(size_t newLimit);

    static size_t SetSingleAllocationByteLimit(size_t);
    static size_t GetSingleAllocationByteLimit();
    static size_t GetEffectiveSingleAllocationByteLimit();

    static void PurgeAll();
    static void CheckMessages();

    static void TestDumpMemoryStatistics();

    /** Dump memory usage statistics of every Rec in the cache using the
        SkTraceMemoryDump interface.
     */
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    /**
     *  Returns the DiscardableFactory used by the global cache, or nullptr.
     */
    static DiscardableFactory GetDiscardableFactory();

    static SkCachedData* NewCachedData(size_t bytes);

    static void PostPurgeSharedID(uint64_t sharedID);

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    static void Dump();

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Construct the cache to call DiscardableFactory when it
     *  allocates memory for the pixels. In this mode, the cache has
     *  not explicit budget, and so methods like getTotalBytesUsed()
     *  and getTotalByteLimit() will return 0, and setTotalByteLimit
     *  will ignore its argument and return 0.
     */
    SkResourceCache(DiscardableFactory);

    /**
     *  Construct the cache, allocating memory with malloc, and respect the
     *  byteLimit, purging automatically when a new image is added to the cache
     *  that pushes the total bytesUsed over the limit. Note: The limit can be
     *  changed at runtime with setTotalByteLimit.
     */
    explicit SkResourceCache(size_t byteLimit);
    ~SkResourceCache();

    /**
     *  Returns true if the visitor was called on a matching Key, and the visitor returned true.
     *
     *  find() will search the cache for the specified Key. If no match is found, return false and
     *  do not call the FindVisitor. If a match is found, return whatever the visitor returns.
     *  Its return value is interpreted to mean:
     *      true  : Rec is valid
     *      false : Rec is "stale" -- the cache will purge it.
     */
    bool find(const Key&, FindVisitor, void* context);
    void add(Rec*, void* payload = nullptr);
    void visitAll(Visitor, void* context);

    size_t getTotalBytesUsed() const { return fTotalBytesUsed; }
    size_t getTotalByteLimit() const { return fTotalByteLimit; }

    /**
     *  This is respected by SkBitmapProcState::possiblyScaleImage.
     *  0 is no maximum at all; this is the default.
     *  setSingleAllocationByteLimit() returns the previous value.
     */
    size_t setSingleAllocationByteLimit(size_t maximumAllocationSize);
    size_t getSingleAllocationByteLimit() const;
    // returns the logical single allocation size (pinning against the budget when the cache
    // is not backed by discardable memory.
    size_t getEffectiveSingleAllocationByteLimit() const;

    /**
     *  Set the maximum number of bytes available to this cache. If the current
     *  cache exceeds this new value, it will be purged to try to fit within
     *  this new limit.
     */
    size_t setTotalByteLimit(size_t newLimit);

    void purgeSharedID(uint64_t sharedID);

    void purgeAll() {
        this->purgeAsNeeded(true);
    }

    DiscardableFactory discardableFactory() const { return fDiscardableFactory; }

    SkCachedData* newCachedData(size_t bytes);

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    void dump() const;

private:
    Rec*    fHead;
    Rec*    fTail;

    class Hash;
    Hash*   fHash;

    DiscardableFactory  fDiscardableFactory;

    size_t  fTotalBytesUsed;
    size_t  fTotalByteLimit;
    size_t  fSingleAllocationByteLimit;
    int     fCount;

    SkMessageBus<PurgeSharedIDMessage, uint32_t>::Inbox fPurgeSharedIDInbox;

    void checkMessages();
    void purgeAsNeeded(bool forcePurge = false);

    // linklist management
    void moveToHead(Rec*);
    void addToHead(Rec*);
    void release(Rec*);
    void remove(Rec*);

    void init();    // called by constructors

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};
#endif
