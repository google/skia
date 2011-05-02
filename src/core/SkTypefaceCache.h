/*
    Copyright 2011 Google Inc.

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


#ifndef SkTypefaceCache_DEFINED
#define SkTypefaceCache_DEFINED

#include "SkTypeface.h"
#include "SkTDArray.h"

/*  TODO
 *  Provide std way to cache name+requestedStyle aliases to the same typeface.
 *
 *  The current mechanism ends up create a diff typeface for each one, even if
 *  they map to the same internal obj (e.g. CTFontRef on the mac)
 */

class SkTypefaceCache {
public:
    typedef bool (*FindProc)(SkTypeface*, SkTypeface::Style, void* context);

    /**
     *  Helper: returns a unique fontID to pass to the constructor of
     *  your subclass of SkTypeface
     */
    static SkFontID NewFontID();

    /**
     *  Add a typeface to the cache. This ref()s the typeface, so that the
     *  cache is also an owner. Later, if we need to purge the cache, it will
     *  unref() typefaces whose refcnt is 1 (meaning only the cache is an owner).
     */
    static void Add(SkTypeface*, SkTypeface::Style requested);

    /**
     *  Search the cache for a typeface with the specified fontID (uniqueID).
     *  If one is found, return it (its reference count is unmodified). If none
     *  is found, return NULL.
     */
    static SkTypeface* FindByID(SkFontID fontID);

    /**
     *  Iterate through the cache, calling proc(typeface, ctx) with each
     *  typeface. If proc returns true, then we return that typeface (its
     *  reference count is unmodified). If it never returns true, we return NULL.
     */
    static SkTypeface* FindByProc(FindProc proc, void* ctx);

    /**
     *  Debugging only: dumps the status of the typefaces in the cache
     */
    static void Dump();

private:
    static SkTypefaceCache& Get();

    void add(SkTypeface*, SkTypeface::Style requested);
    SkTypeface* findByID(SkFontID findID) const;
    SkTypeface* findByProc(FindProc proc, void* ctx) const;
    void purge(int count);

    struct Rec {
        SkTypeface*         fFace;
        SkTypeface::Style   fRequestedStyle;
    };
    SkTDArray<Rec> fArray;
};

#endif
