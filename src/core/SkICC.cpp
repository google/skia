/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkICC.h"
#include "include/private/SkFixed.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkEndian.h"
#include "src/core/SkICCPriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkUtils.h"

static constexpr char kDescriptionTagBodyPrefix[12] =
        { 'G', 'o', 'o', 'g', 'l', 'e', '/', 'S', 'k', 'i', 'a' , '/'};

static constexpr size_t kICCDescriptionTagSize = 44;

static_assert(kICCDescriptionTagSize ==
              sizeof(kDescriptionTagBodyPrefix) + 2 * sizeof(SkMD5::Digest), "");
static constexpr size_t kDescriptionTagBodySize = kICCDescriptionTagSize * 2;  // ascii->utf16be

static_assert(SkIsAlign4(kDescriptionTagBodySize), "Description must be aligned to 4-bytes.");
static constexpr uint32_t kDescriptionTagHeader[7] {
    SkEndian_SwapBE32(kTAG_TextType),                        // Type signature
    0,                                                       // Reserved
    SkEndian_SwapBE32(1),                                    // Number of records
    SkEndian_SwapBE32(12),                                   // Record size (must be 12)
    SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')), // English USA
    SkEndian_SwapBE32(kDescriptionTagBodySize),              // Length of string
    SkEndian_SwapBE32(28),                                   // Offset of string
};

static constexpr uint32_t kWhitePointTag[5] {
    SkEndian_SwapBE32(kXYZ_PCSSpace),
    0,
    SkEndian_SwapBE32(0x0000f6d6), // X = 0.96420 (D50)
    SkEndian_SwapBE32(0x00010000), // Y = 1.00000 (D50)
    SkEndian_SwapBE32(0x0000d32d), // Z = 0.82491 (D50)
};

// Google Inc. 2016 (UTF-16)
static constexpr uint8_t kCopyrightTagBody[] = {
        0x00, 0x47, 0x00, 0x6f,
        0x00, 0x6f, 0x00, 0x67,
        0x00, 0x6c, 0x00, 0x65,
        0x00, 0x20, 0x00, 0x49,
        0x00, 0x6e, 0x00, 0x63,
        0x00, 0x2e, 0x00, 0x20,
        0x00, 0x32, 0x00, 0x30,
        0x00, 0x31, 0x00, 0x36,
};
static_assert(SkIsAlign4(sizeof(kCopyrightTagBody)), "Copyright must be aligned to 4-bytes.");
static constexpr uint32_t kCopyrightTagHeader[7] {
    SkEndian_SwapBE32(kTAG_TextType),                        // Type signature
    0,                                                       // Reserved
    SkEndian_SwapBE32(1),                                    // Number of records
    SkEndian_SwapBE32(12),                                   // Record size (must be 12)
    SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')), // English USA
    SkEndian_SwapBE32(sizeof(kCopyrightTagBody)),            // Length of string
    SkEndian_SwapBE32(28),                                   // Offset of string
};

// We will write a profile with the minimum nine required tags.
static constexpr uint32_t kICCNumEntries = 9;

static constexpr uint32_t kTAG_desc = SkSetFourByteTag('d', 'e', 's', 'c');
static constexpr uint32_t kTAG_desc_Bytes = sizeof(kDescriptionTagHeader) +
                                            kDescriptionTagBodySize;
static constexpr uint32_t kTAG_desc_Offset = kICCHeaderSize +
                                             kICCNumEntries * kICCTagTableEntrySize;

static constexpr uint32_t kTAG_XYZ_Bytes = 20;
static constexpr uint32_t kTAG_rXYZ_Offset = kTAG_desc_Offset + kTAG_desc_Bytes;
static constexpr uint32_t kTAG_gXYZ_Offset = kTAG_rXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_bXYZ_Offset = kTAG_gXYZ_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kTAG_TRC_Bytes = 40;
static constexpr uint32_t kTAG_rTRC_Offset = kTAG_bXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_gTRC_Offset = kTAG_rTRC_Offset;
static constexpr uint32_t kTAG_bTRC_Offset = kTAG_rTRC_Offset;

static constexpr uint32_t kTAG_wtpt = SkSetFourByteTag('w', 't', 'p', 't');
static constexpr uint32_t kTAG_wtpt_Offset = kTAG_bTRC_Offset + kTAG_TRC_Bytes;

static constexpr uint32_t kTAG_cprt = SkSetFourByteTag('c', 'p', 'r', 't');
static constexpr uint32_t kTAG_cprt_Bytes = sizeof(kCopyrightTagHeader) +
                                            sizeof(kCopyrightTagBody);
static constexpr uint32_t kTAG_cprt_Offset = kTAG_wtpt_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kICCProfileSize = kTAG_cprt_Offset + kTAG_cprt_Bytes;

static constexpr uint32_t kICCHeader[kICCHeaderSize / 4] {
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

static constexpr uint32_t kICCTagTable[3 * kICCNumEntries] {
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

// This is like SkFloatToFixed, but rounds to nearest, preserving as much accuracy as possible
// when going float -> fixed -> float (it has the same accuracy when going fixed -> float -> fixed).
// The use of double is necessary to accomodate the full potential 32-bit mantissa of the 16.16
// SkFixed value, and so avoiding rounding problems with float. Also, see the comment in SkFixed.h.
static SkFixed float_round_to_fixed(float x) {
    return sk_float_saturate2int((float)floor((double)x * SK_Fixed1 + 0.5));
}

static void write_xyz_tag(uint32_t* ptr, const skcms_Matrix3x3& toXYZD50, int col) {
    ptr[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(float_round_to_fixed(toXYZD50.vals[0][col]));
    ptr[3] = SkEndian_SwapBE32(float_round_to_fixed(toXYZD50.vals[1][col]));
    ptr[4] = SkEndian_SwapBE32(float_round_to_fixed(toXYZD50.vals[2][col]));
}

static void write_trc_tag(uint32_t* ptr, const skcms_TransferFunction& fn) {
    ptr[0] = SkEndian_SwapBE32(kTAG_ParaCurveType);
    ptr[1] = 0;
    ptr[2] = (uint32_t) (SkEndian_SwapBE16(kGABCDEF_ParaCurveType));
    ptr[3] = SkEndian_SwapBE32(float_round_to_fixed(fn.g));
    ptr[4] = SkEndian_SwapBE32(float_round_to_fixed(fn.a));
    ptr[5] = SkEndian_SwapBE32(float_round_to_fixed(fn.b));
    ptr[6] = SkEndian_SwapBE32(float_round_to_fixed(fn.c));
    ptr[7] = SkEndian_SwapBE32(float_round_to_fixed(fn.d));
    ptr[8] = SkEndian_SwapBE32(float_round_to_fixed(fn.e));
    ptr[9] = SkEndian_SwapBE32(float_round_to_fixed(fn.f));
}

static bool nearly_equal(float x, float y) {
    // A note on why I chose this tolerance:  transfer_fn_almost_equal() uses a
    // tolerance of 0.001f, which doesn't seem to be enough to distinguish
    // between similar transfer functions, for example: gamma2.2 and sRGB.
    //
    // If the tolerance is 0.0f, then this we can't distinguish between two
    // different encodings of what is clearly the same colorspace.  Some
    // experimentation with example files lead to this number:
    static constexpr float kTolerance = 1.0f / (1 << 11);
    return ::fabsf(x - y) <= kTolerance;
}

static bool nearly_equal(const skcms_TransferFunction& u,
                         const skcms_TransferFunction& v) {
    return nearly_equal(u.g, v.g)
        && nearly_equal(u.a, v.a)
        && nearly_equal(u.b, v.b)
        && nearly_equal(u.c, v.c)
        && nearly_equal(u.d, v.d)
        && nearly_equal(u.e, v.e)
        && nearly_equal(u.f, v.f);
}

static bool nearly_equal(const skcms_Matrix3x3& u, const skcms_Matrix3x3& v) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (!nearly_equal(u.vals[r][c], v.vals[r][c])) {
                return false;
            }
        }
    }
    return true;
}

// Return nullptr if the color profile doen't have a special name.
const char* get_color_profile_description(const skcms_TransferFunction& fn,
                                          const skcms_Matrix3x3& toXYZD50) {
    bool srgb_xfer = nearly_equal(fn, SkNamedTransferFn::kSRGB);
    bool srgb_gamut = nearly_equal(toXYZD50, SkNamedGamut::kSRGB);
    if (srgb_xfer && srgb_gamut) {
        return "sRGB";
    }
    bool line_xfer = nearly_equal(fn, SkNamedTransferFn::kLinear);
    if (line_xfer && srgb_gamut) {
        return "Linear Transfer with sRGB Gamut";
    }
    bool twoDotTwo = nearly_equal(fn, SkNamedTransferFn::k2Dot2);
    if (twoDotTwo && srgb_gamut) {
        return "2.2 Transfer with sRGB Gamut";
    }
    if (twoDotTwo && nearly_equal(toXYZD50, SkNamedGamut::kAdobeRGB)) {
        return "AdobeRGB";
    }
    bool dcip3_gamut = nearly_equal(toXYZD50, SkNamedGamut::kDCIP3);
    if (srgb_xfer || line_xfer) {
        if (srgb_xfer && dcip3_gamut) {
            return "sRGB Transfer with DCI-P3 Gamut";
        }
        if (line_xfer && dcip3_gamut) {
            return "Linear Transfer with DCI-P3 Gamut";
        }
        bool rec2020 = nearly_equal(toXYZD50, SkNamedGamut::kRec2020);
        if (srgb_xfer && rec2020) {
            return "sRGB Transfer with Rec-BT-2020 Gamut";
        }
        if (line_xfer && rec2020) {
            return "Linear Transfer with Rec-BT-2020 Gamut";
        }
    }
    return nullptr;
}

static void get_color_profile_tag(char dst[kICCDescriptionTagSize],
                                 const skcms_TransferFunction& fn,
                                 const skcms_Matrix3x3& toXYZD50) {
    SkASSERT(dst);
    if (const char* description = get_color_profile_description(fn, toXYZD50)) {
        SkASSERT(strlen(description) < kICCDescriptionTagSize);
        strncpy(dst, description, kICCDescriptionTagSize);
        // "If the length of src is less than n, strncpy() writes additional
        // null bytes to dest to ensure that a total of n bytes are written."
    } else {
        strncpy(dst, kDescriptionTagBodyPrefix, sizeof(kDescriptionTagBodyPrefix));
        SkMD5 md5;
        md5.write(&toXYZD50, sizeof(toXYZD50));
        static_assert(sizeof(fn) == sizeof(float) * 7, "packed");
        md5.write(&fn, sizeof(fn));
        SkMD5::Digest digest = md5.finish();
        char* ptr = dst + sizeof(kDescriptionTagBodyPrefix);
        for (unsigned i = 0; i < sizeof(SkMD5::Digest); ++i) {
            uint8_t byte = digest.data[i];
            *ptr++ = SkHexadecimalDigits::gUpper[byte >> 4];
            *ptr++ = SkHexadecimalDigits::gUpper[byte & 0xF];
        }
        SkASSERT(ptr == dst + kICCDescriptionTagSize);
    }
}

sk_sp<SkData> SkWriteICCProfile(const skcms_TransferFunction& fn,
                                const skcms_Matrix3x3& toXYZD50) {
    if (!is_valid_transfer_fn(fn)) {
        return nullptr;
    }

    SkAutoMalloc profile(kICCProfileSize);
    uint8_t* ptr = (uint8_t*) profile.get();

    // Write profile header
    memcpy(ptr, kICCHeader, sizeof(kICCHeader));
    ptr += sizeof(kICCHeader);

    // Write tag table
    memcpy(ptr, kICCTagTable, sizeof(kICCTagTable));
    ptr += sizeof(kICCTagTable);

    // Write profile description tag
    memcpy(ptr, kDescriptionTagHeader, sizeof(kDescriptionTagHeader));
    ptr += sizeof(kDescriptionTagHeader);
    {
        char colorProfileTag[kICCDescriptionTagSize];
        get_color_profile_tag(colorProfileTag, fn, toXYZD50);

        // ASCII --> big-endian UTF-16.
        for (size_t i = 0; i < kICCDescriptionTagSize; i++) {
            *ptr++ = 0;
            *ptr++ = colorProfileTag[i];
        }
    }

    // Write XYZ tags
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 0);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 1);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 2);
    ptr += kTAG_XYZ_Bytes;

    // Write TRC tag
    write_trc_tag((uint32_t*) ptr, fn);
    ptr += kTAG_TRC_Bytes;

    // Write white point tag (must be D50)
    memcpy(ptr, kWhitePointTag, sizeof(kWhitePointTag));
    ptr += sizeof(kWhitePointTag);

    // Write copyright tag
    memcpy(ptr, kCopyrightTagHeader, sizeof(kCopyrightTagHeader));
    ptr += sizeof(kCopyrightTagHeader);
    memcpy(ptr, kCopyrightTagBody, sizeof(kCopyrightTagBody));
    ptr += sizeof(kCopyrightTagBody);

    SkASSERT(kICCProfileSize == ptr - (uint8_t*) profile.get());
    return SkData::MakeFromMalloc(profile.release(), kICCProfileSize);
}
