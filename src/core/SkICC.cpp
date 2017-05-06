/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkEndian.h"
#include "SkFixed.h"
#include "SkICC.h"
#include "SkICCPriv.h"

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

// Google Skia (UTF-16)
static constexpr uint8_t kDescriptionTagBody[] = {
        0x00, 0x47, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x20, 0x00,
        0x53, 0x00, 0x6b, 0x00, 0x69, 0x00, 0x61, 0x00, 0x20,
    };
static_assert(SkIsAlign4(sizeof(kDescriptionTagBody)), "Description must be aligned to 4-bytes.");
static constexpr uint32_t kDescriptionTagHeader[7] {
    SkEndian_SwapBE32(kTAG_TextType),                        // Type signature
    0,                                                       // Reserved
    SkEndian_SwapBE32(1),                                    // Number of records
    SkEndian_SwapBE32(12),                                   // Record size (must be 12)
    SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')), // English USA
    SkEndian_SwapBE32(sizeof(kDescriptionTagBody)),          // Length of string
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
                                            sizeof(kDescriptionTagBody);
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
    memcpy(ptr, kDescriptionTagBody, sizeof(kDescriptionTagBody));
    ptr += sizeof(kDescriptionTagBody);

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
