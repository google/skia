/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>

// packed int at the beginning of the serialized font:
//
//  control_bits:8 size_as_byte:8 flags:12 edging:2 hinting:2

enum {
    kSize_Is_Byte_Bit   = 1 << 31,
    kHas_ScaleX_Bit     = 1 << 30,
    kHas_SkewX_Bit      = 1 << 29,
    kHas_Typeface_Bit   = 1 << 28,

    kShift_for_Size     = 16,
    kMask_For_Size      = 0xFF,

    kShift_For_Flags    = 4,
    kMask_For_Flags     = 0xFFF,

    kShift_For_Edging   = 2,
    kMask_For_Edging    = 0x3,

    kShift_For_Hinting  = 0,
    kMask_For_Hinting   = 0x3
};

static bool scalar_is_byte(SkScalar x) {
    int ix = (int)x;
    return ix == x && ix >= 0 && ix <= kMask_For_Size;
}

void SkFontPriv::Flatten(const SkFont& font, SkWriteBuffer& buffer) {
    SkASSERT(font.fFlags <= SkFont::kAllFlags);
    SkASSERT((font.fFlags & ~kMask_For_Flags) == 0);
    SkASSERT((font.fEdging & ~kMask_For_Edging) == 0);
    SkASSERT((font.fHinting & ~kMask_For_Hinting) == 0);

    uint32_t packed = 0;
    packed |= font.fFlags << kShift_For_Flags;
    packed |= font.fEdging << kShift_For_Edging;
    packed |= font.fHinting << kShift_For_Hinting;

    if (scalar_is_byte(font.fSize)) {
        packed |= kSize_Is_Byte_Bit;
        packed |= (int)font.fSize << kShift_for_Size;
    }
    if (font.fScaleX != 1) {
        packed |= kHas_ScaleX_Bit;
    }
    if (font.fSkewX != 0) {
        packed |= kHas_SkewX_Bit;
    }
    if (font.fTypeface) {
        packed |= kHas_Typeface_Bit;
    }

    buffer.write32(packed);
    if (!(packed & kSize_Is_Byte_Bit)) {
        buffer.writeScalar(font.fSize);
    }
    if (packed & kHas_ScaleX_Bit) {
        buffer.writeScalar(font.fScaleX);
    }
    if (packed & kHas_SkewX_Bit) {
        buffer.writeScalar(font.fSkewX);
    }
    if (packed & kHas_Typeface_Bit) {
        buffer.writeTypeface(font.fTypeface.get());
    }
}

bool SkFontPriv::Unflatten(SkFont* font, SkReadBuffer& buffer) {
    const uint32_t packed = buffer.read32();

    if (packed & kSize_Is_Byte_Bit) {
        font->fSize = (packed >> kShift_for_Size) & kMask_For_Size;
    } else {
        font->fSize = buffer.readScalar();
    }
    if (packed & kHas_ScaleX_Bit) {
        font->fScaleX = buffer.readScalar();
    }
    if (packed & kHas_SkewX_Bit) {
        font->fSkewX = buffer.readScalar();
    }
    if (packed & kHas_Typeface_Bit) {
        font->fTypeface = buffer.readTypeface();
    }

    SkASSERT(SkFont::kAllFlags <= kMask_For_Flags);
    // we & with kAllFlags, to clear out any unknown flag bits
    font->fFlags = SkToU8((packed >> kShift_For_Flags) & SkFont::kAllFlags);

    unsigned edging = (packed >> kShift_For_Edging) & kMask_For_Edging;
    if (edging > (unsigned)SkFont::Edging::kSubpixelAntiAlias) {
        edging = 0;
    }
    font->fEdging = SkToU8(edging);

    unsigned hinting = (packed >> kShift_For_Hinting) & kMask_For_Hinting;
    if (hinting > (unsigned)SkFontHinting::kFull) {
        hinting = 0;
    }
    font->fHinting = SkToU8(hinting);

    return buffer.isValid();
}
