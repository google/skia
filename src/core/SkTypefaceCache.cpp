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

void SkTypefaceCache::add(SkTypeface* face) {
    if (fTypefaces.count() >= TYPEFACE_CACHE_LIMIT) {
        this->purge(TYPEFACE_CACHE_LIMIT >> 2);
    }

    fTypefaces.emplace_back(SkRef(face));
}

SkTypeface* SkTypefaceCache::findByProcAndRef(FindProc proc, void* ctx) const {
    for (const sk_sp<SkTypeface>& typeface : fTypefaces) {
        if (proc(typeface.get(), ctx)) {
            return SkRef(typeface.get());
        }
    }
    return nullptr;
}

void SkTypefaceCache::purge(int numToPurge) {
    int count = fTypefaces.count();
    int i = 0;
    while (i < count) {
        if (fTypefaces[i]->unique()) {
            fTypefaces.removeShuffle(i);
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
    this->purge(fTypefaces.count());
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

void SkTypefaceCache::Add(SkTypeface* face) {
    SkAutoMutexAcquire ama(gMutex);
    Get().add(face);
}

SkTypeface* SkTypefaceCache::FindByProcAndRef(FindProc proc, void* ctx) {
    SkAutoMutexAcquire ama(gMutex);
    return Get().findByProcAndRef(proc, ctx);
}

void SkTypefaceCache::PurgeAll() {
    SkAutoMutexAcquire ama(gMutex);
    Get().purgeAll();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
static bool DumpProc(SkTypeface* face, void* ctx) {
    SkString n;
    face->getFamilyName(&n);
    SkFontStyle s = face->fontStyle();
    SkFontID id = face->uniqueID();
    SkDebugf("SkTypefaceCache: face %p fontID %d weight %d width %d style %d refcnt %d name %s\n",
             face, id, s.weight(), s.width(), s.slant(), face->getRefCnt(), n.c_str());
    return false;
}
#endif

void SkTypefaceCache::Dump() {
#ifdef SK_DEBUG
    (void)Get().findByProcAndRef(DumpProc, nullptr);
#endif
}
