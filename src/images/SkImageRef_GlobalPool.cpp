
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRef_GlobalPool.h"
#include "SkImageRefPool.h"
#include "SkThread.h"

SK_DECLARE_STATIC_MUTEX(gGlobalPoolMutex);

/*
 *  This returns the lazily-allocated global pool. It must be called
 *  from inside the guard mutex, so we safely only ever allocate 1.
 */
static SkImageRefPool* GetGlobalPool() {
    static SkImageRefPool* gPool;
    if (NULL == gPool) {
        gPool = SkNEW(SkImageRefPool);
        // call sk_atexit(...) when we have that, to free the global pool
    }
    return gPool;
}

SkImageRef_GlobalPool::SkImageRef_GlobalPool(SkStreamRewindable* stream,
                                             SkBitmap::Config config,
                                             int sampleSize)
        : SkImageRef(stream, config, sampleSize, &gGlobalPoolMutex) {
    SkASSERT(&gGlobalPoolMutex == this->mutex());
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->addToHead(this);
}

SkImageRef_GlobalPool::~SkImageRef_GlobalPool() {
    SkASSERT(&gGlobalPoolMutex == this->mutex());
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->detach(this);
}

/*  By design, onUnlockPixels() already is inside the mutex-lock,
 *  and it is the (indirect) caller of onDecode(), therefore we can assume
 *  that we also are already inside the mutex. Hence, we can reference
 *  the global-pool directly.
 */
bool SkImageRef_GlobalPool::onDecode(SkImageDecoder* codec, SkStreamRewindable* stream,
                                     SkBitmap* bitmap, SkBitmap::Config config,
                                     SkImageDecoder::Mode mode) {
    if (!this->INHERITED::onDecode(codec, stream, bitmap, config, mode)) {
        return false;
    }
    if (mode == SkImageDecoder::kDecodePixels_Mode) {
        // no need to grab the mutex here, it has already been acquired.
        GetGlobalPool()->justAddedPixels(this);
    }
    return true;
}

void SkImageRef_GlobalPool::onUnlockPixels() {
    this->INHERITED::onUnlockPixels();

    // by design, onUnlockPixels() already is inside the mutex-lock
    GetGlobalPool()->canLosePixels(this);
}

SkImageRef_GlobalPool::SkImageRef_GlobalPool(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer, &gGlobalPoolMutex) {
    SkASSERT(&gGlobalPoolMutex == this->mutex());
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->addToHead(this);
}

///////////////////////////////////////////////////////////////////////////////
// global imagerefpool wrappers

size_t SkImageRef_GlobalPool::GetRAMBudget() {
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    return GetGlobalPool()->getRAMBudget();
}

void SkImageRef_GlobalPool::SetRAMBudget(size_t size) {
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->setRAMBudget(size);
}

size_t SkImageRef_GlobalPool::GetRAMUsed() {
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    return GetGlobalPool()->getRAMUsed();
}

void SkImageRef_GlobalPool::SetRAMUsed(size_t usage) {
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->setRAMUsed(usage);
}

void SkImageRef_GlobalPool::DumpPool() {
    SkAutoMutexAcquire ac(gGlobalPoolMutex);
    GetGlobalPool()->dump();
}
