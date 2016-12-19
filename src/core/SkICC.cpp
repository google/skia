/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
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
