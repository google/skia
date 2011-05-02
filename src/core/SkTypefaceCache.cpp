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


#include "SkTypefaceCache.h"
#include "SkThread.h"

#define TYPEFACE_CACHE_LIMIT    128

void SkTypefaceCache::add(SkTypeface* face, SkTypeface::Style requestedStyle) {
    if (fArray.count() >= TYPEFACE_CACHE_LIMIT) {
        this->purge(TYPEFACE_CACHE_LIMIT >> 2);
    }

    Rec* rec = fArray.append();
    rec->fFace = face;
    rec->fRequestedStyle = requestedStyle;
    face->ref();
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

SkTypeface* SkTypefaceCache::findByProc(FindProc proc, void* ctx) const {
    const Rec* curr = fArray.begin();
    const Rec* stop = fArray.end();
    while (curr < stop) {
        if (proc(curr->fFace, curr->fRequestedStyle, ctx)) {
            return curr->fFace;
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
        if (1 == face->getRefCnt()) {
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

///////////////////////////////////////////////////////////////////////////////

SkTypefaceCache& SkTypefaceCache::Get() {
    static SkTypefaceCache gCache;
    return gCache;
}

SkFontID SkTypefaceCache::NewFontID() {
    static int32_t gFontID;
    return sk_atomic_inc(&gFontID) + 1;
}

static SkMutex gMutex;

void SkTypefaceCache::Add(SkTypeface* face, SkTypeface::Style requestedStyle) {
    SkAutoMutexAcquire ama(gMutex);
    Get().add(face, requestedStyle);
}

SkTypeface* SkTypefaceCache::FindByID(SkFontID fontID) {
    SkAutoMutexAcquire ama(gMutex);
    return Get().findByID(fontID);
}

SkTypeface* SkTypefaceCache::FindByProc(FindProc proc, void* ctx) {
    SkAutoMutexAcquire ama(gMutex);
    return Get().findByProc(proc, ctx);
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
    (void)Get().findByProc(DumpProc, NULL);
#endif
}

