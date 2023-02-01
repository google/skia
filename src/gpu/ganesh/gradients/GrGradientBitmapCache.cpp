/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/ganesh/gradients/GrGradientBitmapCache.h"

#include "include/private/base/SkFloatBits.h"
#include "src/base/SkHalf.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/shaders/gradients/SkGradientShaderBase.h"

#include <functional>

using namespace skia_private;

struct GrGradientBitmapCache::Entry {
    Entry*      fPrev;
    Entry*      fNext;

    void*       fBuffer;
    size_t      fSize;
    SkBitmap    fBitmap;

    Entry(const void* buffer, size_t size, const SkBitmap& bm)
            : fPrev(nullptr),
              fNext(nullptr),
              fBitmap(bm) {
        fBuffer = sk_malloc_throw(size);
        fSize = size;
        memcpy(fBuffer, buffer, size);
    }

    ~Entry() { sk_free(fBuffer); }

    bool equals(const void* buffer, size_t size) const {
        return (fSize == size) && !memcmp(fBuffer, buffer, size);
    }
};

GrGradientBitmapCache::GrGradientBitmapCache(int max, int res)
        : fMaxEntries(max)
        , fResolution(res) {
    fEntryCount = 0;
    fHead = fTail = nullptr;

    this->validate();
}

GrGradientBitmapCache::~GrGradientBitmapCache() {
    this->validate();

    Entry* entry = fHead;
    while (entry) {
        Entry* next = entry->fNext;
        delete entry;
        entry = next;
    }
}

GrGradientBitmapCache::Entry* GrGradientBitmapCache::release(Entry* entry) const {
    if (entry->fPrev) {
        SkASSERT(fHead != entry);
        entry->fPrev->fNext = entry->fNext;
    } else {
        SkASSERT(fHead == entry);
        fHead = entry->fNext;
    }
    if (entry->fNext) {
        SkASSERT(fTail != entry);
        entry->fNext->fPrev = entry->fPrev;
    } else {
        SkASSERT(fTail == entry);
        fTail = entry->fPrev;
    }
    return entry;
}

void GrGradientBitmapCache::attachToHead(Entry* entry) const {
    entry->fPrev = nullptr;
    entry->fNext = fHead;
    if (fHead) {
        fHead->fPrev = entry;
    } else {
        fTail = entry;
    }
    fHead = entry;
}

bool GrGradientBitmapCache::find(const void* buffer, size_t size, SkBitmap* bm) const {
    AutoValidate av(this);

    Entry* entry = fHead;
    while (entry) {
        if (entry->equals(buffer, size)) {
            if (bm) {
                *bm = entry->fBitmap;
            }
            // move to the head of our list, so we purge it last
            this->release(entry);
            this->attachToHead(entry);
            return true;
        }
        entry = entry->fNext;
    }
    return false;
}

void GrGradientBitmapCache::add(const void* buffer, size_t len, const SkBitmap& bm) {
    AutoValidate av(this);

    if (fEntryCount == fMaxEntries) {
        SkASSERT(fTail);
        delete this->release(fTail);
        fEntryCount -= 1;
    }

    Entry* entry = new Entry(buffer, len, bm);
    this->attachToHead(entry);
    fEntryCount += 1;
}

///////////////////////////////////////////////////////////////////////////////

void GrGradientBitmapCache::fillGradient(const SkPMColor4f* colors, const SkScalar* positions,
                                         int count, SkBitmap* bitmap) {
    SkArenaAlloc alloc(/*firstHeapAllocation=*/0);
    SkRasterPipeline p(&alloc);
    SkRasterPipeline_MemoryCtx ctx = { bitmap->getPixels(), 0 };

    p.append(SkRasterPipelineOp::seed_shader);
    p.append_matrix(&alloc, SkMatrix::Scale(1.0f / bitmap->width(), 1.0f));
    SkGradientShaderBase::AppendGradientFillStages(&p, &alloc, colors, positions, count);
    p.append_store(bitmap->colorType(), &ctx);
    p.run(0, 0, bitmap->width(), 1);
}

void GrGradientBitmapCache::getGradient(const SkPMColor4f* colors,
                                        const SkScalar* positions,
                                        int count,
                                        SkColorType colorType,
                                        SkAlphaType alphaType,
                                        SkBitmap* bitmap) {
    // build our key: [numColors + colors[] + positions[] + alphaType + colorType ]
    static_assert(sizeof(SkPMColor4f) % sizeof(int32_t) == 0, "");
    const int colorsAsIntCount = count * sizeof(SkPMColor4f) / sizeof(int32_t);
    SkASSERT(count > 2);  // Otherwise, we should have used the single-interval colorizer
    const int keyCount = 1 +                 // count
                         colorsAsIntCount +  // colors
                         (count - 2) +       // positions
                         1 +                 // alphaType
                         1;                  // colorType

    AutoSTMalloc<64, int32_t> storage(keyCount);
    int32_t* buffer = storage.get();

    *buffer++ = count;
    memcpy(buffer, colors, count * sizeof(SkPMColor4f));
    buffer += colorsAsIntCount;
    for (int i = 1; i < count - 1; i++) {
        *buffer++ = SkFloat2Bits(positions[i]);
    }
    *buffer++ = static_cast<int32_t>(alphaType);
    *buffer++ = static_cast<int32_t>(colorType);
    SkASSERT(buffer - storage.get() == keyCount);

    ///////////////////////////////////

    // acquire lock for checking/adding to cache
    SkAutoMutexExclusive ama(fMutex);
    size_t size = keyCount * sizeof(int32_t);
    if (!this->find(storage.get(), size, bitmap)) {
        SkImageInfo info = SkImageInfo::Make(fResolution, 1, colorType, alphaType);
        bitmap->allocPixels(info);
        this->fillGradient(colors, positions, count, bitmap);
        bitmap->setImmutable();
        this->add(storage.get(), size, *bitmap);
    }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void GrGradientBitmapCache::validate() const {
    SkASSERT(fEntryCount >= 0 && fEntryCount <= fMaxEntries);

    if (fEntryCount > 0) {
        SkASSERT(nullptr == fHead->fPrev);
        SkASSERT(nullptr == fTail->fNext);

        if (fEntryCount == 1) {
            SkASSERT(fHead == fTail);
        } else {
            SkASSERT(fHead != fTail);
        }

        Entry* entry = fHead;
        int count = 0;
        while (entry) {
            count += 1;
            entry = entry->fNext;
        }
        SkASSERT(count == fEntryCount);

        entry = fTail;
        while (entry) {
            count -= 1;
            entry = entry->fPrev;
        }
        SkASSERT(0 == count);
    } else {
        SkASSERT(nullptr == fHead);
        SkASSERT(nullptr == fTail);
    }
}

#endif
