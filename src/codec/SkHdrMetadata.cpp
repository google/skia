/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "src/core/SkStreamPriv.h"

namespace skhdr {

SkString ContentLightLevelInformation::toString() const {
    return SkStringPrintf("{maxCLL:%f, maxFALL:%f}", fMaxCLL, fMaxFALL);
}

bool ContentLightLevelInformation::parse(const SkData* data) {
    if (data->size() != 4) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint16_t max_cll = 0;
    uint16_t max_fall = 0;
    if (!SkStreamPriv::ReadU16BE(&s, &max_cll)) {
        return false;
    }
    if (!SkStreamPriv::ReadU16BE(&s, &max_fall)) {
        return false;
    }

    fMaxCLL = max_cll;
    fMaxFALL = max_fall;
    return true;
}

sk_sp<SkData> ContentLightLevelInformation::serialize() const {
    SkDynamicMemoryWStream s;
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fMaxCLL));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fMaxFALL));
    return s.detachAsData();
}

// The PNG CLLI metadata stores luminance as an integer equal to the floating point value, multipled
// by clli_png_luminance_divisor.
static constexpr float clli_png_luminance_divisor = 10000.f;

bool ContentLightLevelInformation::parsePngChunk(const SkData* data) {
    if (data->size() != 8) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint32_t max_cll_times_10000 = 0;
    uint32_t max_fall_times_10000 = 0;
    if (!SkStreamPriv::ReadU32BE(&s, &max_cll_times_10000)) {
        return false;
    }
    if (!SkStreamPriv::ReadU32BE(&s, &max_fall_times_10000)) {
        return false;
    }

    fMaxCLL = max_cll_times_10000 / clli_png_luminance_divisor;
    fMaxFALL = max_fall_times_10000 / clli_png_luminance_divisor;
    return true;
}

sk_sp<SkData> ContentLightLevelInformation::serializePngChunk() const {
    SkDynamicMemoryWStream s;
    SkStreamPriv::WriteU32BE(&s, std::llroundf(fMaxCLL * clli_png_luminance_divisor));
    SkStreamPriv::WriteU32BE(&s, std::llroundf(fMaxFALL * clli_png_luminance_divisor));
    return s.detachAsData();
}

bool ContentLightLevelInformation::operator==(const ContentLightLevelInformation& other) const {
    return fMaxCLL == other.fMaxCLL &&
           fMaxFALL == other.fMaxFALL;
}

// The MDCV metadata stores chrominance [luminance] as an integer equal to the floating point value,
// multipled by mdcv_chrominance[luminance]_divisor.
static constexpr float mdcv_chrominance_divisor = 50000.f;
static constexpr float mdcv_luminance_divisor = 10000.f;

SkString MasteringDisplayColorVolume::toString() const {
    return SkStringPrintf(
        "{red:[%1.8f,%1.8f], green:[%1.8f,%1.8f], blue:[%1.8f,%1.8f], white:[%1.8f,%1.8f], maxLum:%f, minLum:%f}",
        fDisplayPrimaries.fRX, fDisplayPrimaries.fRY,
        fDisplayPrimaries.fGX, fDisplayPrimaries.fGY,
        fDisplayPrimaries.fBX, fDisplayPrimaries.fBY,
        fDisplayPrimaries.fWX, fDisplayPrimaries.fWY,
        fMaximumDisplayMasteringLuminance,
        fMinimumDisplayMasteringLuminance);
}

bool MasteringDisplayColorVolume::parse(const SkData* data) {
    if (data->size() != 24) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint16_t chromaticities_times_50000[8];
    for (auto& chromaticity_times_50000 : chromaticities_times_50000) {
        if (!SkStreamPriv::ReadU16BE(&s, &chromaticity_times_50000)) {
            return false;
        }
    }
    uint32_t max_luminance_times_10000 = 0;
    uint32_t min_luminance_times_10000 = 0;
    if (!SkStreamPriv::ReadU32BE(&s, &max_luminance_times_10000)) {
        return false;
    }
    if (!SkStreamPriv::ReadU32BE(&s, &min_luminance_times_10000)) {
        return false;
    }

    fDisplayPrimaries = SkColorSpacePrimaries({
        chromaticities_times_50000[0] / mdcv_chrominance_divisor,
        chromaticities_times_50000[1] / mdcv_chrominance_divisor,
        chromaticities_times_50000[2] / mdcv_chrominance_divisor,
        chromaticities_times_50000[3] / mdcv_chrominance_divisor,
        chromaticities_times_50000[4] / mdcv_chrominance_divisor,
        chromaticities_times_50000[5] / mdcv_chrominance_divisor,
        chromaticities_times_50000[6] / mdcv_chrominance_divisor,
        chromaticities_times_50000[7] / mdcv_chrominance_divisor,
    });
    fMaximumDisplayMasteringLuminance = max_luminance_times_10000 / mdcv_luminance_divisor;
    fMinimumDisplayMasteringLuminance = min_luminance_times_10000 / mdcv_luminance_divisor;
    return true;
}

sk_sp<SkData> MasteringDisplayColorVolume::serialize() const {
    SkDynamicMemoryWStream s;
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fRX * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fRY * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fGX * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fGY * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fBX * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fBY * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fWX * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU16BE(&s, std::llroundf(fDisplayPrimaries.fWY * mdcv_chrominance_divisor));
    SkStreamPriv::WriteU32BE(
            &s, std::llroundf(fMaximumDisplayMasteringLuminance * mdcv_luminance_divisor));
    SkStreamPriv::WriteU32BE(
            &s, std::llroundf(fMinimumDisplayMasteringLuminance * mdcv_luminance_divisor));
    return s.detachAsData();
}

bool MasteringDisplayColorVolume::operator==(const MasteringDisplayColorVolume& other) const {
    return fDisplayPrimaries.fRX == other.fDisplayPrimaries.fRX &&
           fDisplayPrimaries.fRY == other.fDisplayPrimaries.fRY &&
           fDisplayPrimaries.fGX == other.fDisplayPrimaries.fGX &&
           fDisplayPrimaries.fGY == other.fDisplayPrimaries.fGY &&
           fDisplayPrimaries.fBX == other.fDisplayPrimaries.fBX &&
           fDisplayPrimaries.fBY == other.fDisplayPrimaries.fBY &&
           fDisplayPrimaries.fWX == other.fDisplayPrimaries.fWX &&
           fDisplayPrimaries.fWY == other.fDisplayPrimaries.fWY &&
           fMaximumDisplayMasteringLuminance == other.fMaximumDisplayMasteringLuminance &&
           fMinimumDisplayMasteringLuminance == other.fMinimumDisplayMasteringLuminance;
}

Metadata Metadata::MakeEmpty() {
    return Metadata();
}

bool Metadata::getContentLightLevelInformation(ContentLightLevelInformation* clli) const {
    if (!fContentLightLevelInformation.has_value()) {
        return false;
    }
    if (clli) {
        *clli = fContentLightLevelInformation.value();
    }
    return true;
}

bool Metadata::getMasteringDisplayColorVolume(MasteringDisplayColorVolume* mdcv) const {
    if (!fMasteringDisplayColorVolume.has_value()) {
        return false;
    }
    if (mdcv) {
        *mdcv = fMasteringDisplayColorVolume.value();
    }
    return true;
}

bool Metadata::getAdaptiveGlobalToneMap(AdaptiveGlobalToneMap* agtm) const {
    if (!fAgtm) {
        return false;
    }
    AdaptiveGlobalToneMap agtmParsed;
    if (!agtmParsed.parse(fAgtm.get())) {
        return false;
    }
    if (agtm) {
      *agtm = agtmParsed;
    }
    return true;
}

sk_sp<const SkData> Metadata::getSerializedAgtm() const {
    return fAgtm;
}

void Metadata::setMasteringDisplayColorVolume(const MasteringDisplayColorVolume& mdcv) {
    fMasteringDisplayColorVolume = mdcv;
}

void Metadata::setContentLightLevelInformation(const ContentLightLevelInformation& clli) {
    fContentLightLevelInformation = clli;
}

void Metadata::setAdaptiveGlobalToneMap(const AdaptiveGlobalToneMap& agtm) {
    fAgtm = agtm.serialize();
}

void Metadata::setSerializedAgtm(sk_sp<const SkData> agtm) {
    fAgtm = agtm;
}

SkString Metadata::toString() const {
    auto agtm = Agtm::Make(fAgtm.get());
    return SkStringPrintf("{clli:%s, mdcv:%s, agtm:%s}",
        fContentLightLevelInformation.has_value() ?
            fContentLightLevelInformation->toString().c_str() : "None",
        fMasteringDisplayColorVolume.has_value() ?
            fMasteringDisplayColorVolume->toString().c_str() : "None",
        agtm ? agtm->toString().c_str() : "None");
}

bool Metadata::operator==(const Metadata& other) const {
    return fContentLightLevelInformation == other.fContentLightLevelInformation &&
           fMasteringDisplayColorVolume == other.fMasteringDisplayColorVolume &&
           SkData::Equals(fAgtm.get(), other.fAgtm.get());
}

sk_sp<SkColorFilter> Metadata::makeToneMapColorFilter(
        float targetedHdrHeadroom, const SkColorSpace* inputColorSpace) const {
    AdaptiveGlobalToneMap agtm;
    float scaleFactor = 1.f;
    if (!AgtmHelpers::PopulateToneMapAgtmParams(*this, inputColorSpace, &agtm, &scaleFactor)) {
        return nullptr;
    }

    auto& hatm = agtm.fHeadroomAdaptiveToneMap;
    if (!hatm.has_value()) {
        // TODO(https://crbug.com/395659818): Add default tone mapping.
        return nullptr;
    }
    return AgtmHelpers::MakeColorFilter(hatm.value(), targetedHdrHeadroom, scaleFactor);
}

}  // namespace skhdr

