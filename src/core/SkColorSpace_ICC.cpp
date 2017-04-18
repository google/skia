/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkEndian.h"
#include "SkFixed.h"
#include "SkICCPriv.h"
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

static constexpr float kWhitePointD50[] = { 0.96420f, 1.00000f, 0.82491f, };

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

        switch (fInputColorSpace) {
            case kRGB_ColorSpace:
                SkColorSpacePrintf("RGB Input Color Space");
                break;
            case kCMYK_ColorSpace:
                SkColorSpacePrintf("CMYK Input Color Space\n");
                break;
            case kGray_ColorSpace:
                SkColorSpacePrintf("Gray Input Color Space\n");
                break;
            default:
                SkColorSpacePrintf("Unsupported Input Color Space: %c%c%c%c\n",
                                   (fInputColorSpace>>24)&0xFF, (fInputColorSpace>>16)&0xFF,
                                   (fInputColorSpace>> 8)&0xFF, (fInputColorSpace>> 0)&0xFF);
                return false;
        }

        switch (fPCS) {
            case kXYZ_PCSSpace:
                SkColorSpacePrintf("XYZ PCS\n");
                break;
            case kLAB_PCSSpace:
                SkColorSpacePrintf("Lab PCS\n");
                break;
            default:
                // ICC currently (V4.3) only specifices XYZ and Lab PCS spaces
                SkColorSpacePrintf("Unsupported PCS space: %c%c%c%c\n",
                                   (fPCS>>24)&0xFF, (fPCS>>16)&0xFF,
                                   (fPCS>> 8)&0xFF, (fPCS>> 0)&0xFF);
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

        return_if_false(
                color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[0]), kWhitePointD50[0]) &&
                color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[1]), kWhitePointD50[1]) &&
                color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[2]), kWhitePointD50[2]),
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
            // Y = (aX + b)^g + e  for X >= d
            // Y = cX + f          otherwise
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

                    // Y = (aX + b)^g + e  for X >= -b/a
                    // Y = e               otherwise
                    e = read_big_endian_16_dot_16(src + 24);
                    d = -b / a;
                    f = e;
                    break;
                case kGABDE_ParaCurveType:
                    tagBytes = 12 + 20;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return SkGammas::Type::kNone_Type;
                    }

                    // Y = (aX + b)^g  for X >= d
                    // Y = cX          otherwise
                    c = read_big_endian_16_dot_16(src + 24);
                    d = read_big_endian_16_dot_16(src + 28);
                    break;
                case kGABCDEF_ParaCurveType:
                    tagBytes = 12 + 28;
                    if (len < tagBytes) {
                        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                        return SkGammas::Type::kNone_Type;
                    }

                    // Y = (aX + b)^g + e  for X >= d
                    // Y = cX + f          otherwise
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
    for (uint8_t i = 0; i < gammas->channels(); ++i) {
        if (!gammas->isNamed(i) || gammas->data(i).fNamed != gammas->data(0).fNamed) {
            return kNonStandard_SkGammaNamed;
        }
    }
    return gammas->data(0).fNamed;
}

/**
 *  Parse and load an entire stored curve. Handles invalid gammas as well.
 *
 *  There's nothing to do for the simple cases, but for table gammas we need to actually
 *  read the table into heap memory.  And for parametric gammas, we need to copy over the
 *  parameter values.
 *
 *  @param gammaNamed    Out-variable. The named gamma curve.
 *  @param gammas        Out-variable. The stored gamma curve information. Can be null if
 *                       gammaNamed is a named curve
 *  @param inputChannels The number of gamma input channels
 *  @param rTagPtr       Pointer to start of the gamma tag.
 *  @param taglen        The size in bytes of the tag
 *
 *  @return              false on failure, true on success
 */
static bool parse_and_load_gamma(SkGammaNamed* gammaNamed, sk_sp<SkGammas>* gammas,
                                 uint8_t inputChannels, const uint8_t* tagSrc, size_t tagLen) {
    SkGammas::Data data[kMaxColorChannels];
    SkColorSpaceTransferFn params[kMaxColorChannels];
    SkGammas::Type type[kMaxColorChannels];
    const uint8_t* tagPtr[kMaxColorChannels];

    tagPtr[0] = tagSrc;

    *gammaNamed = kNonStandard_SkGammaNamed;

    // On an invalid first gamma, tagBytes remains set as zero.  This causes the two
    // subsequent to be treated as identical (which is what we want).
    size_t tagBytes = 0;
    type[0] = parse_gamma(&data[0], &params[0], &tagBytes, tagPtr[0], tagLen);
    handle_invalid_gamma(&type[0], &data[0]);
    size_t alignedTagBytes = SkAlign4(tagBytes);

    bool allChannelsSame = false;
    if (inputChannels * alignedTagBytes <= tagLen) {
        allChannelsSame = true;
        for (uint8_t i = 1; i < inputChannels; ++i) {
            if (0 != memcmp(tagSrc, tagSrc + i * alignedTagBytes, tagBytes)) {
                allChannelsSame = false;
                break;
            }
        }
    }
    if (allChannelsSame) {
        if (SkGammas::Type::kNamed_Type == type[0]) {
            *gammaNamed = data[0].fNamed;
        } else {
            size_t allocSize = sizeof(SkGammas);
            return_if_false(safe_add(allocSize, gamma_alloc_size(type[0], data[0]), &allocSize),
                            "SkGammas struct is too large to allocate");
            void* memory = sk_malloc_throw(allocSize);
            *gammas = sk_sp<SkGammas>(new (memory) SkGammas(inputChannels));
            load_gammas(memory, 0, type[0], &data[0], params[0], tagPtr[0]);

            for (uint8_t channel = 0; channel < inputChannels; ++channel) {
                (*gammas)->fType[channel] = type[0];
                (*gammas)->fData[channel] = data[0];
            }
        }
    } else {
        for (uint8_t channel = 1; channel < inputChannels; ++channel) {
            tagPtr[channel] = tagPtr[channel - 1] + alignedTagBytes;
            tagLen = tagLen > alignedTagBytes ? tagLen - alignedTagBytes : 0;
            tagBytes = 0;
            type[channel] = parse_gamma(&data[channel], &params[channel], &tagBytes,
                                        tagPtr[channel], tagLen);
            handle_invalid_gamma(&type[channel], &data[channel]);
            alignedTagBytes = SkAlign4(tagBytes);
        }

        size_t allocSize = sizeof(SkGammas);
        for (uint8_t channel = 0; channel < inputChannels; ++channel) {
            return_if_false(safe_add(allocSize, gamma_alloc_size(type[channel], data[channel]),
                                     &allocSize),
                            "SkGammas struct is too large to allocate");
        }
        void* memory = sk_malloc_throw(allocSize);
        *gammas = sk_sp<SkGammas>(new (memory) SkGammas(inputChannels));

        uint32_t offset = 0;
        for (uint8_t channel = 0; channel < inputChannels; ++channel) {
            (*gammas)->fType[channel] = type[channel];
            offset += load_gammas(memory,offset, type[channel], &data[channel], params[channel],
                                  tagPtr[channel]);
            (*gammas)->fData[channel] = data[channel];

        }
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

static bool is_lut_gamma_linear(const uint8_t* src, size_t count, size_t precision) {
    // check for linear gamma (this is very common in lut gammas, as they aren't optional)
    const float normalizeX = 1.f / (count - 1);
    for (uint32_t x = 0; x < count; ++x) {
        const float y = precision == 1 ? (src[x] / 255.f)
                                       : (read_big_endian_u16(src + 2*x) / 65535.f);
        if (!color_space_almost_equal(x * normalizeX, y)) {
            return false;
        }
    }
    return true;
}

static bool load_lut_gammas(sk_sp<SkGammas>* gammas, SkGammaNamed* gammaNamed, size_t numTables,
                            size_t entriesPerTable, size_t precision, const uint8_t* src,
                            size_t len) {
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

    if (1 == numTablesToUse) {
        if (is_lut_gamma_linear(src, entriesPerTable, precision)) {
            *gammaNamed = kLinear_SkGammaNamed;
            return true;
        }
    }
    *gammaNamed = kNonStandard_SkGammaNamed;

    uint32_t writetableBytes;
    return_if_false(safe_mul(numTablesToUse, writeBytesPerChannel, &writetableBytes),
                    "SkGammas struct is too large to allocate");
    size_t allocSize = sizeof(SkGammas);
    return_if_false(safe_add(allocSize, (size_t)writetableBytes, &allocSize),
                    "SkGammas struct is too large to allocate");

    void* memory = sk_malloc_throw(allocSize);
    *gammas = sk_sp<SkGammas>(new (memory) SkGammas(numTables));

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

    SkASSERT(1 == numTablesToUse|| numTables == numTablesToUse);

    size_t tableOffset = 0;
    for (size_t tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        (*gammas)->fType[tableIndex]                = SkGammas::Type::kTable_Type;
        (*gammas)->fData[tableIndex].fTable.fOffset = tableOffset;
        (*gammas)->fData[tableIndex].fTable.fSize   = entriesPerTable;
        if (numTablesToUse > 1) {
            tableOffset += writeBytesPerChannel;
        }
    }

    return true;
}

bool load_a2b0_a_to_b_type(std::vector<SkColorSpace_A2B::Element>* elements, const uint8_t* src,
                           size_t len, SkColorSpace_A2B::PCS pcs) {
    SkASSERT(len >= 32);
    // Read the number of channels.  The four bytes (4-7) that we skipped are reserved and
    // must be zero.
    const uint8_t inputChannels = src[8];
    const uint8_t outputChannels = src[9];
    if (SkColorLookUpTable::kOutputChannels != outputChannels) {
        // We only handle RGB outputs. The number of output channels must be 3.
        SkColorSpacePrintf("Output channels (%d) must equal 3 in A to B tag.\n", outputChannels);
        return false;
    }
    if (inputChannels == 0 || inputChannels > 4) {
        // And we only support 4 input channels.
        // ICC says up to 16 but our decode can only handle 4.
        // It could easily be extended to support up to 8, but we only allow CMYK/RGB
        // input color spaces which are 3 and 4 so let's restrict it to 4 instead of 8.
        // We can always change this check when we support bigger input spaces.
        SkColorSpacePrintf("Input channels (%d) must be between 1 and 4 in A to B tag.\n",
                           inputChannels);
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
        if (!parse_and_load_gamma(&gammaNamed, &gammas, inputChannels, src + offsetToACurves,
                                  tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else if (kLinear_SkGammaNamed != gammaNamed) {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed, inputChannels));
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

        SkASSERT(inputChannels <= kMaxColorChannels);
        uint8_t gridPoints[kMaxColorChannels];
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
        if (!parse_and_load_gamma(&gammaNamed, &gammas, outputChannels, src + offsetToMCurves,
                                  tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else if (kLinear_SkGammaNamed != gammaNamed) {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed, outputChannels));
        }
    }

    const uint32_t offsetToMatrix = read_big_endian_i32(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        SkMatrix44 matrix(SkMatrix44::kUninitialized_Constructor);
        if (!load_matrix(&matrix, src + offsetToMatrix, len - offsetToMatrix, true, pcs)) {
            SkColorSpacePrintf("Failed to read matrix from A to B tag.\n");
        } else if (!matrix.isIdentity()) {
            elements->push_back(SkColorSpace_A2B::Element(matrix));
        }
    }

    const uint32_t offsetToBCurves = read_big_endian_i32(src + 12);
    if (0 != offsetToBCurves && offsetToBCurves < len) {
        const size_t tagLen = len - offsetToBCurves;
        SkGammaNamed gammaNamed;
        sk_sp<SkGammas> gammas;
        if (!parse_and_load_gamma(&gammaNamed, &gammas, outputChannels, src + offsetToBCurves,
                                  tagLen)) {
            return false;
        }
        if (gammas) {
            elements->push_back(SkColorSpace_A2B::Element(std::move(gammas)));
        } else if (kLinear_SkGammaNamed != gammaNamed) {
            elements->push_back(SkColorSpace_A2B::Element(gammaNamed, outputChannels));
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
    if (SkColorLookUpTable::kOutputChannels != outputChannels) {
        // We only handle RGB outputs. The number of output channels must be 3.
        SkColorSpacePrintf("Output channels (%d) must equal 3 in A to B tag.\n", outputChannels);
        return false;
    }
    if (inputChannels == 0 || inputChannels > 4) {
        // And we only support 4 input channels.
        // ICC says up to 16 but our decode can only handle 4.
        // It could easily be extended to support up to 8, but we only allow CMYK/RGB
        // input color spaces which are 3 and 4 so let's restrict it to 4 instead of 8.
        // We can always change this check when we support bigger input spaces.
        SkColorSpacePrintf("Input channels (%d) must be between 1 and 4 in A to B tag.\n",
                           inputChannels);
        return false;
    }

    const uint8_t clutGridPoints = src[10];
    // 11th byte reserved for padding (required to be zero)

    SkMatrix44 matrix(SkMatrix44::kUninitialized_Constructor);
    load_matrix(&matrix, &src[12], len - 12, false, pcs);
    if (!matrix.isIdentity()) {
        // ICC specs (10.8/10.9) say lut8/16Type profiles must have identity matrices
        // if the input color space is not PCSXYZ, and we do not support PCSXYZ input color spaces
        // so we should never encounter a non-identity matrix here.
        // However, 2 test images from the ICC website have RGB input spaces and non-identity
        // matrices so we're not going to fail here, despite being against the spec.
        SkColorSpacePrintf("Warning: non-Identity matrix found in non-XYZ input color space"
                           "lut profile");
        elements->push_back(SkColorSpace_A2B::Element(matrix));
    }

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
    SkGammaNamed inputGammaNamed;
    if (!load_lut_gammas(&inputGammas, &inputGammaNamed, inputChannels, inTableEntries, precision,
                         src + inputOffset, len - inputOffset)) {
        SkColorSpacePrintf("Failed to read input gammas from lutnType tag.\n");
        return false;
    }
    SkASSERT(inputGammas || inputGammaNamed != kNonStandard_SkGammaNamed);
    if (kLinear_SkGammaNamed != inputGammaNamed) {
        if (kNonStandard_SkGammaNamed != inputGammaNamed) {
            elements->push_back(SkColorSpace_A2B::Element(inputGammaNamed, inputChannels));
        } else {
            elements->push_back(SkColorSpace_A2B::Element(std::move(inputGammas)));
        }
    }

    const size_t clutOffset = inputOffset + precision*inTableEntries*inputChannels;
    return_if_false(len >= clutOffset, "A2B0 lutnType tag too small for CLUT");
    sk_sp<SkColorLookUpTable> colorLUT;
    const uint8_t gridPoints[kMaxColorChannels] = {
        clutGridPoints, clutGridPoints, clutGridPoints, clutGridPoints
    };
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
    SkGammaNamed outputGammaNamed;
    if (!load_lut_gammas(&outputGammas, &outputGammaNamed, outputChannels, outTableEntries,
                         precision, src + outputOffset, len - outputOffset)) {
        SkColorSpacePrintf("Failed to read output gammas from lutnType tag.\n");
        return false;
    }
    SkASSERT(outputGammas || outputGammaNamed != kNonStandard_SkGammaNamed);
    if (kLinear_SkGammaNamed != outputGammaNamed) {
        if (kNonStandard_SkGammaNamed != outputGammaNamed) {
            elements->push_back(SkColorSpace_A2B::Element(outputGammaNamed, outputChannels));
        } else {
            elements->push_back(SkColorSpace_A2B::Element(std::move(outputGammas)));
        }
    }

    return true;
}

static inline int icf_channels(SkColorSpace_Base::ICCTypeFlag iccType) {
    switch (iccType) {
        case SkColorSpace_Base::kRGB_ICCTypeFlag:
            return 3;
        case SkColorSpace_Base::kCMYK_ICCTypeFlag:
            return 4;
        default:
            SkASSERT(false);
            return 0;
    }
}

static bool load_a2b0(std::vector<SkColorSpace_A2B::Element>* elements, const uint8_t* src,
                      size_t len, SkColorSpace_A2B::PCS pcs,
                      SkColorSpace_Base::ICCTypeFlag iccType) {
    if (len < 4) {
        return false;
    }
    const uint32_t type = read_big_endian_u32(src);

    switch (type) {
        case kTAG_AtoBType:
            if (len < 32) {
                SkColorSpacePrintf("A to B tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag is of type lutAtoBType\n");
            if (!load_a2b0_a_to_b_type(elements, src, len, pcs)) {
                return false;
            }
            break;
        case kTAG_lut8Type:
            if (len < 48) {
                SkColorSpacePrintf("lut8 tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag of type lut8Type\n");
            if (!load_a2b0_lutn_type(elements, src, len, pcs)) {
                return false;
            }
            break;
        case kTAG_lut16Type:
            if (len < 52) {
                SkColorSpacePrintf("lut16 tag is too small (%d bytes).", len);
                return false;
            }
            SkColorSpacePrintf("A2B0 tag of type lut16Type\n");
            if (!load_a2b0_lutn_type(elements, src, len, pcs)) {
                return false;
            }
            break;
        default:
            SkColorSpacePrintf("Unsupported A to B tag type: %c%c%c%c\n", (type>>24)&0xFF,
                               (type>>16)&0xFF, (type>>8)&0xFF, type&0xFF);
            return false;
    }
    SkASSERT(SkColorSpace_A2B::PCS::kLAB == pcs || SkColorSpace_A2B::PCS::kXYZ == pcs);
    static constexpr int kPCSChannels = 3; // must be PCSLAB or PCSXYZ
    if (elements->empty()) {
        return kPCSChannels == icf_channels(iccType);
    }
    // now let's verify that the input/output channels of each A2B element actually match up
    if (icf_channels(iccType) != elements->front().inputChannels()) {
        SkColorSpacePrintf("Input channel count does not match first A2B element's input count");
        return false;
    }
    for (size_t i = 1; i < elements->size(); ++i) {
        if ((*elements)[i - 1].outputChannels() != (*elements)[i].inputChannels()) {
            SkColorSpacePrintf("A2B elements don't agree in input/output channel counts");
            return false;
        }
    }
    if (kPCSChannels != elements->back().outputChannels()) {
        SkColorSpacePrintf("PCS channel count doesn't match last A2B element's output count");
        return false;
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

static sk_sp<SkColorSpace> make_xyz(const ICCProfileHeader& header, ICCTag* tags, int tagCount,
                                    const uint8_t* base, sk_sp<SkData> profileData) {
    if (kLAB_PCSSpace == header.fPCS) {
        return nullptr;
    }

    // Recognize the rXYZ, gXYZ, and bXYZ tags.
    const ICCTag* r = ICCTag::Find(tags, tagCount, kTAG_rXYZ);
    const ICCTag* g = ICCTag::Find(tags, tagCount, kTAG_gXYZ);
    const ICCTag* b = ICCTag::Find(tags, tagCount, kTAG_bXYZ);
    if (!r || !g || !b) {
        return nullptr;
    }

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
        return_null("XYZ matrix is not D50");
    }

    // If some, but not all, of the gamma tags are missing, assume that all
    // gammas are meant to be the same.
    r = ICCTag::Find(tags, tagCount, kTAG_rTRC);
    g = ICCTag::Find(tags, tagCount, kTAG_gTRC);
    b = ICCTag::Find(tags, tagCount, kTAG_bTRC);
    if ((!r || !g || !b)) {
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
                gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));
                load_gammas(memory, 0, type, &data, params, r->addr(base));

                for (int i = 0; i < 3; ++i) {
                    gammas->fType[i] = type;
                    gammas->fData[i] = data;
                }
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
                !safe_add(allocSize, gamma_alloc_size(bType, bData), &allocSize)) {
                return_null("SkGammas struct is too large to allocate");
            }
            void* memory = sk_malloc_throw(allocSize);
            gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));

            uint32_t offset = 0;
            gammas->fType[0] = rType;
            offset += load_gammas(memory, offset, rType, &rData, rParams,
                                  r->addr(base));

            gammas->fType[1] = gType;
            offset += load_gammas(memory, offset, gType, &gData, gParams,
                                  g->addr(base));

            gammas->fType[2] = bType;
            load_gammas(memory, offset, bType, &bData, bParams, b->addr(base));

            gammas->fData[0] = rData;
            gammas->fData[1] = gData;
            gammas->fData[2] = bData;
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
                                                        mat, std::move(profileData)));
    }

    return SkColorSpace_Base::MakeRGB(gammaNamed, mat);
}

static sk_sp<SkColorSpace> make_gray(const ICCProfileHeader& header, ICCTag* tags, int tagCount,
                                     const uint8_t* base, sk_sp<SkData> profileData) {
    if (kLAB_PCSSpace == header.fPCS) {
        return nullptr;
    }

    const ICCTag* grayTRC = ICCTag::Find(tags, tagCount, kTAG_kTRC);
    if (!grayTRC) {
        return_null("grayTRC tag required for monochrome profiles.");
    }
    SkGammas::Data data;
    SkColorSpaceTransferFn params;
    size_t tagBytes;
    SkGammas::Type type =
            parse_gamma(&data, &params, &tagBytes, grayTRC->addr(base), grayTRC->fLength);
    handle_invalid_gamma(&type, &data);

    SkMatrix44 toXYZD50(SkMatrix44::kIdentity_Constructor);
    toXYZD50.setFloat(0, 0, kWhitePointD50[0]);
    toXYZD50.setFloat(1, 1, kWhitePointD50[1]);
    toXYZD50.setFloat(2, 2, kWhitePointD50[2]);
    if (SkGammas::Type::kNamed_Type == type) {
        return SkColorSpace_Base::MakeRGB(data.fNamed, toXYZD50);
    }

    size_t allocSize = sizeof(SkGammas);
    if (!safe_add(allocSize, gamma_alloc_size(type, data), &allocSize)) {
        return_null("SkGammas struct is too large to allocate");
    }
    void* memory = sk_malloc_throw(allocSize);
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));
    load_gammas(memory, 0, type, &data, params, grayTRC->addr(base));
    for (int i = 0; i < 3; ++i) {
        gammas->fType[i] = type;
        gammas->fData[i] = data;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(kNonStandard_SkGammaNamed,
                                                    std::move(gammas),
                                                    toXYZD50, std::move(profileData)));
}

static sk_sp<SkColorSpace> make_a2b(SkColorSpace_Base::ICCTypeFlag iccType,
                                    const ICCProfileHeader& header, ICCTag* tags, int tagCount,
                                    const uint8_t* base, sk_sp<SkData> profileData) {
    const ICCTag* a2b0 = ICCTag::Find(tags, tagCount, kTAG_A2B0);
    if (a2b0) {
        const SkColorSpace_A2B::PCS pcs = kXYZ_PCSSpace == header.fPCS
                                        ? SkColorSpace_A2B::PCS::kXYZ
                                        : SkColorSpace_A2B::PCS::kLAB;
        std::vector<SkColorSpace_A2B::Element> elements;
        if (load_a2b0(&elements, a2b0->addr(base), a2b0->fLength, pcs, iccType)) {
            return sk_sp<SkColorSpace>(new SkColorSpace_A2B(iccType, std::move(elements),
                                                            pcs, std::move(profileData)));
        }
    }

    return nullptr;
}

sk_sp<SkColorSpace> SkColorSpace::MakeICC(const void* input, size_t len) {
    return SkColorSpace_Base::MakeICC(input, len, SkColorSpace_Base::kRGB_ICCTypeFlag);
}

sk_sp<SkColorSpace> SkColorSpace_Base::MakeICC(const void* input, size_t len,
                                               ICCTypeFlag desiredType) {
    if (!input || len < kICCHeaderSize) {
        return_null("Data is null or not large enough to contain an ICC profile");
    }

    // Create our own copy of the input.
    void* memory = sk_malloc_throw(len);
    memcpy(memory, input, len);
    sk_sp<SkData> profileData = SkData::MakeFromMalloc(memory, len);
    const uint8_t* base = profileData->bytes();
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
            if (!(kRGB_ICCTypeFlag & desiredType)) {
                return_null("Provided input color format (RGB) does not match profile.");
            }

            sk_sp<SkColorSpace> colorSpace =
                    make_xyz(header, tags.get(), tagCount, base, profileData);
            if (colorSpace) {
                return colorSpace;
            }

            desiredType = kRGB_ICCTypeFlag;
            break;
        }
        case kGray_ColorSpace: {
            if (!(kGray_ICCTypeFlag & desiredType)) {
                return_null("Provided input color format (Gray) does not match profile.");
            }

            return make_gray(header, tags.get(), tagCount, base, profileData);
        }
        case kCMYK_ColorSpace:
            if (!(kCMYK_ICCTypeFlag & desiredType)) {
                return_null("Provided input color format (CMYK) does not match profile.");
            }

            desiredType = kCMYK_ICCTypeFlag;
            break;
        default:
            return_null("ICC profile contains unsupported colorspace");
    }

    return make_a2b(desiredType, header, tags.get(), tagCount, base, profileData);
}
