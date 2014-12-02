
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

SkColorTable::~SkColorTable() {
    sk_free(fColors);
    // f16BitCache frees itself
}

#include "SkColorPriv.h"

namespace {
struct Build16BitCache {
    const SkPMColor* fColors;
    int fCount;

    uint16_t* operator()() const {
        uint16_t* cache = (uint16_t*)sk_malloc_throw(fCount * sizeof(uint16_t));
        for (int i = 0; i < fCount; i++) {
            cache[i] = SkPixel32ToPixel16_ToU16(fColors[i]);
        }
        return cache;
    }
};
}//namespace

void SkColorTable::Free16BitCache(uint16_t* cache) { sk_free(cache); }

const uint16_t* SkColorTable::read16BitCache() const {
    const Build16BitCache create = { fColors, fCount };
    return f16BitCache.get(create);
}

///////////////////////////////////////////////////////////////////////////////

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
        fColors = NULL;
    }
#ifdef SK_DEBUG
    SkASSERT((unsigned)fCount <= 256);
    SkASSERT(success);
#endif
}

void SkColorTable::writeToBuffer(SkWriteBuffer& buffer) const {
    buffer.writeColorArray(fColors, fCount);
}
