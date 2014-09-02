/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static bool color_type_supports_sRGB(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        default:
            return false;
    }
}

static bool color_type_supports_gamma(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        // case kLuminance ...
            return true;
        default:
            return false;
    }
}

static float pin_gamma_to_legal(float gamma) {
    if (!SkScalarIsFinite(gamma)) {
        return 1;
    }
    // these limits are just made up -- feel free to change them within reason
    const float min_gamma = 0.01f;
    const float max_gamma = 4.0;
    return SkScalarPin(gamma, min_gamma, max_gamma);
}

SkImageInfo SkImageInfo::MakeSRGB(int width, int height, SkColorType ct, SkAlphaType at) {
    Profile p = color_type_supports_sRGB(ct) ? kSRGB_Profile : kUnknown_Profile;
    return SkImageInfo(width, height, ct, at, p, 0);
}

SkImageInfo SkImageInfo::MakeWithGamma(int width, int height, SkColorType ct, SkAlphaType at,
                                       float gamma) {
    Profile p;
    if (color_type_supports_gamma(ct)) {
        gamma = pin_gamma_to_legal(gamma);
        p = kExponential_Profile;
    } else {
        p = kUnknown_Profile;
        gamma = 0;
    }
    return SkImageInfo(width, height, ct, at, p, gamma);
}

bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical) {
    switch (colorType) {
        case kUnknown_SkColorType:
            alphaType = kIgnore_SkAlphaType;
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
            if (kIgnore_SkAlphaType == alphaType) {
                return false;
            }
            break;
        case kRGB_565_SkColorType:
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

void SkImageInfo::unflatten(SkReadBuffer& buffer) {
    *this = Unflatten(buffer);
}

////////////////////////////////////////////////////////////////////////////////////////////

static bool alpha_type_is_valid(SkAlphaType alphaType) {
    return (alphaType >= 0) && (alphaType <= kLastEnum_SkAlphaType);
}

static bool color_type_is_valid(SkColorType colorType) {
    return (colorType >= 0) && (colorType <= kLastEnum_SkColorType);
}

static float igamma_to_gamma(int gamma3dot9) {
    return gamma3dot9 / 512.0f;
}

static unsigned gamma_to_igamma(float gamma) {
    SkASSERT(gamma >= 0 && gamma < 8);
    int igamma = SkScalarRoundToInt(gamma * 512);
    SkASSERT(igamma >= 0 && igamma <= 0xFFF);
    return igamma;
}

SkImageInfo SkImageInfo::Unflatten(SkReadBuffer& buffer) {
    int width = buffer.read32();
    int height = buffer.read32();
    uint32_t packed = buffer.read32();
    
    SkColorType ct = (SkColorType)((packed >> 0) & 0xFF);   // 8 bits for colortype
    SkAlphaType at = (SkAlphaType)((packed >> 8) & 0xFF);   // 8 bits for alphatype
    if (!alpha_type_is_valid(at) || !color_type_is_valid(ct)) {
        return MakeUnknown();
    }

    // Earlier formats always stored 0 in the upper 16 bits. That corresponds to
    // days before we had gamma/profile. That happens to correspond to kUnknown_Profile,
    // which means we can just ignore the gamma value anyways.
    //
    int iprofile = ((packed >> 16) & 0xF);  // 4 bits for profile
    
    switch (iprofile) {
        case kUnknown_Profile:
            return Make(width, height, ct, at);
        case kSRGB_Profile:
            return MakeSRGB(width, height, ct, at);
        case kExponential_Profile: {
            int igamma = packed >> 20;      // 12 bits for gamma 3.9
            float gamma = igamma_to_gamma(igamma);
            return MakeWithGamma(width, height, ct, at, gamma);
        }
        default:
            (void)buffer.validate(false);
            return MakeUnknown();
    }
}

void SkImageInfo::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);
    
    SkASSERT(0 == (fColorType & ~0xFF));    // 8 bits for colortype
    SkASSERT(0 == (fAlphaType & ~0xFF));    // 8 bits for alphatype
    SkASSERT(0 == (fProfile & ~0xF));       // 4 bits for profile
    int igamma = gamma_to_igamma(fGamma);   // 12 bits for gamma (if needed)
 
    uint32_t packed = (igamma << 20) | (fProfile << 16) | (fAlphaType << 8) | fColorType;
    buffer.write32(packed);
}
