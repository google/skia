/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkTypefaceCache_DEFINED
#define SkTypefaceCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTArray.h"

class SkTypefaceCache {
public:
    SkTypefaceCache();

    /**
     * Callback for FindByProc. Returns true if the given typeface is a match
     * for the given context. The passed typeface is owned by the cache and is
     * not additionally ref()ed. The typeface may be in the disposed state.
     */
    typedef bool(*FindProc)(SkTypeface*, void* context);

    /**
     *  Add a typeface to the cache. Later, if we need to purge the cache,
     *  typefaces uniquely owned by the cache will be unref()ed.
     */
    void add(sk_sp<SkTypeface>);

    /**
     *  Iterate through the cache, calling proc(typeface, ctx) for each typeface.
     *  If proc returns true, then return that typeface.
     *  If it never returns true, return nullptr.
     */
    sk_sp<SkTypeface> findByProcAndRef(FindProc proc, void* ctx) const;

    /**
     *  This will unref all of the typefaces in the cache for which the cache
     *  is the only owner. Normally this is handled automatically as needed.
     *  This function is exposed for clients that explicitly want to purge the
     *  cache (e.g. to look for leaks).
     */
    void purgeAll();

    /**
     *  Helper: returns a unique fontID to pass to the constructor of
     *  your subclass of SkTypeface
     */
    static SkFontID NewFontID();

    // These are static wrappers around a global instance of a cache.

    static void Add(sk_sp<SkTypeface>);
    static sk_sp<SkTypeface> FindByProcAndRef(FindProc proc, void* ctx);
    static void PurgeAll();

    /**
     *  Debugging only: dumps the status of the typefaces in the cache
     */
    static void Dump();

private:
    static SkTypefaceCache& Get();

    void purge(int count);

    SkTArray<sk_sp<SkTypeface>> fTypefaces;
};

#endif
