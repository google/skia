
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRef_GlobalPool.h"
#include "SkImageRefPool.h"
#include "SkThread.h"

extern SkBaseMutex gImageRefMutex;

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

SkImageRef_GlobalPool::SkImageRef_GlobalPool(SkStream* stream,
                                             SkBitmap::Config config,
                                             int sampleSize)
        : SkImageRef(stream, config, sampleSize) {
    this->mutex()->acquire();
    GetGlobalPool()->addToHead(this);
    this->mutex()->release();
}

SkImageRef_GlobalPool::~SkImageRef_GlobalPool() {
    this->mutex()->acquire();
    GetGlobalPool()->detach(this);
    this->mutex()->release();
}

/*  By design, onUnlockPixels() already is inside the mutex-lock,
 *  and it is the (indirect) caller of onDecode(), therefore we can assume
 *  that we also are already inside the mutex. Hence, we can reference
 *  the global-pool directly.
 */
bool SkImageRef_GlobalPool::onDecode(SkImageDecoder* codec, SkStream* stream,
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
        : INHERITED(buffer) {
    this->mutex()->acquire();
    GetGlobalPool()->addToHead(this);
    this->mutex()->release();
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkImageRef_GlobalPool)

///////////////////////////////////////////////////////////////////////////////
// global imagerefpool wrappers

size_t SkImageRef_GlobalPool::GetRAMBudget() {
    SkAutoMutexAcquire ac(gImageRefMutex);
    return GetGlobalPool()->getRAMBudget();
}

void SkImageRef_GlobalPool::SetRAMBudget(size_t size) {
    SkAutoMutexAcquire ac(gImageRefMutex);
    GetGlobalPool()->setRAMBudget(size);
}

size_t SkImageRef_GlobalPool::GetRAMUsed() {
    SkAutoMutexAcquire ac(gImageRefMutex);
    return GetGlobalPool()->getRAMUsed();
}

void SkImageRef_GlobalPool::SetRAMUsed(size_t usage) {
    SkAutoMutexAcquire ac(gImageRefMutex);
    GetGlobalPool()->setRAMUsed(usage);
}

void SkImageRef_GlobalPool::DumpPool() {
    SkAutoMutexAcquire ac(gImageRefMutex);
    GetGlobalPool()->dump();
}
