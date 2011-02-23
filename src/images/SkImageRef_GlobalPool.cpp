#include "SkImageRef_GlobalPool.h"
#include "SkImageRefPool.h"
#include "SkThread.h"

extern SkMutex gImageRefMutex;

static SkImageRefPool gGlobalImageRefPool;

SkImageRef_GlobalPool::SkImageRef_GlobalPool(SkStream* stream,
                                             SkBitmap::Config config,
                                             int sampleSize)
        : SkImageRef(stream, config, sampleSize) {
    this->mutex()->acquire();
    gGlobalImageRefPool.addToHead(this);
    this->mutex()->release();
}

SkImageRef_GlobalPool::~SkImageRef_GlobalPool() {
    this->mutex()->acquire();
    gGlobalImageRefPool.detach(this);
    this->mutex()->release();
}
    
bool SkImageRef_GlobalPool::onDecode(SkImageDecoder* codec, SkStream* stream,
                                     SkBitmap* bitmap, SkBitmap::Config config,
                                     SkImageDecoder::Mode mode) {
    if (!this->INHERITED::onDecode(codec, stream, bitmap, config, mode)) {
        return false;
    }
    if (mode == SkImageDecoder::kDecodePixels_Mode) {
        gGlobalImageRefPool.justAddedPixels(this);
    }
    return true;
}
    
void SkImageRef_GlobalPool::onUnlockPixels() {
    this->INHERITED::onUnlockPixels();
    
    gGlobalImageRefPool.canLosePixels(this);
}

SkImageRef_GlobalPool::SkImageRef_GlobalPool(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    this->mutex()->acquire();
    gGlobalImageRefPool.addToHead(this);
    this->mutex()->release();
}

SkPixelRef* SkImageRef_GlobalPool::Create(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkImageRef_GlobalPool, (buffer));
}

static SkPixelRef::Registrar reg("SkImageRef_GlobalPool",
                                 SkImageRef_GlobalPool::Create);

///////////////////////////////////////////////////////////////////////////////
// global imagerefpool wrappers

size_t SkImageRef_GlobalPool::GetRAMBudget() {
    SkAutoMutexAcquire ac(gImageRefMutex);
    return gGlobalImageRefPool.getRAMBudget();
}

void SkImageRef_GlobalPool::SetRAMBudget(size_t size) {
    SkAutoMutexAcquire ac(gImageRefMutex);
    gGlobalImageRefPool.setRAMBudget(size);
}

size_t SkImageRef_GlobalPool::GetRAMUsed() {
    SkAutoMutexAcquire ac(gImageRefMutex);    
    return gGlobalImageRefPool.getRAMUsed();
}

void SkImageRef_GlobalPool::SetRAMUsed(size_t usage) {
    SkAutoMutexAcquire ac(gImageRefMutex);
    gGlobalImageRefPool.setRAMUsed(usage);
}

void SkImageRef_GlobalPool::DumpPool() {
    SkAutoMutexAcquire ac(gImageRefMutex);
    gGlobalImageRefPool.dump();
}
