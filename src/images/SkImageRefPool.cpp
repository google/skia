
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRefPool.h"
#include "SkImageRef.h"
#include "SkThread.h"

SkImageRefPool::SkImageRefPool() {
    fRAMBudget = 0; // means no explicit limit
    fRAMUsed = 0;
    fCount = 0;
    fHead = fTail = NULL;
}

SkImageRefPool::~SkImageRefPool() {
    //    SkASSERT(NULL == fHead);
}

void SkImageRefPool::setRAMBudget(size_t size) {
    if (fRAMBudget != size) {
        fRAMBudget = size;
        this->purgeIfNeeded();
    }
}

void SkImageRefPool::justAddedPixels(SkImageRef* ref) {
#ifdef DUMP_IMAGEREF_LIFECYCLE
    SkDebugf("=== ImagePool: add pixels %s [%d %d %d] bytes=%d heap=%d\n",
             ref->getURI(),
             ref->fBitmap.width(), ref->fBitmap.height(),
             ref->fBitmap.bytesPerPixel(),
             ref->fBitmap.getSize(), (int)fRAMUsed);
#endif
    fRAMUsed += ref->ramUsed();
    this->purgeIfNeeded();
}

void SkImageRefPool::canLosePixels(SkImageRef* ref) {
    // the refs near fHead have recently been released (used)
    // if we purge, we purge from the tail
    this->detach(ref);
    this->addToHead(ref);
    this->purgeIfNeeded();
}

void SkImageRefPool::purgeIfNeeded() {
    // do nothing if we have a zero-budget (i.e. unlimited)
    if (fRAMBudget != 0) {
        this->setRAMUsed(fRAMBudget);
    }
}

void SkImageRefPool::setRAMUsed(size_t limit) {
    SkImageRef* ref = fTail;

    while (NULL != ref && fRAMUsed > limit) {
        // only purge it if its pixels are unlocked
        if (!ref->isLocked() && ref->fBitmap.getPixels()) {
            size_t size = ref->ramUsed();
            SkASSERT(size <= fRAMUsed);
            fRAMUsed -= size;

#ifdef DUMP_IMAGEREF_LIFECYCLE
            SkDebugf("=== ImagePool: purge %s [%d %d %d] bytes=%d heap=%d\n",
                     ref->getURI(),
                     ref->fBitmap.width(), ref->fBitmap.height(),
                     ref->fBitmap.bytesPerPixel(),
                     (int)size, (int)fRAMUsed);
#endif

            // remember the bitmap config (don't call reset),
            // just clear the pixel memory
            ref->fBitmap.setPixels(NULL);
            SkASSERT(NULL == ref->fBitmap.getPixels());
        }
        ref = ref->fPrev;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkImageRefPool::addToHead(SkImageRef* ref) {
    ref->fNext = fHead;
    ref->fPrev = NULL;

    if (fHead) {
        SkASSERT(NULL == fHead->fPrev);
        fHead->fPrev = ref;
    }
    fHead = ref;

    if (NULL == fTail) {
        fTail = ref;
    }
    fCount += 1;
    SkASSERT(computeCount() == fCount);

    fRAMUsed += ref->ramUsed();
}

void SkImageRefPool::addToTail(SkImageRef* ref) {
    ref->fNext = NULL;
    ref->fPrev = fTail;

    if (fTail) {
        SkASSERT(NULL == fTail->fNext);
        fTail->fNext = ref;
    }
    fTail = ref;

    if (NULL == fHead) {
        fHead = ref;
    }
    fCount += 1;
    SkASSERT(computeCount() == fCount);

    fRAMUsed += ref->ramUsed();
}

void SkImageRefPool::detach(SkImageRef* ref) {
    SkASSERT(fCount > 0);

    if (fHead == ref) {
        fHead = ref->fNext;
    }
    if (fTail == ref) {
        fTail = ref->fPrev;
    }
    if (ref->fPrev) {
        ref->fPrev->fNext = ref->fNext;
    }
    if (ref->fNext) {
        ref->fNext->fPrev = ref->fPrev;
    }

    ref->fNext = ref->fPrev = NULL;

    fCount -= 1;
    SkASSERT(computeCount() == fCount);

    SkASSERT(fRAMUsed >= ref->ramUsed());
    fRAMUsed -= ref->ramUsed();
}

int SkImageRefPool::computeCount() const {
    SkImageRef* ref = fHead;
    int count = 0;

    while (ref != NULL) {
        count += 1;
        ref = ref->fNext;
    }

#ifdef SK_DEBUG
    ref = fTail;
    int count2 = 0;

    while (ref != NULL) {
        count2 += 1;
        ref = ref->fPrev;
    }
    SkASSERT(count2 == count);
#endif

    return count;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

void SkImageRefPool::dump() const {
#if defined(SK_DEBUG) || defined(DUMP_IMAGEREF_LIFECYCLE)
    SkDebugf("ImagePool dump: bugdet: %d used: %d count: %d\n",
             (int)fRAMBudget, (int)fRAMUsed, fCount);

    SkImageRef* ref = fHead;

    while (ref != NULL) {
        SkDebugf("  [%3d %3d %d] ram=%d data=%d locked=%d %s\n", ref->fBitmap.width(),
                 ref->fBitmap.height(), ref->fBitmap.config(),
                 ref->ramUsed(), (int)ref->fStream->getLength(),
                 ref->isLocked(), ref->getURI());

        ref = ref->fNext;
    }
#endif
}
