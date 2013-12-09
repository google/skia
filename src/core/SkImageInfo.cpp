/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkFlattenableBuffers.h"

void SkImageInfo::unflatten(SkFlattenableReadBuffer& buffer) {
    fWidth = buffer.read32();
    fHeight = buffer.read32();

    uint32_t packed = buffer.read32();
    SkASSERT(0 == (packed >> 16));
    fAlphaType = (SkAlphaType)((packed >> 8) & 0xFF);
    fColorType = (SkColorType)((packed >> 0) & 0xFF);
}

void SkImageInfo::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkASSERT(0 == (fAlphaType & ~0xFF));
    SkASSERT(0 == (fColorType & ~0xFF));
    uint32_t packed = (fAlphaType << 8) | fColorType;
    buffer.write32(packed);
}
