/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkEncodedInfo.h"

#include "src/codec/SkCodecPriv.h"

SkImageInfo SkEncodedInfo::makeImageInfo() const {
    auto ct =  kGray_Color == fColor ? kGray_8_SkColorType   :
             kXAlpha_Color == fColor ? kAlpha_8_SkColorType  :
                k565_Color == fColor ? kRGB_565_SkColorType  :
                                       kN32_SkColorType      ;
    auto alpha = kOpaque_Alpha == fAlpha ? kOpaque_SkAlphaType
                                         : kUnpremul_SkAlphaType;
    auto cs = fColorProfile ? fColorProfile->getExactColorSpace() : SkColorSpace::MakeSRGB();
    if (!cs) {
        cs = SkColorSpace::MakeSRGB();
    }
    return SkImageInfo::Make(fWidth, fHeight, ct, alpha, std::move(cs));
}

// static
SkEncodedInfo SkEncodedInfo::Make(
        int width, int height, Color color, Alpha alpha, int bitsPerComponent) {
    return Make(width, height, color, alpha, bitsPerComponent, nullptr);
}

// static
SkEncodedInfo SkEncodedInfo::Make(
        int width, int height, Color color, Alpha alpha, int bitsPerComponent,
        std::unique_ptr<SkCodecs::ColorProfile> profile) {
    return Make(width, height, color, alpha, /*bitsPerComponent*/ bitsPerComponent,
            std::move(profile), /*colorDepth*/ bitsPerComponent);
}

// static
SkEncodedInfo SkEncodedInfo::Make(
        int width, int height, Color color, Alpha alpha, int bitsPerComponent,
        std::unique_ptr<SkCodecs::ColorProfile> profile, int colorDepth) {
    return Make(width, height, color, alpha, bitsPerComponent, colorDepth, std::move(profile),
                skhdr::Metadata::MakeEmpty());
}

// static
SkEncodedInfo SkEncodedInfo::Make(
        int width, int height, Color color, Alpha alpha, int bitsPerComponent, int colorDepth,
        std::unique_ptr<SkCodecs::ColorProfile> profile, const skhdr::Metadata& hdrMetadata) {
    SkASSERT(1 == bitsPerComponent ||
             2 == bitsPerComponent ||
             4 == bitsPerComponent ||
             8 == bitsPerComponent ||
             16 == bitsPerComponent);
    VerifyColor(color, alpha, bitsPerComponent);
    return SkEncodedInfo(width, height, color, alpha, SkToU8(bitsPerComponent), SkToU8(colorDepth),
                         std::move(profile), hdrMetadata);
}

const skcms_ICCProfile* SkEncodedInfo::profile() const {
    if (!fColorProfile) return nullptr;
    return fColorProfile->profile();
}

sk_sp<const SkData> SkEncodedInfo::profileData() const {
    if (!fColorProfile) return nullptr;
    return fColorProfile->data();
}

SkEncodedInfo SkEncodedInfo::copy() const {
    return SkEncodedInfo(
        fWidth, fHeight, fColor, fAlpha, fBitsPerComponent, fColorDepth,
        fColorProfile ? fColorProfile->clone() : nullptr, fHdrMetadata);
}

SkEncodedInfo::SkEncodedInfo(SkEncodedInfo&& orig) = default;
SkEncodedInfo& SkEncodedInfo::operator=(SkEncodedInfo&&) = default;
SkEncodedInfo::~SkEncodedInfo() = default;

SkEncodedInfo::SkEncodedInfo(
        int width, int height, Color color, Alpha alpha, uint8_t bitsPerComponent,
        uint8_t colorDepth,
        std::unique_ptr<SkCodecs::ColorProfile> profile, const skhdr::Metadata& hdrMetadata)
            : fWidth(width)
            , fHeight(height)
            , fColor(color)
            , fAlpha(alpha)
            , fBitsPerComponent(bitsPerComponent)
            , fColorDepth(colorDepth)
            , fColorProfile(std::move(profile))
            , fHdrMetadata(hdrMetadata) {}

