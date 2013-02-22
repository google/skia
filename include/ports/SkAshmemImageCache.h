/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAshmemImageCache_DEFINED
#define SkAshmemImageCache_DEFINED

#include "SkImageCache.h"
#include "SkTDArray.h"
#include "SkTypes.h"

class SkAshmemImageCache : public SkImageCache {

public:
    /**
     *  Get a pointer to the single global instance of SkAshmemImageCache.
     */
    static SkAshmemImageCache* GetAshmemImageCache();

    virtual void* allocAndPinCache(size_t bytes, intptr_t* ID) SK_OVERRIDE;
    virtual void* pinCache(intptr_t ID) SK_OVERRIDE;
    virtual void releaseCache(intptr_t ID) SK_OVERRIDE;
    virtual void throwAwayCache(intptr_t ID) SK_OVERRIDE;

#ifdef SK_DEBUG
    SkImageCache::CacheStatus getCacheStatus(intptr_t ID) const SK_OVERRIDE;

    virtual ~SkAshmemImageCache();
#endif

private:
    struct AshmemRec {
        int    fFD;
        void*  fAddr;
        size_t fSize;
#ifdef SK_DEBUG
        bool   fPinned;

        static int Compare(const AshmemRec*, const AshmemRec*);
#endif
    };

    /**
     *  Constructor is private. The correct way to get this cache is through
     *  GetAshmemImageCache, so that all callers can get the single global.
     */
    SkAshmemImageCache();

#ifdef SK_DEBUG
    // Stores a list of AshmemRecs to track deletion.
    SkTDArray<AshmemRec*> fRecs;

    /**
     *  Debug only function to add an AshmemRec to the list.
     */
    void appendRec(AshmemRec*);

    /**
     *  Return the index of AshmemRec.
     */
    int findRec(const AshmemRec*) const;
#endif

    /**
     *  Deletes AshmemRec. In debug, also removes from the list.
     */
    void removeRec(AshmemRec*);
};
#endif // SkAshmemImageCache_DEFINED

