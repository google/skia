/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpacePriv.h"
#include "SkEndian.h"
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

static uint16_t read_big_endian_u16(const uint8_t* ptr) {
    return ptr[0] << 8 | ptr[1];
}

static uint32_t read_big_endian_u32(const uint8_t* ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static int32_t read_big_endian_i32(const uint8_t* ptr) {
    return (int32_t) read_big_endian_u32(ptr);
}

// This is equal to the header size according to the ICC specification (128)
// plus the size of the tag count (4).  We include the tag count since we
// always require it to be present anyway.
static constexpr size_t kICCHeaderSize = 132;

// Contains a signature (4), offset (4), and size (4).
static constexpr size_t kICCTagTableEntrySize = 12;

static constexpr uint32_t kRGB_ColorSpace     = SkSetFourByteTag('R', 'G', 'B', ' ');
static constexpr uint32_t kDisplay_Profile    = SkSetFourByteTag('m', 'n', 't', 'r');
static constexpr uint32_t kInput_Profile      = SkSetFourByteTag('s', 'c', 'n', 'r');
static constexpr uint32_t kOutput_Profile     = SkSetFourByteTag('p', 'r', 't', 'r');
static constexpr uint32_t kColorSpace_Profile = SkSetFourByteTag('s', 'p', 'a', 'c');
static constexpr uint32_t kXYZ_PCSSpace       = SkSetFourByteTag('X', 'Y', 'Z', ' ');
static constexpr uint32_t kACSP_Signature     = SkSetFourByteTag('a', 'c', 's', 'p');

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
            dst[i] = read_big_endian_u32(src);
        }
    }

    bool valid() const {
        return_if_false(fSize >= kICCHeaderSize, "Size is too small");

        uint8_t majorVersion = fVersion >> 24;
        return_if_false(majorVersion <= 4, "Unsupported version");

        // These are the four basic classes of profiles that we might expect to see embedded
        // in images.  Additional classes exist, but they generally are used as a convenient
        // way for CMMs to store calculated transforms.
        return_if_false(fProfileClass == kDisplay_Profile ||
                        fProfileClass == kInput_Profile ||
                        fProfileClass == kOutput_Profile ||
                        fProfileClass == kColorSpace_Profile,
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
        if (fRenderingIntent > 3) {
            // Warn rather than fail here.  Occasionally, we see perfectly
            // normal profiles with wacky rendering intents.
            SkColorSpacePrintf("Warning, bad rendering intent.\n");
        }

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
        fSignature = read_big_endian_u32(src);
        fOffset = read_big_endian_u32(src + 4);
        fLength = read_big_endian_u32(src + 8);
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

    dst[0] = SkFixedToFloat(read_big_endian_i32(src + 8));
    dst[1] = SkFixedToFloat(read_big_endian_i32(src + 12));
    dst[2] = SkFixedToFloat(read_big_endian_i32(src + 16));
    SkColorSpacePrintf("XYZ %g %g %g\n", dst[0], dst[1], dst[2]);
    return true;
}

static constexpr uint32_t kTAG_CurveType     = SkSetFourByteTag('c', 'u', 'r', 'v');
static constexpr uint32_t kTAG_ParaCurveType = SkSetFourByteTag('p', 'a', 'r', 'a');

static SkGammas::Type set_gamma_value(SkGammas::Data* data, float value) {
    if (color_space_almost_equal(2.2f, value)) {
        data->fNamed = k2Dot2Curve_SkGammaNamed;
        return SkGammas::Type::kNamed_Type;
    }

    if (color_space_almost_equal(1.0f, value)) {
        data->fNamed = kLinear_SkGammaNamed;
        return SkGammas::Type::kNamed_Type;
    }

    if (color_space_almost_equal(0.0f, value)) {
        return SkGammas::Type::kNone_Type;
    }

    data->fValue = value;
    return SkGammas::Type::kValue_Type;
}

static float read_big_endian_16_dot_16(const uint8_t buf[4]) {
    // It just so happens that SkFixed is also 16.16!
    return SkFixedToFloat(read_big_endian_i32(buf));
}

/**
 *  @param outData     Set to the appropriate value on success.  If we have table or
 *                     parametric gamma, it is the responsibility of the caller to set
 *                     fOffset.
 *  @param outParams   If this is a parametric gamma, this is set to the appropriate
 *                     parameters on success.
 *  @param outTagBytes Will be set to the length of the tag on success.
 *  @src               Pointer to tag data.
 *  @len               Length of tag data in bytes.
 *
 *  @return            kNone_Type on failure, otherwise the type of the gamma tag.
 */
static SkGammas::Type parse_gamma(SkGammas::Data* outData, SkGammas::Params* outParams,
                                      size_t* outTagBytes, const uint8_t* src, size_t len) {
    if (len < 12) {
        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
        return SkGammas::Type::kNone_Type;
    }

    // In the case of consecutive gamma tags, we need to count the number of bytes in the
    // tag, so that we can move on to the next tag.
    size_t tagBytes;

    uint32_t type = read_big_endian_u32(src);
    // Bytes 4-7 are reserved and should be set to zero.
    switch (type) {
        case kTAG_CurveType: {
            uint32_t count = read_big_endian_u32(src + 8);

            // tagBytes = 12 + 2 * count
            // We need to do safe addition here to avoid integer overflow.
            if (!safe_add(count, count, &tagBytes) ||
                !safe_add((size_t) 12, tagBytes, &tagBytes))
            {
                SkColorSpacePrintf("Invalid gamma count");
                return SkGammas::Type::kNone_Type;
            }

            if (len < tagBytes) {
                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                return SkGammas::Type::kNone_Type;
            }
            *outTagBytes = tagBytes;

            if (0 == count) {
                // Some tags require a gamma curve, but the author doesn't actually want
                // to transform the data.  In this case, it is common to see a curve with
                // a count of 0.
                outData->fNamed = kLinear_SkGammaNamed;
                return SkGammas::Type::kNamed_Type;
            }

            const uint16_t* table = (const uint16_t*) (src + 12);
            if (1 == count) {
                // The table entry is the gamma (with a bias of 256).
                float value = (read_big_endian_u16((const uint8_t*) table)) / 256.0f;
                SkColorSpacePrintf("gamma %g\n", value);

                return set_gamma_value(outData, value);
            }

            // Check for frequently occurring sRGB curves.
            // We do this by sampling a few values and see if they match our expectation.
            // A more robust solution would be to compare each value in this curve against
            // an sRGB curve to see if we remain below an error threshold.  At this time,
            // we haven't seen any images in the wild that make this kind of
            // calculation necessary.  We encounter identical gamma curves over and
            // over again, but relatively few variations.
            if (1024 == count) {
                // The magic values were chosen because they match both the very common
                // HP sRGB gamma table and the less common Canon sRGB gamma table (which use
                // different rounding rules).
                if (0 == read_big_endian_u16((const uint8_t*) &table[0]) &&
                        3366 == read_big_endian_u16((const uint8_t*) &table[257]) &&
                        14116 == read_big_endian_u16((const uint8_t*) &table[513]) &&
                        34318 == read_big_endian_u16((const uint8_t*) &table[768]) &&
                        65535 == read_big_endian_u16((const uint8_t*) &table[1023])) {
                    outData->fNamed = kSRGB_SkGammaNamed;
                    return SkGammas::Type::kNamed_Type;
                }
            }

            if (26 == count) {
                // The magic values were chosen because they match a very common LCMS sRGB
                // gamma table.
                if (0 == read_big_endian_u16((const uint8_t*) &table[0]) &&
                        3062 == read_big_endian_u16((const uint8_t*) &table[6]) &&
                        12824 == read_big_endian_u16((const uint8_t*) &table[12]) &&
                        31237 == read_big_endian_u16((const uint8_t*) &table[18]) &&
                        65535 == read_big_endian_u16((const uint8_t*) &table[25])) {
                    outData->fNamed = kSRGB_SkGammaNamed;
                    return SkGammas::Type::kNamed_Type;
                }
            }

            if (4096 == count) {
                // The magic values were chosen because they match Nikon, Epson, and
                // LCMS sRGB gamma tables (all of which use different rounding rules).
                if (0 == read_big_endian_u16((const uint8_t*) &table[0]) &&
                        950 == read_big_endian_u16((const uint8_t*) &table[515]) &&
                        3342 == read_big_endian_u16((const uint8_t*) &table[1025]) &&
                        14079 == read_big_endian_u16((const uint8_t*) &table[2051]) &&
                        65535 == read_big_endian_u16((const uint8_t*) &table[4095])) {
                    outData->fNamed = kSRGB_SkGammaNamed;
                    return SkGammas::Type::kNamed_Type;
                }
            }

            // Otherwise, we will represent gamma with a table.
            outData->fTable.fSize = count;
            return SkGammas::Type::kTable_Type;
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
            uint16_t format = read_big_endian_u16(src + 8);
            if (format > kGABCDEF_ParaCurveType) {
                SkColorSpacePrintf("Unsupported gamma tag type %d\n", type);
                return SkGammas::Type::kNone_Type;
            }

            if (kExponential_ParaCurveType == format) {
                tagBytes = 12 + 4;
                if (len < tagBytes) {
                    SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                    return SkGammas::Type::kNone_Type;
                }

                // Y = X^g
                float g = read_big_endian_16_dot_16(src + 12);

                *outTagBytes = tagBytes;
                return set_gamma_value(outData, g);
            }

            // Here's where the real parametric gammas start.  There are many
            // permutations of the same equations.
            //
            // Y = (aX + b)^g + c  for X >= d
            // Y = eX + f          otherwise
            //
            // We will fill in with zeros as necessary to always match the above form.
            if (len < 24) {
                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                return SkGammas::Type::kNone_Type;
            }
            float g = read_big_endian_16_dot_16(src + 12);
            float a = read_big_endian_16_dot_16(src + 16);
            float b = read_big_endian_16_dot_16(src + 20);
            float c = 0.0f, d = 0.0f, e = 0.0f, f = 0.0f;
            switch(format) {
                case kGAB_ParaCurveType:
                    tagBytes = 12 + 12;

                    // Y = (aX + b)^g  for X >= -b/a
                    // Y = 0           otherwise
                    d = -b / a;
                    break;
                case kGABC_ParaCurveType:
                    tagBytes = 12 + 16;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return SkGammas::Type::kNone_Type;
                    }

                    // Y = (aX + b)^g + c  for X >= -b/a
                    // Y = c               otherwise
                    c = read_big_endian_16_dot_16(src + 24);
                    d = -b / a;
                    f = c;
                    break;
                case kGABDE_ParaCurveType:
                    tagBytes = 12 + 20;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return SkGammas::Type::kNone_Type;
                    }

                    // Y = (aX + b)^g  for X >= d
                    // Y = eX          otherwise
                    d = read_big_endian_16_dot_16(src + 28);

                    // Not a bug!  We define |e| to always be the coefficient on X in the
                    // second equation.  The spec calls this |c| in this particular equation.
                    // We don't follow their convention because then |c| would have a
                    // different meaning in each of our cases.
                    e = read_big_endian_16_dot_16(src + 24);
                    break;
                case kGABCDEF_ParaCurveType:
                    tagBytes = 12 + 28;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return SkGammas::Type::kNone_Type;
                    }

                    // Y = (aX + b)^g + c  for X >= d
                    // Y = eX + f          otherwise
                    // NOTE: The ICC spec writes "cX" in place of "eX" but I think
                    //       it's a typo.
                    c = read_big_endian_16_dot_16(src + 24);
                    d = read_big_endian_16_dot_16(src + 28);
                    e = read_big_endian_16_dot_16(src + 32);
                    f = read_big_endian_16_dot_16(src + 36);
                    break;
                default:
                    SkASSERT(false);
                    return SkGammas::Type::kNone_Type;
            }

            // Recognize and simplify a very common parametric representation of sRGB gamma.
            if (color_space_almost_equal(0.9479f, a) &&
                    color_space_almost_equal(0.0521f, b) &&
                    color_space_almost_equal(0.0000f, c) &&
                    color_space_almost_equal(0.0405f, d) &&
                    color_space_almost_equal(0.0774f, e) &&
                    color_space_almost_equal(0.0000f, f) &&
                    color_space_almost_equal(2.4000f, g)) {
                outData->fNamed = kSRGB_SkGammaNamed;
                return SkGammas::Type::kNamed_Type;
            }

            // Fail on invalid gammas.
            if (SkScalarIsNaN(d)) {
                return SkGammas::Type::kNone_Type;
            }

            if (d <= 0.0f) {
                // Y = (aX + b)^g + c  for always
                if (0.0f == a || 0.0f == g) {
                    SkColorSpacePrintf("A or G is zero, constant gamma function "
                                       "is nonsense");
                    return SkGammas::Type::kNone_Type;
                }
            }

            if (d >= 1.0f) {
                // Y = eX + f          for always
                if (0.0f == e) {
                    SkColorSpacePrintf("E is zero, constant gamma function is "
                                       "nonsense");
                    return SkGammas::Type::kNone_Type;
                }
            }

            if ((0.0f == a || 0.0f == g) && 0.0f == e) {
                SkColorSpacePrintf("A or G, and E are zero, constant gamma function "
                                   "is nonsense");
                return SkGammas::Type::kNone_Type;
            }

            *outTagBytes = tagBytes;

            outParams->fG = g;
            outParams->fA = a;
            outParams->fB = b;
            outParams->fC = c;
            outParams->fD = d;
            outParams->fE = e;
            outParams->fF = f;
            return SkGammas::Type::kParam_Type;
        }
        default:
            SkColorSpacePrintf("Unsupported gamma tag type %d\n", type);
            return SkGammas::Type::kNone_Type;
    }
}

/**
 *  Returns the additional size in bytes needed to store the gamma tag.
 */
static size_t gamma_alloc_size(SkGammas::Type type, const SkGammas::Data& data) {
    switch (type) {
        case SkGammas::Type::kNamed_Type:
        case SkGammas::Type::kValue_Type:
            return 0;
        case SkGammas::Type::kTable_Type:
            return sizeof(float) * data.fTable.fSize;
        case SkGammas::Type::kParam_Type:
            return sizeof(SkGammas::Params);
        default:
            SkASSERT(false);
            return 0;
    }
}

/**
 *  Sets invalid gamma to the default value.
 */
static void handle_invalid_gamma(SkGammas::Type* type, SkGammas::Data* data) {
    if (SkGammas::Type::kNone_Type == *type) {
        *type = SkGammas::Type::kNamed_Type;

        // Guess sRGB in the case of a malformed transfer function.
        data->fNamed = kSRGB_SkGammaNamed;
    }
}

/**
 *  Finish loading the gammas, now that we have allocated memory for the SkGammas struct.
 *
 *  There's nothing to do for the simple cases, but for table gammas we need to actually
 *  read the table into heap memory.  And for parametric gammas, we need to copy over the
 *  parameter values.
 *
 *  @param memory Pointer to start of the SkGammas memory block
 *  @param offset Bytes of memory (after the SkGammas struct) that are already in use.
 *  @param data   In-out variable.  Will fill in the offset to the table or parameters
 *                if necessary.
 *  @param params Parameters for gamma curve.  Only initialized/used when we have a
 *                parametric gamma.
 *  @param src    Pointer to start of the gamma tag.
 *
 *  @return       Additional bytes of memory that are being used by this gamma curve.
 */
static size_t load_gammas(void* memory, size_t offset, SkGammas::Type type,
                        SkGammas::Data* data, const SkGammas::Params& params,
                        const uint8_t* src) {
    void* storage = SkTAddOffset<void>(memory, offset + sizeof(SkGammas));

    switch (type) {
        case SkGammas::Type::kNamed_Type:
        case SkGammas::Type::kValue_Type:
            // Nothing to do here.
            return 0;
        case SkGammas::Type::kTable_Type: {
            data->fTable.fOffset = offset;

            float* outTable = (float*) storage;
            const uint16_t* inTable = (const uint16_t*) (src + 12);
            for (int i = 0; i < data->fTable.fSize; i++) {
                outTable[i] = (read_big_endian_u16((const uint8_t*) &inTable[i])) / 65535.0f;
            }

            return sizeof(float) * data->fTable.fSize;
        }
        case SkGammas::Type::kParam_Type:
            data->fTable.fOffset = offset;
            memcpy(storage, &params, sizeof(SkGammas::Params));
            return sizeof(SkGammas::Params);
        default:
            SkASSERT(false);
            return 0;
    }
}

static constexpr uint32_t kTAG_AtoBType = SkSetFourByteTag('m', 'A', 'B', ' ');

static bool load_color_lut(sk_sp<SkColorLookUpTable>* colorLUT, uint32_t inputChannels,
                           const uint8_t* src, size_t len) {
    // 16 bytes reserved for grid points, 2 for precision, 2 for padding.
    // The color LUT data follows after this header.
    static constexpr uint32_t kColorLUTHeaderSize = 20;
    if (len < kColorLUTHeaderSize) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }
    size_t dataLen = len - kColorLUTHeaderSize;

    SkASSERT(3 == inputChannels);
    uint8_t gridPoints[3];
    uint32_t numEntries = 1;
    for (uint32_t i = 0; i < inputChannels; i++) {
        gridPoints[i] = src[i];
        if (0 == src[i]) {
            SkColorSpacePrintf("Each input channel must have at least one grid point.");
            return false;
        }

        if (!safe_mul(numEntries, src[i], &numEntries)) {
            SkColorSpacePrintf("Too many entries in Color LUT.");
            return false;
        }
    }

    if (!safe_mul(numEntries, SkColorLookUpTable::kOutputChannels, &numEntries)) {
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
    void* memory = sk_malloc_throw(sizeof(SkColorLookUpTable) + sizeof(float) * numEntries);
    *colorLUT = sk_sp<SkColorLookUpTable>(new (memory) SkColorLookUpTable(inputChannels,
                                                                          gridPoints));

    float* table = SkTAddOffset<float>(memory, sizeof(SkColorLookUpTable));
    const uint8_t* ptr = src + kColorLUTHeaderSize;
    for (uint32_t i = 0; i < numEntries; i++, ptr += precision) {
        if (1 == precision) {
            table[i] = ((float) ptr[i]) / 255.0f;
        } else {
            table[i] = ((float) read_big_endian_u16(ptr)) / 65535.0f;
        }
    }

    return true;
}

static bool load_matrix(SkMatrix44* toXYZ, const uint8_t* src, size_t len) {
    if (len < 48) {
        SkColorSpacePrintf("Matrix tag is too small (%d bytes).", len);
        return false;
    }

    // For this matrix to behave like our "to XYZ D50" matrices, it needs to be scaled.
    constexpr float scale = 65535.0 / 32768.0;
    float array[16];
    array[ 0] = scale * SkFixedToFloat(read_big_endian_i32(src));
    array[ 1] = scale * SkFixedToFloat(read_big_endian_i32(src + 4));
    array[ 2] = scale * SkFixedToFloat(read_big_endian_i32(src + 8));
    array[ 3] = scale * SkFixedToFloat(read_big_endian_i32(src + 36)); // translate R
    array[ 4] = scale * SkFixedToFloat(read_big_endian_i32(src + 12));
    array[ 5] = scale * SkFixedToFloat(read_big_endian_i32(src + 16));
    array[ 6] = scale * SkFixedToFloat(read_big_endian_i32(src + 20));
    array[ 7] = scale * SkFixedToFloat(read_big_endian_i32(src + 40)); // translate G
    array[ 8] = scale * SkFixedToFloat(read_big_endian_i32(src + 24));
    array[ 9] = scale * SkFixedToFloat(read_big_endian_i32(src + 28));
    array[10] = scale * SkFixedToFloat(read_big_endian_i32(src + 32));
    array[11] = scale * SkFixedToFloat(read_big_endian_i32(src + 44)); // translate B
    array[12] = 0.0f;
    array[13] = 0.0f;
    array[14] = 0.0f;
    array[15] = 1.0f;
    toXYZ->setRowMajorf(array);
    return true;
}

static inline SkGammaNamed is_named(const sk_sp<SkGammas>& gammas) {
    if (gammas->isNamed(0) && gammas->isNamed(1) && gammas->isNamed(2) &&
        gammas->fRedData.fNamed == gammas->fGreenData.fNamed &&
        gammas->fRedData.fNamed == gammas->fBlueData.fNamed)
    {
        return gammas->fRedData.fNamed;
    }

    return kNonStandard_SkGammaNamed;
}


static bool load_a2b0(sk_sp<SkColorLookUpTable>* colorLUT, SkGammaNamed* gammaNamed,
                      sk_sp<SkGammas>* gammas, SkMatrix44* toXYZ, const uint8_t* src, size_t len) {
    if (len < 32) {
        SkColorSpacePrintf("A to B tag is too small (%d bytes).", len);
        return false;
    }

    uint32_t type = read_big_endian_u32(src);
    if (kTAG_AtoBType != type) {
        // FIXME (msarett): Need to support lut8Type and lut16Type.
        SkColorSpacePrintf("Unsupported A to B tag type.\n");
        return false;
    }

    // Read the number of channels.  The four bytes that we skipped are reserved and
    // must be zero.
    uint8_t inputChannels = src[8];
    uint8_t outputChannels = src[9];
    if (3 != inputChannels || SkColorLookUpTable::kOutputChannels != outputChannels) {
        // We only handle (supposedly) RGB inputs and RGB outputs.  The numbers of input
        // channels and output channels both must be 3.
        // TODO (msarett):
        // Support different numbers of input channels.  Ex: CMYK (4).
        SkColorSpacePrintf("Input and output channels must equal 3 in A to B tag.\n");
        return false;
    }

    // Read the offsets of each element in the A to B tag.  With the exception of A curves and
    // B curves (which we do not yet support), we will handle these elements in the order in
    // which they should be applied (rather than the order in which they occur in the tag).
    // If the offset is non-zero it indicates that the element is present.
    uint32_t offsetToACurves = read_big_endian_i32(src + 28);
    uint32_t offsetToBCurves = read_big_endian_i32(src + 12);
    if ((0 != offsetToACurves) || (0 != offsetToBCurves)) {
        // FIXME (msarett): Handle A and B curves.
        // Note that the A curve is technically required in order to have a color LUT.
        // However, all the A curves I have seen so far have are just placeholders that
        // don't actually transform the data.
        SkColorSpacePrintf("Ignoring A and/or B curve.  Output may be wrong.\n");
    }

    uint32_t offsetToColorLUT = read_big_endian_i32(src + 24);
    if (0 != offsetToColorLUT && offsetToColorLUT < len) {
        if (!load_color_lut(colorLUT, inputChannels, src + offsetToColorLUT,
                            len - offsetToColorLUT)) {
            SkColorSpacePrintf("Failed to read color LUT from A to B tag.\n");
        }
    }

    uint32_t offsetToMCurves = read_big_endian_i32(src + 20);
    if (0 != offsetToMCurves && offsetToMCurves < len) {
        const uint8_t* rTagPtr = src + offsetToMCurves;
        size_t tagLen = len - offsetToMCurves;

        SkGammas::Data rData;
        SkGammas::Params rParams;

        // On an invalid first gamma, tagBytes remains set as zero.  This causes the two
        // subsequent to be treated as identical (which is what we want).
        size_t tagBytes = 0;
        SkGammas::Type rType = parse_gamma(&rData, &rParams, &tagBytes, rTagPtr, tagLen);
        handle_invalid_gamma(&rType, &rData);
        size_t alignedTagBytes = SkAlign4(tagBytes);

        if ((3 * alignedTagBytes <= tagLen) &&
            !memcmp(rTagPtr, rTagPtr + 1 * alignedTagBytes, tagBytes) &&
            !memcmp(rTagPtr, rTagPtr + 2 * alignedTagBytes, tagBytes))
        {
            if (SkGammas::Type::kNamed_Type == rType) {
                *gammaNamed = rData.fNamed;
            } else {
                size_t allocSize = sizeof(SkGammas);
                return_if_false(safe_add(allocSize, gamma_alloc_size(rType, rData), &allocSize),
                                "SkGammas struct is too large to allocate");
                void* memory = sk_malloc_throw(allocSize);
                *gammas = sk_sp<SkGammas>(new (memory) SkGammas());
                load_gammas(memory, 0, rType, &rData, rParams, rTagPtr);

                (*gammas)->fRedType = rType;
                (*gammas)->fGreenType = rType;
                (*gammas)->fBlueType = rType;

                (*gammas)->fRedData = rData;
                (*gammas)->fGreenData = rData;
                (*gammas)->fBlueData = rData;
            }
        } else {
            const uint8_t* gTagPtr = rTagPtr + alignedTagBytes;
            tagLen = tagLen > alignedTagBytes ? tagLen - alignedTagBytes : 0;
            SkGammas::Data gData;
            SkGammas::Params gParams;
            tagBytes = 0;
            SkGammas::Type gType = parse_gamma(&gData, &gParams, &tagBytes, gTagPtr,
                                                       tagLen);
            handle_invalid_gamma(&gType, &gData);

            alignedTagBytes = SkAlign4(tagBytes);
            const uint8_t* bTagPtr = gTagPtr + alignedTagBytes;
            tagLen = tagLen > alignedTagBytes ? tagLen - alignedTagBytes : 0;
            SkGammas::Data bData;
            SkGammas::Params bParams;
            SkGammas::Type bType = parse_gamma(&bData, &bParams, &tagBytes, bTagPtr,
                                                       tagLen);
            handle_invalid_gamma(&bType, &bData);

            size_t allocSize = sizeof(SkGammas);
            return_if_false(safe_add(allocSize, gamma_alloc_size(rType, rData), &allocSize),
                            "SkGammas struct is too large to allocate");
            return_if_false(safe_add(allocSize, gamma_alloc_size(gType, gData), &allocSize),
                            "SkGammas struct is too large to allocate");
            return_if_false(safe_add(allocSize, gamma_alloc_size(bType, bData), &allocSize),
                            "SkGammas struct is too large to allocate");
            void* memory = sk_malloc_throw(allocSize);
            *gammas = sk_sp<SkGammas>(new (memory) SkGammas());

            uint32_t offset = 0;
            (*gammas)->fRedType = rType;
            offset += load_gammas(memory, offset, rType, &rData, rParams, rTagPtr);

            (*gammas)->fGreenType = gType;
            offset += load_gammas(memory, offset, gType, &gData, gParams, gTagPtr);

            (*gammas)->fBlueType = bType;
            load_gammas(memory, offset, bType, &bData, bParams, bTagPtr);

            (*gammas)->fRedData = rData;
            (*gammas)->fGreenData = gData;
            (*gammas)->fBlueData = bData;
        }
    } else {
        // Guess sRGB if the chunk is missing a transfer function.
        *gammaNamed = kSRGB_SkGammaNamed;
    }

    if (kNonStandard_SkGammaNamed == *gammaNamed) {
        *gammaNamed = is_named(*gammas);
        if (kNonStandard_SkGammaNamed != *gammaNamed) {
            // No need to keep the gammas struct, the enum is enough.
            *gammas = nullptr;
        }
    }

    uint32_t offsetToMatrix = read_big_endian_i32(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        if (!load_matrix(toXYZ, src + offsetToMatrix, len - offsetToMatrix)) {
            SkColorSpacePrintf("Failed to read matrix from A to B tag.\n");
            toXYZ->setIdentity();
        }
    }

    return true;
}

static bool tag_equals(const ICCTag* a, const ICCTag* b, const uint8_t* base) {
    if (!a || !b) {
        return a == b;
    }

    if (a->fLength != b->fLength) {
        return false;
    }

    if (a->fOffset == b->fOffset) {
        return true;
    }

    return !memcmp(a->addr(base), b->addr(base), a->fLength);
}

sk_sp<SkColorSpace> SkColorSpace::NewICC(const void* input, size_t len) {
    if (!input || len < kICCHeaderSize) {
        return_null("Data is null or not large enough to contain an ICC profile");
    }

    // Create our own copy of the input.
    void* memory = sk_malloc_throw(len);
    memcpy(memory, input, len);
    sk_sp<SkData> data = SkData::MakeFromMalloc(memory, len);
    const uint8_t* base = data->bytes();
    const uint8_t* ptr = base;

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
                if (!load_xyz(&toXYZ[0], r->addr(base), r->fLength) ||
                    !load_xyz(&toXYZ[3], g->addr(base), g->fLength) ||
                    !load_xyz(&toXYZ[6], b->addr(base), b->fLength))
                {
                    return_null("Need valid rgb tags for XYZ space");
                }
                SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
                mat.set3x3(toXYZ[0], toXYZ[1], toXYZ[2],
                           toXYZ[3], toXYZ[4], toXYZ[5],
                           toXYZ[6], toXYZ[7], toXYZ[8]);

                r = ICCTag::Find(tags.get(), tagCount, kTAG_rTRC);
                g = ICCTag::Find(tags.get(), tagCount, kTAG_gTRC);
                b = ICCTag::Find(tags.get(), tagCount, kTAG_bTRC);

                // If some, but not all, of the gamma tags are missing, assume that all
                // gammas are meant to be the same.  This behavior is an arbitrary guess,
                // but it simplifies the code below.
                if ((!r || !g || !b) && (r || g || b)) {
                    if (!r) {
                        r = g ? g : b;
                    }

                    if (!g) {
                        g = r ? r : b;
                    }

                    if (!b) {
                        b = r ? r : g;
                    }
                }

                SkGammaNamed gammaNamed = kNonStandard_SkGammaNamed;
                sk_sp<SkGammas> gammas = nullptr;
                size_t tagBytes;
                if (r && g && b) {
                    if (tag_equals(r, g, base) && tag_equals(g, b, base)) {
                        SkGammas::Data data;
                        SkGammas::Params params;
                        SkGammas::Type type =
                                parse_gamma(&data, &params, &tagBytes, r->addr(base), r->fLength);
                        handle_invalid_gamma(&type, &data);

                        if (SkGammas::Type::kNamed_Type == type) {
                            gammaNamed = data.fNamed;
                        } else {
                            size_t allocSize = sizeof(SkGammas);
                            if (!safe_add(allocSize, gamma_alloc_size(type, data), &allocSize)) {
                                return_null("SkGammas struct is too large to allocate");
                            }
                            void* memory = sk_malloc_throw(allocSize);
                            gammas = sk_sp<SkGammas>(new (memory) SkGammas());
                            load_gammas(memory, 0, type, &data, params, r->addr(base));

                            gammas->fRedType = type;
                            gammas->fGreenType = type;
                            gammas->fBlueType = type;

                            gammas->fRedData = data;
                            gammas->fGreenData = data;
                            gammas->fBlueData = data;
                        }
                    } else {
                        SkGammas::Data rData;
                        SkGammas::Params rParams;
                        SkGammas::Type rType =
                                parse_gamma(&rData, &rParams, &tagBytes, r->addr(base), r->fLength);
                        handle_invalid_gamma(&rType, &rData);

                        SkGammas::Data gData;
                        SkGammas::Params gParams;
                        SkGammas::Type gType =
                                parse_gamma(&gData, &gParams, &tagBytes, g->addr(base), g->fLength);
                        handle_invalid_gamma(&gType, &gData);

                        SkGammas::Data bData;
                        SkGammas::Params bParams;
                        SkGammas::Type bType =
                                parse_gamma(&bData, &bParams, &tagBytes, b->addr(base), b->fLength);
                        handle_invalid_gamma(&bType, &bData);

                        size_t allocSize = sizeof(SkGammas);
                        if (!safe_add(allocSize, gamma_alloc_size(rType, rData), &allocSize) ||
                            !safe_add(allocSize, gamma_alloc_size(gType, gData), &allocSize) ||
                            !safe_add(allocSize, gamma_alloc_size(bType, bData), &allocSize))
                        {
                            return_null("SkGammas struct is too large to allocate");
                        }
                        void* memory = sk_malloc_throw(allocSize);
                        gammas = sk_sp<SkGammas>(new (memory) SkGammas());

                        uint32_t offset = 0;
                        gammas->fRedType = rType;
                        offset += load_gammas(memory, offset, rType, &rData, rParams,
                                              r->addr(base));

                        gammas->fGreenType = gType;
                        offset += load_gammas(memory, offset, gType, &gData, gParams,
                                              g->addr(base));

                        gammas->fBlueType = bType;
                        load_gammas(memory, offset, bType, &bData, bParams, b->addr(base));

                        gammas->fRedData = rData;
                        gammas->fGreenData = gData;
                        gammas->fBlueData = bData;
                    }
                } else {
                    // Guess sRGB if the profile is missing transfer functions.
                    gammaNamed = kSRGB_SkGammaNamed;
                }

                if (kNonStandard_SkGammaNamed == gammaNamed) {
                    // It's possible that we'll initially detect non-matching gammas, only for
                    // them to evaluate to the same named gamma curve.
                    gammaNamed = is_named(gammas);
                    if (kNonStandard_SkGammaNamed == gammaNamed) {
                        return sk_sp<SkColorSpace>(new SkColorSpace_Base(nullptr, gammaNamed,
                                                                         std::move(gammas), mat,
                                                                         std::move(data)));
                    }
                }

                return SkColorSpace_Base::NewRGB(gammaNamed, mat);
            }

            // Recognize color profile specified by A2B0 tag.
            const ICCTag* a2b0 = ICCTag::Find(tags.get(), tagCount, kTAG_A2B0);
            if (a2b0) {
                SkGammaNamed gammaNamed = kNonStandard_SkGammaNamed;
                sk_sp<SkGammas> gammas = nullptr;
                sk_sp<SkColorLookUpTable> colorLUT = nullptr;
                SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
                if (!load_a2b0(&colorLUT, &gammaNamed, &gammas, &toXYZ, a2b0->addr(base),
                               a2b0->fLength)) {
                    return_null("Failed to parse A2B0 tag");
                }

                if (colorLUT || kNonStandard_SkGammaNamed == gammaNamed) {
                    return sk_sp<SkColorSpace>(new SkColorSpace_Base(std::move(colorLUT),
                                                                     gammaNamed, std::move(gammas),
                                                                     toXYZ, std::move(data)));
                }

                return SkColorSpace_Base::NewRGB(gammaNamed, toXYZ);
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

static void write_xyz_tag(uint32_t* ptr, const SkMatrix44& toXYZ, int col) {
    ptr[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(0, col)));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(1, col)));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(2, col)));
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
    SkGammaNamed gammaNamed = this->gammaNamed();
    if (kNonStandard_SkGammaNamed == gammaNamed) {
        // FIXME (msarett):
        // Write the correct gamma representation rather than 2.2f.
        write_trc_tag((uint32_t*) ptr, 2.2f);
        ptr += SkAlign4(kTAG_TRC_Bytes);
        write_trc_tag((uint32_t*) ptr, 2.2f);
        ptr += SkAlign4(kTAG_TRC_Bytes);
        write_trc_tag((uint32_t*) ptr, 2.2f);
        ptr += SkAlign4(kTAG_TRC_Bytes);
    } else {
        switch (gammaNamed) {
            case kSRGB_SkGammaNamed:
                // FIXME (msarett):
                // kSRGB cannot be represented by a value.  Here we fall through to 2.2f,
                // which is a close guess.  To be more accurate, we need to represent sRGB
                // gamma with a parametric curve.
            case k2Dot2Curve_SkGammaNamed:
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                write_trc_tag((uint32_t*) ptr, 2.2f);
                ptr += SkAlign4(kTAG_TRC_Bytes);
                break;
            case kLinear_SkGammaNamed:
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
