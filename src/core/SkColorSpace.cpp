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

SkColorSpace::SkColorSpace(GammaNamed gammaNamed, const SkMatrix44& toXYZD50, Named named)
    : fGammaNamed(gammaNamed)
    , fToXYZD50(toXYZD50)
    , fNamed(named)
{}

SkColorSpace_Base::SkColorSpace_Base(GammaNamed gammaNamed, const SkMatrix44& toXYZD50, Named named)
    : INHERITED(gammaNamed, toXYZD50, named)
    , fGammas(nullptr)
    , fProfileData(nullptr)
{}

SkColorSpace_Base::SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, GammaNamed gammaNamed,
                                     sk_sp<SkGammas> gammas, const SkMatrix44& toXYZD50,
                                     sk_sp<SkData> profileData)
    : INHERITED(gammaNamed, toXYZD50, kUnknown_Named)
    , fColorLUT(std::move(colorLUT))
    , fGammas(std::move(gammas))
    , fProfileData(std::move(profileData))
{}

static constexpr float gSRGB_toXYZD50[] {
    0.4358f, 0.2224f, 0.0139f,    // * R
    0.3853f, 0.7170f, 0.0971f,    // * G
    0.1430f, 0.0606f, 0.7139f,    // * B
};

static constexpr float gAdobeRGB_toXYZD50[] {
    0.6098f, 0.3111f, 0.0195f,    // * R
    0.2052f, 0.6257f, 0.0609f,    // * G
    0.1492f, 0.0632f, 0.7448f,    // * B
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

    GammaNamed gammaNamed = kNonStandard_GammaNamed;
    if (color_space_almost_equal(2.2f, values[0]) &&
            color_space_almost_equal(2.2f, values[1]) &&
            color_space_almost_equal(2.2f, values[2])) {
        gammaNamed = k2Dot2Curve_GammaNamed;
    } else if (color_space_almost_equal(1.0f, values[0]) &&
            color_space_almost_equal(1.0f, values[1]) &&
            color_space_almost_equal(1.0f, values[2])) {
        gammaNamed = kLinear_GammaNamed;
    }

    if (kNonStandard_GammaNamed == gammaNamed) {
        sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
        gammas->fRedType = SkGammas::Type::kValue_Type;
        gammas->fGreenType = SkGammas::Type::kValue_Type;
        gammas->fBlueType = SkGammas::Type::kValue_Type;
        gammas->fRedData.fValue = values[0];
        gammas->fGreenData.fValue = values[1];
        gammas->fBlueData.fValue = values[2];
        return sk_sp<SkColorSpace>(new SkColorSpace_Base(nullptr, kNonStandard_GammaNamed, gammas,
                                                         toXYZD50, nullptr));
    }

    return SkColorSpace_Base::NewRGB(gammaNamed, toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace_Base::NewRGB(GammaNamed gammaNamed, const SkMatrix44& toXYZD50) {
    switch (gammaNamed) {
        case kSRGB_GammaNamed:
            if (xyz_almost_equal(toXYZD50, gSRGB_toXYZD50)) {
                return SkColorSpace::NewNamed(kSRGB_Named);
            }
            break;
        case k2Dot2Curve_GammaNamed:
            if (xyz_almost_equal(toXYZD50, gAdobeRGB_toXYZD50)) {
                return SkColorSpace::NewNamed(kAdobeRGB_Named);
            }
            break;
        case kNonStandard_GammaNamed:
            // This is not allowed.
            return nullptr;
        default:
            break;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace_Base(gammaNamed, toXYZD50, kUnknown_Named));
}

sk_sp<SkColorSpace> SkColorSpace::NewRGB(GammaNamed gammaNamed, const SkMatrix44& toXYZD50) {
    return SkColorSpace_Base::NewRGB(gammaNamed, toXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace::NewNamed(Named named) {
    static SkOnce sRGBOnce;
    static sk_sp<SkColorSpace> sRGB;
    static SkOnce adobeRGBOnce;
    static sk_sp<SkColorSpace> adobeRGB;

    switch (named) {
        case kSRGB_Named: {
            sRGBOnce([] {
                SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);

                // Force the mutable type mask to be computed.  This avoids races.
                (void)srgbToxyzD50.getType();
                sRGB.reset(new SkColorSpace_Base(kSRGB_GammaNamed, srgbToxyzD50, kSRGB_Named));
            });
            return sRGB;
        }
        case kAdobeRGB_Named: {
            adobeRGBOnce([] {
                SkMatrix44 adobergbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                adobergbToxyzD50.set3x3RowMajorf(gAdobeRGB_toXYZD50);

                // Force the mutable type mask to be computed.  This avoids races.
                (void)adobergbToxyzD50.getType();
                adobeRGB.reset(new SkColorSpace_Base(k2Dot2Curve_GammaNamed, adobergbToxyzD50,
                                                     kAdobeRGB_Named));
            });
            return adobeRGB;
        }
        default:
            break;
    }
    return nullptr;
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

    static ColorSpaceHeader Pack(Version version, SkColorSpace::Named named,
                                 SkColorSpace::GammaNamed gammaNamed, uint8_t flags) {
        ColorSpaceHeader header;

        SkASSERT(k0_Version == version);
        header.fVersion = (uint8_t) version;

        SkASSERT(named <= SkColorSpace::kAdobeRGB_Named);
        header.fNamed = (uint8_t) named;

        SkASSERT(gammaNamed <= SkColorSpace::kNonStandard_GammaNamed);
        header.fGammaNamed = (uint8_t) gammaNamed;

        SkASSERT(flags <= kFloatGamma_Flag);
        header.fFlags = flags;
        return header;
    }

    uint8_t fVersion;    // Always zero
    uint8_t fNamed;      // Must be a SkColorSpace::Named
    uint8_t fGammaNamed; // Must be a SkColorSpace::GammaNamed
    uint8_t fFlags;      // Some combination of the flags listed above
};

size_t SkColorSpace::writeToMemory(void* memory) const {
    // Start by trying the serialization fast path.  If we haven't saved ICC profile data,
    // we must have a profile that we can serialize easily.
    if (!as_CSB(this)->fProfileData) {
        // If we have a named profile, only write the enum.
        switch (fNamed) {
            case kSRGB_Named:
            case kAdobeRGB_Named: {
                if (memory) {
                    *((ColorSpaceHeader*) memory) =
                            ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed, 0);
                }
                return sizeof(ColorSpaceHeader);
            }
            default:
                break;
        }

        // If we have a named gamma, write the enum and the matrix.
        switch (fGammaNamed) {
            case kSRGB_GammaNamed:
            case k2Dot2Curve_GammaNamed:
            case kLinear_GammaNamed: {
                if (memory) {
                    *((ColorSpaceHeader*) memory) =
                            ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed,
                                                   ColorSpaceHeader::kMatrix_Flag);
                    memory = SkTAddOffset<void>(memory, sizeof(ColorSpaceHeader));
                    fToXYZD50.as4x3ColMajorf((float*) memory);
                }
                return sizeof(ColorSpaceHeader) + 12 * sizeof(float);
            }
            default:
                // Otherwise, write the gamma values and the matrix.
                if (memory) {
                    *((ColorSpaceHeader*) memory) =
                            ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed,
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

                    fToXYZD50.as4x3ColMajorf((float*) memory);
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
        *((ColorSpaceHeader*) memory) = ColorSpaceHeader::Pack(k0_Version, kUnknown_Named,
                                                               kNonStandard_GammaNamed,
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
    switch ((Named) header.fNamed) {
        case kSRGB_Named:
        case kAdobeRGB_Named:
            return NewNamed((Named) header.fNamed);
        default:
            break;
    }

    switch ((GammaNamed) header.fGammaNamed) {
        case kSRGB_GammaNamed:
        case k2Dot2Curve_GammaNamed:
        case kLinear_GammaNamed: {
            if (ColorSpaceHeader::kMatrix_Flag != header.fFlags || length < 12 * sizeof(float)) {
                return nullptr;
            }

            SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
            toXYZ.set4x3ColMajorf((const float*) data);
            return NewRGB((GammaNamed) header.fGammaNamed, toXYZ);
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
            toXYZ.set4x3ColMajorf((const float*) data);
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

    switch (src->fNamed) {
        case kSRGB_Named:
        case kAdobeRGB_Named:
            return src->fNamed == dst->fNamed;
        case kUnknown_Named:
            if (kUnknown_Named != dst->fNamed) {
                return false;
            }
            break;
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
    switch (src->fGammaNamed) {
        case kSRGB_GammaNamed:
        case k2Dot2Curve_GammaNamed:
        case kLinear_GammaNamed:
            return (src->fGammaNamed == dst->fGammaNamed) && (src->fToXYZD50 == dst->fToXYZD50);
        default:
            if (src->fGammaNamed != dst->fGammaNamed) {
                return false;
            }

            // It is unlikely that we will reach this case.
            sk_sp<SkData> srcData = src->serialize();
            sk_sp<SkData> dstData = dst->serialize();
            return srcData->size() == dstData->size() &&
                   0 == memcmp(srcData->data(), dstData->data(), srcData->size());
    }
}

bool SkColorSpace::gammasAreMatching() const {
    const SkGammas* gammas = as_CSB(this)->gammas();
    SkASSERT(gammas);
    return gammas->fRedType == gammas->fGreenType && gammas->fGreenType == gammas->fBlueType &&
           gammas->fRedData == gammas->fGreenData && gammas->fGreenData == gammas->fBlueData;
}

bool SkColorSpace::gammasAreNamed() const {
    const SkGammas* gammas = as_CSB(this)->gammas();
    SkASSERT(gammas);
    return gammas->fRedType == SkGammas::Type::kNamed_Type &&
           gammas->fGreenType == SkGammas::Type::kNamed_Type &&
           gammas->fBlueType == SkGammas::Type::kNamed_Type;
}

bool SkColorSpace::gammasAreValues() const {
    const SkGammas* gammas = as_CSB(this)->gammas();
    SkASSERT(gammas);
    return gammas->fRedType == SkGammas::Type::kValue_Type &&
           gammas->fGreenType == SkGammas::Type::kValue_Type &&
           gammas->fBlueType == SkGammas::Type::kValue_Type;
}

bool SkColorSpace::gammasAreTables() const {
    const SkGammas* gammas = as_CSB(this)->gammas();
    SkASSERT(gammas);
    return gammas->fRedType == SkGammas::Type::kTable_Type &&
           gammas->fGreenType == SkGammas::Type::kTable_Type &&
           gammas->fBlueType == SkGammas::Type::kTable_Type;
}

bool SkColorSpace::gammasAreParams() const {
    const SkGammas* gammas = as_CSB(this)->gammas();
    SkASSERT(gammas);
    return gammas->fRedType == SkGammas::Type::kParam_Type &&
           gammas->fGreenType == SkGammas::Type::kParam_Type &&
           gammas->fBlueType == SkGammas::Type::kParam_Type;
}
