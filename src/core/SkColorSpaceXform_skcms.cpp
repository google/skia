/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform.h"
#include "SkData.h"
#include "SkMakeUnique.h"
#include "skcms.h"

class SkColorSpaceXform_skcms : public SkColorSpaceXform {
public:
    SkColorSpaceXform_skcms(const skcms_ICCProfile& srcProfile,
                            const skcms_ICCProfile& dstProfile,
                            skcms_AlphaFormat premulFormat)
        : fSrcProfile(srcProfile)
        , fDstProfile(dstProfile)
        , fPremulFormat(premulFormat) {
    #ifndef SK_DONT_OPTIMIZE_SRC_PROFILES_FOR_SPEED
        skcms_OptimizeForSpeed(&fSrcProfile);
    #endif
    #ifndef SK_DONT_OPTIMIZE_DST_PROFILES_FOR_SPEED
        // (This doesn't do anything yet, but we'd sure like it to.)
        skcms_OptimizeForSpeed(&fDstProfile);
    #endif
    }

    bool apply(ColorFormat, void*, ColorFormat, const void*, int, SkAlphaType) const override;

private:
    skcms_ICCProfile  fSrcProfile;
    skcms_ICCProfile  fDstProfile;
    skcms_AlphaFormat fPremulFormat;
};

static skcms_PixelFormat get_skcms_format(SkColorSpaceXform::ColorFormat fmt) {
    switch (fmt) {
        case SkColorSpaceXform::kRGBA_8888_ColorFormat:
            return skcms_PixelFormat_RGBA_8888;
        case SkColorSpaceXform::kBGRA_8888_ColorFormat:
            return skcms_PixelFormat_BGRA_8888;
        case SkColorSpaceXform::kRGB_U16_BE_ColorFormat:
            return skcms_PixelFormat_RGB_161616;
        case SkColorSpaceXform::kRGBA_U16_BE_ColorFormat:
            return skcms_PixelFormat_RGBA_16161616;
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
    skcms_AlphaFormat dstAlpha = kPremul_SkAlphaType == alphaType ? fPremulFormat
                                                                  : skcms_AlphaFormat_Unpremul;

    return skcms_Transform(src, get_skcms_format(srcFormat), srcAlpha, &fSrcProfile,
                           dst, get_skcms_format(dstFormat), dstAlpha, &fDstProfile, count);
}

void SkColorSpace::toProfile(skcms_ICCProfile* profile) const {
    if (auto blob = this->onProfileData()) {
        SkAssertResult(skcms_Parse(blob->data(), blob->size(), profile));
    } else {
        SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
        SkColorSpaceTransferFn tf;
        SkAssertResult(this->toXYZD50(&toXYZ) && this->isNumericalTransferFn(&tf));

        skcms_Matrix3x3 m = {{
            { toXYZ.get(0, 0), toXYZ.get(0, 1), toXYZ.get(0, 2) },
            { toXYZ.get(1, 0), toXYZ.get(1, 1), toXYZ.get(1, 2) },
            { toXYZ.get(2, 0), toXYZ.get(2, 1), toXYZ.get(2, 2) },
        }};

        skcms_Init(profile);
        skcms_SetTransferFunction(profile, (const skcms_TransferFunction*)&tf);
        skcms_SetXYZD50(profile, &m);
    }
}

std::unique_ptr<SkColorSpaceXform> MakeSkcmsXform(SkColorSpace* src, SkColorSpace* dst,
                                                  SkTransferFunctionBehavior premulBehavior) {
    // Construct skcms_ICCProfiles from each color space. For now, support A2B and XYZ.
    // Eventually, only need to support XYZ. Map premulBehavior to one of the two premul formats
    // in skcms.
    skcms_ICCProfile srcProfile, dstProfile;

    src->toProfile(&srcProfile);
    dst->toProfile(&dstProfile);

    if (!skcms_MakeUsableAsDestination(&dstProfile)) {
        return nullptr;
    }

    skcms_AlphaFormat premulFormat = SkTransferFunctionBehavior::kRespect == premulBehavior
            ? skcms_AlphaFormat_PremulLinear : skcms_AlphaFormat_PremulAsEncoded;
    return skstd::make_unique<SkColorSpaceXform_skcms>(srcProfile, dstProfile, premulFormat);
}

sk_sp<SkColorSpace> SkColorSpace::Make(const skcms_ICCProfile& profile) {
    if (!profile.has_toXYZD50 || !profile.has_trc) {
        return nullptr;
    }

    if (skcms_ApproximatelyEqualProfiles(&profile, skcms_sRGB_profile())) {
        return SkColorSpace::MakeSRGB();
    }

    SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
    toXYZD50.set3x3RowMajorf(&profile.toXYZD50.vals[0][0]);
    if (!toXYZD50.invert(nullptr)) {
        return nullptr;
    }

    const skcms_Curve* trc = profile.trc;
    if (trc[0].table_entries ||
        trc[1].table_entries ||
        trc[2].table_entries ||
        memcmp(&trc[0].parametric, &trc[1].parametric, sizeof(trc[0].parametric)) ||
        memcmp(&trc[0].parametric, &trc[2].parametric, sizeof(trc[0].parametric))) {
        return nullptr;
    }

    SkColorSpaceTransferFn skia_tf;
    memcpy(&skia_tf, &profile.trc[0].parametric, sizeof(skia_tf));

    return SkColorSpace::MakeRGB(skia_tf, toXYZD50);
}

bool skcms_can_parse(const void* buf, size_t len) {
    skcms_ICCProfile p;
    return skcms_Parse(buf, len, &p);
}
