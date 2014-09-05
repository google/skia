
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorTable.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkStream.h"
#include "SkTemplates.h"

// As copy constructor is hidden in the class hierarchy, we need to call
// default constructor explicitly to suppress a compiler warning.
SkColorTable::SkColorTable(const SkColorTable& src) : INHERITED() {
    f16BitCache = NULL;
    fAlphaType = src.fAlphaType;
    int count = src.count();
    fCount = SkToU16(count);
    fColors = reinterpret_cast<SkPMColor*>(
                                    sk_malloc_throw(count * sizeof(SkPMColor)));
    memcpy(fColors, src.fColors, count * sizeof(SkPMColor));

    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

SkColorTable::SkColorTable(const SkPMColor colors[], int count, SkAlphaType at)
    : f16BitCache(NULL), fAlphaType(SkToU8(at))
{
    SkASSERT(0 == count || colors);

    if (count < 0) {
        count = 0;
    } else if (count > 256) {
        count = 256;
    }

    fCount = SkToU16(count);
    fColors = reinterpret_cast<SkPMColor*>(
                                    sk_malloc_throw(count * sizeof(SkPMColor)));

    memcpy(fColors, colors, count * sizeof(SkPMColor));

    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

SkColorTable::~SkColorTable()
{
    SkASSERT(fColorLockCount == 0);
    SkASSERT(f16BitCacheLockCount == 0);

    sk_free(fColors);
    sk_free(f16BitCache);
}

void SkColorTable::unlockColors() {
    SkASSERT(fColorLockCount != 0);
    SkDEBUGCODE(sk_atomic_dec(&fColorLockCount);)
}

#include "SkColorPriv.h"

static inline void build_16bitcache(uint16_t dst[], const SkPMColor src[],
                                    int count) {
    while (--count >= 0) {
        *dst++ = SkPixel32ToPixel16_ToU16(*src++);
    }
}

const uint16_t* SkColorTable::lock16BitCache() {
    if (this->isOpaque() && NULL == f16BitCache) {
        f16BitCache = (uint16_t*)sk_malloc_throw(fCount * sizeof(uint16_t));
        build_16bitcache(f16BitCache, fColors, fCount);
    }

    SkDEBUGCODE(f16BitCacheLockCount += 1);
    return f16BitCache;
}

///////////////////////////////////////////////////////////////////////////////

SkColorTable::SkColorTable(SkReadBuffer& buffer) {
    f16BitCache = NULL;
    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)

    fAlphaType = SkToU8(buffer.readUInt());
    fCount = buffer.getArrayCount();
    size_t allocSize = fCount * sizeof(SkPMColor);
    SkDEBUGCODE(bool success = false;)
    if (buffer.validateAvailable(allocSize)) {
        fColors = (SkPMColor*)sk_malloc_throw(allocSize);
        SkDEBUGCODE(success =) buffer.readColorArray(fColors, fCount);
    } else {
        fCount = 0;
        fColors = NULL;
    }
#ifdef SK_DEBUG
    SkASSERT((unsigned)fCount <= 256);
    SkASSERT(success);
#endif
}

void SkColorTable::writeToBuffer(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fAlphaType);
    buffer.writeColorArray(fColors, fCount);
}
