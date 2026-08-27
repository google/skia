/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCachedData_DEFINED
#define SkCachedData_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTemplates.h"

#include <cstddef>
#include <memory>
#include <variant>

class SkDiscardableMemory;

class SkCachedData : ::SkNoncopyable {
public:
    SkCachedData(void* mallocData, size_t size);
    SkCachedData(size_t size, SkDiscardableMemory*);
    virtual ~SkCachedData();

    size_t size() const { return fSize; }
    const void* data() const { return fData; }

    void* writable_data() { return fData; }

    void ref() const { this->internalRef(false); }
    void unref() const { this->internalUnref(false); }

    int testing_only_getRefCnt() const { return fRefCnt; }
    bool testing_only_isLocked() const { return fIsLocked; }
    bool testing_only_isInCache() const { return fInCache; }

    SkDiscardableMemory* diagnostic_only_getDiscardable() const;

protected:
    // called when fData changes. could be nullptr.
    virtual void onDataChange(void* oldData, void* newData) {}

private:
    using MallocStorage = std::unique_ptr<void, SkFunctionObject<sk_free>>;
    using DiscardableStorage = std::unique_ptr<SkDiscardableMemory>;
    using Storage = std::variant<DiscardableStorage, MallocStorage>;

    SkMutex fMutex;     // could use a pool of these...

    Storage fStorage;
    void* fData = nullptr;
    size_t fSize = 0;
    int fRefCnt = 1;  // low-bit means we're owned by the cache
    bool fInCache = false;
    bool fIsLocked = true;

    void internalRef(bool fromCache) const;
    void internalUnref(bool fromCache) const;

    void inMutexRef(bool fromCache);
    bool inMutexUnref(bool fromCache);  // returns true if we should delete "this"
    void inMutexLock();
    void inMutexUnlock();

    // called whenever our fData might change (lock or unlock)
    void setData(void* newData) {
        if (newData != fData) {
            // notify our subclasses of the change
            this->onDataChange(fData, newData);
            fData = newData;
        }
    }

    class AutoMutexWritable;

public:
#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

   /*
     *  Attaching a data to to a SkResourceCache (only one at a time) enables the data to be
     *  unlocked when the cache is the only owner, thus freeing it to be purged (assuming the
     *  data is backed by a SkDiscardableMemory).
     *
     *  When attached, it also automatically attempts to "lock" the data when the first client
     *  ref's the data (typically from a find(key, visitor) call).
     *
     *  Thus the data will always be "locked" when a non-cache has a ref on it (whether or not
     *  the lock succeeded to recover the memory -- check data() to see if it is nullptr).
     */

    /*
     *  Call when adding this instance to a SkResourceCache::Rec subclass
     *  (typically in the Rec's constructor).
     */
    void attachToCacheAndRef() const { this->internalRef(true); }

    /*
     *  Call when removing this instance from a SkResourceCache::Rec subclass
     *  (typically in the Rec's destructor).
     */
    void detachFromCacheAndUnref() const { this->internalUnref(true); }
};

#endif
