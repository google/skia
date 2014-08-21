
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGradientBitmapCache_DEFINED
#define SkGradientBitmapCache_DEFINED

#include "SkBitmap.h"

class SkGradientBitmapCache : SkNoncopyable {
public:
    SkGradientBitmapCache(int maxEntries);
    ~SkGradientBitmapCache();

    bool find(const void* buffer, size_t len, SkBitmap*) const;
    void add(const void* buffer, size_t len, const SkBitmap&);

private:
    int fEntryCount;
    const int fMaxEntries;

    struct Entry;
    mutable Entry*  fHead;
    mutable Entry*  fTail;

    inline Entry* detach(Entry*) const;
    inline void attachToHead(Entry*) const;

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate : SkNoncopyable {
    public:
        AutoValidate(const SkGradientBitmapCache* bc) : fBC(bc) { bc->validate(); }
        ~AutoValidate() { fBC->validate(); }
    private:
        const SkGradientBitmapCache* fBC;
    };
};

#endif
