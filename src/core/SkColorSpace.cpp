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

// TODO: this is kind of ridiculous
static_assert(sizeof(SkColorSpace) == 45*4, "");

bool SkColorSpacePrimaries::toXYZD50(SkMatrix44* toXYZ_D50) const {
    skcms_Matrix3x3 toXYZ;
    if (!skcms_PrimariesToXYZD50(fRX, fRY, fGX, fGY, fBX, fBY, fWX, fWY, &toXYZ)) {
        return false;
    }
    toXYZ_D50->set3x3RowMajorf(&toXYZ.vals[0][0]);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorSpace::SkColorSpace(SkGammaNamed gammaNamed,
                           const SkColorSpaceTransferFn* transferFn,
                           const SkMatrix44& toXYZD50)
        : fGammaNamed(gammaNamed)
        , fToXYZD50(toXYZD50)
        , fToXYZD50Hash(SkOpts::hash_fn(toXYZD50.values(), 16 * sizeof(SkMScalar), 0))
        , fFromXYZD50(SkMatrix44::kUninitialized_Constructor) {
    SkASSERT(fGammaNamed != kNonStandard_SkGammaNamed || transferFn);
    if (transferFn) {
        fTransferFn = *transferFn;
    }
}

/**
 *  Checks if our toXYZ matrix is a close match to a known color gamut.
 *
 *  @param toXYZD50 transformation matrix deduced from profile data
 *  @param standard 3x3 canonical transformation matrix
 */
static bool xyz_almost_equal(const SkMatrix44& toXYZD50, const float* standard) {
    return color_space_almost_equal(toXYZD50.getFloat(0, 0), standard[0]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 1), standard[1]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 2), standard[2]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 0), standard[3]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 1), standard[4]) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 2), standard[5]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 0), standard[6]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 1), standard[7]) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 2), standard[8]) &&
           color_space_almost_equal(toXYZD50.getFloat(0, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(1, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(2, 3), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 0), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 1), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 2), 0.0f) &&
           color_space_almost_equal(toXYZD50.getFloat(3, 3), 1.0f);
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50)
{
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

    return sk_sp<SkColorSpace>(new SkColorSpace(kNonStandard_SkGammaNamed, &coeffs, toXYZD50));
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(RenderTargetGamma gamma, Gamut gamut) {
    SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
    to_xyz_d50(&toXYZD50, gamut);
    return SkColorSpace::MakeRGB(gamma, toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::MakeRGB(const SkColorSpaceTransferFn& coeffs, Gamut gamut) {
    SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
    to_xyz_d50(&toXYZD50, gamut);
    return SkColorSpace::MakeRGB(coeffs, toXYZD50);
}

class SkColorSpaceSingletonFactory {
public:
    static SkColorSpace* Make(SkGammaNamed gamma, const float to_xyz[9]) {
        SkMatrix44 m44(SkMatrix44::kUninitialized_Constructor);
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

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkColorSpace::isNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    switch (fGammaNamed) {
        case kSRGB_SkGammaNamed:
            *coeffs = gSRGB_TransferFn;
            break;
        case k2Dot2Curve_SkGammaNamed:
            *coeffs = g2Dot2_TransferFn;
            break;
        case kLinear_SkGammaNamed:
            *coeffs = gLinear_TransferFn;
            break;
        case kNonStandard_SkGammaNamed:
            *coeffs = fTransferFn;
            break;
        default:
            SkDEBUGFAIL("Unknown named gamma");
            return false;
    }

    return true;
}

bool SkColorSpace::toXYZD50(SkMatrix44* toXYZD50) const {
    const SkMatrix44* matrix = this->toXYZD50();
    if (matrix) {
        *toXYZD50 = *matrix;
        return true;
    }

    return false;
}

const SkMatrix44* SkColorSpace::fromXYZD50() const {
    fFromXYZOnce([this] {
        if (!fToXYZD50.invert(&fFromXYZD50)) {
            // If a client gives us a dst gamut with a transform that we can't invert, we will
            // simply give them back a transform to sRGB gamut.
            SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
            srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);
            srgbToxyzD50.invert(&fFromXYZD50);
        }
    });
    return &fFromXYZD50;
}

bool SkColorSpace::isSRGB() const {
    return sk_srgb_singleton() == this;
}

sk_sp<SkColorSpace> SkColorSpace::makeLinearGamma() const {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    return SkColorSpace::MakeRGB(kLinear_SkGammaNamed, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::makeSRGBGamma() const {
    if (this->gammaCloseToSRGB()) {
        return sk_ref_sp(const_cast<SkColorSpace*>(this));
    }
    return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::makeColorSpin() const {
    SkMatrix44 spin(SkMatrix44::kUninitialized_Constructor);
    spin.set3x3(0, 1, 0, 0, 0, 1, 1, 0, 0);
    spin.postConcat(fToXYZD50);
    (void)spin.getType();  // Pre-cache spin matrix type to avoid races in future getType() calls.
    return sk_sp<SkColorSpace>(new SkColorSpace(fGammaNamed, &fTransferFn, spin));
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
                this->toXYZD50()->as3x4RowMajorf((float*) memory);
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

                this->toXYZD50()->as3x4RowMajorf((float*) memory);
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

            SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
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

            SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
            toXYZ.set3x4RowMajorf((const float*) data);
            return SkColorSpace::MakeRGB(transferFn, toXYZ);
        }
        default:
            return nullptr;
    }
}

bool SkColorSpace::Equals(const SkColorSpace* src, const SkColorSpace* dst) {
    if (src == dst) {
        return true;
    }

    if (!src || !dst) {
        return false;
    }

    if (src->gammaNamed() != dst->gammaNamed()) {
        return false;
    }

    switch (src->gammaNamed()) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed:
            if (src->toXYZD50Hash() == dst->toXYZD50Hash()) {
                SkASSERT(*src->toXYZD50() == *dst->toXYZD50() && "Hash collision");
                return true;
            }
            return false;
        default:
            // It is unlikely that we will reach this case.
            // TODO: Simplify this case now that color spaces have one representation.
            sk_sp<SkData> serializedSrcData = src->serialize();
            sk_sp<SkData> serializedDstData = dst->serialize();
            return serializedSrcData->size() == serializedDstData->size() &&
                   0 == memcmp(serializedSrcData->data(), serializedDstData->data(),
                               serializedSrcData->size());
    }
}

SkColorSpaceTransferFn SkColorSpaceTransferFn::invert() const {
    // Original equation is:       y = (ax + b)^g + e   for x >= d
    //                             y = cx + f           otherwise
    //
    // so 1st inverse is:          (y - e)^(1/g) = ax + b
    //                             x = ((y - e)^(1/g) - b) / a
    //
    // which can be re-written as: x = (1/a)(y - e)^(1/g) - b/a
    //                             x = ((1/a)^g)^(1/g) * (y - e)^(1/g) - b/a
    //                             x = ([(1/a)^g]y + [-((1/a)^g)e]) ^ [1/g] + [-b/a]
    //
    // and 2nd inverse is:         x = (y - f) / c
    // which can be re-written as: x = [1/c]y + [-f/c]
    //
    // and now both can be expressed in terms of the same parametric form as the
    // original - parameters are enclosed in square brackets.
    SkColorSpaceTransferFn inv = { 0, 0, 0, 0, 0, 0, 0 };

    // find inverse for linear segment (if possible)
    if (!transfer_fn_almost_equal(0.f, fC)) {
        inv.fC = 1.f / fC;
        inv.fF = -fF / fC;
    } else {
        // otherwise assume it should be 0 as it is the lower segment
        // as y = f is a constant function
    }

    // find inverse for the other segment (if possible)
    if (transfer_fn_almost_equal(0.f, fA) || transfer_fn_almost_equal(0.f, fG)) {
        // otherwise assume it should be 1 as it is the top segment
        // as you can't invert the constant functions y = b^g + e, or y = 1 + e
        inv.fG = 1.f;
        inv.fE = 1.f;
    } else {
        inv.fG = 1.f / fG;
        inv.fA = powf(1.f / fA, fG);
        inv.fB = -inv.fA * fE;
        inv.fE = -fB / fA;
    }
    inv.fD = fC * fD + fF;

    return inv;
}
