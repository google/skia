/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkWriteBuffer.h"

int SkColorTypeBytesPerPixel(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return 0;
        case kAlpha_8_SkColorType:            return 1;
        case kRGB_565_SkColorType:            return 2;
        case kARGB_4444_SkColorType:          return 2;
        case kRGBA_8888_SkColorType:          return 4;
        case kBGRA_8888_SkColorType:          return 4;
        case kRGB_888x_SkColorType:           return 4;
        case kRGBA_1010102_SkColorType:       return 4;
        case kRGB_101010x_SkColorType:        return 4;
        case kBGRA_1010102_SkColorType:       return 4;
        case kBGR_101010x_SkColorType:        return 4;
        case kGray_8_SkColorType:             return 1;
        case kRGBA_F16Norm_SkColorType:       return 8;
        case kRGBA_F16_SkColorType:           return 8;
        case kRGBA_F32_SkColorType:           return 16;
        case kR8G8_unorm_SkColorType:         return 2;
        case kA16_unorm_SkColorType:          return 2;
        case kR16G16_unorm_SkColorType:       return 4;
        case kA16_float_SkColorType:          return 2;
        case kR16G16_float_SkColorType:       return 4;
        case kR16G16B16A16_unorm_SkColorType: return 8;
    }
    SkUNREACHABLE;
}

bool SkColorTypeIsAlwaysOpaque(SkColorType ct) {
    return !(SkColorTypeChannelFlags(ct) & kAlpha_SkColorChannelFlag);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int SkColorInfo::bytesPerPixel() const { return SkColorTypeBytesPerPixel(fColorType); }

int SkColorInfo::shiftPerPixel() const { return SkColorTypeShiftPerPixel(fColorType); }

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkImageInfo::computeOffset(int x, int y, size_t rowBytes) const {
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());
    return SkColorTypeComputeOffset(this->colorType(), x, y, rowBytes);
}

size_t SkImageInfo::computeByteSize(size_t rowBytes) const {
    if (0 == this->height()) {
        return 0;
    }
    SkSafeMath safe;
    size_t bytes = safe.add(safe.mul(safe.addInt(this->height(), -1), rowBytes),
                            safe.mul(this->width(), this->bytesPerPixel()));
    return safe.ok() ? bytes : SIZE_MAX;
}

SkImageInfo SkImageInfo::MakeS32(int width, int height, SkAlphaType at) {
    return SkImageInfo({width, height}, {kN32_SkColorType, at, SkColorSpace::MakeSRGB()});
}

#ifdef SK_DEBUG
void SkImageInfo::validate() const {
    SkASSERT(fDimensions.width() >= 0);
    SkASSERT(fDimensions.height() >= 0);
    SkASSERT(SkColorTypeIsValid(this->colorType()));
    SkASSERT(SkAlphaTypeIsValid(this->alphaType()));
}
#endif

bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical) {
    switch (colorType) {
        case kUnknown_SkColorType:
            alphaType = kUnknown_SkAlphaType;
            break;
        case kAlpha_8_SkColorType:         // fall-through
        case kA16_unorm_SkColorType:       // fall-through
        case kA16_float_SkColorType:
            if (kUnpremul_SkAlphaType == alphaType) {
                alphaType = kPremul_SkAlphaType;
            }
            // fall-through
        case kARGB_4444_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
        case kR16G16B16A16_unorm_SkColorType:
            if (kUnknown_SkAlphaType == alphaType) {
                return false;
            }
            break;
        case kGray_8_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kRGB_565_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
            alphaType = kOpaque_SkAlphaType;
            break;
    }
    if (canonical) {
        *canonical = alphaType;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/image/SkReadPixelsRec.h"

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
    if (!srcR.intersect({0, 0, srcWidth, srcHeight})) {
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
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    fPixels = ((char*)fPixels + -y*fRowBytes + -x*fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeDimensions(srcR.size());
    fX = srcR.x();
    fY = srcR.y();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkWritePixelsRec.h"

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
    if (!dstR.intersect({0, 0, dstWidth, dstHeight})) {
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
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    fPixels = ((const char*)fPixels + -y*fRowBytes + -x*fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeDimensions(dstR.size());
    fX = dstR.x();
    fY = dstR.y();

    return true;
}
