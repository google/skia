
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkTypefaceCache.h"
#include "SkThread.h"

#define TYPEFACE_CACHE_LIMIT    1024

SkTypefaceCache::SkTypefaceCache() {}

SkTypefaceCache::~SkTypefaceCache() {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        if (curr->fStrong) {
            curr->fFace->unref();
        } else {
            curr->fFace->weak_unref();
        }
        curr += 1;
    }
}

void SkTypefaceCache::add(SkTypeface* face,
                          SkTypeface::Style requestedStyle,
                          bool strong) {
    if (fArray.count() >= TYPEFACE_CACHE_LIMIT) {
        this->purge(TYPEFACE_CACHE_LIMIT >> 2);
    }

    Rec* rec = fArray.append();
    rec->fFace = face;
    rec->fRequestedStyle = requestedStyle;
    rec->fStrong = strong;
    if (strong) {
        face->ref();
    } else {
        face->weak_ref();
    }
}

SkTypeface* SkTypefaceCache::findByID(SkFontID fontID) const {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        if (curr->fFace->uniqueID() == fontID) {
            return curr->fFace;
        }
        curr += 1;
    }
    return NULL;
}

SkTypeface* SkTypefaceCache::findByProcAndRef(FindProc proc, void* ctx) const {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        SkTypeface* currFace = curr->fFace;
        if (proc(currFace, curr->fRequestedStyle, ctx)) {
            if (curr->fStrong) {
                currFace->ref();
                return currFace;
            } else if (currFace->try_ref()) {
                return currFace;
            } else {
                //remove currFace from fArray?
            }
        }
        curr += 1;
    }
    return NULL;
}

void SkTypefaceCache::purge(int numToPurge) {
    int count = fArray.count();
    int i = 0;
    while (i < count) {
        SkTypeface* face = fArray[i].fFace;
        bool strong = fArray[i].fStrong;
        if ((strong && face->unique()) || (!strong && face->weak_expired())) {
            if (strong) {
                face->unref();
            } else {
                face->weak_unref();
            }
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

void SkTypefaceCache::Add(SkTypeface* face,
                          SkTypeface::Style requestedStyle,
                          bool strong) {
    SkAutoMutexAcquire ama(gMutex);
    Get().add(face, requestedStyle, strong);
}

SkTypeface* SkTypefaceCache::FindByID(SkFontID fontID) {
    SkAutoMutexAcquire ama(gMutex);
    return Get().findByID(fontID);
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
static bool DumpProc(SkTypeface* face, SkTypeface::Style style, void* ctx) {
    SkDebugf("SkTypefaceCache: face %p fontID %d style %d refcnt %d\n",
             face, face->uniqueID(), style, face->getRefCnt());
    return false;
}
#endif

void SkTypefaceCache::Dump() {
#ifdef SK_DEBUG
    SkAutoMutexAcquire ama(gMutex);
    (void)Get().findByProcAndRef(DumpProc, NULL);
#endif
}
