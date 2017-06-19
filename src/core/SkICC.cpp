/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkEndian.h"
#include "SkFixed.h"
#include "SkICC.h"
#include "SkICCPriv.h"
#include "SkMD5.h"
#include "SkString.h"
#include "SkUtils.h"

SkICC::SkICC(sk_sp<SkColorSpace> colorSpace)
    : fColorSpace(std::move(colorSpace))
{}

sk_sp<SkICC> SkICC::Make(const void* ptr, size_t len) {
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(ptr, len);
    if (!colorSpace) {
        return nullptr;
    }

    return sk_sp<SkICC>(new SkICC(std::move(colorSpace)));
}

bool SkICC::toXYZD50(SkMatrix44* toXYZD50) const {
    const SkMatrix44* m = as_CSB(fColorSpace)->toXYZD50();
    if (!m) {
        return false;
    }

    *toXYZD50 = *m;
    return true;
}

bool SkICC::isNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    return as_CSB(fColorSpace)->onIsNumericalTransferFn(coeffs);
}

static const int kDefaultTableSize = 512; // Arbitrary

void fn_to_table(float* tablePtr, const SkColorSpaceTransferFn& fn) {
    // Y = (aX + b)^g + e  for X >= d
    // Y = cX + f          otherwise
    for (int i = 0; i < kDefaultTableSize; i++) {
        float x = ((float) i) / ((float) (kDefaultTableSize - 1));
        if (x >= fn.fD) {
            tablePtr[i] = clamp_0_1(powf(fn.fA * x + fn.fB, fn.fG) + fn.fE);
        } else {
            tablePtr[i] = clamp_0_1(fn.fC * x + fn.fF);
        }
    }
}

void copy_to_table(float* tablePtr, const SkGammas* gammas, int index) {
    SkASSERT(gammas->isTable(index));
    const float* ptr = gammas->table(index);
    const size_t bytes = gammas->tableSize(index) * sizeof(float);
    memcpy(tablePtr, ptr, bytes);
}

bool SkICC::rawTransferFnData(Tables* tables) const {
    if (SkColorSpace_Base::Type::kA2B == as_CSB(fColorSpace)->type()) {
        return false;
    }
    SkColorSpace_XYZ* colorSpace = (SkColorSpace_XYZ*) fColorSpace.get();

    SkColorSpaceTransferFn fn;
    if (this->isNumericalTransferFn(&fn)) {
        tables->fStorage = SkData::MakeUninitialized(kDefaultTableSize * sizeof(float));
        fn_to_table((float*) tables->fStorage->writable_data(), fn);
        tables->fRed.fOffset = tables->fGreen.fOffset = tables->fBlue.fOffset = 0;
        tables->fRed.fCount = tables->fGreen.fCount = tables->fBlue.fCount = kDefaultTableSize;
        return true;
    }

    const SkGammas* gammas = colorSpace->gammas();
    SkASSERT(gammas);
    if (gammas->data(0) == gammas->data(1) && gammas->data(0) == gammas->data(2)) {
        SkASSERT(gammas->isTable(0));
        tables->fStorage = SkData::MakeUninitialized(gammas->tableSize(0) * sizeof(float));
        copy_to_table((float*) tables->fStorage->writable_data(), gammas, 0);
        tables->fRed.fOffset = tables->fGreen.fOffset = tables->fBlue.fOffset = 0;
        tables->fRed.fCount = tables->fGreen.fCount = tables->fBlue.fCount = gammas->tableSize(0);
        return true;
    }

    // Determine the storage size.
    size_t storageSize = 0;
    for (int i = 0; i < 3; i++) {
        if (gammas->isTable(i)) {
            storageSize += gammas->tableSize(i) * sizeof(float);
        } else {
            storageSize += kDefaultTableSize * sizeof(float);
        }
    }

    // Fill in the tables.
    tables->fStorage = SkData::MakeUninitialized(storageSize);
    float* ptr = (float*) tables->fStorage->writable_data();
    size_t offset = 0;
    Channel rgb[3];
    for (int i = 0; i < 3; i++) {
        if (gammas->isTable(i)) {
            copy_to_table(ptr, gammas, i);
            rgb[i].fOffset = offset;
            rgb[i].fCount = gammas->tableSize(i);
            offset += rgb[i].fCount * sizeof(float);
            ptr += rgb[i].fCount;
            continue;
        }

        if (gammas->isNamed(i)) {
            SkAssertResult(named_to_parametric(&fn, gammas->data(i).fNamed));
        } else if (gammas->isValue(i)) {
            value_to_parametric(&fn, gammas->data(i).fValue);
        } else {
            SkASSERT(gammas->isParametric(i));
            fn = gammas->params(i);
        }

        fn_to_table(ptr, fn);
        rgb[i].fOffset = offset;
        rgb[i].fCount = kDefaultTableSize;
        offset += kDefaultTableSize * sizeof(float);
        ptr += kDefaultTableSize;
    }

    tables->fRed = rgb[0];
    tables->fGreen = rgb[1];
    tables->fBlue = rgb[2];
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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
        0x00, 0x47, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x20, 0x00,
        0x49, 0x00, 0x6e, 0x00, 0x63, 0x00, 0x2e, 0x00, 0x20, 0x00, 0x32, 0x00, 0x30, 0x00, 0x31,
        0x00, 0x36,
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

static void write_xyz_tag(uint32_t* ptr, const SkMatrix44& toXYZ, int col) {
    ptr[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(0, col)));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(1, col)));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(2, col)));
}

static void write_trc_tag(uint32_t* ptr, const SkColorSpaceTransferFn& fn) {
    ptr[0] = SkEndian_SwapBE32(kTAG_ParaCurveType);
    ptr[1] = 0;
    ptr[2] = (uint32_t) (SkEndian_SwapBE16(kGABCDEF_ParaCurveType));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(fn.fG));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(fn.fA));
    ptr[5] = SkEndian_SwapBE32(SkFloatToFixed(fn.fB));
    ptr[6] = SkEndian_SwapBE32(SkFloatToFixed(fn.fC));
    ptr[7] = SkEndian_SwapBE32(SkFloatToFixed(fn.fD));
    ptr[8] = SkEndian_SwapBE32(SkFloatToFixed(fn.fE));
    ptr[9] = SkEndian_SwapBE32(SkFloatToFixed(fn.fF));
}

static bool is_3x3(const SkMatrix44& toXYZD50) {
    return 0.0f == toXYZD50.get(3, 0) && 0.0f == toXYZD50.get(3, 1) && 0.0f == toXYZD50.get(3, 2) &&
           0.0f == toXYZD50.get(0, 3) && 0.0f == toXYZD50.get(1, 3) && 0.0f == toXYZD50.get(2, 3) &&
           1.0f == toXYZD50.get(3, 3);
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

static bool nearly_equal(const SkColorSpaceTransferFn& u,
                         const SkColorSpaceTransferFn& v) {
    return nearly_equal(u.fG, v.fG)
        && nearly_equal(u.fA, v.fA)
        && nearly_equal(u.fB, v.fB)
        && nearly_equal(u.fC, v.fC)
        && nearly_equal(u.fD, v.fD)
        && nearly_equal(u.fE, v.fE)
        && nearly_equal(u.fF, v.fF);
}

static bool nearly_equal(const SkMatrix44& toXYZD50, const float standard[9]) {
    return nearly_equal(toXYZD50.getFloat(0, 0), standard[0])
        && nearly_equal(toXYZD50.getFloat(0, 1), standard[1])
        && nearly_equal(toXYZD50.getFloat(0, 2), standard[2])
        && nearly_equal(toXYZD50.getFloat(1, 0), standard[3])
        && nearly_equal(toXYZD50.getFloat(1, 1), standard[4])
        && nearly_equal(toXYZD50.getFloat(1, 2), standard[5])
        && nearly_equal(toXYZD50.getFloat(2, 0), standard[6])
        && nearly_equal(toXYZD50.getFloat(2, 1), standard[7])
        && nearly_equal(toXYZD50.getFloat(2, 2), standard[8])
        && nearly_equal(toXYZD50.getFloat(0, 3), 0.0f)
        && nearly_equal(toXYZD50.getFloat(1, 3), 0.0f)
        && nearly_equal(toXYZD50.getFloat(2, 3), 0.0f)
        && nearly_equal(toXYZD50.getFloat(3, 0), 0.0f)
        && nearly_equal(toXYZD50.getFloat(3, 1), 0.0f)
        && nearly_equal(toXYZD50.getFloat(3, 2), 0.0f)
        && nearly_equal(toXYZD50.getFloat(3, 3), 1.0f);
}

// Return nullptr if the color profile doen't have a special name.
const char* get_color_profile_description(const SkColorSpaceTransferFn& fn,
                                          const SkMatrix44& toXYZD50) {
    bool srgb_xfer = nearly_equal(fn, gSRGB_TransferFn);
    bool srgb_gamut = nearly_equal(toXYZD50, gSRGB_toXYZD50);
    if (srgb_xfer && srgb_gamut) {
        return "sRGB";
    }
    bool line_xfer = nearly_equal(fn, gLinear_TransferFn);
    if (line_xfer && srgb_gamut) {
        return "Linear Transfer with sRGB Gamut";
    }
    bool twoDotTwo = nearly_equal(fn, g2Dot2_TransferFn);
    if (twoDotTwo && srgb_gamut) {
        return "2.2 Transfer with sRGB Gamut";
    }
    if (twoDotTwo && nearly_equal(toXYZD50, gAdobeRGB_toXYZD50)) {
        return "AdobeRGB";
    }
    bool dcip3_gamut = nearly_equal(toXYZD50, gDCIP3_toXYZD50);
    if (srgb_xfer || line_xfer) {
        if (srgb_xfer && dcip3_gamut) {
            return "sRGB Transfer with DCI-P3 Gamut";
        }
        if (line_xfer && dcip3_gamut) {
            return "Linear Transfer with DCI-P3 Gamut";
        }
        bool rec2020 = nearly_equal(toXYZD50, gRec2020_toXYZD50);
        if (srgb_xfer && rec2020) {
            return "sRGB Transfer with Rec-BT-2020 Gamut";
        }
        if (line_xfer && rec2020) {
            return "Linear Transfer with Rec-BT-2020 Gamut";
        }
    }
    if (dcip3_gamut && nearly_equal(fn, gDCIP3_TransferFn)) {
        return "DCI-P3";
    }
    return nullptr;
}

static void get_color_profile_tag(char dst[kICCDescriptionTagSize],
                                 const SkColorSpaceTransferFn& fn,
                                 const SkMatrix44& toXYZD50) {
    SkASSERT(dst);
    if (const char* description = get_color_profile_description(fn, toXYZD50)) {
        SkASSERT(strlen(description) < kICCDescriptionTagSize);
        strncpy(dst, description, kICCDescriptionTagSize);
        // "If the length of src is less than n, strncpy() writes additional
        // null bytes to dest to ensure that a total of n bytes are written."
    } else {
        strncpy(dst, kDescriptionTagBodyPrefix, sizeof(kDescriptionTagBodyPrefix));
        SkMD5 md5;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                float value = toXYZD50.getFloat(i,j);
                md5.write(&value, sizeof(value));
            }
        }
        static_assert(sizeof(fn) == sizeof(float) * 7, "packed");
        md5.write(&fn, sizeof(fn));
        SkMD5::Digest digest;
        md5.finish(digest);
        char* ptr = dst + sizeof(kDescriptionTagBodyPrefix);
        for (unsigned i = 0; i < sizeof(SkMD5::Digest); ++i) {
            uint8_t byte = digest.data[i];
            *ptr++ = SkHexadecimalDigits::gUpper[byte >> 4];
            *ptr++ = SkHexadecimalDigits::gUpper[byte & 0xF];
        }
        SkASSERT(ptr == dst + kICCDescriptionTagSize);
    }
}

SkString SkICCGetColorProfileTag(const SkColorSpaceTransferFn& fn,
                                 const SkMatrix44& toXYZD50) {
    char tag[kICCDescriptionTagSize];
    get_color_profile_tag(tag, fn, toXYZD50);
    size_t len = kICCDescriptionTagSize;
    while (len > 0 && tag[len - 1] == '\0') {
        --len;  // tag is padded out with zeros
    }
    SkASSERT(len != 0);
    return SkString(tag, len);
}

// returns pointer just beyond where we just wrote.
static uint8_t* string_copy_ascii_to_utf16be(uint8_t* dst, const char* src, size_t count) {
    while (count-- > 0) {
        *dst++ = 0;
        *dst++ = (uint8_t)(*src++);
    }
    return dst;
}

sk_sp<SkData> SkICC::WriteToICC(const SkColorSpaceTransferFn& fn, const SkMatrix44& toXYZD50) {
    if (!is_3x3(toXYZD50) || !is_valid_transfer_fn(fn)) {
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
        ptr = string_copy_ascii_to_utf16be(ptr, colorProfileTag, kICCDescriptionTagSize);
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
