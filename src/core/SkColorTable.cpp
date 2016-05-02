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

void SkColorTable::init(const SkPMColor colors[], int count) {
    SkASSERT((unsigned)count <= 256);

    fCount = count;
    fColors = reinterpret_cast<SkPMColor*>(sk_malloc_throw(count * sizeof(SkPMColor)));

    memcpy(fColors, colors, count * sizeof(SkPMColor));
}

SkColorTable::SkColorTable(const SkPMColor colors[], int count) {
    SkASSERT(0 == count || colors);
    if (count < 0) {
        count = 0;
    } else if (count > 256) {
        count = 256;
    }
    this->init(colors, count);
}

SkColorTable::SkColorTable(SkPMColor* colors, int count, AllocatedWithMalloc)
    : fColors(colors)
    , fCount(count)
{
    SkASSERT(count > 0 && count <= 256);
    SkASSERT(colors);
}

SkColorTable::~SkColorTable() {
    sk_free(fColors);
    sk_free(f16BitCache);
}

#include "SkColorPriv.h"

const uint16_t* SkColorTable::read16BitCache() const {
    f16BitCacheOnce([this] {
        f16BitCache = (uint16_t*)sk_malloc_throw(fCount * sizeof(uint16_t));
        for (int i = 0; i < fCount; i++) {
            f16BitCache[i] = SkPixel32ToPixel16_ToU16(fColors[i]);
        }
    });
    return f16BitCache;
}

///////////////////////////////////////////////////////////////////////////////

#if 0
SkColorTable::SkColorTable(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkReadBuffer::kRemoveColorTableAlpha_Version)) {
        /*fAlphaType = */buffer.readUInt();
    }

    fCount = buffer.getArrayCount();
    size_t allocSize = fCount * sizeof(SkPMColor);
    SkDEBUGCODE(bool success = false;)
    if (buffer.validateAvailable(allocSize)) {
        fColors = (SkPMColor*)sk_malloc_throw(allocSize);
        SkDEBUGCODE(success =) buffer.readColorArray(fColors, fCount);
    } else {
        fCount = 0;
        fColors = nullptr;
    }
#ifdef SK_DEBUG
    SkASSERT((unsigned)fCount <= 256);
    SkASSERT(success);
#endif
}
#endif

void SkColorTable::writeToBuffer(SkWriteBuffer& buffer) const {
    buffer.writeColorArray(fColors, fCount);
}

SkColorTable* SkColorTable::Create(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkReadBuffer::kRemoveColorTableAlpha_Version)) {
        /*fAlphaType = */buffer.readUInt();
    }

    const int count = buffer.getArrayCount();
    if (0 == count) {
        return new SkColorTable(nullptr, 0);
    }

    if (count < 0 || count > 256) {
        buffer.validate(false);
        return nullptr;
    }

    const size_t allocSize = count * sizeof(SkPMColor);
    SkAutoTDelete<SkPMColor> colors((SkPMColor*)sk_malloc_throw(allocSize));
    if (!buffer.readColorArray(colors, count)) {
        return nullptr;
    }

    return new SkColorTable(colors.release(), count, kAllocatedWithMalloc);
}
