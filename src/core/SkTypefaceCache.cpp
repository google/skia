/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkTypefaceCache.h"
#include "SkAtomics.h"
#include "SkMutex.h"

#define TYPEFACE_CACHE_LIMIT    1024

SkTypefaceCache::SkTypefaceCache() {}

SkTypefaceCache::~SkTypefaceCache() {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        curr->fFace->unref();
        curr += 1;
    }
}

void SkTypefaceCache::add(SkTypeface* face, const SkFontStyle& requestedStyle) {
    if (fArray.count() >= TYPEFACE_CACHE_LIMIT) {
        this->purge(TYPEFACE_CACHE_LIMIT >> 2);
    }

    Rec* rec = fArray.append();
    rec->fFace = SkRef(face);
    rec->fRequestedStyle = requestedStyle;
}

SkTypeface* SkTypefaceCache::findByProcAndRef(FindProc proc, void* ctx) const {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        SkTypeface* currFace = curr->fFace;
        if (proc(currFace, curr->fRequestedStyle, ctx)) {
            return SkRef(currFace);
        }
        curr += 1;
    }
    return nullptr;
}

void SkTypefaceCache::purge(int numToPurge) {
    int count = fArray.count();
    int i = 0;
    while (i < count) {
        SkTypeface* face = fArray[i].fFace;
        if (face->unique()) {
            face->unref();
            fArray.remove(i);
            --count;
            if (--numToPurge == 0) {
                return;
            }
        } else {
            ++i;
        }
    }
}

void SkTypefaceCache::purgeAll() {
    this->purge(fArray.count());
}

///////////////////////////////////////////////////////////////////////////////

SkTypefaceCache& SkTypefaceCache::Get() {
    static SkTypefaceCache gCache;
    return gCache;
}

SkFontID SkTypefaceCache::NewFontID() {
    static int32_t gFontID;
    return sk_atomic_inc(&gFontID) + 1;
}

SK_DECLARE_STATIC_MUTEX(gMutex);

void SkTypefaceCache::Add(SkTypeface* face, const SkFontStyle& requestedStyle) {
    SkAutoMutexAcquire ama(gMutex);
    Get().add(face, requestedStyle);
}

SkTypeface* SkTypefaceCache::FindByProcAndRef(FindProc proc, void* ctx) {
    SkAutoMutexAcquire ama(gMutex);
    SkTypeface* typeface = Get().findByProcAndRef(proc, ctx);
    return typeface;
}

void SkTypefaceCache::PurgeAll() {
    SkAutoMutexAcquire ama(gMutex);
    Get().purgeAll();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
static bool DumpProc(SkTypeface* face, const SkFontStyle& s, void* ctx) {
    SkDebugf("SkTypefaceCache: face %p fontID %d weight %d width %d style %d refcnt %d\n",
             face, face->uniqueID(), s.weight(), s.width(), s.slant(), face->getRefCnt());
    return false;
}
#endif

void SkTypefaceCache::Dump() {
#ifdef SK_DEBUG
    SkAutoMutexAcquire ama(gMutex);
    (void)Get().findByProcAndRef(DumpProc, nullptr);
#endif
}
