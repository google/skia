/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkEndian.h"
#include "SkOnce.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#define SkColorSpacePrintf(...)

static bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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

SkColorSpace_Base::SkColorSpace_Base(SkColorLookUpTable* colorLUT, sk_sp<SkGammas> gammas,
                                     const SkMatrix44& toXYZD50, sk_sp<SkData> profileData)
    : INHERITED(kNonStandard_GammaNamed, toXYZD50, kUnknown_Named)
    , fColorLUT(colorLUT)
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

static void set_gamma_value(SkGammaCurve* gamma, float value) {
    if (color_space_almost_equal(2.2f, value)) {
        gamma->fNamed = SkColorSpace::k2Dot2Curve_GammaNamed;
    } else if (color_space_almost_equal(1.0f, value)) {
        gamma->fNamed = SkColorSpace::kLinear_GammaNamed;
    } else if (color_space_almost_equal(0.0f, value)) {
        SkColorSpacePrintf("Treating invalid zero gamma as linear.");
        gamma->fNamed = SkColorSpace::kLinear_GammaNamed;
    } else {
        gamma->fValue = value;
    }
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
                srgbToxyzD50.set3x3ColMajorf(gSRGB_toXYZD50);
                sRGB = new SkColorSpace_Base(kSRGB_GammaNamed, srgbToxyzD50, kSRGB_Named);
            });
            return sk_ref_sp(sRGB);
        }
        case kAdobeRGB_Named: {
            adobeRGBOnce([] {
                SkMatrix44 adobergbToxyzD50(SkMatrix44::kUninitialized_Constructor);
                adobergbToxyzD50.set3x3ColMajorf(gAdobeRGB_toXYZD50);
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

#include "SkFixed.h"
#include "SkTemplates.h"

#define return_if_false(pred, msg)                                   \
    do {                                                             \
        if (!(pred)) {                                               \
            SkColorSpacePrintf("Invalid ICC Profile: %s.\n", (msg)); \
            return false;                                            \
        }                                                            \
    } while (0)

#define return_null(msg)                                             \
    do {                                                             \
        SkColorSpacePrintf("Invalid ICC Profile: %s.\n", (msg));     \
        return nullptr;                                              \
    } while (0)

static uint16_t read_big_endian_short(const uint8_t* ptr) {
    return ptr[0] << 8 | ptr[1];
}

static uint32_t read_big_endian_uint(const uint8_t* ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static int32_t read_big_endian_int(const uint8_t* ptr) {
    return (int32_t) read_big_endian_uint(ptr);
}

// This is equal to the header size according to the ICC specification (128)
// plus the size of the tag count (4).  We include the tag count since we
// always require it to be present anyway.
static constexpr size_t kICCHeaderSize = 132;

// Contains a signature (4), offset (4), and size (4).
static constexpr size_t kICCTagTableEntrySize = 12;

static constexpr uint32_t kRGB_ColorSpace  = SkSetFourByteTag('R', 'G', 'B', ' ');
static constexpr uint32_t kDisplay_Profile = SkSetFourByteTag('m', 'n', 't', 'r');
static constexpr uint32_t kInput_Profile   = SkSetFourByteTag('s', 'c', 'n', 'r');
static constexpr uint32_t kOutput_Profile  = SkSetFourByteTag('p', 'r', 't', 'r');
static constexpr uint32_t kXYZ_PCSSpace    = SkSetFourByteTag('X', 'Y', 'Z', ' ');
static constexpr uint32_t kACSP_Signature  = SkSetFourByteTag('a', 'c', 's', 'p');

struct ICCProfileHeader {
    uint32_t fSize;

    // No reason to care about the preferred color management module (ex: Adobe, Apple, etc.).
    // We're always going to use this one.
    uint32_t fCMMType_ignored;

    uint32_t fVersion;
    uint32_t fProfileClass;
    uint32_t fInputColorSpace;
    uint32_t fPCS;
    uint32_t fDateTime_ignored[3];
    uint32_t fSignature;

    // Indicates the platform that this profile was created for (ex: Apple, Microsoft).  This
    // doesn't really matter to us.
    uint32_t fPlatformTarget_ignored;

    // Flags can indicate:
    // (1) Whether this profile was embedded in a file.  This flag is consistently wrong.
    //     Ex: The profile came from a file but indicates that it did not.
    // (2) Whether we are allowed to use the profile independently of the color data.  If set,
    //     this may allow us to use the embedded profile for testing separate from the original
    //     image.
    uint32_t fFlags_ignored;

    // We support many output devices.  It doesn't make sense to think about the attributes of
    // the device in the context of the image profile.
    uint32_t fDeviceManufacturer_ignored;
    uint32_t fDeviceModel_ignored;
    uint32_t fDeviceAttributes_ignored[2];

    uint32_t fRenderingIntent;
    int32_t  fIlluminantXYZ[3];

    // We don't care who created the profile.
    uint32_t fCreator_ignored;

    // This is an MD5 checksum.  Could be useful for checking if profiles are equal.
    uint32_t fProfileId_ignored[4];

    // Reserved for future use.
    uint32_t fReserved_ignored[7];

    uint32_t fTagCount;

    void init(const uint8_t* src, size_t len) {
        SkASSERT(kICCHeaderSize == sizeof(*this));

        uint32_t* dst = (uint32_t*) this;
        for (uint32_t i = 0; i < kICCHeaderSize / 4; i++, src+=4) {
            dst[i] = read_big_endian_uint(src);
        }
    }

    bool valid() const {
        return_if_false(fSize >= kICCHeaderSize, "Size is too small");

        uint8_t majorVersion = fVersion >> 24;
        return_if_false(majorVersion <= 4, "Unsupported version");

        // These are the three basic classes of profiles that we might expect to see embedded
        // in images.  Four additional classes exist, but they generally are used as a convenient
        // way for CMMs to store calculated transforms.
        return_if_false(fProfileClass == kDisplay_Profile ||
                        fProfileClass == kInput_Profile ||
                        fProfileClass == kOutput_Profile,
                        "Unsupported profile");

        // TODO (msarett):
        // All the profiles we've tested so far use RGB as the input color space.
        return_if_false(fInputColorSpace == kRGB_ColorSpace, "Unsupported color space");

        // TODO (msarett):
        // All the profiles we've tested so far use XYZ as the profile connection space.
        return_if_false(fPCS == kXYZ_PCSSpace, "Unsupported PCS space");

        return_if_false(fSignature == kACSP_Signature, "Bad signature");

        // TODO (msarett):
        // Should we treat different rendering intents differently?
        // Valid rendering intents include kPerceptual (0), kRelative (1),
        // kSaturation (2), and kAbsolute (3).
        return_if_false(fRenderingIntent <= 3, "Bad rendering intent");

        return_if_false(color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[0]), 0.96420f) &&
                        color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[1]), 1.00000f) &&
                        color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[2]), 0.82491f),
                        "Illuminant must be D50");

        return_if_false(fTagCount <= 100, "Too many tags");

        return true;
    }
};

template <class T>
static bool safe_add(T arg1, T arg2, size_t* result) {
    SkASSERT(arg1 >= 0);
    SkASSERT(arg2 >= 0);
    if (arg1 >= 0 && arg2 <= std::numeric_limits<T>::max() - arg1) {
        T sum = arg1 + arg2;
        if (sum <= std::numeric_limits<size_t>::max()) {
            *result = static_cast<size_t>(sum);
            return true;
        }
    }
    return false;
}

static bool safe_mul(uint32_t arg1, uint32_t arg2, uint32_t* result) {
    uint64_t product64 = (uint64_t) arg1 * (uint64_t) arg2;
    uint32_t product32 = (uint32_t) product64;
    if (product32 != product64) {
        return false;
    }

    *result = product32;
    return true;
}

struct ICCTag {
    uint32_t fSignature;
    uint32_t fOffset;
    uint32_t fLength;

    const uint8_t* init(const uint8_t* src) {
        fSignature = read_big_endian_uint(src);
        fOffset = read_big_endian_uint(src + 4);
        fLength = read_big_endian_uint(src + 8);
        return src + 12;
    }

    bool valid(size_t len) {
        size_t tagEnd;
        return_if_false(safe_add(fOffset, fLength, &tagEnd),
                        "Tag too large, overflows integer addition");
        return_if_false(tagEnd <= len, "Tag too large for ICC profile");
        return true;
    }

    const uint8_t* addr(const uint8_t* src) const {
        return src + fOffset;
    }

    static const ICCTag* Find(const ICCTag tags[], int count, uint32_t signature) {
        for (int i = 0; i < count; ++i) {
            if (tags[i].fSignature == signature) {
                return &tags[i];
            }
        }
        return nullptr;
    }
};

static constexpr uint32_t kTAG_rXYZ = SkSetFourByteTag('r', 'X', 'Y', 'Z');
static constexpr uint32_t kTAG_gXYZ = SkSetFourByteTag('g', 'X', 'Y', 'Z');
static constexpr uint32_t kTAG_bXYZ = SkSetFourByteTag('b', 'X', 'Y', 'Z');
static constexpr uint32_t kTAG_rTRC = SkSetFourByteTag('r', 'T', 'R', 'C');
static constexpr uint32_t kTAG_gTRC = SkSetFourByteTag('g', 'T', 'R', 'C');
static constexpr uint32_t kTAG_bTRC = SkSetFourByteTag('b', 'T', 'R', 'C');
static constexpr uint32_t kTAG_A2B0 = SkSetFourByteTag('A', '2', 'B', '0');

static bool load_xyz(float dst[3], const uint8_t* src, size_t len) {
    if (len < 20) {
        SkColorSpacePrintf("XYZ tag is too small (%d bytes)", len);
        return false;
    }

    dst[0] = SkFixedToFloat(read_big_endian_int(src + 8));
    dst[1] = SkFixedToFloat(read_big_endian_int(src + 12));
    dst[2] = SkFixedToFloat(read_big_endian_int(src + 16));
    SkColorSpacePrintf("XYZ %g %g %g\n", dst[0], dst[1], dst[2]);
    return true;
}

static constexpr uint32_t kTAG_CurveType     = SkSetFourByteTag('c', 'u', 'r', 'v');
static constexpr uint32_t kTAG_ParaCurveType = SkSetFourByteTag('p', 'a', 'r', 'a');

static bool load_gammas(SkGammaCurve* gammas, uint32_t numGammas, const uint8_t* src, size_t len) {
    for (uint32_t i = 0; i < numGammas; i++) {
        if (len < 12) {
            // FIXME (msarett):
            // We could potentially return false here after correctly parsing *some* of the
            // gammas correctly.  Should we somehow try to indicate a partial success?
            SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
            return false;
        }

        // We need to count the number of bytes in the tag, so we are able to move to the
        // next tag on the next loop iteration.
        size_t tagBytes;

        uint32_t type = read_big_endian_uint(src);
        switch (type) {
            case kTAG_CurveType: {
                uint32_t count = read_big_endian_uint(src + 8);

                // tagBytes = 12 + 2 * count
                // We need to do safe addition here to avoid integer overflow.
                if (!safe_add(count, count, &tagBytes) ||
                    !safe_add((size_t) 12, tagBytes, &tagBytes))
                {
                    SkColorSpacePrintf("Invalid gamma count");
                    return false;
                }

                if (0 == count) {
                    // Some tags require a gamma curve, but the author doesn't actually want
                    // to transform the data.  In this case, it is common to see a curve with
                    // a count of 0.
                    gammas[i].fNamed = SkColorSpace::kLinear_GammaNamed;
                    break;
                } else if (len < tagBytes) {
                    SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                    return false;
                }

                const uint16_t* table = (const uint16_t*) (src + 12);
                if (1 == count) {
                    // The table entry is the gamma (with a bias of 256).
                    float value = (read_big_endian_short((const uint8_t*) table)) / 256.0f;
                    set_gamma_value(&gammas[i], value);
                    SkColorSpacePrintf("gamma %g\n", value);
                    break;
                }

                // Check for frequently occurring sRGB curves.
                // We do this by sampling a few values and see if they match our expectation.
                // A more robust solution would be to compare each value in this curve against
                // an sRGB curve to see if we remain below an error threshold.  At this time,
                // we haven't seen any images in the wild that make this kind of
                // calculation necessary.  We encounter identical gamma curves over and
                // over again, but relatively few variations.
                if (1024 == count) {
                    // The magic values were chosen because they match a very common sRGB
                    // gamma table and the less common Canon sRGB gamma table (which use
                    // different rounding rules).
                    if (0 == read_big_endian_short((const uint8_t*) &table[0]) &&
                            3366 == read_big_endian_short((const uint8_t*) &table[257]) &&
                            14116 == read_big_endian_short((const uint8_t*) &table[513]) &&
                            34318 == read_big_endian_short((const uint8_t*) &table[768]) &&
                            65535 == read_big_endian_short((const uint8_t*) &table[1023])) {
                        gammas[i].fNamed = SkColorSpace::kSRGB_GammaNamed;
                        break;
                    }
                } else if (26 == count) {
                    // The magic values were chosen because they match a very common sRGB
                    // gamma table.
                    if (0 == read_big_endian_short((const uint8_t*) &table[0]) &&
                            3062 == read_big_endian_short((const uint8_t*) &table[6]) &&
                            12824 == read_big_endian_short((const uint8_t*) &table[12]) &&
                            31237 == read_big_endian_short((const uint8_t*) &table[18]) &&
                            65535 == read_big_endian_short((const uint8_t*) &table[25])) {
                        gammas[i].fNamed = SkColorSpace::kSRGB_GammaNamed;
                        break;
                    }
                } else if (4096 == count) {
                    // The magic values were chosen because they match Nikon, Epson, and
                    // LCMS sRGB gamma tables (all of which use different rounding rules).
                    if (0 == read_big_endian_short((const uint8_t*) &table[0]) &&
                            950 == read_big_endian_short((const uint8_t*) &table[515]) &&
                            3342 == read_big_endian_short((const uint8_t*) &table[1025]) &&
                            14079 == read_big_endian_short((const uint8_t*) &table[2051]) &&
                            65535 == read_big_endian_short((const uint8_t*) &table[4095])) {
                        gammas[i].fNamed = SkColorSpace::kSRGB_GammaNamed;
                        break;
                    }
                }

                // Otherwise, fill in the interpolation table.
                gammas[i].fTableSize = count;
                gammas[i].fTable = std::unique_ptr<float[]>(new float[count]);
                for (uint32_t j = 0; j < count; j++) {
                    gammas[i].fTable[j] =
                            (read_big_endian_short((const uint8_t*) &table[j])) / 65535.0f;
                }
                break;
            }
            case kTAG_ParaCurveType: {
                enum ParaCurveType {
                    kExponential_ParaCurveType = 0,
                    kGAB_ParaCurveType         = 1,
                    kGABC_ParaCurveType        = 2,
                    kGABDE_ParaCurveType       = 3,
                    kGABCDEF_ParaCurveType     = 4,
                };

                // Determine the format of the parametric curve tag.
                uint16_t format = read_big_endian_short(src + 8);
                if (kExponential_ParaCurveType == format) {
                    tagBytes = 12 + 4;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return false;
                    }

                    // Y = X^g
                    int32_t g = read_big_endian_int(src + 12);
                    set_gamma_value(&gammas[i], SkFixedToFloat(g));
                } else {
                    // Here's where the real parametric gammas start.  There are many
                    // permutations of the same equations.
                    //
                    // Y = (aX + b)^g + c  for X >= d
                    // Y = eX + f          otherwise
                    //
                    // We will fill in with zeros as necessary to always match the above form.
                    float g = 0.0f, a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f, e = 0.0f, f = 0.0f;
                    switch(format) {
                        case kGAB_ParaCurveType: {
                            tagBytes = 12 + 12;
                            if (len < tagBytes) {
                                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                                return false;
                            }

                            // Y = (aX + b)^g  for X >= -b/a
                            // Y = 0           otherwise
                            g = SkFixedToFloat(read_big_endian_int(src + 12));
                            a = SkFixedToFloat(read_big_endian_int(src + 16));
                            if (0.0f == a) {
                                return false;
                            }

                            b = SkFixedToFloat(read_big_endian_int(src + 20));
                            d = -b / a;
                            break;
                        }
                        case kGABC_ParaCurveType:
                            tagBytes = 12 + 16;
                            if (len < tagBytes) {
                                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                                return false;
                            }

                            // Y = (aX + b)^g + c  for X >= -b/a
                            // Y = c               otherwise
                            g = SkFixedToFloat(read_big_endian_int(src + 12));
                            a = SkFixedToFloat(read_big_endian_int(src + 16));
                            if (0.0f == a) {
                                return false;
                            }

                            b = SkFixedToFloat(read_big_endian_int(src + 20));
                            c = SkFixedToFloat(read_big_endian_int(src + 24));
                            d = -b / a;
                            f = c;
                            break;
                        case kGABDE_ParaCurveType:
                            tagBytes = 12 + 20;
                            if (len < tagBytes) {
                                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                                return false;
                            }

                            // Y = (aX + b)^g  for X >= d
                            // Y = cX          otherwise
                            g = SkFixedToFloat(read_big_endian_int(src + 12));
                            a = SkFixedToFloat(read_big_endian_int(src + 16));
                            b = SkFixedToFloat(read_big_endian_int(src + 20));
                            d = SkFixedToFloat(read_big_endian_int(src + 28));
                            e = SkFixedToFloat(read_big_endian_int(src + 24));
                            break;
                        case kGABCDEF_ParaCurveType:
                            tagBytes = 12 + 28;
                            if (len < tagBytes) {
                                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                                return false;
                            }

                            // Y = (aX + b)^g + c  for X >= d
                            // Y = eX + f          otherwise
                            // NOTE: The ICC spec writes "cX" in place of "eX" but I think
                            //       it's a typo.
                            g = SkFixedToFloat(read_big_endian_int(src + 12));
                            a = SkFixedToFloat(read_big_endian_int(src + 16));
                            b = SkFixedToFloat(read_big_endian_int(src + 20));
                            c = SkFixedToFloat(read_big_endian_int(src + 24));
                            d = SkFixedToFloat(read_big_endian_int(src + 28));
                            e = SkFixedToFloat(read_big_endian_int(src + 32));
                            f = SkFixedToFloat(read_big_endian_int(src + 36));
                            break;
                        default:
                            SkColorSpacePrintf("Invalid parametric curve type\n");
                            return false;
                    }

                    // Recognize and simplify a very common parametric representation of sRGB gamma.
                    if (color_space_almost_equal(0.9479f, a) &&
                            color_space_almost_equal(0.0521f, b) &&
                            color_space_almost_equal(0.0000f, c) &&
                            color_space_almost_equal(0.0405f, d) &&
                            color_space_almost_equal(0.0774f, e) &&
                            color_space_almost_equal(0.0000f, f) &&
                            color_space_almost_equal(2.4000f, g)) {
                        gammas[i].fNamed = SkColorSpace::kSRGB_GammaNamed;
                    } else {
                        // Fail on invalid gammas.
                        if (d <= 0.0f) {
                            // Y = (aX + b)^g + c  for always
                            if (0.0f == a || 0.0f == g) {
                                SkColorSpacePrintf("A or G is zero, constant gamma function "
                                                   "is nonsense");
                                return false;
                            }
                        } else if (d >= 1.0f) {
                            // Y = eX + f          for always
                            if (0.0f == e) {
                                SkColorSpacePrintf("E is zero, constant gamma function is "
                                                   "nonsense");
                                return false;
                            }
                        } else if ((0.0f == a || 0.0f == g) && 0.0f == e) {
                            SkColorSpacePrintf("A or G, and E are zero, constant gamma function "
                                               "is nonsense");
                            return false;
                        }

                        gammas[i].fG = g;
                        gammas[i].fA = a;
                        gammas[i].fB = b;
                        gammas[i].fC = c;
                        gammas[i].fD = d;
                        gammas[i].fE = e;
                        gammas[i].fF = f;
                    }
                }

                break;
            }
            default:
                SkColorSpacePrintf("Unsupported gamma tag type %d\n", type);
                return false;
        }

        // Ensure that we have successfully read a gamma representation.
        SkASSERT(gammas[i].isNamed() || gammas[i].isValue() || gammas[i].isTable() ||
                 gammas[i].isParametric());

        // Adjust src and len if there is another gamma curve to load.
        if (i != numGammas - 1) {
            // Each curve is padded to 4-byte alignment.
            tagBytes = SkAlign4(tagBytes);
            if (len < tagBytes) {
                return false;
            }

            src += tagBytes;
            len -= tagBytes;
        }
    }

    return true;
}

static constexpr uint32_t kTAG_AtoBType = SkSetFourByteTag('m', 'A', 'B', ' ');

bool load_color_lut(SkColorLookUpTable* colorLUT, uint32_t inputChannels, uint32_t outputChannels,
                    const uint8_t* src, size_t len) {
    // 16 bytes reserved for grid points, 2 for precision, 2 for padding.
    // The color LUT data follows after this header.
    static constexpr uint32_t kColorLUTHeaderSize = 20;
    if (len < kColorLUTHeaderSize) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }
    size_t dataLen = len - kColorLUTHeaderSize;

    SkASSERT(inputChannels <= SkColorLookUpTable::kMaxChannels && 3 == outputChannels);
    colorLUT->fInputChannels = inputChannels;
    colorLUT->fOutputChannels = outputChannels;
    uint32_t numEntries = 1;
    for (uint32_t i = 0; i < inputChannels; i++) {
        colorLUT->fGridPoints[i] = src[i];
        if (0 == src[i]) {
            SkColorSpacePrintf("Each input channel must have at least one grid point.");
            return false;
        }

        if (!safe_mul(numEntries, src[i], &numEntries)) {
            SkColorSpacePrintf("Too many entries in Color LUT.");
            return false;
        }
    }

    if (!safe_mul(numEntries, outputChannels, &numEntries)) {
        SkColorSpacePrintf("Too many entries in Color LUT.");
        return false;
    }

    // Space is provided for a maximum of the 16 input channels.  Now we determine the precision
    // of the table values.
    uint8_t precision = src[16];
    switch (precision) {
        case 1: // 8-bit data
        case 2: // 16-bit data
            break;
        default:
            SkColorSpacePrintf("Color LUT precision must be 8-bit or 16-bit.\n");
            return false;
    }

    uint32_t clutBytes;
    if (!safe_mul(numEntries, precision, &clutBytes)) {
        SkColorSpacePrintf("Too many entries in Color LUT.");
        return false;
    }

    if (dataLen < clutBytes) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }

    // Movable struct colorLUT has ownership of fTable.
    colorLUT->fTable = std::unique_ptr<float[]>(new float[numEntries]);
    const uint8_t* ptr = src + kColorLUTHeaderSize;
    for (uint32_t i = 0; i < numEntries; i++, ptr += precision) {
        if (1 == precision) {
            colorLUT->fTable[i] = ((float) ptr[i]) / 255.0f;
        } else {
            colorLUT->fTable[i] = ((float) read_big_endian_short(ptr)) / 65535.0f;
        }
    }

    return true;
}

bool load_matrix(SkMatrix44* toXYZ, const uint8_t* src, size_t len) {
    if (len < 48) {
        SkColorSpacePrintf("Matrix tag is too small (%d bytes).", len);
        return false;
    }

    // For this matrix to behave like our "to XYZ D50" matrices, it needs to be scaled.
    constexpr float scale = 65535.0 / 32768.0;
    float array[16];
    array[ 0] = scale * SkFixedToFloat(read_big_endian_int(src));
    array[ 1] = scale * SkFixedToFloat(read_big_endian_int(src + 4));
    array[ 2] = scale * SkFixedToFloat(read_big_endian_int(src + 8));
    array[ 3] = scale * SkFixedToFloat(read_big_endian_int(src + 36)); // translate R
    array[ 4] = scale * SkFixedToFloat(read_big_endian_int(src + 12));
    array[ 5] = scale * SkFixedToFloat(read_big_endian_int(src + 16));
    array[ 6] = scale * SkFixedToFloat(read_big_endian_int(src + 20));
    array[ 7] = scale * SkFixedToFloat(read_big_endian_int(src + 40)); // translate G
    array[ 8] = scale * SkFixedToFloat(read_big_endian_int(src + 24));
    array[ 9] = scale * SkFixedToFloat(read_big_endian_int(src + 28));
    array[10] = scale * SkFixedToFloat(read_big_endian_int(src + 32));
    array[11] = scale * SkFixedToFloat(read_big_endian_int(src + 44)); // translate B
    array[12] = 0.0f;
    array[13] = 0.0f;
    array[14] = 0.0f;
    array[15] = 1.0f;
    toXYZ->setColMajorf(array);
    return true;
}

bool load_a2b0(SkColorLookUpTable* colorLUT, SkGammaCurve* gammas, SkMatrix44* toXYZ,
               const uint8_t* src, size_t len) {
    if (len < 32) {
        SkColorSpacePrintf("A to B tag is too small (%d bytes).", len);
        return false;
    }

    uint32_t type = read_big_endian_uint(src);
    if (kTAG_AtoBType != type) {
        // FIXME (msarett): Need to support lut8Type and lut16Type.
        SkColorSpacePrintf("Unsupported A to B tag type.\n");
        return false;
    }

    // Read the number of channels.  The four bytes that we skipped are reserved and
    // must be zero.
    uint8_t inputChannels = src[8];
    uint8_t outputChannels = src[9];
    if (0 == inputChannels || inputChannels > SkColorLookUpTable::kMaxChannels ||
            3 != outputChannels) {
        // The color LUT assumes that there are at most 16 input channels.  For RGB
        // profiles, output channels should be 3.
        SkColorSpacePrintf("Too many input or output channels in A to B tag.\n");
        return false;
    }

    // Read the offsets of each element in the A to B tag.  With the exception of A curves and
    // B curves (which we do not yet support), we will handle these elements in the order in
    // which they should be applied (rather than the order in which they occur in the tag).
    // If the offset is non-zero it indicates that the element is present.
    uint32_t offsetToACurves = read_big_endian_int(src + 28);
    uint32_t offsetToBCurves = read_big_endian_int(src + 12);
    if ((0 != offsetToACurves) || (0 != offsetToBCurves)) {
        // FIXME (msarett): Handle A and B curves.
        // Note that the A curve is technically required in order to have a color LUT.
        // However, all the A curves I have seen so far have are just placeholders that
        // don't actually transform the data.
        SkColorSpacePrintf("Ignoring A and/or B curve.  Output may be wrong.\n");
    }

    uint32_t offsetToColorLUT = read_big_endian_int(src + 24);
    if (0 != offsetToColorLUT && offsetToColorLUT < len) {
        if (!load_color_lut(colorLUT, inputChannels, outputChannels, src + offsetToColorLUT,
                            len - offsetToColorLUT)) {
            SkColorSpacePrintf("Failed to read color LUT from A to B tag.\n");
        }
    }

    uint32_t offsetToMCurves = read_big_endian_int(src + 20);
    if (0 != offsetToMCurves && offsetToMCurves < len) {
        if (!load_gammas(gammas, outputChannels, src + offsetToMCurves, len - offsetToMCurves)) {
            SkColorSpacePrintf("Failed to read M curves from A to B tag.  Using linear gamma.\n");
            gammas[0].fNamed = SkColorSpace::kLinear_GammaNamed;
            gammas[1].fNamed = SkColorSpace::kLinear_GammaNamed;
            gammas[2].fNamed = SkColorSpace::kLinear_GammaNamed;
        }
    }

    uint32_t offsetToMatrix = read_big_endian_int(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        if (!load_matrix(toXYZ, src + offsetToMatrix, len - offsetToMatrix)) {
            SkColorSpacePrintf("Failed to read matrix from A to B tag.\n");
            toXYZ->setIdentity();
        }
    }

    return true;
}

sk_sp<SkColorSpace> SkColorSpace::NewICC(const void* input, size_t len) {
    if (!input || len < kICCHeaderSize) {
        return_null("Data is null or not large enough to contain an ICC profile");
    }

    // Create our own copy of the input.
    void* memory = sk_malloc_throw(len);
    memcpy(memory, input, len);
    sk_sp<SkData> data = SkData::MakeFromMalloc(memory, len);
    const void* base = data->data();
    const uint8_t* ptr = (const uint8_t*) base;

    // Read the ICC profile header and check to make sure that it is valid.
    ICCProfileHeader header;
    header.init(ptr, len);
    if (!header.valid()) {
        return nullptr;
    }

    // Adjust ptr and len before reading the tags.
    if (len < header.fSize) {
        SkColorSpacePrintf("ICC profile might be truncated.\n");
    } else if (len > header.fSize) {
        SkColorSpacePrintf("Caller provided extra data beyond the end of the ICC profile.\n");
        len = header.fSize;
    }
    ptr += kICCHeaderSize;
    len -= kICCHeaderSize;

    // Parse tag headers.
    uint32_t tagCount = header.fTagCount;
    SkColorSpacePrintf("ICC profile contains %d tags.\n", tagCount);
    if (len < kICCTagTableEntrySize * tagCount) {
        return_null("Not enough input data to read tag table entries");
    }

    SkAutoTArray<ICCTag> tags(tagCount);
    for (uint32_t i = 0; i < tagCount; i++) {
        ptr = tags[i].init(ptr);
        SkColorSpacePrintf("[%d] %c%c%c%c %d %d\n", i, (tags[i].fSignature >> 24) & 0xFF,
                (tags[i].fSignature >> 16) & 0xFF, (tags[i].fSignature >>  8) & 0xFF,
                (tags[i].fSignature >>  0) & 0xFF, tags[i].fOffset, tags[i].fLength);

        if (!tags[i].valid(kICCHeaderSize + len)) {
            return_null("Tag is too large to fit in ICC profile");
        }
    }

    switch (header.fInputColorSpace) {
        case kRGB_ColorSpace: {
            // Recognize the rXYZ, gXYZ, and bXYZ tags.
            const ICCTag* r = ICCTag::Find(tags.get(), tagCount, kTAG_rXYZ);
            const ICCTag* g = ICCTag::Find(tags.get(), tagCount, kTAG_gXYZ);
            const ICCTag* b = ICCTag::Find(tags.get(), tagCount, kTAG_bXYZ);
            if (r && g && b) {
                float toXYZ[9];
                if (!load_xyz(&toXYZ[0], r->addr((const uint8_t*) base), r->fLength) ||
                    !load_xyz(&toXYZ[3], g->addr((const uint8_t*) base), g->fLength) ||
                    !load_xyz(&toXYZ[6], b->addr((const uint8_t*) base), b->fLength))
                {
                    return_null("Need valid rgb tags for XYZ space");
                }
                SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
                mat.set3x3ColMajorf(toXYZ);

                // It is not uncommon to see missing or empty gamma tags.  This indicates
                // that we should use unit gamma.
                SkGammaCurve curves[3];
                r = ICCTag::Find(tags.get(), tagCount, kTAG_rTRC);
                g = ICCTag::Find(tags.get(), tagCount, kTAG_gTRC);
                b = ICCTag::Find(tags.get(), tagCount, kTAG_bTRC);
                if (!r || !load_gammas(&curves[0], 1, r->addr((const uint8_t*) base), r->fLength))
                {
                    SkColorSpacePrintf("Failed to read R gamma tag.\n");
                    curves[0].fNamed = SkColorSpace::kLinear_GammaNamed;
                }
                if (!g || !load_gammas(&curves[1], 1, g->addr((const uint8_t*) base), g->fLength))
                {
                    SkColorSpacePrintf("Failed to read G gamma tag.\n");
                    curves[1].fNamed = SkColorSpace::kLinear_GammaNamed;
                }
                if (!b || !load_gammas(&curves[2], 1, b->addr((const uint8_t*) base), b->fLength))
                {
                    SkColorSpacePrintf("Failed to read B gamma tag.\n");
                    curves[2].fNamed = SkColorSpace::kLinear_GammaNamed;
                }

                GammaNamed gammaNamed = SkGammas::Named(curves);
                if (kNonStandard_GammaNamed == gammaNamed) {
                    sk_sp<SkGammas> gammas = sk_make_sp<SkGammas>(std::move(curves[0]),
                                                                  std::move(curves[1]),
                                                                  std::move(curves[2]));
                    return sk_sp<SkColorSpace>(new SkColorSpace_Base(nullptr, std::move(gammas),
                                                                     mat, std::move(data)));
                } else {
                    return SkColorSpace_Base::NewRGB(gammaNamed, mat);
                }
            }

            // Recognize color profile specified by A2B0 tag.
            const ICCTag* a2b0 = ICCTag::Find(tags.get(), tagCount, kTAG_A2B0);
            if (a2b0) {
                SkAutoTDelete<SkColorLookUpTable> colorLUT(new SkColorLookUpTable());
                SkGammaCurve curves[3];
                SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
                if (!load_a2b0(colorLUT, curves, &toXYZ, a2b0->addr((const uint8_t*) base),
                               a2b0->fLength)) {
                    return_null("Failed to parse A2B0 tag");
                }

                GammaNamed gammaNamed = SkGammas::Named(curves);
                if (colorLUT->fTable || kNonStandard_GammaNamed == gammaNamed) {
                    sk_sp<SkGammas> gammas = sk_make_sp<SkGammas>(std::move(curves[0]),
                                                                  std::move(curves[1]),
                                                                  std::move(curves[2]));

                    return sk_sp<SkColorSpace>(new SkColorSpace_Base(colorLUT.release(),
                                                                     std::move(gammas), toXYZ,
                                                                     std::move(data)));
                } else {
                    return SkColorSpace_Base::NewRGB(gammaNamed, toXYZ);
                }
            }
        }
        default:
            break;
    }

    return_null("ICC profile contains unsupported colorspace");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// We will write a profile with the minimum nine required tags.
static constexpr uint32_t kICCNumEntries = 9;

static constexpr uint32_t kTAG_desc = SkSetFourByteTag('d', 'e', 's', 'c');
static constexpr uint32_t kTAG_desc_Bytes = 12;
static constexpr uint32_t kTAG_desc_Offset = kICCHeaderSize + kICCNumEntries*kICCTagTableEntrySize;

static constexpr uint32_t kTAG_XYZ_Bytes = 20;
static constexpr uint32_t kTAG_rXYZ_Offset = kTAG_desc_Offset + kTAG_desc_Bytes;
static constexpr uint32_t kTAG_gXYZ_Offset = kTAG_rXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_bXYZ_Offset = kTAG_gXYZ_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kTAG_TRC_Bytes = 14;
static constexpr uint32_t kTAG_rTRC_Offset = kTAG_bXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_gTRC_Offset = kTAG_rTRC_Offset + SkAlign4(kTAG_TRC_Bytes);
static constexpr uint32_t kTAG_bTRC_Offset = kTAG_gTRC_Offset + SkAlign4(kTAG_TRC_Bytes);

static constexpr uint32_t kTAG_wtpt = SkSetFourByteTag('w', 't', 'p', 't');
static constexpr uint32_t kTAG_wtpt_Offset = kTAG_bTRC_Offset + SkAlign4(kTAG_TRC_Bytes);

static constexpr uint32_t kTAG_cprt = SkSetFourByteTag('c', 'p', 'r', 't');
static constexpr uint32_t kTAG_cprt_Bytes = 12;
static constexpr uint32_t kTAG_cprt_Offset = kTAG_wtpt_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kICCProfileSize = kTAG_cprt_Offset + kTAG_cprt_Bytes;

static constexpr uint32_t gICCHeader[kICCHeaderSize / 4] {
    SkEndian_SwapBE32(kICCProfileSize),  // Size of the profile
    0,                                   // Preferred CMM type (ignored)
    SkEndian_SwapBE32(0x02100000),       // Version 2.1
    SkEndian_SwapBE32(kDisplay_Profile), // Display device profile
    SkEndian_SwapBE32(kRGB_ColorSpace),  // RGB input color space
    SkEndian_SwapBE32(kXYZ_PCSSpace),    // XYZ profile connection space
    0, 0, 0,                             // Date and time (ignored)
    SkEndian_SwapBE32(kACSP_Signature),  // Profile signature
    0,                                   // Platform target (ignored)
    0x00000000,                          // Flags: not embedded, can be used independently
    0,                                   // Device manufacturer (ignored)
    0,                                   // Device model (ignored)
    0, 0,                                // Device attributes (ignored)
    SkEndian_SwapBE32(1),                // Relative colorimetric rendering intent
    SkEndian_SwapBE32(0x0000f6d6),       // D50 standard illuminant (X)
    SkEndian_SwapBE32(0x00010000),       // D50 standard illuminant (Y)
    SkEndian_SwapBE32(0x0000d32d),       // D50 standard illuminant (Z)
    0,                                   // Profile creator (ignored)
    0, 0, 0, 0,                          // Profile id checksum (ignored)
    0, 0, 0, 0, 0, 0, 0,                 // Reserved (ignored)
    SkEndian_SwapBE32(kICCNumEntries),   // Number of tags
};

static constexpr uint32_t gICCTagTable[3 * kICCNumEntries] {
    // Profile description
    SkEndian_SwapBE32(kTAG_desc),
    SkEndian_SwapBE32(kTAG_desc_Offset),
    SkEndian_SwapBE32(kTAG_desc_Bytes),

    // rXYZ
    SkEndian_SwapBE32(kTAG_rXYZ),
    SkEndian_SwapBE32(kTAG_rXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // gXYZ
    SkEndian_SwapBE32(kTAG_gXYZ),
    SkEndian_SwapBE32(kTAG_gXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // bXYZ
    SkEndian_SwapBE32(kTAG_bXYZ),
    SkEndian_SwapBE32(kTAG_bXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // rTRC
    SkEndian_SwapBE32(kTAG_rTRC),
    SkEndian_SwapBE32(kTAG_rTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // gTRC
    SkEndian_SwapBE32(kTAG_gTRC),
    SkEndian_SwapBE32(kTAG_gTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // bTRC
    SkEndian_SwapBE32(kTAG_bTRC),
    SkEndian_SwapBE32(kTAG_bTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // White point
    SkEndian_SwapBE32(kTAG_wtpt),
    SkEndian_SwapBE32(kTAG_wtpt_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // Copyright
    SkEndian_SwapBE32(kTAG_cprt),
    SkEndian_SwapBE32(kTAG_cprt_Offset),
    SkEndian_SwapBE32(kTAG_cprt_Bytes),
};

static constexpr uint32_t kTAG_TextType = SkSetFourByteTag('m', 'l', 'u', 'c');
static constexpr uint32_t gEmptyTextTag[3] {
    SkEndian_SwapBE32(kTAG_TextType), // Type signature
    0,                                // Reserved
    0,                                // Zero records
};

static void write_xyz_tag(uint32_t* ptr, const SkMatrix44& toXYZ, int row) {
    ptr[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(row, 0)));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(row, 1)));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(row, 2)));
}

static void write_trc_tag(uint32_t* ptr, float value) {
    ptr[0] = SkEndian_SwapBE32(kTAG_CurveType);
    ptr[1] = 0;

    // Gamma will be specified with a single value.
    ptr[2] = SkEndian_SwapBE32(1);

    // Convert gamma to 16-bit fixed point.
    uint16_t* ptr16 = (uint16_t*) (ptr + 3);
    ptr16[0] = SkEndian_SwapBE16((uint16_t) (value * 256.0f));

    // Pad tag with zero.
    ptr16[1] = 0;
}

static float get_gamma_value(const SkGammaCurve* curve) {
    switch (curve->fNamed) {
        case SkColorSpace::kSRGB_GammaNamed:
            // FIXME (msarett):
            // kSRGB cannot be represented by a value.  Here we fall through to 2.2f,
            // which is a close guess.  To be more accurate, we need to represent sRGB
            // gamma with a parametric curve.
        case SkColorSpace::k2Dot2Curve_GammaNamed:
            return 2.2f;
        case SkColorSpace::kLinear_GammaNamed:
            return 1.0f;
        default:
            SkASSERT(curve->isValue());
            return curve->fValue;
    }
}

sk_sp<SkData> SkColorSpace_Base::writeToICC() const {
    // Return if this object was created from a profile, or if we have already serialized
    // the profile.
    if (fProfileData) {
        return fProfileData;
    }

    // The client may create an SkColorSpace using an SkMatrix44, but currently we only
    // support writing profiles with 3x3 matrices.
    // TODO (msarett): Fix this!
    if (0.0f != fToXYZD50.getFloat(3, 0) || 0.0f != fToXYZD50.getFloat(3, 1) ||
        0.0f != fToXYZD50.getFloat(3, 2) || 0.0f != fToXYZD50.getFloat(0, 3) ||
        0.0f != fToXYZD50.getFloat(1, 3) || 0.0f != fToXYZD50.getFloat(2, 3))
    {
        return nullptr;
    }

    SkAutoMalloc profile(kICCProfileSize);
    uint8_t* ptr = (uint8_t*) profile.get();

    // Write profile header
    memcpy(ptr, gICCHeader, sizeof(gICCHeader));
    ptr += sizeof(gICCHeader);

    // Write tag table
    memcpy(ptr, gICCTagTable, sizeof(gICCTagTable));
    ptr += sizeof(gICCTagTable);

    // Write profile description tag
    memcpy(ptr, gEmptyTextTag, sizeof(gEmptyTextTag));
    ptr += sizeof(gEmptyTextTag);

    // Write XYZ tags
    write_xyz_tag((uint32_t*) ptr, fToXYZD50, 0);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, fToXYZD50, 1);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, fToXYZD50, 2);
    ptr += kTAG_XYZ_Bytes;

    // Write TRC tags
    GammaNamed gammaNamed = this->gammaNamed();
    if (kNonStandard_GammaNamed == gammaNamed) {
        write_trc_tag((uint32_t*) ptr, get_gamma_value(&as_CSB(this)->fGammas->fRed));
        ptr += SkAlign4(kTAG_TRC_Bytes);
        write_trc_tag((uint32_t*) ptr, get_gamma_value(&as_CSB(this)->fGammas->fGreen));
        ptr += SkAlign4(kTAG_TRC_Bytes);
        write_trc_tag((uint32_t*) ptr, get_gamma_value(&as_CSB(this)->fGammas->fBlue));
        ptr += SkAlign4(kTAG_TRC_Bytes);
    } else {
        switch (gammaNamed) {
            case SkColorSpace::kSRGB_GammaNamed:
                // FIXME (msarett):
                // kSRGB cannot be represented by a value.  Here we fall through to 2.2f,
                // which is a close guess.  To be more accurate, we need to represent sRGB
                // gamma with a parametric curve.
            case SkColorSpace::k2Dot2Curve_GammaNamed:
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                break;
            case SkColorSpace::kLinear_GammaNamed:
                write_trc_tag((uint32_t*) ptr, 1.0f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 1.0f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 1.0f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                break;
            default:
                SkASSERT(false);
                break;
        }
    }

    // Write white point tag
    uint32_t* ptr32 = (uint32_t*) ptr;
    ptr32[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr32[1] = 0;
    // TODO (msarett): These values correspond to the D65 white point.  This may not always be
    //                 correct.
    ptr32[2] = SkEndian_SwapBE32(0x0000f351);
    ptr32[3] = SkEndian_SwapBE32(0x00010000);
    ptr32[4] = SkEndian_SwapBE32(0x000116cc);
    ptr += kTAG_XYZ_Bytes;

    // Write copyright tag
    memcpy(ptr, gEmptyTextTag, sizeof(gEmptyTextTag));

    // TODO (msarett): Should we try to hold onto the data so we can return immediately if
    //                 the client calls again?
    return SkData::MakeFromMalloc(profile.release(), kICCProfileSize);
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
