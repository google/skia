/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorSpacePriv.h"

#include <cmath>
#include <cstring>

static bool xyz_almost_equal(const skcms_Matrix3x3& mA, const skcms_Matrix3x3& mB) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (!color_space_almost_equal(mA.vals[r][c], mB.vals[r][c])) {
                return false;
            }
        }
    }

    return true;
}

namespace SkNamedPrimaries {

// Rec. ITU-T H.273, Table 2.
struct TableEntry {
    CicpId cicp_id;
    SkColorSpacePrimaries sk_primaries;
};
const static TableEntry cicp_table[] = {
    { CicpId::kRec709, kRec709 },
    { CicpId::kRec470SystemM, kRec470SystemM },
    { CicpId::kRec470SystemBG, kRec470SystemBG },
    { CicpId::kRec601, kRec601 },
    { CicpId::kSMPTE_ST_240, kSMPTE_ST_240 },
    { CicpId::kGenericFilm, kGenericFilm },
    { CicpId::kRec2020, kRec2020 },
    { CicpId::kSMPTE_ST_428_1, kSMPTE_ST_428_1 },
    { CicpId::kSMPTE_RP_431_2, kSMPTE_RP_431_2 },
    { CicpId::kSMPTE_EG_432_1, kSMPTE_EG_432_1 },
    { CicpId::kITU_T_H273_Value22, kITU_T_H273_Value22 },
};
// Value 2 indicates "characteristics are unknown or are determined by the application". In
// practice, this means we will delegate the primaries to the toXYZD50 tags.
uint8_t kCicpIdApplicationDefined = 2;

bool GetCicp(CicpId primaries, SkColorSpacePrimaries& sk_primaries) {
    for (const auto& table_entry : cicp_table) {
        if (primaries == table_entry.cicp_id) {
            sk_primaries = table_entry.sk_primaries;
            return true;
        }
    }
    return false;
}

bool GetCicpFromMatrix(const skcms_Matrix3x3& m, CicpId& primaries) {
    for (const auto& table_entry : cicp_table) {
        skcms_Matrix3x3 table_entry_m;
        if (table_entry.sk_primaries.toXYZD50(&table_entry_m)) {
            if (xyz_almost_equal(m, table_entry_m)) {
                primaries = table_entry.cicp_id;
                return true;
            }
        }
    }
    return false;
}

}  // namespace SkNamedPrimaries

namespace SkNamedTransferFn {

// Rec. ITU-T H.273, Table 3.
struct TableEntry {
    CicpId cicp_id;
    skcms_TransferFunction trfn;
};
const static TableEntry cicp_table[] = {
    { SkNamedTransferFn::CicpId::kRec709, kRec709 },
    { SkNamedTransferFn::CicpId::kRec470SystemM, kRec470SystemM },
    { SkNamedTransferFn::CicpId::kRec470SystemBG, kRec470SystemBG },
    { SkNamedTransferFn::CicpId::kRec601, kRec601 },
    { SkNamedTransferFn::CicpId::kSMPTE_ST_240, kSMPTE_ST_240 },
    { SkNamedTransferFn::CicpId::kLinear, SkNamedTransferFn::kLinear },
    { SkNamedTransferFn::CicpId::kIEC61966_2_4, kIEC61966_2_4 },
    { SkNamedTransferFn::CicpId::kIEC61966_2_1, SkNamedTransferFn::kIEC61966_2_1 },
    { SkNamedTransferFn::CicpId::kRec2020_10bit, kRec2020_10bit },
    { SkNamedTransferFn::CicpId::kRec2020_12bit, kRec2020_12bit },
    { SkNamedTransferFn::CicpId::kPQ, SkNamedTransferFn::kPQ },
    { SkNamedTransferFn::CicpId::kSMPTE_ST_428_1, kSMPTE_ST_428_1 },
    { SkNamedTransferFn::CicpId::kHLG, SkNamedTransferFn::kHLG },
};
// Value 2 indicates "characteristics are unknown or are determined by the application". In
// practice, this means we will delegate the primaries to the trc tags.
uint8_t kCicpIdApplicationDefined = 2;

bool GetCicp(SkNamedTransferFn::CicpId transfer_characteristics, skcms_TransferFunction& trfn) {
    for (const auto& table_entry : cicp_table) {
      if (transfer_characteristics == table_entry.cicp_id) {
        trfn = table_entry.trfn;
        return true;
      }
    }
    return false;
}

}  // namespace SkNamedTransferFn

bool SkColorSpacePrimaries::toXYZD50(skcms_Matrix3x3* toXYZ_D50) const {
    return skcms_PrimariesToXYZD50(fRX, fRY, fGX, fGY, fBX, fBY, fWX, fWY, toXYZ_D50);
}

SkColorSpace::SkColorSpace(const skcms_TransferFunction& transferFn,
                           const skcms_Matrix3x3& toXYZD50)
        : fTransferFn(transferFn)
        , fToXYZD50(toXYZD50) {
    fTransferFnHash = SkChecksum::Hash32(&fTransferFn, 7*sizeof(float));
    fToXYZD50Hash = SkChecksum::Hash32(&fToXYZD50, 9*sizeof(float));
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(const skcms_TransferFunction& transferFn,
                                          const skcms_Matrix3x3& toXYZ) {
    if (skcms_TransferFunction_getType(&transferFn) == skcms_TFType_Invalid) {
        return nullptr;
    }

    const skcms_TransferFunction* tf = &transferFn;

    if (is_almost_srgb(transferFn)) {
        if (xyz_almost_equal(toXYZ, SkNamedGamut::kSRGB)) {
            return SkColorSpace::MakeSRGB();
        }
        tf = &SkNamedTransferFn::kSRGB;
    } else if (is_almost_2dot2(transferFn)) {
        tf = &SkNamedTransferFn::k2Dot2;
    } else if (is_almost_linear(transferFn)) {
        if (xyz_almost_equal(toXYZ, SkNamedGamut::kSRGB)) {
            return SkColorSpace::MakeSRGBLinear();
        }
        tf = &SkNamedTransferFn::kLinear;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace(*tf, toXYZ));
}

sk_sp<SkColorSpace> SkColorSpace::MakeCICP(SkNamedPrimaries::CicpId color_primaries,
                                           SkNamedTransferFn::CicpId transfer_characteristics) {
    skcms_TransferFunction trfn;
    if (!SkNamedTransferFn::GetCicp(transfer_characteristics, trfn)) {
        return nullptr;
    }

    SkColorSpacePrimaries primaries;
    if (!SkNamedPrimaries::GetCicp(color_primaries, primaries)) {
        return nullptr;
    }

    skcms_Matrix3x3 primaries_matrix;
    if (!primaries.toXYZD50(&primaries_matrix)) {
        return nullptr;
    }

    return SkColorSpace::MakeRGB(trfn, primaries_matrix);
}

class SkColorSpaceSingletonFactory {
public:
    static SkColorSpace* Make(const skcms_TransferFunction& transferFn,
                              const skcms_Matrix3x3& to_xyz) {
        return new SkColorSpace(transferFn, to_xyz);
    }
};

SkColorSpace* sk_srgb_singleton() {
    static SkColorSpace* cs = SkColorSpaceSingletonFactory::Make(SkNamedTransferFn::kSRGB,
                                                                 SkNamedGamut::kSRGB);
    return cs;
}

SkColorSpace* sk_srgb_linear_singleton() {
    static SkColorSpace* cs = SkColorSpaceSingletonFactory::Make(SkNamedTransferFn::kLinear,
                                                                 SkNamedGamut::kSRGB);
    return cs;
}

sk_sp<SkColorSpace> SkColorSpace::MakeSRGB() {
    return sk_ref_sp(sk_srgb_singleton());
}

sk_sp<SkColorSpace> SkColorSpace::MakeSRGBLinear() {
    return sk_ref_sp(sk_srgb_linear_singleton());
}

void SkColorSpace::computeLazyDstFields() const {
    fLazyDstFieldsOnce([this] {

        // Invert 3x3 gamut, defaulting to sRGB if we can't.
        {
            if (!skcms_Matrix3x3_invert(&fToXYZD50, &fFromXYZD50)) {
                SkAssertResult(skcms_Matrix3x3_invert(&skcms_sRGB_profile()->toXYZD50,
                                                      &fFromXYZD50));
            }
        }

        // Invert transfer function, defaulting to sRGB if we can't.
        {
            if (!skcms_TransferFunction_invert(&fTransferFn, &fInvTransferFn)) {
                fInvTransferFn = *skcms_sRGB_Inverse_TransferFunction();
            }
        }

    });
}

bool SkColorSpace::isNumericalTransferFn(skcms_TransferFunction* coeffs) const {
    // TODO: Change transferFn/invTransferFn to just operate on skcms_TransferFunction (all callers
    // already pass pointers to an skcms struct). Then remove this function, and update the two
    // remaining callers to do the right thing with transferFn and classify.
    this->transferFn(coeffs);
    return skcms_TransferFunction_getType(coeffs) == skcms_TFType_sRGBish;
}

void SkColorSpace::transferFn(float gabcdef[7]) const {
    memcpy(gabcdef, &fTransferFn, 7*sizeof(float));
}

void SkColorSpace::transferFn(skcms_TransferFunction* fn) const {
    *fn = fTransferFn;
}

void SkColorSpace::invTransferFn(skcms_TransferFunction* fn) const {
    this->computeLazyDstFields();
    *fn = fInvTransferFn;
}

bool SkColorSpace::toXYZD50(skcms_Matrix3x3* toXYZD50) const {
    *toXYZD50 = fToXYZD50;
    return true;
}

void SkColorSpace::gamutTransformTo(const SkColorSpace* dst, skcms_Matrix3x3* src_to_dst) const {
    dst->computeLazyDstFields();
    *src_to_dst = skcms_Matrix3x3_concat(&dst->fFromXYZD50, &fToXYZD50);
}

bool SkColorSpace::isSRGB() const {
    return sk_srgb_singleton() == this;
}

bool SkColorSpace::gammaCloseToSRGB() const {
    // Nearly-equal transfer functions were snapped at construction time, so just do an exact test
    return memcmp(&fTransferFn, &SkNamedTransferFn::kSRGB, 7*sizeof(float)) == 0;
}

bool SkColorSpace::gammaIsLinear() const {
    // Nearly-equal transfer functions were snapped at construction time, so just do an exact test
    return memcmp(&fTransferFn, &SkNamedTransferFn::kLinear, 7*sizeof(float)) == 0;
}

sk_sp<SkColorSpace> SkColorSpace::makeLinearGamma() const {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::makeSRGBGamma() const {
    if (this->gammaCloseToSRGB()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::makeColorSpin() const {
    skcms_Matrix3x3 spin = {{
        { 0, 0, 1 },
        { 1, 0, 0 },
        { 0, 1, 0 },
    }};

    skcms_Matrix3x3 spun = skcms_Matrix3x3_concat(&fToXYZD50, &spin);

    return sk_sp<SkColorSpace>(new SkColorSpace(fTransferFn, spun));
}

void SkColorSpace::toProfile(skcms_ICCProfile* profile) const {
    skcms_Init               (profile);
    // TODO(https://issues.skia.org/issues/420956739): This value should only be
    // set for sRGB-ish transfer functions. All other values are invalid.
    skcms_SetTransferFunction(profile, &fTransferFn);
    skcms_SetXYZD50          (profile, &fToXYZD50);

    switch (skcms_TransferFunction_getType(&fTransferFn)) {
        case skcms_TFType_PQ:
        case skcms_TFType_PQish:
            profile->has_CICP = true;
            profile->CICP.transfer_characteristics =
                static_cast<uint8_t>(SkNamedTransferFn::CicpId::kPQ);
            break;
        case skcms_TFType_HLG:
        case skcms_TFType_HLGish:
            profile->has_CICP = true;
            profile->CICP.transfer_characteristics =
                static_cast<uint8_t>(SkNamedTransferFn::CicpId::kHLG);
            break;
        default:
            break;
    }
    if (profile->has_CICP) {
        profile->CICP.matrix_coefficients = 0;
        profile->CICP.video_full_range_flag = 1;
        SkNamedPrimaries::CicpId primaries_id = SkNamedPrimaries::CicpId::kRec709;
        if (SkNamedPrimaries::GetCicpFromMatrix(fToXYZD50, primaries_id)) {
            profile->CICP.color_primaries = static_cast<uint8_t>(primaries_id);
        } else {
            profile->CICP.color_primaries = SkNamedPrimaries::kCicpIdApplicationDefined;
        }
    }
}

sk_sp<SkColorSpace> SkColorSpace::Make(const skcms_ICCProfile& profile) {
    // The CICP values are only valid for full-range, with no matrix.
    bool use_cicp = profile.has_CICP &&
                    profile.CICP.matrix_coefficients == 0 &&
                    profile.CICP.video_full_range_flag == 1;
    auto cicp_color_primaries = static_cast<SkNamedPrimaries::CicpId>(profile.CICP.color_primaries);
    auto cicp_transfer_characteristics =
        static_cast<SkNamedTransferFn::CicpId>(profile.CICP.transfer_characteristics);

    // Early checks for exact sRGB matches.
    if (use_cicp &&
        cicp_color_primaries == SkNamedPrimaries::CicpId::kRec709 &&
        cicp_transfer_characteristics == SkNamedTransferFn::CicpId::kIEC61966_2_4) {
        return SkColorSpace::MakeSRGB();
    } else if (skcms_ApproximatelyEqualProfiles(&profile, skcms_sRGB_profile())) {
        return SkColorSpace::MakeSRGB();
    }

    // Set the toXYZD50 matrix, preferring CICP over the matrix itself.
    skcms_Matrix3x3 toXYZD50;
    bool hasSetToXYZD50 = false;
    if (use_cicp) {
        SkColorSpacePrimaries primaries;
        if (SkNamedPrimaries::GetCicp(cicp_color_primaries, primaries)) {
            if (primaries.toXYZD50(&toXYZD50)) {
                hasSetToXYZD50 = true;
            }
        } else if (profile.CICP.color_primaries != SkNamedPrimaries::kCicpIdApplicationDefined) {
            return nullptr;
        }
    }
    if (profile.has_toXYZD50 && !hasSetToXYZD50) {
        // TODO: can we save this work and skip lazily inverting the matrix later?
        skcms_Matrix3x3 inv;
        toXYZD50 = profile.toXYZD50;
        if (skcms_Matrix3x3_invert(&toXYZD50, &inv)) {
            hasSetToXYZD50 = true;
        }
    }
    if (!hasSetToXYZD50) {
        return nullptr;
    }

    // Set the transfer function, preferring CICP over the curves.
    skcms_TransferFunction trfn;
    bool hasSetTrfn = false;
    if (use_cicp) {
        if (SkNamedTransferFn::GetCicp(cicp_transfer_characteristics, trfn)) {
            hasSetTrfn = true;
        } else if (profile.CICP.transfer_characteristics !=
                   SkNamedTransferFn::kCicpIdApplicationDefined) {
            return nullptr;
        }
    }
    if (profile.has_trc && !hasSetTrfn) {
        // We can't work with tables or mismatched parametric curves.
        const skcms_Curve* trc = profile.trc;
        if (trc[0].table_entries == 0 &&
            trc[1].table_entries == 0 &&
            trc[2].table_entries == 0 &&
            0 == memcmp(&trc[0].parametric, &trc[1].parametric, sizeof(trc[0].parametric)) &&
            0 == memcmp(&trc[0].parametric, &trc[2].parametric, sizeof(trc[0].parametric)))
        {
            trfn = profile.trc[0].parametric;
            hasSetTrfn = true;
        } else {
            // If all curves look close enough to sRGB, that's fine.
            // TODO: should we maybe do this unconditionally to snap near-sRGB parametrics to sRGB?
            if (skcms_TRCs_AreApproximateInverse(&profile, skcms_sRGB_Inverse_TransferFunction())) {
                trfn = SkNamedTransferFn::kSRGB;
                hasSetTrfn = true;
            }
        }
    }
    if (!hasSetTrfn) {
        return nullptr;
    }

    return SkColorSpace::MakeRGB(trfn, toXYZD50);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

enum Version {
    k0_Version, // Initial (deprecated) version, no longer supported
    k1_Version, // Simple header (version tag) + 16 floats

    kCurrent_Version = k1_Version,
};

struct ColorSpaceHeader {
    uint8_t fVersion = kCurrent_Version;

    // Other fields were only used by k0_Version. Could be re-purposed in future versions.
    uint8_t fReserved0 = 0;
    uint8_t fReserved1 = 0;
    uint8_t fReserved2 = 0;
};

size_t SkColorSpace::writeToMemory(void* memory) const {
    if (memory) {
        *((ColorSpaceHeader*) memory) = ColorSpaceHeader();
        memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));

        memcpy(memory, &fTransferFn, 7 * sizeof(float));
        memory = SkTAddOffset<void>(memory, 7 * sizeof(float));

        memcpy(memory, &fToXYZD50, 9 * sizeof(float));
    }

    return sizeof(ColorSpaceHeader) + 16 * sizeof(float);
}

sk_sp<SkData> SkColorSpace::serialize() const {
    sk_sp<SkData> data = SkData::MakeUninitialized(this->writeToMemory(nullptr));
    this->writeToMemory(data->writable_data());
    return data;
}

sk_sp<SkColorSpace> SkColorSpace::Deserialize(const void* data, size_t length) {
    if (length < sizeof(ColorSpaceHeader)) {
        return nullptr;
    }

    ColorSpaceHeader header = *((const ColorSpaceHeader*) data);
    data = SkTAddOffset<const void>(data, sizeof(ColorSpaceHeader));
    length -= sizeof(ColorSpaceHeader);
    if (header.fVersion != k1_Version) {
        return nullptr;
    }

    if (length < 16 * sizeof(float)) {
        return nullptr;
    }

    skcms_TransferFunction transferFn;
    memcpy(&transferFn, data, 7 * sizeof(float));
    data = SkTAddOffset<const void>(data, 7 * sizeof(float));

    skcms_Matrix3x3 toXYZ;
    memcpy(&toXYZ, data, 9 * sizeof(float));
    return SkColorSpace::MakeRGB(transferFn, toXYZ);
}

bool SkColorSpace::Equals(const SkColorSpace* x, const SkColorSpace* y) {
    if (x == y) {
        return true;
    }

    if (!x || !y) {
        return false;
    }

    if (x->hash() == y->hash()) {
    #if defined(SK_DEBUG)
        // Do these floats function equivalently?
        // This returns true more often than simple float comparison   (NaN vs. NaN) and,
        // also returns true more often than simple bitwise comparison (+0 vs. -0) and,
        // even returns true more often than those two OR'd together   (two different NaNs).
        auto equiv = [](float X, float Y) {
            return (X==Y)
                || (std::isnan(X) && std::isnan(Y));
        };

        for (int i = 0; i < 7; i++) {
            float X = (&x->fTransferFn.g)[i],
                  Y = (&y->fTransferFn.g)[i];
            SkASSERTF(equiv(X,Y), "Hash collision at tf[%d], !equiv(%g,%g)\n", i, X,Y);
        }
        for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            float X = x->fToXYZD50.vals[r][c],
                  Y = y->fToXYZD50.vals[r][c];
            SkASSERTF(equiv(X,Y), "Hash collision at toXYZD50[%d][%d], !equiv(%g,%g)\n", r,c, X,Y);
        }
    #endif
        return true;
    }
    return false;
}
