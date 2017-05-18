/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkTypefaceCache_DEFINED
#define SkTypefaceCache_DEFINED

#include "SkRefCnt.h"
#include "SkTypeface.h"
#include "SkTArray.h"

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
     *  Add a typeface to the cache. This ref()s the typeface, so that the
     *  cache is also an owner. Later, if we need to purge the cache, typefaces
     *  whose refcnt is 1 (meaning only the cache is an owner) will be
     *  unref()ed.
     */
    void add(SkTypeface*);

    /**
     *  Iterate through the cache, calling proc(typeface, ctx) with each
     *  typeface. If proc returns true, then we return that typeface (this
     *  ref()s the typeface). If it never returns true, we return nullptr.
     */
    SkTypeface* findByProcAndRef(FindProc proc, void* ctx) const;

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

    static void Add(SkTypeface*);
    static SkTypeface* FindByProcAndRef(FindProc proc, void* ctx);
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
