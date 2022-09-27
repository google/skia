/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkICC.h"
#include "include/core/SkStream.h"
#include "include/private/SkFixed.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkEndian.h"
#include "src/core/SkICCPriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkUtils.h"

#include <string>
#include <vector>

static constexpr uint32_t kTAG_desc = SkSetFourByteTag('d', 'e', 's', 'c');
static constexpr uint32_t kTAG_cicp = SkSetFourByteTag('c', 'i', 'c', 'p');
static constexpr uint32_t kTAG_wtpt = SkSetFourByteTag('w', 't', 'p', 't');
static constexpr uint32_t kTAG_cprt = SkSetFourByteTag('c', 'p', 'r', 't');

struct ICCHeader {
    // Size of the profile (computed)
    uint32_t size;

    // Preferred CMM type (ignored)
    uint32_t cmm_type = 0;

    // Version 4.3 or 4.4 if CICP is included.
    uint32_t version = SkEndian_SwapBE32(0x04300000);

    // Display device profile
    uint32_t profile_class = SkEndian_SwapBE32(kDisplay_Profile);

    // RGB input color space;
    uint32_t data_color_space = SkEndian_SwapBE32(kRGB_ColorSpace);

    // XYZ profile connection space
    uint32_t pcs = SkEndian_SwapBE32(kXYZ_PCSSpace);

    // Date and time (ignored)
    uint8_t creation_date_time[12] = {0};

    // Profile signature
    uint32_t signature = SkEndian_SwapBE32(kACSP_Signature);

    // Platform target (ignored)
    uint32_t platform = 0;

    // Flags: not embedded, can be used independently
    uint32_t flags = 0x00000000;

    // Device manufacturer (ignored)
    uint32_t device_manufacturer = 0;

    // Device model (ignored)
    uint32_t device_model = 0;

    // Device attributes (ignored)
    uint8_t device_attributes[8] = {0};

    // Relative colorimetric rendering intent
    uint32_t rendering_intent = SkEndian_SwapBE32(1);

    // D50 standard illuminant (X, Y, Z)
    uint32_t illuminant_X = SkEndian_SwapBE32(0x0000f6d6);
    uint32_t illuminant_Y = SkEndian_SwapBE32(0x00010000);
    uint32_t illuminant_Z = SkEndian_SwapBE32(0x0000d32d);

    // Profile creator (ignored)
    uint32_t creator = 0;

    // Profile id checksum (ignored)
    uint8_t profile_id[16] = {0};

    // Reserved (ignored)
    uint8_t reserved[28] = {0};

    // Technically not part of header, but required
    uint32_t tag_count = 0;
};

// This is like SkFloatToFixed, but rounds to nearest, preserving as much accuracy as possible
// when going float -> fixed -> float (it has the same accuracy when going fixed -> float -> fixed).
// The use of double is necessary to accommodate the full potential 32-bit mantissa of the 16.16
// SkFixed value, and so avoiding rounding problems with float. Also, see the comment in SkFixed.h.
static SkFixed float_round_to_fixed(float x) {
    return sk_float_saturate2int((float)floor((double)x * SK_Fixed1 + 0.5));
}

static sk_sp<SkData> write_xyz_tag(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t data[] = {
            SkEndian_SwapBE32(kXYZ_PCSSpace),
            0,
            SkEndian_SwapBE32(x),
            SkEndian_SwapBE32(y),
            SkEndian_SwapBE32(z),
    };
    return SkData::MakeWithCopy(data, sizeof(data));
}

static sk_sp<SkData> write_xyz_tag(const skcms_Matrix3x3& toXYZD50, int col) {
    return write_xyz_tag(float_round_to_fixed(toXYZD50.vals[0][col]),
                         float_round_to_fixed(toXYZD50.vals[1][col]),
                         float_round_to_fixed(toXYZD50.vals[2][col]));
}

static sk_sp<SkData> write_wtpt_tag() {
    return write_xyz_tag(0x0000f6d6,   // X = 0.96420 (D50)
                         0x00010000,   // Y = 1.00000 (D50)
                         0x0000d32d);  // Z = 0.82491 (D50)
}

static sk_sp<SkData> write_para_tag(const skcms_TransferFunction& fn) {
    SkASSERT(skcms_TransferFunction_isSRGBish(&fn));
    const uint32_t data[] = {
            SkEndian_SwapBE32(kTAG_ParaCurveType),
            0,
            (uint32_t)(SkEndian_SwapBE16(kGABCDEF_ParaCurveType)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.g)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.a)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.b)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.c)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.d)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.e)),
            SkEndian_SwapBE32(float_round_to_fixed(fn.f)),
    };
    return SkData::MakeWithCopy(data, sizeof(data));
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

static constexpr uint32_t kCICPPrimariesSRGB = 1;
static constexpr uint32_t kCICPPrimariesP3 = 12;
static constexpr uint32_t kCICPPrimariesRec2020 = 9;

static uint32_t get_cicp_primaries(const skcms_Matrix3x3& toXYZD50) {
    if (nearly_equal(toXYZD50, SkNamedGamut::kSRGB)) {
        return kCICPPrimariesSRGB;
    } else if (nearly_equal(toXYZD50, SkNamedGamut::kDisplayP3)) {
        return kCICPPrimariesP3;
    } else if (nearly_equal(toXYZD50, SkNamedGamut::kRec2020)) {
        return kCICPPrimariesRec2020;
    }
    return 0;
}

static constexpr uint32_t kCICPTrfnSRGB = 1;
static constexpr uint32_t kCICPTrfn2Dot2 = 4;
static constexpr uint32_t kCICPTrfnLinear = 8;
static constexpr uint32_t kCICPTrfnPQ = 16;
static constexpr uint32_t kCICPTrfnHLG = 18;

static uint32_t get_cicp_trfn(const skcms_TransferFunction& fn) {
    switch (classify_transfer_fn(fn)) {
        case Bad_TF:
            return 0;
        case sRGBish_TF:
            if (nearly_equal(fn, SkNamedTransferFn::kSRGB)) {
                return kCICPTrfnSRGB;
            } else if (nearly_equal(fn, SkNamedTransferFn::k2Dot2)) {
                return kCICPTrfn2Dot2;
            } else if (nearly_equal(fn, SkNamedTransferFn::kLinear)) {
                return kCICPTrfnLinear;
            }
            break;
        case PQish_TF:
            // All PQ transfer functions are mapped to the single PQ value,
            // ignoring their SDR white level.
            return kCICPTrfnPQ;
            break;
        case HLGish_TF:
            // All HLG transfer functions are mapped to the single HLG value.
            return kCICPTrfnHLG;
            break;
        case HLGinvish_TF:
            return 0;
    }
    return 0;
}

static std::string get_desc_string(const skcms_TransferFunction& fn,
                                   const skcms_Matrix3x3& toXYZD50,
                                   uint32_t cicp_trfn,
                                   uint32_t cicp_primaries) {
    // Use a unique string for sRGB.
    if (cicp_trfn == kCICPPrimariesSRGB && cicp_primaries == kCICPTrfnSRGB) {
        return "sRGB";
    }

    // If available, use the named CICP primaries and transfer function.
    if (cicp_primaries && cicp_trfn) {
        std::string result;
        switch (cicp_trfn) {
            case kCICPTrfnSRGB:
                result += "sRGB";
                break;
            case kCICPTrfnLinear:
                result += "Linear";
                break;
            case kCICPTrfn2Dot2:
                result += "2.2";
                break;
            case kCICPTrfnPQ:
                result += "PQ";
                break;
            case kCICPTrfnHLG:
                result += "HLG";
                break;
            default:
                result += "Unknown";
                break;
        }
        result += " Transfer with ";
        switch (cicp_primaries) {
            case kCICPPrimariesSRGB:
                result += "sRGB";
                break;
            case kCICPPrimariesP3:
                result += "Display P3";
                break;
            case kCICPPrimariesRec2020:
                result += "Rec2020";
                break;
            default:
                result += "Unknown";
                break;
        }
        result += " Gamut";
        return result;
    }

    // Fall back to a prefix plus md5 hash.
    SkMD5 md5;
    md5.write(&toXYZD50, sizeof(toXYZD50));
    md5.write(&fn, sizeof(fn));
    SkMD5::Digest digest = md5.finish();
    std::string md5_hexstring(2 * sizeof(SkMD5::Digest), ' ');
    for (unsigned i = 0; i < sizeof(SkMD5::Digest); ++i) {
        uint8_t byte = digest.data[i];
        md5_hexstring[2 * i + 0] = SkHexadecimalDigits::gUpper[byte >> 4];
        md5_hexstring[2 * i + 1] = SkHexadecimalDigits::gUpper[byte & 0xF];
    }
    return "Google/Skia/" + md5_hexstring;
}

static sk_sp<SkData> write_text_tag(const std::string& text) {
    uint32_t header[] = {
            SkEndian_SwapBE32(kTAG_TextType),                         // Type signature
            0,                                                        // Reserved
            SkEndian_SwapBE32(1),                                     // Number of records
            SkEndian_SwapBE32(12),                                    // Record size (must be 12)
            SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')),  // English USA
            SkEndian_SwapBE32(2 * text.length()),                     // Length of string in bytes
            SkEndian_SwapBE32(28),                                    // Offset of string
    };
    SkDynamicMemoryWStream s;
    s.write(header, sizeof(header));
    for (size_t i = 0; i < text.length(); i++) {
        // Convert ASCII to big-endian UTF-16.
        s.write8(0);
        s.write8(text[i]);
    }
    s.padToAlign4();
    return s.detachAsData();
}

static sk_sp<SkData> write_cicp_tag(uint32_t primaries, uint32_t trfn) {
    SkDynamicMemoryWStream s;
    s.write32(SkEndian_SwapBE32(kTAG_cicp));  // Type signature
    s.write32(0);                             // Reserved
    s.write8(primaries);                      // Color primaries
    s.write8(trfn);                           // Transfer characteristics
    s.write8(0);                              // RGB matrix
    s.write8(1);                              // Full range
    return s.detachAsData();
}

sk_sp<SkData> SkWriteICCProfile(const skcms_TransferFunction& fn, const skcms_Matrix3x3& toXYZD50) {
    // Compute the CICP primaries and transfer function, if they can be
    // identified.
    uint32_t cicp_primaries = get_cicp_primaries(toXYZD50);
    uint32_t cicp_trfn = get_cicp_trfn(fn);
    if (classify_transfer_fn(fn) != sRGBish_TF) {
        // Non-sRGB-ish transfer functions can only be represented by CICP. IF
        // the transfer function is not sRGB-ish, and we don't have a CICP
        // representation, then fail.
        if (!cicp_primaries || !cicp_trfn) {
            return nullptr;
        }
    }

    std::vector<std::pair<uint32_t, sk_sp<SkData>>> tags;

    // Compute profile description tag
    std::string description = get_desc_string(fn, toXYZD50, cicp_trfn, cicp_primaries);
    tags.emplace_back(kTAG_desc, write_text_tag(description));

    // Compute XYZ tags
    tags.emplace_back(kTAG_rXYZ, write_xyz_tag(toXYZD50, 0));
    tags.emplace_back(kTAG_gXYZ, write_xyz_tag(toXYZD50, 1));
    tags.emplace_back(kTAG_bXYZ, write_xyz_tag(toXYZD50, 2));

    // If this is an HLG or PQ profile, include a CICP tag.
    bool has_cicp = false;
    if (cicp_trfn == kCICPTrfnPQ || cicp_trfn == kCICPTrfnHLG) {
        has_cicp = true;
        tags.emplace_back(kTAG_cicp, write_cicp_tag(cicp_primaries, cicp_trfn));

        // Use sRGB as the transfer function.
        // TODO(https://crbug.com/1366315): Provide a LUT based transform to
        // perform tone mapping.
        tags.emplace_back(kTAG_rTRC, write_para_tag(SkNamedTransferFn::kSRGB));
    } else {
        tags.emplace_back(kTAG_rTRC, write_para_tag(fn));
    }
    // Use empty data to indicate that the entry should use the previous tag's
    // data.
    tags.emplace_back(kTAG_gTRC, SkData::MakeEmpty());
    tags.emplace_back(kTAG_bTRC, SkData::MakeEmpty());

    // Compute white point tag (must be D50)
    tags.emplace_back(kTAG_wtpt, write_wtpt_tag());

    // Compute copyright tag
    tags.emplace_back(kTAG_cprt, write_text_tag("Google Inc. 2016"));

    // Compute the size of the profile.
    size_t tag_data_size = 0;
    for (const auto& tag : tags) {
        tag_data_size += tag.second->size();
    }
    size_t tag_table_size = kICCTagTableEntrySize * tags.size();
    size_t profile_size = kICCHeaderSize + tag_table_size + tag_data_size;

    // Write the header.
    ICCHeader header;
    header.size = SkEndian_SwapBE32(profile_size);
    header.tag_count = SkEndian_SwapBE32(tags.size());
    if (has_cicp) {
        header.version = SkEndian_SwapBE32(0x04400000);
    }
    static_assert(sizeof(header) == kICCHeaderSize);

    SkAutoMalloc profile(profile_size);
    uint8_t* ptr = (uint8_t*)profile.get();
    memcpy(ptr, &header, sizeof(header));
    ptr += sizeof(header);

    // Write the tag table. Track the offset and size of the previous tag to
    // compute each tag's offset. An empty SkData indicates that the previous
    // tag is to be reused.
    size_t last_tag_offset = sizeof(header) + tag_table_size;
    size_t last_tag_size = 0;
    for (const auto& tag : tags) {
        if (!tag.second->isEmpty()) {
            last_tag_offset = last_tag_offset + last_tag_size;
            last_tag_size = tag.second->size();
        }
        uint32_t tag_table_entry[3] = {
                SkEndian_SwapBE32(tag.first),
                SkEndian_SwapBE32(last_tag_offset),
                SkEndian_SwapBE32(last_tag_size),
        };
        memcpy(ptr, tag_table_entry, sizeof(tag_table_entry));
        ptr += sizeof(tag_table_entry);
    }

    // Write the tags.
    for (const auto& tag : tags) {
        if (tag.second->isEmpty()) continue;
        memcpy(ptr, tag.second->data(), tag.second->size());
        ptr += tag.second->size();
    }

    SkASSERT(profile_size == static_cast<size_t>(ptr - (uint8_t*)profile.get()));
    return SkData::MakeFromMalloc(profile.release(), profile_size);
}
