/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorTable.h"

#include "include/core/SkImageInfo.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkColorTable> SkColorTable::Make(const uint8_t tableA[256],
                                       const uint8_t tableR[256],
                                       const uint8_t tableG[256],
                                       const uint8_t tableB[256]) {
    if (!tableA && !tableR && !tableG && !tableB) {
        return nullptr; // The table is the identity
    }

    SkBitmap table;
    if (!table.tryAllocPixels(SkImageInfo::MakeA8(256, 4))) {
        return nullptr;
    }
    uint8_t *a = table.getAddr8(0,0),
            *r = table.getAddr8(0,1),
            *g = table.getAddr8(0,2),
            *b = table.getAddr8(0,3);
    for (int i = 0; i < 256; i++) {
        a[i] = tableA ? tableA[i] : i;
        r[i] = tableR ? tableR[i] : i;
        g[i] = tableG ? tableG[i] : i;
        b[i] = tableB ? tableB[i] : i;
    }
    table.setImmutable();

    return sk_sp<SkColorTable>(new SkColorTable(table));
}

void SkColorTable::flatten(SkWriteBuffer& buffer) const {
    buffer.writeByteArray(fTable.getAddr8(0, 0), 4 * 256);
}

sk_sp<SkColorTable> SkColorTable::Deserialize(SkReadBuffer& buffer) {
    uint8_t argb[4*256];
    if (buffer.readByteArray(argb, sizeof(argb))) {
        return SkColorTable::Make(argb+0*256, argb+1*256, argb+2*256, argb+3*256);
    }
    return nullptr;
}
