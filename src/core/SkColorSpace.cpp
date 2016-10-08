/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpacePriv.h"
#include "SkOnce.h"

SkColorSpace_Base::SkColorSpace_Base(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50)
    : fGammaNamed(gammaNamed)
    , fGammas(nullptr)
    , fProfileData(nullptr)
    , fToXYZD50(toXYZD50)
    , fFromXYZD50(SkMatrix44::kUninitialized_Constructor)
{}

SkColorSpace_Base::SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, SkGammaNamed gammaNamed,
                                     sk_sp<SkGammas> gammas, const SkMatrix44& toXYZD50,
                                     sk_sp<SkData> profileData)
    : fColorLUT(std::move(colorLUT))
    , fGammaNamed(gammaNamed)
    , fGammas(std::move(gammas))
    , fProfileData(std::move(profileData))
    , fToXYZD50(toXYZD50)
    , fFromXYZD50(SkMatrix44::kUninitialized_Constructor)
{}

static constexpr float gSRGB_toXYZD50[] {
    0.4358f, 0.3853f, 0.1430f,    // Rx, Gx, Bx
    0.2224f, 0.7170f, 0.0606f,    // Ry, Gy, Gz
    0.0139f, 0.0971f, 0.7139f,    // Rz, Gz, Bz
};

static constexpr float gAdobeRGB_toXYZD50[] {
    0.6098f, 0.2052f, 0.1492f,    // Rx, Gx, Bx
    0.3111f, 0.6257f, 0.0632f,    // Ry, Gy, By
    0.0195f, 0.0609f, 0.7448f,    // Rz, Gz, Bz
};

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

sk_sp<SkColorSpace> SkColorSpace_Base::NewRGB(const float values[3], const SkMatrix44& toXYZD50) {
    if (0.0f > values[0] || 0.0f > values[1] || 0.0f > values[2]) {
        return nullptr;
    }

    SkGammaNamed gammaNamed = kNonStandard_SkGammaNamed;
    if (color_space_almost_equal(2.2f, values[0]) &&
            color_space_almost_equal(2.2f, values[1]) &&
            color_space_almost_equal(2.2f, values[2])) {
        gammaNamed = k2Dot2Curve_SkGammaNamed;
    } else if (color_space_almost_equal(1.0f, values[0]) &&
            color_space_almost_equal(1.0f, values[1]) &&
            color_space_almost_equal(1.0f, values[2])) {
        gammaNamed = kLinear_SkGammaNamed;
    }

    if (kNonStandard_SkGammaNamed == gammaNamed) {
        sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
        gammas->fRedType = SkGammas::Type::kValue_Type;
        gammas->fGreenType = SkGammas::Type::kValue_Type;
        gammas->fBlueType = SkGammas::Type::kValue_Type;
        gammas->fRedData.fValue = values[0];
        gammas->fGreenData.fValue = values[1];
        gammas->fBlueData.fValue = values[2];
        return sk_sp<SkColorSpace>(new SkColorSpace_Base(nullptr, kNonStandard_SkGammaNamed, gammas,
                                                         toXYZD50, nullptr));
    }

    return SkColorSpace_Base::NewRGB(gammaNamed, toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace_Base::NewRGB(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50) {
    switch (gammaNamed) {
        case kSRGB_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gSRGB_toXYZD50)) {
                return SkColorSpace::NewNamed(kSRGB_Named);
            }
            break;
        case k2Dot2Curve_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gAdobeRGB_toXYZD50)) {
                return SkColorSpace::NewNamed(kAdobeRGB_Named);
            }
            break;
        case kLinear_SkGammaNamed:
            if (xyz_almost_equal(toXYZD50, gSRGB_toXYZD50)) {
                return SkColorSpace::NewNamed(kSRGBLinear_Named);
            }
            break;
        case kNonStandard_SkGammaNamed:
            // This is not allowed.
            return nullptr;
        default:
            break;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace_Base(gammaNamed, toXYZD50));
}

sk_sp<SkColorSpace> SkColorSpace::NewRGB(RenderTargetGamma gamma, const SkMatrix44& toXYZD50) {
    switch (gamma) {
        case kLinear_RenderTargetGamma:
            return SkColorSpace_Base::NewRGB(kLinear_SkGammaNamed, toXYZD50);
        case kSRGB_RenderTargetGamma:
            return SkColorSpace_Base::NewRGB(kSRGB_SkGammaNamed, toXYZD50);
        default:
            return nullptr;
    }
}

static SkColorSpace* gAdobeRGB;
static SkColorSpace* gSRGB;
static SkColorSpace* gSRGBLinear;

sk_sp<SkColorSpace> SkColorSpace::NewNamed(Named named) {
    static SkOnce sRGBOnce;
    static SkOnce adobeRGBOnce;
    static SkOnce sRGBLinearOnce;

    switch (named) {
        case kSRGB_Named: {
            sRGBOnce([] {
                SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);

                // Force the mutable type mask to be computed.  This avoids races.
                (void)srgbToxyzD50.getType();
                gSRGB = new SkColorSpace_Base(kSRGB_SkGammaNamed, srgbToxyzD50);
            });
            return sk_ref_sp<SkColorSpace>(gSRGB);
        }
        case kAdobeRGB_Named: {
            adobeRGBOnce([] {
                SkMatrix44 adobergbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                adobergbToxyzD50.set3x3RowMajorf(gAdobeRGB_toXYZD50);

                // Force the mutable type mask to be computed.  This avoids races.
                (void)adobergbToxyzD50.getType();
                gAdobeRGB = new SkColorSpace_Base(k2Dot2Curve_SkGammaNamed, adobergbToxyzD50);
            });
            return sk_ref_sp<SkColorSpace>(gAdobeRGB);
        }
        case kSRGBLinear_Named: {
            sRGBLinearOnce([] {
                SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);

                // Force the mutable type mask to be computed.  This avoids races.
                (void)srgbToxyzD50.getType();
                gSRGBLinear = new SkColorSpace_Base(kLinear_SkGammaNamed, srgbToxyzD50);
            });
            return sk_ref_sp<SkColorSpace>(gSRGBLinear);
        }
        default:
            break;
    }
    return nullptr;
}

sk_sp<SkColorSpace> SkColorSpace::makeLinearGamma() {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(this);
    }
    return SkColorSpace_Base::NewRGB(kLinear_SkGammaNamed, as_CSB(this)->fToXYZD50);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkColorSpace::gammaCloseToSRGB() const {
    return kSRGB_SkGammaNamed == as_CSB(this)->fGammaNamed ||
           k2Dot2Curve_SkGammaNamed == as_CSB(this)->fGammaNamed;
}

bool SkColorSpace::gammaIsLinear() const {
    return kLinear_SkGammaNamed == as_CSB(this)->fGammaNamed;
}

const SkMatrix44& SkColorSpace_Base::fromXYZD50() const {
    fFromXYZOnce([this] {
        if (!fToXYZD50.invert(&fFromXYZD50)) {
            // If a client gives us a dst gamut with a transform that we can't invert, we will
            // simply give them back a transform to sRGB gamut.
            SkDEBUGFAIL("Non-invertible XYZ matrix, defaulting to sRGB");
            SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
            srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);
            srgbToxyzD50.invert(&fFromXYZD50);
        }
    });
    return fFromXYZD50;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

enum Version {
    k0_Version, // Initial version, header + flags for matrix and profile
};

struct ColorSpaceHeader {
    /**
     *  If kMatrix_Flag is set, we will write 12 floats after the header.
     *  Should not be set at the same time as the kICC_Flag or kFloatGamma_Flag.
     */
    static constexpr uint8_t kMatrix_Flag     = 1 << 0;

    /**
     *  If kICC_Flag is set, we will write an ICC profile after the header.
     *  The ICC profile will be written as a uint32 size, followed immediately
     *  by the data (padded to 4 bytes).
     *  Should not be set at the same time as the kMatrix_Flag or kFloatGamma_Flag.
     */
    static constexpr uint8_t kICC_Flag        = 1 << 1;

    /**
     *  If kFloatGamma_Flag is set, we will write 15 floats after the header.
     *  The first three are the gamma values, and the next twelve are the
     *  matrix.
     *  Should not be set at the same time as the kICC_Flag or kMatrix_Flag.
     */
    static constexpr uint8_t kFloatGamma_Flag = 1 << 2;

    static ColorSpaceHeader Pack(Version version, uint8_t named, uint8_t gammaNamed, uint8_t flags)
    {
        ColorSpaceHeader header;

        SkASSERT(k0_Version == version);
        header.fVersion = (uint8_t) version;

        SkASSERT(named <= SkColorSpace::kSRGBLinear_Named);
        header.fNamed = (uint8_t) named;

        SkASSERT(gammaNamed <= kNonStandard_SkGammaNamed);
        header.fGammaNamed = (uint8_t) gammaNamed;

        SkASSERT(flags <= kFloatGamma_Flag);
        header.fFlags = flags;
        return header;
    }

    uint8_t fVersion;    // Always zero
    uint8_t fNamed;      // Must be a SkColorSpace::Named
    uint8_t fGammaNamed; // Must be a SkGammaNamed
    uint8_t fFlags;      // Some combination of the flags listed above
};

size_t SkColorSpace::writeToMemory(void* memory) const {
    // Start by trying the serialization fast path.  If we haven't saved ICC profile data,
    // we must have a profile that we can serialize easily.
    if (!as_CSB(this)->fProfileData) {
        // If we have a named profile, only write the enum.
        if (this == gSRGB) {
            if (memory) {
                *((ColorSpaceHeader*) memory) =
                        ColorSpaceHeader::Pack(k0_Version, kSRGB_Named,
                                               as_CSB(this)->fGammaNamed, 0);
            }
            return sizeof(ColorSpaceHeader);
        } else if (this == gAdobeRGB) {
            if (memory) {
                *((ColorSpaceHeader*) memory) =
                        ColorSpaceHeader::Pack(k0_Version, kAdobeRGB_Named,
                                               as_CSB(this)->fGammaNamed, 0);
            }
            return sizeof(ColorSpaceHeader);
        } else if (this == gSRGBLinear) {
            if (memory) {
                *((ColorSpaceHeader*)memory) =
                        ColorSpaceHeader::Pack(k0_Version, kSRGBLinear_Named,
                                               as_CSB(this)->fGammaNamed, 0);
            }
            return sizeof(ColorSpaceHeader);
        }

        // If we have a named gamma, write the enum and the matrix.
        switch (as_CSB(this)->fGammaNamed) {
            case kSRGB_SkGammaNamed:
            case k2Dot2Curve_SkGammaNamed:
            case kLinear_SkGammaNamed: {
                if (memory) {
                    *((ColorSpaceHeader*) memory) =
                            ColorSpaceHeader::Pack(k0_Version, 0, as_CSB(this)->fGammaNamed,
                                                   ColorSpaceHeader::kMatrix_Flag);
                    memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));
                    as_CSB(this)->fToXYZD50.as3x4RowMajorf((float*) memory);
                }
                return sizeof(ColorSpaceHeader) + 12 * sizeof(float);
            }
            default:
                // Otherwise, write the gamma values and the matrix.
                if (memory) {
                    *((ColorSpaceHeader*) memory) =
                            ColorSpaceHeader::Pack(k0_Version, 0, as_CSB(this)->fGammaNamed,
                                                   ColorSpaceHeader::kFloatGamma_Flag);
                    memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));

                    const SkGammas* gammas = as_CSB(this)->gammas();
                    SkASSERT(gammas);
                    SkASSERT(SkGammas::Type::kValue_Type == gammas->fRedType &&
                             SkGammas::Type::kValue_Type == gammas->fGreenType &&
                             SkGammas::Type::kValue_Type == gammas->fBlueType);
                    *(((float*) memory) + 0) = gammas->fRedData.fValue;
                    *(((float*) memory) + 1) = gammas->fGreenData.fValue;
                    *(((float*) memory) + 2) = gammas->fBlueData.fValue;
                    memory = SkTAddOffset<void>(memory, 3 * sizeof(float));

                    as_CSB(this)->fToXYZD50.as3x4RowMajorf((float*) memory);
                }
                return sizeof(ColorSpaceHeader) + 15 * sizeof(float);
        }
    }

    // Otherwise, serialize the ICC data.
    size_t profileSize = as_CSB(this)->fProfileData->size();
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

        memcpy(memory, as_CSB(this)->fProfileData->data(), profileSize);
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
        return NewNamed((Named) header.fNamed);
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
            return SkColorSpace_Base::NewRGB((SkGammaNamed) header.fGammaNamed, toXYZ);
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

            return NewICC(data, profileSize);
        }
        case ColorSpaceHeader::kFloatGamma_Flag: {
            if (length < 15 * sizeof(float)) {
                return nullptr;
            }

            float gammas[3];
            gammas[0] = *(((const float*) data) + 0);
            gammas[1] = *(((const float*) data) + 1);
            gammas[2] = *(((const float*) data) + 2);
            data = SkTAddOffset<const void>(data, 3 * sizeof(float));

            SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
            toXYZ.set3x4RowMajorf((const float*) data);
            return SkColorSpace_Base::NewRGB(gammas, toXYZ);
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

    SkData* srcData = as_CSB(src)->fProfileData.get();
    SkData* dstData = as_CSB(dst)->fProfileData.get();
    if (srcData || dstData) {
        if (srcData && dstData) {
            return srcData->size() == dstData->size() &&
                   0 == memcmp(srcData->data(), dstData->data(), srcData->size());
        }

        return false;
    }

    // It's important to check fProfileData before named gammas.  Some profiles may have named
    // gammas, but also include other wacky features that cause us to save the data.
    switch (as_CSB(src)->fGammaNamed) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed:
            return (as_CSB(src)->fGammaNamed == as_CSB(dst)->fGammaNamed) &&
                   (as_CSB(src)->fToXYZD50 == as_CSB(dst)->fToXYZD50);
        default:
            if (as_CSB(src)->fGammaNamed != as_CSB(dst)->fGammaNamed) {
                return false;
            }

            // It is unlikely that we will reach this case.
            sk_sp<SkData> srcData = src->serialize();
            sk_sp<SkData> dstData = dst->serialize();
            return srcData->size() == dstData->size() &&
                   0 == memcmp(srcData->data(), dstData->data(), srcData->size());
    }
}
