/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static bool alpha_type_is_valid(SkAlphaType alphaType) {
    return (alphaType >= 0) && (alphaType <= kLastEnum_SkAlphaType);
}

static bool color_type_is_valid(SkColorType colorType) {
    return (colorType >= 0) && (colorType <= kLastEnum_SkColorType);
}

void SkImageInfo::unflatten(SkReadBuffer& buffer) {
    fWidth = buffer.read32();
    fHeight = buffer.read32();

    uint32_t packed = buffer.read32();
    SkASSERT(0 == (packed >> 16));
    fAlphaType = (SkAlphaType)((packed >> 8) & 0xFF);
    fColorType = (SkColorType)((packed >> 0) & 0xFF);
    buffer.validate(alpha_type_is_valid(fAlphaType) &&
                    color_type_is_valid(fColorType));
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkASSERT(0 == (fAlphaType & ~0xFF));
    SkASSERT(0 == (fColorType & ~0xFF));
    uint32_t packed = (fAlphaType << 8) | fColorType;
    buffer.write32(packed);
}
