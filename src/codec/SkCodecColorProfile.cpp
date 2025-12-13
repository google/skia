/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkColorSpacePriv.h"

#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
#include "src/codec/SkCodecColorProfileRust.h"
#endif

namespace SkCodecs {

namespace {

sk_sp<SkColorSpace> cicp_get_android_sk_color_space(uint8_t color_primaries,
                                                    uint8_t transfer_characteristics,
                                                    uint8_t matrix_coefficients,
                                                    uint8_t full_range_flag) {
    if (matrix_coefficients != 0) {
        return nullptr;
    }
    if (full_range_flag != 1) {
        return nullptr;
    }
    skcms_Matrix3x3 primaries;
    if (!SkNamedPrimaries::GetCicp(
            static_cast<SkNamedPrimaries::CicpId>(color_primaries), primaries)) {
        return nullptr;
    }
    skcms_TransferFunction trfn;
    if (!SkNamedTransferFn::GetCicp(
            static_cast<SkNamedTransferFn::CicpId>(transfer_characteristics), trfn)) {
        return nullptr;
    }
    switch (transfer_characteristics) {
        case 16:
            // Android expects PQ to match 203 nits to SDR white
            trfn = {-2.f, -1.55522297832f, 1.86045365631f, 32 / 2523.0f,
                    2413 / 128.0f, -2392 / 128.0f, 8192 / 1305.0f};
            break;
        case 18:
            // Android expects HLG to match 203 nits to SDR white
            skcms_TransferFunction_makeScaledHLGish(
                &trfn, 0.314509843f, 2.f, 2.f, 1.f / 0.17883277f, 0.28466892f, 0.55991073f);
            break;
        default:
            break;
    }
    return SkColorSpace::MakeRGB(trfn, primaries);
}

}  // namespace

std::unique_ptr<ColorProfile> ColorProfile::MakeICCProfileWithSkCMS(
        sk_sp<const SkData> data) {
    if (data) {
        skcms_ICCProfile profile;
        if (skcms_Parse(data->data(), data->size(), &profile)) {
            return std::unique_ptr<ColorProfile>(new ColorProfile(profile, std::move(data)));
        }
    }
    return nullptr;
}

std::unique_ptr<ColorProfile> ColorProfile::MakeICCProfile(
        sk_sp<const SkData> data) {
#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
    return MakeICCProfileWithRust(data);
#else
    return MakeICCProfileWithSkCMS(data);
#endif
}

std::unique_ptr<ColorProfile> ColorProfile::Make(sk_sp<SkColorSpace> cs) {
    if (!cs) {
        return nullptr;
    }
    skcms_ICCProfile profile;
    cs->toProfile(&profile);
    return std::unique_ptr<ColorProfile>(new ColorProfile(profile));
}

std::unique_ptr<ColorProfile> ColorProfile::Make(
        const skcms_TransferFunction& trfn,
        const skcms_Matrix3x3& toXYZD50) {
    skcms_ICCProfile profile;
    skcms_Init(&profile);
    skcms_SetTransferFunction(&profile, &trfn);
    skcms_SetXYZD50(&profile, &toXYZD50);
    return std::unique_ptr<ColorProfile>(new ColorProfile(profile));
}

std::unique_ptr<ColorProfile> ColorProfile::MakeCICP(
        uint8_t color_primaries,
        uint8_t transfer_characteristics,
        uint8_t matrix_coefficients,
        uint8_t full_range_flag) {
    skcms_ICCProfile profile;
    skcms_Init(&profile);
    profile.has_CICP = true;
    profile.CICP.color_primaries = color_primaries;
    profile.CICP.transfer_characteristics = transfer_characteristics;
    profile.CICP.matrix_coefficients = matrix_coefficients;
    profile.CICP.video_full_range_flag = full_range_flag;
    return std::unique_ptr<ColorProfile>(new ColorProfile(profile));
}

std::unique_ptr<ColorProfile> ColorProfile::clone() const {
    return std::unique_ptr<ColorProfile>(new ColorProfile(fProfile, fData));
}

ColorProfile::DataSpace ColorProfile::dataSpace() const {
    switch (fProfile.data_color_space) {
        case skcms_Signature_RGB:
            return DataSpace::kRGB;
        case skcms_Signature_CMYK:
            return DataSpace::kCMYK;
        case skcms_Signature_Gray:
            return DataSpace::kGray;
        default:
            return DataSpace::kOther;
    }
}

sk_sp<SkColorSpace> ColorProfile::getExactColorSpace() const {
    return SkColorSpace::Make(fProfile);
}

sk_sp<SkColorSpace> ColorProfile::getAndroidOutputColorSpace() const {
    // Prefer CICP information if it exists.
    if (fProfile.has_CICP) {
        const auto cicpColorSpace = cicp_get_android_sk_color_space(
                fProfile.CICP.color_primaries,
                fProfile.CICP.transfer_characteristics,
                fProfile.CICP.matrix_coefficients,
                fProfile.CICP.video_full_range_flag);
        if (cicpColorSpace) {
            return cicpColorSpace;
        }
    }
    if (auto encodedSpace = SkColorSpace::Make(fProfile)) {
        // Leave the pixels in the encoded color space.  Color space conversion
        // will be handled after decode time.
        return encodedSpace;
    }
    if (fProfile.has_toXYZD50) {
        return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                     fProfile.toXYZD50);
    }
    return SkColorSpace::MakeSRGB();
}

ColorProfile::ColorProfile(const skcms_ICCProfile& profile, sk_sp<const SkData> data)
    : fProfile(profile)
    , fData(std::move(data))
{}

}  // namespace SkCodecs

