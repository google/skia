/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkImageInfoPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

/*
 *  We store this as a byte in the ImageInfo flatten buffer.
 */
enum class SkFlattenColorSpaceEnum {
    kUnspecified,
    kSRGB,
    kAdobe1998,
    // ... add more here
    kLastEnum = kAdobe1998,
    // final value means the actual profile data follows the info
    kICCProfile = 0xFF,
};

static sk_sp<SkColorSpace> make_from_enum(SkFlattenColorSpaceEnum value) {
    switch (value) {
        case SkFlattenColorSpaceEnum::kSRGB:
            return SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
        case SkFlattenColorSpaceEnum::kAdobe1998:
            return SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named);
        default:
            return nullptr;
    }
}

SkColorSpace::Named sk_deduce_named_from_colorspace(SkColorSpace* cs) {
    return cs->fNamed;
}

static SkFlattenColorSpaceEnum deduce_from_colorspace(SkColorSpace* cs) {
    if (!cs) {
        return SkFlattenColorSpaceEnum::kUnspecified;
    }
    switch (sk_deduce_named_from_colorspace(cs)) {
        case SkColorSpace::kSRGB_Named:
            return SkFlattenColorSpaceEnum::kSRGB;
        case SkColorSpace::kAdobeRGB_Named:
            return SkFlattenColorSpaceEnum::kAdobe1998;
        default:
            return SkFlattenColorSpaceEnum::kICCProfile;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_COLORPROFILETYPE
SkColorProfileType SkImageInfo::profileType() const {
    return fColorSpace && fColorSpace->gammaCloseToSRGB()
            ? kSRGB_SkColorProfileType : kLinear_SkColorProfileType;
}
#endif

// Indicate how images and gradients should interpret colors by default.
bool gDefaultProfileIsSRGB;

SkColorProfileType SkDefaultColorProfile() {
    return gDefaultProfileIsSRGB ? kSRGB_SkColorProfileType
                                 : kLinear_SkColorProfileType;
}

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
    SkFlattenColorSpaceEnum csenum = (SkFlattenColorSpaceEnum)((packed >> 16) & 0xFF);
    buffer.validate(alpha_type_is_valid(fAlphaType) && color_type_is_valid(fColorType));

    if (SkFlattenColorSpaceEnum::kICCProfile == csenum) {
        SkASSERT(false);    // we shouldn't hit this yet, as we don't write these yet
        fColorSpace.reset();
    } else {
        if (csenum > SkFlattenColorSpaceEnum::kLastEnum) {
            csenum = SkFlattenColorSpaceEnum::kUnspecified;
        }
        fColorSpace = make_from_enum(csenum);
    }
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);

    SkFlattenColorSpaceEnum csenum = deduce_from_colorspace(fColorSpace.get());

    // TODO: when we actually support flattening the colorspace to a profile blob, remove this
    //       hack (and write the blob after we write packed.
    if (SkFlattenColorSpaceEnum::kICCProfile == csenum) {
        csenum = SkFlattenColorSpaceEnum::kUnspecified;
    }

    SkASSERT(0 == ((int)csenum & ~0xFF));
    SkASSERT(0 == (fAlphaType & ~0xFF));
    SkASSERT(0 == (fColorType & ~0xFF));
    uint32_t packed = ((int)csenum << 16) | (fAlphaType << 8) | fColorType;
    buffer.write32(packed);

    if (SkFlattenColorSpaceEnum::kICCProfile == csenum) {
        // TODO: write the ICCProfile blob
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
