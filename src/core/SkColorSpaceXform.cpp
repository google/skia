/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform.h"
#include "SkData.h"
#include "SkMakeUnique.h"
#include "../../third_party/skcms/skcms.h"

bool SkColorSpaceXform::Apply(SkColorSpace* dstCS, ColorFormat dstFormat, void* dst,
                              SkColorSpace* srcCS, ColorFormat srcFormat, const void* src,
                              int len, AlphaOp op) {
    SkAlphaType at;
    switch (op) {
        case kPreserve_AlphaOp:    at = kUnpremul_SkAlphaType; break;
        case kPremul_AlphaOp:      at = kPremul_SkAlphaType;   break;
        case kSrcIsOpaque_AlphaOp: at = kOpaque_SkAlphaType;   break;
    }
    return SkColorSpaceXform::New(srcCS, dstCS)->apply(dstFormat, dst, srcFormat, src, len, at);
}

class SkColorSpaceXform_skcms : public SkColorSpaceXform {
public:
    SkColorSpaceXform_skcms(const skcms_ICCProfile& srcProfile,
                            const skcms_ICCProfile& dstProfile)
        : fSrcProfile(srcProfile)
        , fDstProfile(dstProfile) {
    }

    bool apply(ColorFormat, void*, ColorFormat, const void*, int, SkAlphaType) const override;

private:
    skcms_ICCProfile  fSrcProfile;
    skcms_ICCProfile  fDstProfile;
};

static skcms_PixelFormat get_skcms_format(SkColorSpaceXform::ColorFormat fmt) {
    switch (fmt) {
        case SkColorSpaceXform::kRGBA_8888_ColorFormat:
            return skcms_PixelFormat_RGBA_8888;
        case SkColorSpaceXform::kBGRA_8888_ColorFormat:
            return skcms_PixelFormat_BGRA_8888;
        case SkColorSpaceXform::kRGB_U16_BE_ColorFormat:
            return skcms_PixelFormat_RGB_161616BE;
        case SkColorSpaceXform::kRGBA_U16_BE_ColorFormat:
            return skcms_PixelFormat_RGBA_16161616BE;
        case SkColorSpaceXform::kRGBA_F16_ColorFormat:
            return skcms_PixelFormat_RGBA_hhhh;
        case SkColorSpaceXform::kRGBA_F32_ColorFormat:
            return skcms_PixelFormat_RGBA_ffff;
        case SkColorSpaceXform::kBGR_565_ColorFormat:
            return skcms_PixelFormat_BGR_565;
        default:
            SkDEBUGFAIL("Invalid ColorFormat");
            return skcms_PixelFormat_RGBA_8888;
    }
}

bool SkColorSpaceXform_skcms::apply(ColorFormat dstFormat, void* dst,
                                    ColorFormat srcFormat, const void* src,
                                    int count, SkAlphaType alphaType) const {
    skcms_AlphaFormat srcAlpha = skcms_AlphaFormat_Unpremul;
    skcms_AlphaFormat dstAlpha = kPremul_SkAlphaType == alphaType
        ? skcms_AlphaFormat_PremulAsEncoded
        : skcms_AlphaFormat_Unpremul;

    return skcms_Transform(src, get_skcms_format(srcFormat), srcAlpha, &fSrcProfile,
                           dst, get_skcms_format(dstFormat), dstAlpha, &fDstProfile, count);
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* src, SkColorSpace* dst) {
    if (src && dst && dst->toXYZD50()) {
        // Construct skcms_ICCProfiles from each color space. For now, support A2B and XYZ.
        // Eventually, only need to support XYZ.
        skcms_ICCProfile srcProfile, dstProfile;

        src->toProfile(&srcProfile);
        dst->toProfile(&dstProfile);

        if (!skcms_MakeUsableAsDestination(&dstProfile)) {
            return nullptr;
        }

        return skstd::make_unique<SkColorSpaceXform_skcms>(srcProfile, dstProfile);
    }
    return nullptr;
}
