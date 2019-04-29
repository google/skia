/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/third_party/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkOpts.h"

bool SkColorSpacePrimaries::toXYZD50(skcms_Matrix3x3* toXYZ_D50) const {
    return skcms_PrimariesToXYZD50(fRX, fRY, fGX, fGY, fBX, fBY, fWX, fWY, toXYZ_D50);
}

SkColorSpace::SkColorSpace(const float transferFn[7],
                           const skcms_Matrix3x3& toXYZD50) {
    memcpy(fToXYZD50_3x3, &toXYZD50.vals[0][0], 9*sizeof(float));
    fToXYZD50Hash = SkOpts::hash_fn(fToXYZD50_3x3, 9*sizeof(float), 0);

    memcpy(fTransferFn, transferFn, 7*sizeof(float));
    fTransferFnHash = SkOpts::hash_fn(fTransferFn, 7*sizeof(float), 0);
}

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

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(const skcms_TransferFunction& transferFn,
                                          const skcms_Matrix3x3& toXYZ) {
    if (!is_valid_transfer_fn(transferFn)) {
        return nullptr;
    }

    const float* tf = &transferFn.g;

    if (is_almost_srgb(transferFn)) {
        if (xyz_almost_equal(toXYZ, SkNamedGamut::kSRGB)) {
            return SkColorSpace::MakeSRGB();
        }
        tf = &SkNamedTransferFn::kSRGB.g;
    } else if (is_almost_2dot2(transferFn)) {
        tf = &SkNamedTransferFn::k2Dot2.g;
    } else if (is_almost_linear(transferFn)) {
        if (xyz_almost_equal(toXYZ, SkNamedGamut::kSRGB)) {
            return SkColorSpace::MakeSRGBLinear();
        }
        tf = &SkNamedTransferFn::kLinear.g;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace(tf, toXYZ));
}

class SkColorSpaceSingletonFactory {
public:
    static SkColorSpace* Make(const skcms_TransferFunction& transferFn,
                              const skcms_Matrix3x3& to_xyz) {
        return new SkColorSpace(&transferFn.g, to_xyz);
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
            skcms_Matrix3x3 fwd, inv;
            memcpy(&fwd, fToXYZD50_3x3, 9*sizeof(float));
            if (!skcms_Matrix3x3_invert(&fwd, &inv)) {
                SkAssertResult(skcms_Matrix3x3_invert(&skcms_sRGB_profile()->toXYZD50, &inv));
            }
            memcpy(fFromXYZD50_3x3, &inv, 9*sizeof(float));
        }

        // Invert transfer function, defaulting to sRGB if we can't.
        {
            skcms_TransferFunction fwd, inv;
            this->transferFn(&fwd.g);
            if (!skcms_TransferFunction_invert(&fwd, &inv)) {
                inv = *skcms_sRGB_Inverse_TransferFunction();
            }
            memcpy(fInvTransferFn, &inv, 7*sizeof(float));
        }

    });
}

bool SkColorSpace::isNumericalTransferFn(skcms_TransferFunction* coeffs) const {
    this->transferFn(&coeffs->g);
    return true;
}

void SkColorSpace::transferFn(float gabcdef[7]) const {
    memcpy(gabcdef, &fTransferFn, 7*sizeof(float));
}

void SkColorSpace::invTransferFn(float gabcdef[7]) const {
    this->computeLazyDstFields();
    memcpy(gabcdef, &fInvTransferFn, 7*sizeof(float));
}

bool SkColorSpace::toXYZD50(SkMatrix44* toXYZD50) const {
    toXYZD50->set3x3RowMajorf(fToXYZD50_3x3);
    return true;
}

bool SkColorSpace::toXYZD50(skcms_Matrix3x3* toXYZD50) const {
    memcpy(toXYZD50, fToXYZD50_3x3, 9*sizeof(float));
    return true;
}

void SkColorSpace::gamutTransformTo(const SkColorSpace* dst, float src_to_dst[9]) const {
    dst->computeLazyDstFields();

    skcms_Matrix3x3 toXYZD50,
                  fromXYZD50;

    memcpy(&  toXYZD50, this->  fToXYZD50_3x3, 9*sizeof(float));
    memcpy(&fromXYZD50, dst ->fFromXYZD50_3x3, 9*sizeof(float));

    skcms_Matrix3x3 srcToDst = skcms_Matrix3x3_concat(&fromXYZD50, &toXYZD50);
    memcpy(src_to_dst, &srcToDst, 9*sizeof(float));
}

bool SkColorSpace::isSRGB() const {
    return sk_srgb_singleton() == this;
}

bool SkColorSpace::gammaCloseToSRGB() const {
    // Nearly-equal transfer functions were snapped at construction time, so just do an exact test
    return memcmp(fTransferFn, &SkNamedTransferFn::kSRGB.g, 7*sizeof(float)) == 0;
}

bool SkColorSpace::gammaIsLinear() const {
    // Nearly-equal transfer functions were snapped at construction time, so just do an exact test
    return memcmp(fTransferFn, &SkNamedTransferFn::kLinear.g, 7*sizeof(float)) == 0;
}

sk_sp<SkColorSpace> SkColorSpace::makeLinearGamma() const {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    skcms_Matrix3x3 gamut;
    this->toXYZD50(&gamut);
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, gamut);
}

sk_sp<SkColorSpace> SkColorSpace::makeSRGBGamma() const {
    if (this->gammaCloseToSRGB()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    skcms_Matrix3x3 gamut;
    this->toXYZD50(&gamut);
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, gamut);
}

sk_sp<SkColorSpace> SkColorSpace::makeColorSpin() const {
    skcms_Matrix3x3 spin = {{
        { 0, 0, 1 },
        { 1, 0, 0 },
        { 0, 1, 0 },
    }};

    skcms_Matrix3x3 toXYZ;
    this->toXYZD50(&toXYZ);

    skcms_Matrix3x3 spun = skcms_Matrix3x3_concat(&toXYZ, &spin);

    return sk_sp<SkColorSpace>(new SkColorSpace(fTransferFn, spun));
}

void SkColorSpace::toProfile(skcms_ICCProfile* profile) const {
    skcms_TransferFunction tf;
    skcms_Matrix3x3        toXYZD50;

    memcpy(&tf,       fTransferFn,   7*sizeof(float));
    memcpy(&toXYZD50, fToXYZD50_3x3, 9*sizeof(float));

    skcms_Init               (profile);
    skcms_SetTransferFunction(profile, &tf);
    skcms_SetXYZD50          (profile, &toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::Make(const skcms_ICCProfile& profile) {
    // TODO: move below ≈sRGB test?
    if (!profile.has_toXYZD50 || !profile.has_trc) {
        return nullptr;
    }

    if (skcms_ApproximatelyEqualProfiles(&profile, skcms_sRGB_profile())) {
        return SkColorSpace::MakeSRGB();
    }

    // TODO: can we save this work and skip lazily inverting the matrix later?
    skcms_Matrix3x3 inv;
    if (!skcms_Matrix3x3_invert(&profile.toXYZD50, &inv)) {
        return nullptr;
    }

    // We can't work with tables or mismatched parametric curves,
    // but if they all look close enough to sRGB, that's fine.
    // TODO: should we maybe do this unconditionally to snap near-sRGB parametrics to sRGB?
    const skcms_Curve* trc = profile.trc;
    if (trc[0].table_entries != 0 ||
        trc[1].table_entries != 0 ||
        trc[2].table_entries != 0 ||
        0 != memcmp(&trc[0].parametric, &trc[1].parametric, sizeof(trc[0].parametric)) ||
        0 != memcmp(&trc[0].parametric, &trc[2].parametric, sizeof(trc[0].parametric)))
    {
        if (skcms_TRCs_AreApproximateInverse(&profile, skcms_sRGB_Inverse_TransferFunction())) {
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, profile.toXYZD50);
        }
        return nullptr;
    }

    return SkColorSpace::MakeRGB(profile.trc[0].parametric, profile.toXYZD50);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

enum Version {
    k0_Version, // Initial version, header + flags for matrix and profile
    k1_Version, // Simple header (version tag) + 16 floats

    kCurrent_Version = k1_Version,
};

enum NamedColorSpace {
    kSRGB_NamedColorSpace,
    kAdobeRGB_NamedColorSpace,
    kSRGBLinear_NamedColorSpace,
};

enum NamedGamma {
    kLinear_NamedGamma,
    kSRGB_NamedGamma,
    k2Dot2_NamedGamma,
};

struct ColorSpaceHeader {
    // Flag values, only used by old (k0_Version) serialization
    static constexpr uint8_t kMatrix_Flag     = 1 << 0;
    static constexpr uint8_t kICC_Flag        = 1 << 1;
    static constexpr uint8_t kTransferFn_Flag = 1 << 3;

    uint8_t fVersion = kCurrent_Version;

    // Other fields are only used by k0_Version. Could be re-purposed in future versions.
    uint8_t fNamed      = 0;
    uint8_t fGammaNamed = 0;
    uint8_t fFlags      = 0;
};

size_t SkColorSpace::writeToMemory(void* memory) const {
    if (memory) {
        *((ColorSpaceHeader*) memory) = ColorSpaceHeader();
        memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));

        memcpy(memory, fTransferFn, 7 * sizeof(float));
        memory = SkTAddOffset<void>(memory, 7 * sizeof(float));

        memcpy(memory, fToXYZD50_3x3, 9 * sizeof(float));
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
    if (k1_Version == header.fVersion) {
        if (length < 16 * sizeof(float)) {
            return nullptr;
        }

        skcms_TransferFunction transferFn;
        memcpy(&transferFn, data, 7 * sizeof(float));
        data = SkTAddOffset<const void>(data, 7 * sizeof(float));

        skcms_Matrix3x3 toXYZ;
        memcpy(&toXYZ, data, 9 * sizeof(float));
        return SkColorSpace::MakeRGB(transferFn, toXYZ);
    } else if (k0_Version == header.fVersion) {
        if (0 == header.fFlags) {
            switch ((NamedColorSpace)header.fNamed) {
                case kSRGB_NamedColorSpace:
                    return SkColorSpace::MakeSRGB();
                case kSRGBLinear_NamedColorSpace:
                    return SkColorSpace::MakeSRGBLinear();
                case kAdobeRGB_NamedColorSpace:
                    return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,
                                                 SkNamedGamut::kAdobeRGB);
            }
        }

        auto make_named_tf = [=](const skcms_TransferFunction& tf) {
            if (ColorSpaceHeader::kMatrix_Flag != header.fFlags || length < 12 * sizeof(float)) {
                return sk_sp<SkColorSpace>(nullptr);
            }

            // Version 0 matrix is row-major 3x4
            skcms_Matrix3x3 toXYZ;
            memcpy(&toXYZ.vals[0][0], (const float*)data + 0, 3 * sizeof(float));
            memcpy(&toXYZ.vals[1][0], (const float*)data + 4, 3 * sizeof(float));
            memcpy(&toXYZ.vals[2][0], (const float*)data + 8, 3 * sizeof(float));
            return SkColorSpace::MakeRGB(tf, toXYZ);
        };

        switch ((NamedGamma) header.fGammaNamed) {
            case kSRGB_NamedGamma:
                return make_named_tf(SkNamedTransferFn::kSRGB);
            case k2Dot2_NamedGamma:
                return make_named_tf(SkNamedTransferFn::k2Dot2);
            case kLinear_NamedGamma:
                return make_named_tf(SkNamedTransferFn::kLinear);
            default:
                break;
        }

        switch (header.fFlags) {
            case ColorSpaceHeader::kICC_Flag: {
                // Deprecated and unsupported code path
                return nullptr;
            }
            case ColorSpaceHeader::kTransferFn_Flag: {
                if (length < 19 * sizeof(float)) {
                    return nullptr;
                }

                // Version 0 TF is in abcdefg order
                skcms_TransferFunction transferFn;
                transferFn.a = *(((const float*) data) + 0);
                transferFn.b = *(((const float*) data) + 1);
                transferFn.c = *(((const float*) data) + 2);
                transferFn.d = *(((const float*) data) + 3);
                transferFn.e = *(((const float*) data) + 4);
                transferFn.f = *(((const float*) data) + 5);
                transferFn.g = *(((const float*) data) + 6);
                data = SkTAddOffset<const void>(data, 7 * sizeof(float));

                // Version 0 matrix is row-major 3x4
                skcms_Matrix3x3 toXYZ;
                memcpy(&toXYZ.vals[0][0], (const float*)data + 0, 3 * sizeof(float));
                memcpy(&toXYZ.vals[1][0], (const float*)data + 4, 3 * sizeof(float));
                memcpy(&toXYZ.vals[2][0], (const float*)data + 8, 3 * sizeof(float));
                return SkColorSpace::MakeRGB(transferFn, toXYZ);
            }
            default:
                return nullptr;
        }
    } else {
        return nullptr;
    }
}

bool SkColorSpace::Equals(const SkColorSpace* x, const SkColorSpace* y) {
    if (x == y) {
        return true;
    }

    if (!x || !y) {
        return false;
    }

    if (x->hash() == y->hash()) {
        for (int i = 0; i < 7; i++) {
            SkASSERT(x->  fTransferFn[i] == y->  fTransferFn[i] && "Hash collsion");
        }
        for (int i = 0; i < 9; i++) {
            SkASSERT(x->fToXYZD50_3x3[i] == y->fToXYZD50_3x3[i] && "Hash collsion");
        }
        return true;
    }
    return false;
}
