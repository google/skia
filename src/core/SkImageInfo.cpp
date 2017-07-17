/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// These values must be constant over revisions, though they can be renamed to reflect if/when
// they are deprecated.
enum Stored_SkColorType {
    kUnknown_Stored_SkColorType             = 0,
    kAlpha_8_Stored_SkColorType             = 1,
    kRGB_565_Stored_SkColorType             = 2,
    kARGB_4444_Stored_SkColorType           = 3,
    kRGBA_8888_Stored_SkColorType           = 4,
    kBGRA_8888_Stored_SkColorType           = 5,
    kIndex_8_Stored_SkColorType_DEPRECATED  = 6,
    kGray_8_Stored_SkColorType              = 7,
    kRGBA_F16_Stored_SkColorType            = 8,

    kLast_Stored_SkColorType                = kRGBA_F16_Stored_SkColorType,
};

// Index with Stored_SkColorType
const SkColorType gStoredToLive[] = {
    kUnknown_SkColorType,
    kAlpha_8_SkColorType,
    kRGB_565_SkColorType,
    kARGB_4444_SkColorType,
    kRGBA_8888_SkColorType,
    kBGRA_8888_SkColorType,
    kUnknown_SkColorType,       // was kIndex_8
    kGray_8_SkColorType,
    kRGBA_F16_SkColorType,
};

// Index with SkColorType
const Stored_SkColorType gLiveToStored[] = {
    kUnknown_Stored_SkColorType,
    kAlpha_8_Stored_SkColorType,
    kRGB_565_Stored_SkColorType,
    kARGB_4444_Stored_SkColorType,
    kRGBA_8888_Stored_SkColorType,
    kBGRA_8888_Stored_SkColorType,
    kGray_8_Stored_SkColorType,
    kRGBA_F16_Stored_SkColorType,
};

static uint8_t live_to_stored(unsigned ct) {
    static_assert(SK_ARRAY_COUNT(gLiveToStored) == (kLastEnum_SkColorType + 1), "");

    if (ct >= SK_ARRAY_COUNT(gLiveToStored)) {
        ct = kUnknown_SkColorType;
    }
    return gLiveToStored[ct];
}

static SkColorType stored_to_live(unsigned stored) {
    static_assert(SK_ARRAY_COUNT(gStoredToLive) == (kLast_Stored_SkColorType + 1), "");

    if (stored >= SK_ARRAY_COUNT(gStoredToLive)) {
        stored = kUnknown_Stored_SkColorType;
    }
    return gStoredToLive[stored];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool alpha_type_is_valid(SkAlphaType alphaType) {
    return (alphaType >= 0) && (alphaType <= kLastEnum_SkAlphaType);
}

static bool color_type_is_valid(SkColorType colorType) {
    return (colorType >= 0) && (colorType <= kLastEnum_SkColorType);
}

SkImageInfo SkImageInfo::MakeS32(int width, int height, SkAlphaType at) {
    return SkImageInfo(width, height, kN32_SkColorType, at,
                       SkColorSpace::MakeSRGB());
}

static const int kColorTypeMask = 0x0F;
static const int kAlphaTypeMask = 0x03;

void SkImageInfo::unflatten(SkReadBuffer& buffer) {
    fWidth = buffer.read32();
    fHeight = buffer.read32();

    uint32_t packed = buffer.read32();
    fColorType = stored_to_live((packed >> 0) & kColorTypeMask);
    fAlphaType = (SkAlphaType)((packed >> 8) & kAlphaTypeMask);
    buffer.validate(alpha_type_is_valid(fAlphaType) && color_type_is_valid(fColorType));

    sk_sp<SkData> data = buffer.readByteArrayAsData();
    fColorSpace = SkColorSpace::Deserialize(data->data(), data->size());
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkASSERT(0 == (fAlphaType & ~kAlphaTypeMask));
    SkASSERT(0 == (fColorType & ~kColorTypeMask));
    uint32_t packed = (fAlphaType << 8) | live_to_stored(fColorType);
    buffer.write32(packed);

    if (fColorSpace) {
        sk_sp<SkData> data = fColorSpace->serialize();
        if (data) {
            buffer.writeDataAsByteArray(data.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
    } else {
        sk_sp<SkData> data = SkData::MakeEmpty();
        buffer.writeDataAsByteArray(data.get());
    }
}

bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical) {
    switch (colorType) {
        case kUnknown_SkColorType:
            alphaType = kUnknown_SkAlphaType;
            break;
        case kAlpha_8_SkColorType:
            if (kUnpremul_SkAlphaType == alphaType) {
                alphaType = kPremul_SkAlphaType;
            }
            // fall-through
        case kARGB_4444_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            if (kUnknown_SkAlphaType == alphaType) {
                return false;
            }
            break;
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
            alphaType = kOpaque_SkAlphaType;
            break;
        default:
            return false;
    }
    if (canonical) {
        *canonical = alphaType;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkReadPixelsRec.h"

bool SkReadPixelsRec::trim(int srcWidth, int srcHeight) {
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }

    int x = fX;
    int y = fY;
    SkIRect srcR = SkIRect::MakeXYWH(x, y, fInfo.width(), fInfo.height());
    if (!srcR.intersect(0, 0, srcWidth, srcHeight)) {
        return false;
    }

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    fPixels = ((char*)fPixels - y * fRowBytes - x * fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeWH(srcR.width(), srcR.height());
    fX = srcR.x();
    fY = srcR.y();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkWritePixelsRec.h"

bool SkWritePixelsRec::trim(int dstWidth, int dstHeight) {
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }

    int x = fX;
    int y = fY;
    SkIRect dstR = SkIRect::MakeXYWH(x, y, fInfo.width(), fInfo.height());
    if (!dstR.intersect(0, 0, dstWidth, dstHeight)) {
        return false;
    }

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    fPixels = ((const char*)fPixels - y * fRowBytes - x * fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeWH(dstR.width(), dstR.height());
    fX = dstR.x();
    fY = dstR.y();

    return true;
}
