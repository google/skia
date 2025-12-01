/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"

namespace SkCodecs {

std::unique_ptr<ColorProfile> ColorProfile::MakeICCProfile(
        sk_sp<const SkData> data) {
    if (data) {
        skcms_ICCProfile profile;
        if (skcms_Parse(data->data(), data->size(), &profile)) {
            return std::unique_ptr<ColorProfile>(new ColorProfile(profile, std::move(data)));
        }
    }
    return nullptr;
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

ColorProfile::ColorProfile(const skcms_ICCProfile& profile, sk_sp<const SkData> data)
    : fProfile(profile)
    , fData(std::move(data))
{}

}  // namespace SkCodecs

