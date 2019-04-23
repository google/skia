/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/gradients/GrGradientBitmapCache.h"

#include "include/private/SkFloatBits.h"
#include "include/private/SkHalf.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTemplates.h"

#include <functional>

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
                                         int count, SkColorType colorType, SkBitmap* bitmap) {
    SkHalf* pixelsF16 = reinterpret_cast<SkHalf*>(bitmap->getPixels());
    uint32_t* pixels32 = reinterpret_cast<uint32_t*>(bitmap->getPixels());

    typedef std::function<void(const Sk4f&, int)> pixelWriteFn_t;

    pixelWriteFn_t writeF16Pixel = [&](const Sk4f& x, int index) {
        Sk4h c = SkFloatToHalf_finite_ftz(x);
        pixelsF16[4*index+0] = c[0];
        pixelsF16[4*index+1] = c[1];
        pixelsF16[4*index+2] = c[2];
        pixelsF16[4*index+3] = c[3];
    };
    pixelWriteFn_t write8888Pixel = [&](const Sk4f& c, int index) {
        pixels32[index] = Sk4f_toL32(c);
    };

    pixelWriteFn_t writePixel =
            (colorType == kRGBA_F16_SkColorType) ? writeF16Pixel : write8888Pixel;

    int prevIndex = 0;
    for (int i = 1; i < count; i++) {
        // Historically, stops have been mapped to [0, 256], with 256 then nudged to the next
        // smaller value, then truncate for the texture index. This seems to produce the best
        // results for some common distributions, so we preserve the behavior.
        int nextIndex = SkTMin(positions[i] * fResolution,
                               SkIntToScalar(fResolution - 1));

        if (nextIndex > prevIndex) {
            Sk4f          c0 = Sk4f::Load(colors[i - 1].vec()),
                          c1 = Sk4f::Load(colors[i    ].vec());

            Sk4f step = Sk4f(1.0f / static_cast<float>(nextIndex - prevIndex));
            Sk4f delta = (c1 - c0) * step;

            for (int curIndex = prevIndex; curIndex <= nextIndex; ++curIndex) {
                writePixel(c0, curIndex);
                c0 += delta;
            }
        }
        prevIndex = nextIndex;
    }
    SkASSERT(prevIndex == fResolution - 1);
}

void GrGradientBitmapCache::getGradient(const SkPMColor4f* colors, const SkScalar* positions,
        int count, SkColorType colorType, SkAlphaType alphaType, SkBitmap* bitmap) {
    // build our key: [numColors + colors[] + positions[] + alphaType + colorType ]
    static_assert(sizeof(SkPMColor4f) % sizeof(int32_t) == 0, "");
    const int colorsAsIntCount = count * sizeof(SkPMColor4f) / sizeof(int32_t);
    int keyCount = 1 + colorsAsIntCount + 1 + 1;
    if (count > 2) {
        keyCount += count - 1;
    }

    SkAutoSTMalloc<64, int32_t> storage(keyCount);
    int32_t* buffer = storage.get();

    *buffer++ = count;
    memcpy(buffer, colors, count * sizeof(SkPMColor4f));
    buffer += colorsAsIntCount;
    if (count > 2) {
        for (int i = 1; i < count; i++) {
            *buffer++ = SkFloat2Bits(positions[i]);
        }
    }
    *buffer++ = static_cast<int32_t>(alphaType);
    *buffer++ = static_cast<int32_t>(colorType);
    SkASSERT(buffer - storage.get() == keyCount);

    ///////////////////////////////////

    // acquire lock for checking/adding to cache
    SkAutoExclusive ama(fMutex);
    size_t size = keyCount * sizeof(int32_t);
    if (!this->find(storage.get(), size, bitmap)) {
        SkImageInfo info = SkImageInfo::Make(fResolution, 1, colorType, alphaType);
        bitmap->allocPixels(info);
        GrGradientBitmapCache::fillGradient(colors, positions, count, colorType, bitmap);
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
