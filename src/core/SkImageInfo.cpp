/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static bool profile_type_is_valid(SkColorProfileType profileType) {
    return (profileType >= 0) && (profileType <= kLastEnum_SkColorProfileType);
}

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
    SkASSERT(0 == (packed >> 24));
    fProfileType = (SkColorProfileType)((packed >> 16) & 0xFF);
    fAlphaType = (SkAlphaType)((packed >> 8) & 0xFF);
    fColorType = (SkColorType)((packed >> 0) & 0xFF);
    buffer.validate(profile_type_is_valid(fProfileType) &&
                    alpha_type_is_valid(fAlphaType) &&
                    color_type_is_valid(fColorType));
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkASSERT(0 == (fProfileType & ~0xFF));
    SkASSERT(0 == (fAlphaType & ~0xFF));
    SkASSERT(0 == (fColorType & ~0xFF));
    uint32_t packed = (fProfileType << 16) | (fAlphaType << 8) | fColorType;
    buffer.write32(packed);
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
    if (NULL == fPixels || fRowBytes < fInfo.minRowBytes()) {
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

