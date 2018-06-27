/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkPoint3.h"

bool SkColorSpacePrimaries::toXYZD50(SkMatrix44* toXYZ_D50) const {
    if (!is_zero_to_one(fRX) || !is_zero_to_one(fRY) ||
        !is_zero_to_one(fGX) || !is_zero_to_one(fGY) ||
        !is_zero_to_one(fBX) || !is_zero_to_one(fBY) ||
        !is_zero_to_one(fWX) || !is_zero_to_one(fWY))
    {
        return false;
    }

    // First, we need to convert xy values (primaries) to XYZ.
    SkMatrix primaries;
    primaries.setAll(             fRX,              fGX,              fBX,
                                  fRY,              fGY,              fBY,
                     1.0f - fRX - fRY, 1.0f - fGX - fGY, 1.0f - fBX - fBY);
    SkMatrix primariesInv;
    if (!primaries.invert(&primariesInv)) {
        return false;
    }

    // Assumes that Y is 1.0f.
    SkVector3 wXYZ = SkVector3::Make(fWX / fWY, 1.0f, (1.0f - fWX - fWY) / fWY);
    SkVector3 XYZ;
    XYZ.fX = primariesInv[0] * wXYZ.fX + primariesInv[1] * wXYZ.fY + primariesInv[2] * wXYZ.fZ;
    XYZ.fY = primariesInv[3] * wXYZ.fX + primariesInv[4] * wXYZ.fY + primariesInv[5] * wXYZ.fZ;
    XYZ.fZ = primariesInv[6] * wXYZ.fX + primariesInv[7] * wXYZ.fY + primariesInv[8] * wXYZ.fZ;
    SkMatrix toXYZ;
    toXYZ.setAll(XYZ.fX,   0.0f,   0.0f,
                   0.0f, XYZ.fY,   0.0f,
                   0.0f,   0.0f, XYZ.fZ);
    toXYZ.postConcat(primaries);

    // Now convert toXYZ matrix to toXYZD50.
    SkVector3 wXYZD50 = SkVector3::Make(0.96422f, 1.0f, 0.82521f);

    // Calculate the chromatic adaptation matrix.  We will use the Bradford method, thus
    // the matrices below.  The Bradford method is used by Adobe and is widely considered
    // to be the best.
    SkMatrix mA, mAInv;
    mA.setAll(+0.8951f, +0.2664f, -0.1614f,
              -0.7502f, +1.7135f, +0.0367f,
              +0.0389f, -0.0685f, +1.0296f);
    mAInv.setAll(+0.9869929f, -0.1470543f, +0.1599627f,
                 +0.4323053f, +0.5183603f, +0.0492912f,
                 -0.0085287f, +0.0400428f, +0.9684867f);

    SkVector3 srcCone;
    srcCone.fX = mA[0] * wXYZ.fX + mA[1] * wXYZ.fY + mA[2] * wXYZ.fZ;
    srcCone.fY = mA[3] * wXYZ.fX + mA[4] * wXYZ.fY + mA[5] * wXYZ.fZ;
    srcCone.fZ = mA[6] * wXYZ.fX + mA[7] * wXYZ.fY + mA[8] * wXYZ.fZ;
    SkVector3 dstCone;
    dstCone.fX = mA[0] * wXYZD50.fX + mA[1] * wXYZD50.fY + mA[2] * wXYZD50.fZ;
    dstCone.fY = mA[3] * wXYZD50.fX + mA[4] * wXYZD50.fY + mA[5] * wXYZD50.fZ;
    dstCone.fZ = mA[6] * wXYZD50.fX + mA[7] * wXYZD50.fY + mA[8] * wXYZD50.fZ;

    SkMatrix DXToD50;
    DXToD50.setIdentity();
    DXToD50[0] = dstCone.fX / srcCone.fX;
    DXToD50[4] = dstCone.fY / srcCone.fY;
    DXToD50[8] = dstCone.fZ / srcCone.fZ;
    DXToD50.postConcat(mAInv);
    DXToD50.preConcat(mA);

    toXYZ.postConcat(DXToD50);
    toXYZ_D50->set3x3(toXYZ[0], toXYZ[3], toXYZ[6],
                      toXYZ[1], toXYZ[4], toXYZ[7],
                      toXYZ[2], toXYZ[5], toXYZ[8]);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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
#ifdef SK_SUPPORT_LEGACY_ADOBE_XYZ
        case k2Dot2Curve_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gAdobeRGB_toXYZD50)) {
                SkMatrix44 adobe44(SkMatrix44::kUninitialized_Constructor);
                adobe44.set3x3RowMajorf(gAdobeRGB_toXYZD50);
                return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(k2Dot2Curve_SkGammaNamed, adobe44));
            }
            break;
#endif
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

    return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(gammaNamed, toXYZD50));
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

    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(SkColorSpaceTransferFn));
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));
    SkColorSpaceTransferFn* fn = SkTAddOffset<SkColorSpaceTransferFn>(memory, sizeof(SkGammas));
    *fn = coeffs;
    SkGammas::Data data;
    data.fParamOffset = 0;
    for (int channel = 0; channel < 3; ++channel) {
        gammas->fType[channel] = SkGammas::Type::kParam_Type;
        gammas->fData[channel] = data;
    }
    return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(kNonStandard_SkGammaNamed,
                                                    std::move(gammas), toXYZD50, nullptr));
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

static SkColorSpace* singleton_colorspace(SkGammaNamed gamma, const float to_xyz[9]) {
    SkMatrix44 m44(SkMatrix44::kUninitialized_Constructor);
    m44.set3x3RowMajorf(to_xyz);
    (void)m44.getType();  // Force typemask to be computed to avoid races.
    return new SkColorSpace_XYZ(gamma, m44);
}

static SkColorSpace* srgb() {
    static SkColorSpace* cs = singleton_colorspace(kSRGB_SkGammaNamed, gSRGB_toXYZD50);
    return cs;
}
static SkColorSpace* srgb_linear() {
    static SkColorSpace* cs = singleton_colorspace(kLinear_SkGammaNamed, gSRGB_toXYZD50);
    return cs;
}

sk_sp<SkColorSpace> SkColorSpace::MakeSRGB() {
    return sk_ref_sp(srgb());
}

sk_sp<SkColorSpace> SkColorSpace::MakeSRGBLinear() {
    return sk_ref_sp(srgb_linear());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorSpace::Type SkColorSpace::type() const {
    const SkMatrix44* m = this->toXYZD50();
    if (m) {
        return m->isScale() ? kGray_Type : kRGB_Type;
    }
    return this->onIsCMYK() ? kCMYK_Type : kRGB_Type;
}

SkGammaNamed SkColorSpace::gammaNamed() const {
    return this->onGammaNamed();
}

bool SkColorSpace::gammaCloseToSRGB() const {
    return this->onGammaCloseToSRGB();
}

bool SkColorSpace::gammaIsLinear() const {
    return this->onGammaIsLinear();
}

bool SkColorSpace::isNumericalTransferFn(SkColorSpaceTransferFn* fn) const {
    return this->onIsNumericalTransferFn(fn);
}

bool SkColorSpace::toXYZD50(SkMatrix44* toXYZD50) const {
    const SkMatrix44* matrix = this->onToXYZD50();
    if (matrix) {
        *toXYZD50 = *matrix;
        return true;
    }

    return false;
}

const SkMatrix44* SkColorSpace::toXYZD50() const {
    return this->onToXYZD50();
}

const SkMatrix44* SkColorSpace::fromXYZD50() const {
    return this->onFromXYZD50();
}

uint32_t SkColorSpace::toXYZD50Hash() const {
    return this->onToXYZD50Hash();
}

bool SkColorSpace::isSRGB() const {
    return srgb() == this;
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
    // Start by trying the serialization fast path.  If we haven't saved ICC profile data,
    // we must have a profile that we can serialize easily.
    if (!this->onProfileData()) {
        // Profile data is mandatory for A2B0 color spaces, so we must be XYZ.
        SkASSERT(this->toXYZD50());
        // If we have a named profile, only write the enum.
        const SkGammaNamed gammaNamed = this->gammaNamed();
        if (this == srgb()) {
            if (memory) {
                *((ColorSpaceHeader*) memory) = ColorSpaceHeader::Pack(
                        k0_Version, kSRGB_NamedColorSpace, gammaNamed, 0);
            }
            return sizeof(ColorSpaceHeader);
        } else if (this == srgb_linear()) {
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

    // Otherwise, serialize the ICC data.
    size_t profileSize = this->onProfileData()->size();
    if (SkAlign4(profileSize) != (uint32_t) SkAlign4(profileSize)) {
        return 0;
    }

    if (memory) {
        *((ColorSpaceHeader*) memory) = ColorSpaceHeader::Pack(k0_Version, 0,
                                                               kNonStandard_SkGammaNamed,
                                                               ColorSpaceHeader::kICC_Flag);
        memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));

        *((uint32_t*) memory) = (uint32_t) SkAlign4(profileSize);
        memory = SkTAddOffset<void>(memory, sizeof(uint32_t));

        memcpy(memory, this->onProfileData()->data(), profileSize);
        memset(SkTAddOffset<void>(memory, profileSize), 0, SkAlign4(profileSize) - profileSize);
    }
    return sizeof(ColorSpaceHeader) + sizeof(uint32_t) + SkAlign4(profileSize);
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
            if (length < sizeof(uint32_t)) {
                return nullptr;
            }

            uint32_t profileSize = *((uint32_t*) data);
            data = SkTAddOffset<const void>(data, sizeof(uint32_t));
            length -= sizeof(uint32_t);
            if (length < profileSize) {
                return nullptr;
            }

            return MakeICC(data, profileSize);
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

    const SkData* srcData = src->onProfileData();
    const SkData* dstData = dst->onProfileData();
    if (srcData || dstData) {
        if (srcData && dstData) {
            return srcData->size() == dstData->size() &&
                   0 == memcmp(srcData->data(), dstData->data(), srcData->size());
        }

        return false;
    }

    // Profiles are mandatory for A2B0 color spaces, so these must be XYZ
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
