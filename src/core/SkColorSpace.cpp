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

SkColorSpace_Base::SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, sk_sp<SkGammas> gammas,
                                     const SkMatrix44& toXYZD50, sk_sp<SkData> profileData)
    : INHERITED(kNonStandard_GammaNamed, toXYZD50, kUnknown_Named)
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

sk_sp<SkColorSpace> SkColorSpace_Base::NewRGB(float values[3], const SkMatrix44& toXYZD50) {
    SkGammaCurve curves[3];
    set_gamma_value(&curves[0], values[0]);
    set_gamma_value(&curves[1], values[1]);
    set_gamma_value(&curves[2], values[2]);

    GammaNamed gammaNamed = SkGammas::Named(curves);
    if (kNonStandard_GammaNamed == gammaNamed) {
        sk_sp<SkGammas> gammas(new SkGammas(std::move(curves[0]), std::move(curves[1]),
                                            std::move(curves[2])));
        return sk_sp<SkColorSpace>(new SkColorSpace_Base(nullptr, gammas, toXYZD50, nullptr));
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
    static SkColorSpace* sRGB;
    static SkOnce adobeRGBOnce;
    static SkColorSpace* adobeRGB;

    switch (named) {
        case kSRGB_Named: {
            sRGBOnce([] {
                SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);
                sRGB = new SkColorSpace_Base(kSRGB_GammaNamed, srgbToxyzD50, kSRGB_Named);
            });
            return sk_ref_sp(sRGB);
        }
        case kAdobeRGB_Named: {
            adobeRGBOnce([] {
                SkMatrix44 adobergbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                adobergbToxyzD50.set3x3RowMajorf(gAdobeRGB_toXYZD50);
                adobeRGB = new SkColorSpace_Base(k2Dot2Curve_GammaNamed, adobergbToxyzD50,
                                                 kAdobeRGB_Named);
            });
            return sk_ref_sp(adobeRGB);
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
     *  Should not be set at the same time as the kICC_Flag.
     */
    static constexpr uint8_t kMatrix_Flag = 1 << 0;

    /**
     *  If kICC_Flag is set, we will write an ICC profile after the header.
     *  The ICC profile will be written as a uint32 size, followed immediately
     *  by the data (padded to 4 bytes).
     *  Should not be set at the same time as the kMatrix_Flag.
     */
    static constexpr uint8_t kICC_Flag    = 1 << 1;

    static ColorSpaceHeader Pack(Version version, SkColorSpace::Named named,
                                 SkColorSpace::GammaNamed gammaNamed, uint8_t flags) {
        ColorSpaceHeader header;

        SkASSERT(k0_Version == version);
        header.fVersion = (uint8_t) version;

        SkASSERT(named <= SkColorSpace::kAdobeRGB_Named);
        header.fNamed = (uint8_t) named;

        SkASSERT(gammaNamed <= SkColorSpace::kNonStandard_GammaNamed);
        header.fGammaNamed = (uint8_t) gammaNamed;

        SkASSERT(flags <= kICC_Flag);
        header.fFlags = flags;
        return header;
    }

    uint8_t fVersion;    // Always zero
    uint8_t fNamed;      // Must be a SkColorSpace::Named
    uint8_t fGammaNamed; // Must be a SkColorSpace::GammaNamed
    uint8_t fFlags;      // Some combination of the flags listed above
};

sk_sp<SkData> SkColorSpace::serialize() const {
    // If we have a named profile, only write the enum.
    switch (fNamed) {
        case kSRGB_Named:
        case kAdobeRGB_Named: {
            sk_sp<SkData> data = SkData::MakeUninitialized(sizeof(ColorSpaceHeader));
            *((ColorSpaceHeader*) data->writable_data()) =
                    ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed, 0);
            return data;
        }
        default:
            break;
    }

    // If we have a named gamma, write the enum and the matrix.
    switch (fGammaNamed) {
        case kSRGB_GammaNamed:
        case k2Dot2Curve_GammaNamed:
        case kLinear_GammaNamed: {
            sk_sp<SkData> data = SkData::MakeUninitialized(sizeof(ColorSpaceHeader) +
                                                           12 * sizeof(float));
            void* dataPtr = data->writable_data();

            *((ColorSpaceHeader*) dataPtr) = ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed,
                                                                    ColorSpaceHeader::kMatrix_Flag);
            dataPtr = SkTAddOffset<void>(dataPtr, sizeof(ColorSpaceHeader));

            fToXYZD50.as4x3ColMajorf((float*) dataPtr);
            return data;
        }
        default:
            break;
    }

    // If we do not have a named gamma, this must have been created from an ICC profile.
    // Since we were unable to recognize the gamma, we will have saved the ICC data.
    SkASSERT(as_CSB(this)->fProfileData);

    size_t profileSize = as_CSB(this)->fProfileData->size();
    if (SkAlign4(profileSize) != (uint32_t) SkAlign4(profileSize)) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(sizeof(ColorSpaceHeader) + sizeof(uint32_t) +
                                                   SkAlign4(profileSize));
    void* dataPtr = data->writable_data();

    *((ColorSpaceHeader*) dataPtr) = ColorSpaceHeader::Pack(k0_Version, fNamed, fGammaNamed,
                                                            ColorSpaceHeader::kICC_Flag);
    dataPtr = SkTAddOffset<void>(dataPtr, sizeof(ColorSpaceHeader));

    *((uint32_t*) dataPtr) = (uint32_t) SkAlign4(profileSize);
    dataPtr = SkTAddOffset<void>(dataPtr, sizeof(uint32_t));

    memcpy(dataPtr, as_CSB(this)->fProfileData->data(), profileSize);
    memset(SkTAddOffset<void>(dataPtr, profileSize), 0, SkAlign4(profileSize) - profileSize);
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

    if (ColorSpaceHeader::kICC_Flag != header.fFlags || length < sizeof(uint32_t)) {
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
