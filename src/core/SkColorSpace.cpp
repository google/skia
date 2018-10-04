/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkOpts.h"
#include "../../third_party/skcms/skcms.h"

bool SkColorSpacePrimaries::toXYZD50(SkMatrix44* toXYZ_D50) const {
    skcms_Matrix3x3 toXYZ;
    if (!skcms_PrimariesToXYZD50(fRX, fRY, fGX, fGY, fBX, fBY, fWX, fWY, &toXYZ)) {
        return false;
    }
    toXYZ_D50->set3x3RowMajorf(&toXYZ.vals[0][0]);
    return true;
}

static bool xyz_almost_equal(const SkMatrix44& toXYZD50, const float m33[9]) {
    return color_space_almost_equal(toXYZD50.getFloat(0, 0), m33[0]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 1), m33[1]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 2), m33[2]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 0), m33[3]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 1), m33[4]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 2), m33[5]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 0), m33[6]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 1), m33[7]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 2), m33[8]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 0), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 1), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 2), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 3), 1.0f);
}

SkColorSpace::SkColorSpace(SkGammaNamed gammaNamed,
                           const float transferFn[7],
                           const SkMatrix44& toXYZD50)
    : fGammaNamed(gammaNamed)
{
    for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++) {
        fToXYZD50_3x3[3*r+c] = toXYZD50.get(r,c);
    }
    SkASSERT(xyz_almost_equal(toXYZD50, fToXYZD50_3x3));
    fToXYZD50Hash = SkOpts::hash_fn(fToXYZD50_3x3, 9*sizeof(float), 0);

    switch (fGammaNamed) {
        case kSRGB_SkGammaNamed:        transferFn = &  gSRGB_TransferFn.fG; break;
        case k2Dot2Curve_SkGammaNamed:  transferFn = & g2Dot2_TransferFn.fG; break;
        case kLinear_SkGammaNamed:      transferFn = &gLinear_TransferFn.fG; break;
        case kNonStandard_SkGammaNamed:                                      break;
    }
    memcpy(fTransferFn, transferFn, 7*sizeof(float));
    fTransferFnHash = SkOpts::hash_fn(fTransferFn, 7*sizeof(float), 0);
}


sk_sp<SkColorSpace> SkColorSpace::MakeRGB(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50) {
    switch (gammaNamed) {
        case kSRGB_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gSRGB_toXYZD50)) {
                return SkColorSpace::MakeSRGB();
            }
            break;
        case kLinear_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gSRGB_toXYZD50)) {
                return SkColorSpace::MakeSRGBLinear();
            }
            break;
        case kNonStandard_SkGammaNamed:
            // This is not allowed.
            return nullptr;
        default:
            break;
    }
    return sk_sp<SkColorSpace>(new SkColorSpace(gammaNamed, nullptr, toXYZD50));
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(RenderTargetGamma gamma, const SkMatrix44& toXYZD50) {
    switch (gamma) {
        case kLinear_RenderTargetGamma:
            return SkColorSpace::MakeRGB(kLinear_SkGammaNamed, toXYZD50);
        case kSRGB_RenderTargetGamma:
            return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, toXYZD50);
        default:
            return nullptr;
    }
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(const SkColorSpaceTransferFn& coeffs,
                                          const SkMatrix44& toXYZD50) {
    if (!is_valid_transfer_fn(coeffs)) {
        return nullptr;
    }

    if (is_almost_srgb(coeffs)) {
        return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, toXYZD50);
    }

    if (is_almost_2dot2(coeffs)) {
        return SkColorSpace::MakeRGB(k2Dot2Curve_SkGammaNamed, toXYZD50);
    }

    if (is_almost_linear(coeffs)) {
        return SkColorSpace::MakeRGB(kLinear_SkGammaNamed, toXYZD50);
    }

    return sk_sp<SkColorSpace>(new SkColorSpace(kNonStandard_SkGammaNamed, &coeffs.fG, toXYZD50));
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(RenderTargetGamma gamma, Gamut gamut) {
    SkMatrix44 toXYZD50;
    to_xyz_d50(&toXYZD50, gamut);
    return SkColorSpace::MakeRGB(gamma, toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(const SkColorSpaceTransferFn& coeffs, Gamut gamut) {
    SkMatrix44 toXYZD50;
    to_xyz_d50(&toXYZD50, gamut);
    return SkColorSpace::MakeRGB(coeffs, toXYZD50);
}

class SkColorSpaceSingletonFactory {
public:
    static SkColorSpace* Make(SkGammaNamed gamma, const float to_xyz[9]) {
        SkMatrix44 m44;
        m44.set3x3RowMajorf(to_xyz);
        (void)m44.getType();  // Force typemask to be computed to avoid races.
        return new SkColorSpace(gamma, nullptr, m44);
    }
};

SkColorSpace* sk_srgb_singleton() {
    static SkColorSpace* cs = SkColorSpaceSingletonFactory::Make(kSRGB_SkGammaNamed,
                                                                 gSRGB_toXYZD50);
    return cs;
}
SkColorSpace* sk_srgb_linear_singleton() {
    static SkColorSpace* cs = SkColorSpaceSingletonFactory::Make(kLinear_SkGammaNamed,
                                                                 gSRGB_toXYZD50);
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

bool SkColorSpace::isNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    this->transferFn(&coeffs->fG);
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

sk_sp<SkColorSpace> SkColorSpace::makeLinearGamma() const {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    SkMatrix44 m44;
    this->toXYZD50(&m44);
    return SkColorSpace::MakeRGB(kLinear_SkGammaNamed, m44);
}

sk_sp<SkColorSpace> SkColorSpace::makeSRGBGamma() const {
    if (this->gammaCloseToSRGB()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    SkMatrix44 m44;
    this->toXYZD50(&m44);
    return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, m44);
}

sk_sp<SkColorSpace> SkColorSpace::makeColorSpin() const {
    SkMatrix44 spin;
    spin.set3x3(0, 1, 0, 0, 0, 1, 1, 0, 0);

    SkMatrix44 m44;
    this->toXYZD50(&m44);
    spin.postConcat(m44);

    (void)spin.getType();  // Pre-cache spin matrix type to avoid races in future getType() calls.
    return sk_sp<SkColorSpace>(new SkColorSpace(fGammaNamed, fTransferFn, spin));
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
    SkMatrix44 toXYZD50;
    toXYZD50.set3x3RowMajorf(&profile.toXYZD50.vals[0][0]);
    if (!toXYZD50.invert(nullptr)) {
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
            return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, toXYZD50);
        }
        return nullptr;
    }

    SkColorSpaceTransferFn skia_tf;
    memcpy(&skia_tf, &profile.trc[0].parametric, sizeof(skia_tf));
    return SkColorSpace::MakeRGB(skia_tf, toXYZD50);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

enum Version {
    k0_Version, // Initial version, header + flags for matrix and profile
};

enum NamedColorSpace {
    kSRGB_NamedColorSpace,
    // No longer a singleton, preserved to support reading data from branches m65 and older
    kAdobeRGB_NamedColorSpace,
    kSRGBLinear_NamedColorSpace,
};

struct ColorSpaceHeader {
    /**
     *  It is only valid to set zero or one flags.
     *  Setting multiple flags is invalid.
     */

    /**
     *  If kMatrix_Flag is set, we will write 12 floats after the header.
     */
    static constexpr uint8_t kMatrix_Flag     = 1 << 0;

    /**
     *  If kICC_Flag is set, we will write an ICC profile after the header.
     *  The ICC profile will be written as a uint32 size, followed immediately
     *  by the data (padded to 4 bytes).
     *  DEPRECATED / UNUSED
     */
    static constexpr uint8_t kICC_Flag        = 1 << 1;

    /**
     *  If kTransferFn_Flag is set, we will write 19 floats after the header.
     *  The first seven represent the transfer fn, and the next twelve are the
     *  matrix.
     */
    static constexpr uint8_t kTransferFn_Flag = 1 << 3;

    static ColorSpaceHeader Pack(Version version, uint8_t named, uint8_t gammaNamed, uint8_t flags)
    {
        ColorSpaceHeader header;

        SkASSERT(k0_Version == version);
        header.fVersion = (uint8_t) version;

        SkASSERT(named <= kSRGBLinear_NamedColorSpace);
        header.fNamed = (uint8_t) named;

        SkASSERT(gammaNamed <= kNonStandard_SkGammaNamed);
        header.fGammaNamed = (uint8_t) gammaNamed;

        SkASSERT(flags <= kTransferFn_Flag);
        header.fFlags = flags;
        return header;
    }

    uint8_t fVersion;            // Always zero
    uint8_t fNamed;              // Must be a SkColorSpace::Named
    uint8_t fGammaNamed;         // Must be a SkGammaNamed
    uint8_t fFlags;
};

size_t SkColorSpace::writeToMemory(void* memory) const {
    // If we have a named profile, only write the enum.
    const SkGammaNamed gammaNamed = this->gammaNamed();
    if (this == sk_srgb_singleton()) {
        if (memory) {
            *((ColorSpaceHeader*) memory) = ColorSpaceHeader::Pack(
                    k0_Version, kSRGB_NamedColorSpace, gammaNamed, 0);
        }
        return sizeof(ColorSpaceHeader);
    } else if (this == sk_srgb_linear_singleton()) {
        if (memory) {
            *((ColorSpaceHeader*) memory) = ColorSpaceHeader::Pack(
                    k0_Version, kSRGBLinear_NamedColorSpace, gammaNamed, 0);
        }
        return sizeof(ColorSpaceHeader);
    }

    // If we have a named gamma, write the enum and the matrix.
    switch (gammaNamed) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed: {
            if (memory) {
                *((ColorSpaceHeader*) memory) =
                        ColorSpaceHeader::Pack(k0_Version, 0, gammaNamed,
                                                ColorSpaceHeader::kMatrix_Flag);
                memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));
                SkMatrix44 m44;
                this->toXYZD50(&m44);
                m44.as3x4RowMajorf((float*) memory);
            }
            return sizeof(ColorSpaceHeader) + 12 * sizeof(float);
        }
        default: {
            SkColorSpaceTransferFn transferFn;
            SkAssertResult(this->isNumericalTransferFn(&transferFn));

            if (memory) {
                *((ColorSpaceHeader*) memory) =
                        ColorSpaceHeader::Pack(k0_Version, 0, gammaNamed,
                                                ColorSpaceHeader::kTransferFn_Flag);
                memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));

                *(((float*) memory) + 0) = transferFn.fA;
                *(((float*) memory) + 1) = transferFn.fB;
                *(((float*) memory) + 2) = transferFn.fC;
                *(((float*) memory) + 3) = transferFn.fD;
                *(((float*) memory) + 4) = transferFn.fE;
                *(((float*) memory) + 5) = transferFn.fF;
                *(((float*) memory) + 6) = transferFn.fG;
                memory = SkTAddOffset<void>(memory, 7 * sizeof(float));

                SkMatrix44 m44;
                this->toXYZD50(&m44);
                m44.as3x4RowMajorf((float*) memory);
            }

            return sizeof(ColorSpaceHeader) + 19 * sizeof(float);
        }
    }
}

sk_sp<SkData> SkColorSpace::serialize() const {
    size_t size = this->writeToMemory(nullptr);
    if (0 == size) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
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
    if (0 == header.fFlags) {
        switch ((NamedColorSpace)header.fNamed) {
            case kSRGB_NamedColorSpace:
                return SkColorSpace::MakeSRGB();
            case kSRGBLinear_NamedColorSpace:
                return SkColorSpace::MakeSRGBLinear();
            case kAdobeRGB_NamedColorSpace:
                return SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);
        }
    }

    switch ((SkGammaNamed) header.fGammaNamed) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed: {
            if (ColorSpaceHeader::kMatrix_Flag != header.fFlags || length < 12 * sizeof(float)) {
                return nullptr;
            }

            SkMatrix44 toXYZ;
            toXYZ.set3x4RowMajorf((const float*) data);
            return SkColorSpace::MakeRGB((SkGammaNamed) header.fGammaNamed, toXYZ);
        }
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

            SkColorSpaceTransferFn transferFn;
            transferFn.fA = *(((const float*) data) + 0);
            transferFn.fB = *(((const float*) data) + 1);
            transferFn.fC = *(((const float*) data) + 2);
            transferFn.fD = *(((const float*) data) + 3);
            transferFn.fE = *(((const float*) data) + 4);
            transferFn.fF = *(((const float*) data) + 5);
            transferFn.fG = *(((const float*) data) + 6);
            data = SkTAddOffset<const void>(data, 7 * sizeof(float));

            SkMatrix44 toXYZ;
            toXYZ.set3x4RowMajorf((const float*) data);
            return SkColorSpace::MakeRGB(transferFn, toXYZ);
        }
        default:
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
