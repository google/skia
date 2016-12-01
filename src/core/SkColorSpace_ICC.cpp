/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
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
static constexpr uint32_t kLAB_PCSSpace       = SkSetFourByteTag('L', 'a', 'b', ' ');
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

        switch (fPCS) {
            case kXYZ_PCSSpace:
                SkColorSpacePrintf("XYZ PCS\n");
                break;
            case kLAB_PCSSpace:
                SkColorSpacePrintf("Lab PCS\n");
                break;
            default:
                // ICC currently (V4.3) only specifices XYZ and Lab PCS spaces
                SkColorSpacePrintf("Unsupported PCS space\n");
                return false;
        }

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
static SkGammas::Type parse_gamma(SkGammas::Data* outData, SkColorSpaceTransferFn* outParams,
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
                // The magic values match a clever "minimum size" approach to representing sRGB.
                // code.facebook.com/posts/411525055626587/under-the-hood-improving-facebook-photos
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
                // lcms2 sRGB gamma tables (all of which use different rounding rules).
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

            outParams->fG = g;
            outParams->fA = a;
            outParams->fB = b;
            outParams->fC = c;
            outParams->fD = d;
            outParams->fE = e;
            outParams->fF = f;

            if (!is_valid_transfer_fn(*outParams)) {
                return SkGammas::Type::kNone_Type;
            }

            if (is_almost_srgb(*outParams)) {
                outData->fNamed = kSRGB_SkGammaNamed;
                return SkGammas::Type::kNamed_Type;
            }

            if (is_almost_2dot2(*outParams)) {
                outData->fNamed = k2Dot2Curve_SkGammaNamed;
                return SkGammas::Type::kNamed_Type;
            }

            *outTagBytes = tagBytes;
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
            return sizeof(SkColorSpaceTransferFn);
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
                        SkGammas::Data* data, const SkColorSpaceTransferFn& params,
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
            memcpy(storage, &params, sizeof(SkColorSpaceTransferFn));
            return sizeof(SkColorSpaceTransferFn);
        default:
            SkASSERT(false);
            return 0;
    }
}

static constexpr uint32_t kTAG_AtoBType  = SkSetFourByteTag('m', 'A', 'B', ' ');
static constexpr uint32_t kTAG_lut8Type  = SkSetFourByteTag('m', 'f', 't', '1');
static constexpr uint32_t kTAG_lut16Type = SkSetFourByteTag('m', 'f', 't', '2');

static bool load_color_lut(sk_sp<SkColorLookUpTable>* colorLUT, uint32_t inputChannels,
                           size_t precision, const uint8_t gridPoints[3], const uint8_t* src,
                           size_t len) {
    switch (precision) {
        case 1: //  8-bit data
        case 2: // 16-bit data
            break;
        default:
            SkColorSpacePrintf("Color LUT precision must be 8-bit or 16-bit. Found: %d-bit\n",
                               8*precision);
            return false;
    }

    SkASSERT(3 == inputChannels);
    uint32_t numEntries = SkColorLookUpTable::kOutputChannels;
    for (uint32_t i = 0; i < inputChannels; i++) {
        if (0 == gridPoints[i]) {
            SkColorSpacePrintf("Each input channel must have at least one grid point.");
            return false;
        }

        if (!safe_mul(numEntries, gridPoints[i], &numEntries)) {
            SkColorSpacePrintf("Too many entries in Color LUT.");
            return false;
        }
    }

    uint32_t clutBytes;
    if (!safe_mul(numEntries, precision, &clutBytes)) {
        SkColorSpacePrintf("Too many entries in Color LUT.\n");
        return false;
    }

    if (len < clutBytes) {
        SkColorSpacePrintf("Color LUT tag is too small (%d / %d bytes).\n", len, clutBytes);
        return false;
    }

    // Movable struct colorLUT has ownership of fTable.
    void* memory = sk_malloc_throw(sizeof(SkColorLookUpTable) + sizeof(float) * numEntries);
    *colorLUT = sk_sp<SkColorLookUpTable>(new (memory) SkColorLookUpTable(inputChannels,
                                                                          gridPoints));

    float* table = SkTAddOffset<float>(memory, sizeof(SkColorLookUpTable));
    const uint8_t* ptr = src;
    for (uint32_t i = 0; i < numEntries; i++, ptr += precision) {
        if (1 == precision) {
            table[i] = ((float) *ptr) / 255.0f;
        } else {
            table[i] = ((float) read_big_endian_u16(ptr)) / 65535.0f;
        }
    }

    return true;
}

/**
 *  Reads a matrix out of an A2B tag of an ICC profile.
 *  If |translate| is true, it will load a 3x4 matrix out that corresponds to a XYZ
 *  transform as well as a translation, and if |translate| is false it only loads a
 *  3x3 matrix with no translation
 *
 *  @param matrix    The matrix to store the result in
 *  @param src       Data to load the matrix out of. 
 *  @param len       The length of |src|.
 *                   Must have 48 bytes if |translate| is set and 36 bytes otherwise.
 *  @param translate Whether to read the translation column or not
 *  @param pcs       The profile connection space of the profile this matrix is for
 *
 *  @return          false on failure, true on success
 */
static bool load_matrix(SkMatrix44* matrix, const uint8_t* src, size_t len, bool translate,
                        SkColorSpace_A2B::PCS pcs) {
    const size_t minLen = translate ? 48 : 36;
    if (len < minLen) {
        SkColorSpacePrintf("Matrix tag is too small (%d bytes).", len);
        return false;
    }

    float encodingFactor;
    switch (pcs) {
        case SkColorSpace_A2B::PCS::kLAB:
            encodingFactor = 1.f;
            break;
        case SkColorSpace_A2B::PCS::kXYZ:
            encodingFactor = 65535 / 32768.f;
            break;
        default:
            encodingFactor = 1.f;
            SkASSERT(false);
            break; 
    }
    float array[16];
    array[ 0] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src));
    array[ 1] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 4));
    array[ 2] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 8));

    array[ 4] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 12));
    array[ 5] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 16));
    array[ 6] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 20));

    array[ 8] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 24));
    array[ 9] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 28));
    array[10] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 32));

    if (translate) {
        array[ 3] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 36)); // translate R
        array[ 7] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 40)); // translate G
        array[11] = encodingFactor * SkFixedToFloat(read_big_endian_i32(src + 44)); // translate B
    } else {
        array[ 3] = 0.0f;
        array[ 7] = 0.0f;
        array[11] = 0.0f;
    }

    array[12] = 0.0f;
    array[13] = 0.0f;
    array[14] = 0.0f;
    array[15] = 1.0f;
    matrix->setRowMajorf(array);
    SkColorSpacePrintf("A2B0 matrix loaded:\n");
    for (int r = 0; r < 4; ++r) {
        SkColorSpacePrintf("|");
        for (int c = 0; c < 4; ++c) {
            SkColorSpacePrintf(" %f ", matrix->get(r, c));
        }
        SkColorSpacePrintf("|\n");
    }
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

/**
 *  Parse and load an entire stored curve. Handles invalid gammas as well.
 *
 *  There's nothing to do for the simple cases, but for table gammas we need to actually
 *  read the table into heap memory.  And for parametric gammas, we need to copy over the
 *  parameter values.
 *
 *  @param gammaNamed Out-variable. The named gamma curve.
 *  @param gammas     Out-variable. The stored gamma curve information. Can be null if
 *                    gammaNamed is a named curve
 *  @param rTagPtr    Pointer to start of the gamma tag.
 *  @param taglen     The size in bytes of the tag
 *
 *  @return           false on failure, true on success
 */
static bool parse_and_load_gamma(SkGammaNamed* gammaNamed, sk_sp<SkGammas>* gammas,
                                 const uint8_t* rTagPtr, size_t tagLen)
{
    SkGammas::Data rData;
    SkColorSpaceTransferFn rParams;

    *gammaNamed = kNonStandard_SkGammaNamed;

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
        SkColorSpaceTransferFn gParams;
        tagBytes = 0;
        SkGammas::Type gType = parse_gamma(&gData, &gParams, &tagBytes, gTagPtr,
                                                   tagLen);
        handle_invalid_gamma(&gType, &gData);

        alignedTagBytes = SkAlign4(tagBytes);
        const uint8_t* bTagPtr = gTagPtr + alignedTagBytes;
        tagLen = tagLen > alignedTagBytes ? tagLen - alignedTagBytes : 0;
        SkGammas::Data bData;
        SkColorSpaceTransferFn bParams;
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

    if (kNonStandard_SkGammaNamed == *gammaNamed) {
        *gammaNamed = is_named(*gammas);
        if (kNonStandard_SkGammaNamed != *gammaNamed) {
            // No need to keep the gammas struct, the enum is enough.
            *gammas = nullptr;
        }
    }
    return true;
}

static bool load_lut_gammas(sk_sp<SkGammas>* gammas, size_t numTables, size_t entriesPerTable,
                            size_t precision, const uint8_t* src, size_t len) {
    if (precision != 1 && precision != 2) {
        SkColorSpacePrintf("Invalid gamma table precision %d\n", precision);
        return false;
    }
    uint32_t totalEntries;
    return_if_false(safe_mul(entriesPerTable, numTables, &totalEntries),
                    "Too many entries in gamma table.");
    uint32_t readBytes;
    return_if_false(safe_mul(precision, totalEntries, &readBytes),
                    "SkGammas struct is too large to read");
    if (len < readBytes) {
        SkColorSpacePrintf("Gamma table is too small. Provided: %d. Required: %d\n",
                           len, readBytes);
        return false;
    }

    uint32_t writeBytesPerChannel;
    return_if_false(safe_mul(sizeof(float), entriesPerTable, &writeBytesPerChannel),
                    "SkGammas struct is too large to allocate");
    const size_t readBytesPerChannel = precision * entriesPerTable;
    size_t numTablesToUse = 1;
    for (size_t tableIndex = 1; tableIndex < numTables; ++tableIndex) {
        if (0 != memcmp(src, src + readBytesPerChannel * tableIndex, readBytesPerChannel)) {
            numTablesToUse = numTables;
            break;
        }
    }

    uint32_t writetableBytes;
    return_if_false(safe_mul(numTablesToUse, writeBytesPerChannel, &writetableBytes),
                    "SkGammas struct is too large to allocate");
    size_t allocSize = sizeof(SkGammas);
    return_if_false(safe_add(allocSize, (size_t)writetableBytes, &allocSize),
                    "SkGammas struct is too large to allocate");

    void* memory = sk_malloc_throw(allocSize);
    *gammas = sk_sp<SkGammas>(new (memory) SkGammas());

    for (size_t tableIndex = 0; tableIndex < numTablesToUse; ++tableIndex) {
        const uint8_t* ptr = src + readBytesPerChannel * tableIndex;
        const size_t offset = sizeof(SkGammas) + tableIndex * writeBytesPerChannel;
        float* table = SkTAddOffset<float>(memory, offset);
        if (1 == precision) {
            for (uint32_t i = 0; i < entriesPerTable; ++i, ptr += 1) {
                table[i] = ((float) *ptr) / 255.0f;
            }
        } else if (2 == precision) {
            for (uint32_t i = 0; i < entriesPerTable; ++i, ptr += 2) {
                table[i] = ((float) read_big_endian_u16(ptr)) / 65535.0f;
            }
        }
    }

    (*gammas)->fRedType   = SkGammas::Type::kTable_Type;
    (*gammas)->fGreenType = SkGammas::Type::kTable_Type;
    (*gammas)->fBlueType  = SkGammas::Type::kTable_Type;

    if (1 == numTablesToUse) {
        (*gammas)->fRedData.fTable.fOffset   = 0;
        (*gammas)->fGreenData.fTable.fOffset = 0;
        (*gammas)->fBlueData.fTable.fOffset  = 0;
    } else {
        (*gammas)->fRedData.fTable.fOffset   = 0;
        (*gammas)->fGreenData.fTable.fOffset = writeBytesPerChannel;
        (*gammas)->fBlueData.fTable.fOffset  = writeBytesPerChannel * 2;
    }

    (*gammas)->fRedData.fTable.fSize   = entriesPerTable;
    (*gammas)->fGreenData.fTable.fSize = entriesPerTable;
    (*gammas)->fBlueData.fTable.fSize  = entriesPerTable;

    return true;
}

bool load_a2b0_a_to_b_type(std::vector<SkColorSpace_A2B::Element>* elements, const uint8_t* src,
                           size_t len, SkColorSpace_A2B::PCS pcs) {
    SkASSERT(len >= 32);
    // Read the number of channels.  The four bytes (4-7) that we skipped are reserved and
    // must be zero.
    const uint8_t inputChannels = src[8];
    const uint8_t outputChannels = src[9];
    if (3 != inputChannels || SkColorLookUpTable::kOutputChannels != outputChannels) {
        // We only handle (supposedly) RGB inputs and RGB outputs.  The numbers of input
        // channels and output channels both must be 3.
        // TODO (msarett):
        // Support different numbers of input channels.  Ex: CMYK (4).
        SkColorSpacePrintf("Input and output channels must equal 3 in A to B tag.\n");
        return false;
    }

    // It is important that these are loaded in the order of application, as the
    // order you construct an A2B color space's elements is the order it is applied

    // If the offset is non-zero it indicates that the element is present.
    const uint32_t offsetToACurves = read_big_endian_i32(src + 28);
    if (0 != offsetToACurves && offsetToACurves < len) {
        const size_t tagLen = len - offsetToACurves;
        SkGammaNamed gammaNamed;
        sk_sp<SkGammas> gammas;
        if (!parse_and_load_gamma(&gammaNamed, &gammas, src + offsetToACurves, tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed));
        }
    }

    const uint32_t offsetToColorLUT = read_big_endian_i32(src + 24);
    if (0 != offsetToColorLUT && offsetToColorLUT < len) {
        sk_sp<SkColorLookUpTable> colorLUT;
        const uint8_t* clutSrc = src + offsetToColorLUT;
        const size_t clutLen = len - offsetToColorLUT;
        // 16 bytes reserved for grid points, 1 for precision, 3 for padding.
        // The color LUT data follows after this header.
        static constexpr uint32_t kColorLUTHeaderSize = 20;
        if (clutLen < kColorLUTHeaderSize) {
            SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", clutLen);
            return false;
        }

        SkASSERT(3 == inputChannels);
        uint8_t gridPoints[3];
        for (uint32_t i = 0; i < inputChannels; ++i) {
            gridPoints[i] = clutSrc[i];
        }
        // Space is provided for a maximum of 16 input channels.
        // Now we determine the precision of the table values.
        const uint8_t precision = clutSrc[16];
        if (!load_color_lut(&colorLUT, inputChannels, precision, gridPoints,
                            clutSrc + kColorLUTHeaderSize, clutLen - kColorLUTHeaderSize)) {
            SkColorSpacePrintf("Failed to read color LUT from A to B tag.\n");
            return false;
        }
        elements->push_back(SkColorSpace_A2B::Element(std::move(colorLUT)));
    }

    const uint32_t offsetToMCurves = read_big_endian_i32(src + 20);
    if (0 != offsetToMCurves && offsetToMCurves < len) {
        const size_t tagLen = len - offsetToMCurves;
        SkGammaNamed gammaNamed;
        sk_sp<SkGammas> gammas;
        if (!parse_and_load_gamma(&gammaNamed, &gammas, src + offsetToMCurves, tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed));
        }
    }

    const uint32_t offsetToMatrix = read_big_endian_i32(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        SkMatrix44 matrix(SkMatrix44::kUninitialized_Constructor);
        if (!load_matrix(&matrix, src + offsetToMatrix, len - offsetToMatrix, true, pcs)) {
            SkColorSpacePrintf("Failed to read matrix from A to B tag.\n");
        } else {
            elements->push_back(SkColorSpace_A2B::Element(matrix));
        }
    }

    const uint32_t offsetToBCurves = read_big_endian_i32(src + 12);
    if (0 != offsetToBCurves && offsetToBCurves < len) {
        const size_t tagLen = len - offsetToBCurves;
        SkGammaNamed gammaNamed;
        sk_sp<SkGammas> gammas;
        if (!parse_and_load_gamma(&gammaNamed, &gammas, src + offsetToBCurves, tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed));
        }
    }

    return true;
}

bool load_a2b0_lutn_type(std::vector<SkColorSpace_A2B::Element>* elements, const uint8_t* src,
                         size_t len, SkColorSpace_A2B::PCS pcs) {
    const uint32_t type = read_big_endian_u32(src);
    switch (type) {
        case kTAG_lut8Type:
            SkASSERT(len >= 48);
            break;
        case kTAG_lut16Type:
            SkASSERT(len >= 52);
            break;
        default:
            SkASSERT(false);
            return false;
    }
    // Read the number of channels.
    // The four bytes (4-7) that we skipped are reserved and must be zero.
    const uint8_t inputChannels = src[8];
    const uint8_t outputChannels = src[9];
    if (3 != inputChannels || SkColorLookUpTable::kOutputChannels != outputChannels) {
        // We only handle (supposedly) RGB inputs and RGB outputs.  The numbers of input
        // channels and output channels both must be 3.
        // TODO (msarett):
        // Support different numbers of input channels.  Ex: CMYK (4).
        SkColorSpacePrintf("Input and output channels must equal 3 in A to B tag.\n");
        return false;
    }

    const uint8_t clutGridPoints = src[10];
    // 11th byte reserved for padding (required to be zero)

    SkMatrix44 matrix(SkMatrix44::kUninitialized_Constructor);
    load_matrix(&matrix, &src[12], len - 12, false, pcs);
    elements->push_back(SkColorSpace_A2B::Element(matrix));

    size_t dataOffset      = 48;
    // # of input table entries
    size_t inTableEntries  = 256;
    // # of output table entries
    size_t outTableEntries = 256;
    size_t precision       = 1;
    if (kTAG_lut16Type == type) {
        dataOffset      = 52;
        inTableEntries  = read_big_endian_u16(src + 48);
        outTableEntries = read_big_endian_u16(src + 50);
        precision       = 2;
        
        constexpr size_t kMaxLut16GammaEntries = 4096;
        if (inTableEntries < 2) {
            SkColorSpacePrintf("Too few (%d) input gamma table entries. Must have at least 2.\n",
                               inTableEntries);
            return false;
        } else if (inTableEntries > kMaxLut16GammaEntries) {
            SkColorSpacePrintf("Too many (%d) input gamma table entries. Must have at most %d.\n",
                               inTableEntries, kMaxLut16GammaEntries);
            return false;
        }
        
        if (outTableEntries < 2) {
            SkColorSpacePrintf("Too few (%d) output gamma table entries. Must have at least 2.\n",
                               outTableEntries);
            return false;
        } else if (outTableEntries > kMaxLut16GammaEntries) {
            SkColorSpacePrintf("Too many (%d) output gamma table entries. Must have at most %d.\n",
                               outTableEntries, kMaxLut16GammaEntries);
            return false;
        }
    }

    const size_t inputOffset = dataOffset;
    return_if_false(len >= inputOffset, "A2B0 lutnType tag too small for input gamma table");
    sk_sp<SkGammas> inputGammas;
    if (!load_lut_gammas(&inputGammas, inputChannels, inTableEntries, precision,
                         src + inputOffset, len - inputOffset)) {
        SkColorSpacePrintf("Failed to read input gammas from lutnType tag.\n");
        return false;
    }
    SkASSERT(inputGammas);
    elements->push_back(SkColorSpace_A2B::Element(std::move(inputGammas)));

    const size_t clutOffset = inputOffset + precision*inTableEntries*inputChannels;
    return_if_false(len >= clutOffset, "A2B0 lutnType tag too small for CLUT");
    sk_sp<SkColorLookUpTable> colorLUT;
    const uint8_t gridPoints[3] = {clutGridPoints, clutGridPoints, clutGridPoints};
    if (!load_color_lut(&colorLUT, inputChannels, precision, gridPoints, src + clutOffset,
                        len - clutOffset)) {
        SkColorSpacePrintf("Failed to read color LUT from lutnType tag.\n");
        return false;
    }
    SkASSERT(colorLUT);
    elements->push_back(SkColorSpace_A2B::Element(std::move(colorLUT)));

    size_t clutSize = precision * outputChannels;
    for (int i = 0; i < inputChannels; ++i) {
        clutSize *= clutGridPoints;
    }
    const size_t outputOffset = clutOffset + clutSize;
    return_if_false(len >= outputOffset, "A2B0 lutnType tag too small for output gamma table");
    sk_sp<SkGammas> outputGammas;
    if (!load_lut_gammas(&outputGammas, outputChannels, outTableEntries, precision,
                         src + outputOffset, len - outputOffset)) {
        SkColorSpacePrintf("Failed to read output gammas from lutnType tag.\n");
        return false;
    }
    SkASSERT(outputGammas);
    elements->push_back(SkColorSpace_A2B::Element(std::move(outputGammas)));

    return true;
}

static bool load_a2b0(std::vector<SkColorSpace_A2B::Element>* elements, const uint8_t* src,
                      size_t len, SkColorSpace_A2B::PCS pcs) {
    const uint32_t type = read_big_endian_u32(src);
    switch (type) {
        case kTAG_AtoBType:
            if (len < 32) {
                SkColorSpacePrintf("A to B tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag is of type lutAtoBType\n");
            return load_a2b0_a_to_b_type(elements, src, len, pcs);
        case kTAG_lut8Type:
            if (len < 48) {
                SkColorSpacePrintf("lut8 tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag of type lut8Type\n");
            return load_a2b0_lutn_type(elements, src, len, pcs);
        case kTAG_lut16Type:
            if (len < 52) {
                SkColorSpacePrintf("lut16 tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag of type lut16Type\n");
            return load_a2b0_lutn_type(elements, src, len, pcs);
        default:
            SkColorSpacePrintf("Unsupported A to B tag type: %c%c%c%c\n", (type>>24)&0xFF,
                               (type>>16)&0xFF, (type>>8)&0xFF, type&0xFF);
    }
    return false;
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

static inline bool is_close_to_d50(const SkMatrix44& matrix) {
    // rX + gX + bX
    float X = matrix.getFloat(0, 0) + matrix.getFloat(0, 1) + matrix.getFloat(0, 2);

    // rY + gY + bY
    float Y = matrix.getFloat(1, 0) + matrix.getFloat(1, 1) + matrix.getFloat(1, 2);

    // rZ + gZ + bZ
    float Z = matrix.getFloat(2, 0) + matrix.getFloat(2, 1) + matrix.getFloat(2, 2);

    static const float kD50_WhitePoint[3] = { 0.96420f, 1.00000f, 0.82491f };

    // This is a bit more lenient than QCMS and Adobe.  Is there a reason to be stricter here?
    return (SkTAbs(X - kD50_WhitePoint[0]) <= 0.04f) &&
           (SkTAbs(Y - kD50_WhitePoint[1]) <= 0.04f) &&
           (SkTAbs(Z - kD50_WhitePoint[2]) <= 0.04f);
}

sk_sp<SkColorSpace> SkColorSpace::MakeICC(const void* input, size_t len) {
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
            // Recognize color profile specified by A2B0 tag.
            // this must be done before XYZ profile checking, as a profile can have both
            // in which case we should use the A2B case to be accurate
            // (XYZ is there as a fallback / quick preview)
            const ICCTag* a2b0 = ICCTag::Find(tags.get(), tagCount, kTAG_A2B0);
            if (a2b0) {
                const SkColorSpace_A2B::PCS pcs = kXYZ_PCSSpace == header.fPCS
                                                ? SkColorSpace_A2B::PCS::kXYZ
                                                : SkColorSpace_A2B::PCS::kLAB;
                std::vector<SkColorSpace_A2B::Element> elements;
                if (load_a2b0(&elements, a2b0->addr(base), a2b0->fLength, pcs)) {
                    return sk_sp<SkColorSpace>(new SkColorSpace_A2B(pcs, std::move(data),
                                                                    std::move(elements)));
                }
                SkColorSpacePrintf("Ignoring malformed A2B0 tag.\n");
            }

            // Recognize the rXYZ, gXYZ, and bXYZ tags.
            const ICCTag* r = ICCTag::Find(tags.get(), tagCount, kTAG_rXYZ);
            const ICCTag* g = ICCTag::Find(tags.get(), tagCount, kTAG_gXYZ);
            const ICCTag* b = ICCTag::Find(tags.get(), tagCount, kTAG_bXYZ);
            // Lab PCS means the profile is required to be an n-component LUT-based
            // profile, so 3-component matrix-based profiles can only have an XYZ PCS
            if (r && g && b && kXYZ_PCSSpace == header.fPCS) {
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
                if (!is_close_to_d50(mat)) {
                    // QCMS treats these profiles as "bogus".  I'm not sure if that's
                    // correct, but we certainly do not handle non-D50 matrices
                    // correctly.  So I'll disable this for now.
                    SkColorSpacePrintf("Matrix is not close to D50");
                    return nullptr;
                }

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
                        SkColorSpaceTransferFn params;
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
                        SkColorSpaceTransferFn rParams;
                        SkGammas::Type rType =
                                parse_gamma(&rData, &rParams, &tagBytes, r->addr(base), r->fLength);
                        handle_invalid_gamma(&rType, &rData);

                        SkGammas::Data gData;
                        SkColorSpaceTransferFn gParams;
                        SkGammas::Type gType =
                                parse_gamma(&gData, &gParams, &tagBytes, g->addr(base), g->fLength);
                        handle_invalid_gamma(&gType, &gData);

                        SkGammas::Data bData;
                        SkColorSpaceTransferFn bParams;
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
                }

                if (kNonStandard_SkGammaNamed == gammaNamed) {
                    return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(gammaNamed,
                                                                    std::move(gammas),
                                                                    mat, std::move(data)));
                }

                return SkColorSpace_Base::MakeRGB(gammaNamed, mat);
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
    // Profile Data is be mandatory for A2B0 Color Spaces
    SkASSERT(type() == Type::kXYZ);

    // The client may create an SkColorSpace using an SkMatrix44, but currently we only
    // support writing profiles with 3x3 matrices.
    // TODO (msarett): Fix this!
    const SkColorSpace_XYZ* thisXYZ = static_cast<const SkColorSpace_XYZ*>(this);
    const SkMatrix44& toXYZD50 = *thisXYZ->toXYZD50();
    if (0.0f != toXYZD50.getFloat(3, 0) || 0.0f != toXYZD50.getFloat(3, 1) ||
        0.0f != toXYZD50.getFloat(3, 2) || 0.0f != toXYZD50.getFloat(0, 3) ||
        0.0f != toXYZD50.getFloat(1, 3) || 0.0f != toXYZD50.getFloat(2, 3))
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
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 0);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 1);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 2);
    ptr += kTAG_XYZ_Bytes;

    // Write TRC tags
    SkGammaNamed gammaNamed = thisXYZ->gammaNamed();
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
