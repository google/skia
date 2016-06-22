/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#ifdef SK_SUPPORT_LEGACY_COLORPROFILETYPE
SkColorProfileType SkImageInfo::profileType() const {
    return fColorSpace && fColorSpace->gammaCloseToSRGB()
            ? kSRGB_SkColorProfileType : kLinear_SkColorProfileType;
}
#endif

static bool alpha_type_is_valid(SkAlphaType alphaType) {
    return (alphaType >= 0) && (alphaType <= kLastEnum_SkAlphaType);
}

static bool color_type_is_valid(SkColorType colorType) {
    return (colorType >= 0) && (colorType <= kLastEnum_SkColorType);
}

SkImageInfo SkImageInfo::MakeS32(int width, int height, SkAlphaType at) {
    return SkImageInfo(width, height, kN32_SkColorType, at,
                       SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named));
}

void SkImageInfo::unflatten(SkReadBuffer& buffer) {
    fWidth = buffer.read32();
    fHeight = buffer.read32();

    uint32_t packed = buffer.read32();
    SkASSERT(0 == (packed >> 24));
    fColorType = (SkColorType)((packed >> 0) & 0xFF);
    fAlphaType = (SkAlphaType)((packed >> 8) & 0xFF);
    buffer.validate(alpha_type_is_valid(fAlphaType) && color_type_is_valid(fColorType));

    sk_sp<SkData> data = buffer.readByteArrayAsData();
    fColorSpace = SkColorSpace::Deserialize(data->data(), data->size());
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkASSERT(0 == (fAlphaType & ~0xFF));
    SkASSERT(0 == (fColorType & ~0xFF));
    uint32_t packed = (fAlphaType << 8) | fColorType;
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
        case kIndex_8_SkColorType:
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
    switch (fInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            return false;
        default:
            break;
    }
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 == fInfo.width() || 0 == fInfo.height()) {
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
