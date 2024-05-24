/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkICC.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatingPoint.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkEndian.h"
#include "src/core/SkMD5.h"
#include "src/core/SkStreamPriv.h"
#include "src/encode/SkICCPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace {

// The number of input and output channels.
constexpr size_t kNumChannels = 3;

// The D50 illuminant.
constexpr float kD50_x = 0.9642f;
constexpr float kD50_y = 1.0000f;
constexpr float kD50_z = 0.8249f;

// This is like SkFloatToFixed, but rounds to nearest, preserving as much accuracy as possible
// when going float -> fixed -> float (it has the same accuracy when going fixed -> float -> fixed).
// The use of double is necessary to accommodate the full potential 32-bit mantissa of the 16.16
// SkFixed value, and so avoiding rounding problems with float. Also, see the comment in SkFixed.h.
SkFixed float_round_to_fixed(float x) {
    return sk_float_saturate2int((float)floor((double)x * SK_Fixed1 + 0.5));
}

// Convert a float to a uInt16Number, with 0.0 mapping go 0 and 1.0 mapping to |one|.
uint16_t float_to_uInt16Number(float x, uint16_t one) {
    x = x * one + 0.5;
    if (x > one) return one;
    if (x < 0) return 0;
    return static_cast<uint16_t>(x);
}

// The uInt16Number used by curveType has 1.0 map to 0xFFFF. See section "10.6. curveType".
constexpr uint16_t kOne16CurveType = 0xFFFF;

// The uInt16Number used to encoude XYZ values has 1.0 map to 0x8000. See section "6.3.4.2 General
// PCS encoding" and Table 11.
constexpr uint16_t kOne16XYZ = 0x8000;

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

    // Profile connection space.
    uint32_t pcs = SkEndian_SwapBE32(kXYZ_PCSSpace);

    // Date and time (ignored)
    uint16_t creation_date_year = SkEndian_SwapBE16(2016);
    uint16_t creation_date_month = SkEndian_SwapBE16(1);  // 1-12
    uint16_t creation_date_day = SkEndian_SwapBE16(1);  // 1-31
    uint16_t creation_date_hours = 0;  // 0-23
    uint16_t creation_date_minutes = 0;  // 0-59
    uint16_t creation_date_seconds = 0;  // 0-59

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
    uint32_t illuminant_X = SkEndian_SwapBE32(float_round_to_fixed(kD50_x));
    uint32_t illuminant_Y = SkEndian_SwapBE32(float_round_to_fixed(kD50_y));
    uint32_t illuminant_Z = SkEndian_SwapBE32(float_round_to_fixed(kD50_z));

    // Profile creator (ignored)
    uint32_t creator = 0;

    // Profile id checksum (ignored)
    uint8_t profile_id[16] = {0};

    // Reserved (ignored)
    uint8_t reserved[28] = {0};

    // Technically not part of header, but required
    uint32_t tag_count = 0;
};

sk_sp<SkData> write_xyz_tag(float x, float y, float z) {
    uint32_t data[] = {
            SkEndian_SwapBE32(kXYZ_PCSSpace),
            0,
            SkEndian_SwapBE32(float_round_to_fixed(x)),
            SkEndian_SwapBE32(float_round_to_fixed(y)),
            SkEndian_SwapBE32(float_round_to_fixed(z)),
    };
    return SkData::MakeWithCopy(data, sizeof(data));
}

sk_sp<SkData> write_matrix(const skcms_Matrix3x4* matrix) {
    uint32_t data[12];
    // See layout details in section "10.12.5 Matrix".
    size_t k = 0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            data[k++] = SkEndian_SwapBE32(float_round_to_fixed(matrix->vals[i][j]));
        }
    }
    for (int i = 0; i < 3; ++i) {
        data[k++] = SkEndian_SwapBE32(float_round_to_fixed(matrix->vals[i][3]));
    }
    return SkData::MakeWithCopy(data, sizeof(data));
}

bool nearly_equal(float x, float y) {
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

bool nearly_equal(const skcms_TransferFunction& u,
                  const skcms_TransferFunction& v) {
    return nearly_equal(u.g, v.g)
        && nearly_equal(u.a, v.a)
        && nearly_equal(u.b, v.b)
        && nearly_equal(u.c, v.c)
        && nearly_equal(u.d, v.d)
        && nearly_equal(u.e, v.e)
        && nearly_equal(u.f, v.f);
}

bool nearly_equal(const skcms_Matrix3x3& u, const skcms_Matrix3x3& v) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (!nearly_equal(u.vals[r][c], v.vals[r][c])) {
                return false;
            }
        }
    }
    return true;
}

constexpr uint32_t kCICPPrimariesSRGB = 1;
constexpr uint32_t kCICPPrimariesP3 = 12;
constexpr uint32_t kCICPPrimariesRec2020 = 9;

uint32_t get_cicp_primaries(const skcms_Matrix3x3& toXYZD50) {
    if (nearly_equal(toXYZD50, SkNamedGamut::kSRGB)) {
        return kCICPPrimariesSRGB;
    } else if (nearly_equal(toXYZD50, SkNamedGamut::kDisplayP3)) {
        return kCICPPrimariesP3;
    } else if (nearly_equal(toXYZD50, SkNamedGamut::kRec2020)) {
        return kCICPPrimariesRec2020;
    }
    return 0;
}

constexpr uint32_t kCICPTrfnSRGB = 1;
constexpr uint32_t kCICPTrfn2Dot2 = 4;
constexpr uint32_t kCICPTrfnLinear = 8;
constexpr uint32_t kCICPTrfnPQ = 16;
constexpr uint32_t kCICPTrfnHLG = 18;

uint32_t get_cicp_trfn(const skcms_TransferFunction& fn) {
    switch (skcms_TransferFunction_getType(&fn)) {
        case skcms_TFType_Invalid:
            return 0;
        case skcms_TFType_sRGBish:
            if (nearly_equal(fn, SkNamedTransferFn::kSRGB)) {
                return kCICPTrfnSRGB;
            } else if (nearly_equal(fn, SkNamedTransferFn::k2Dot2)) {
                return kCICPTrfn2Dot2;
            } else if (nearly_equal(fn, SkNamedTransferFn::kLinear)) {
                return kCICPTrfnLinear;
            }
            break;
        case skcms_TFType_PQish:
            // All PQ transfer functions are mapped to the single PQ value,
            // ignoring their SDR white level.
            return kCICPTrfnPQ;
        case skcms_TFType_HLGish:
            // All HLG transfer functions are mapped to the single HLG value.
            return kCICPTrfnHLG;
        case skcms_TFType_HLGinvish:
            return 0;
    }
    return 0;
}

std::string get_desc_string(const skcms_TransferFunction& fn,
                            const skcms_Matrix3x3& toXYZD50) {
    const uint32_t cicp_trfn = get_cicp_trfn(fn);
    const uint32_t cicp_primaries = get_cicp_primaries(toXYZD50);

    // Use a unique string for sRGB.
    if (cicp_trfn == kCICPPrimariesSRGB && cicp_primaries == kCICPTrfnSRGB) {
        return "sRGB";
    }

    // If available, use the named CICP primaries and transfer function.
    if (cicp_primaries && cicp_trfn) {
        std::string result;
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
        result += " Gamut with ";
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
        result += " Transfer";
        return result;
    }

    // Fall back to a prefix plus md5 hash.
    SkMD5 md5;
    md5.write(&toXYZD50, sizeof(toXYZD50));
    md5.write(&fn, sizeof(fn));
    SkMD5::Digest digest = md5.finish();
    return std::string("Google/Skia/") + digest.toHexString().c_str();
}

sk_sp<SkData> write_text_tag(const char* text) {
    uint32_t text_length = strlen(text);
    uint32_t header[] = {
            SkEndian_SwapBE32(kTAG_TextType),                         // Type signature
            0,                                                        // Reserved
            SkEndian_SwapBE32(1),                                     // Number of records
            SkEndian_SwapBE32(12),                                    // Record size (must be 12)
            SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')),  // English USA
            SkEndian_SwapBE32(2 * text_length),                       // Length of string in bytes
            SkEndian_SwapBE32(28),                                    // Offset of string
    };
    SkDynamicMemoryWStream s;
    s.write(header, sizeof(header));
    for (size_t i = 0; i < text_length; i++) {
        // Convert ASCII to big-endian UTF-16.
        s.write8(0);
        s.write8(text[i]);
    }
    s.padToAlign4();
    return s.detachAsData();
}

// Write a CICP tag.
sk_sp<SkData> write_cicp_tag(const skcms_CICP& cicp) {
    SkDynamicMemoryWStream s;
    SkWStreamWriteU32BE(&s, kTAG_cicp);       // Type signature
    SkWStreamWriteU32BE(&s, 0);               // Reserved
    s.write8(cicp.color_primaries);           // Color primaries
    s.write8(cicp.transfer_characteristics);  // Transfer characteristics
    s.write8(cicp.matrix_coefficients);       // RGB matrix
    s.write8(cicp.video_full_range_flag);     // Full range
    return s.detachAsData();
}

constexpr float kToneMapInputMax = 1000.f / 203.f;
constexpr float kToneMapOutputMax = 1.f;

// Scalar tone map gain function.
float tone_map_gain(float x) {
    // The PQ transfer function will map to the range [0, 1]. Linearly scale
    // it up to the range [0, 1,000/203]. We will then tone map that back
    // down to [0, 1].
    constexpr float kToneMapA = kToneMapOutputMax / (kToneMapInputMax * kToneMapInputMax);
    constexpr float kToneMapB = 1.f / kToneMapOutputMax;
    return (1.f + kToneMapA * x) / (1.f + kToneMapB * x);
}

// Scalar tone map inverse function
float tone_map_inverse(float y) {
    constexpr float kToneMapA = kToneMapOutputMax / (kToneMapInputMax * kToneMapInputMax);
    constexpr float kToneMapB = 1.f / kToneMapOutputMax;

    // This is a quadratic equation of the form a*x*x + b*x + c = 0
    const float a = kToneMapA;
    const float b = (1 - kToneMapB * y);
    const float c = -y;
    const float discriminant = b * b - 4.f * a * c;
    if (discriminant < 0.f) {
        return 0.f;
    }
    return (-b + sqrtf(discriminant)) / (2.f * a);
}

// Evaluate PQ and HLG transfer functions without tonemapping. The maximum returned value is
// kToneMapInputMax.
float hdr_trfn_eval(const skcms_TransferFunction& fn, float x) {
    if (skcms_TransferFunction_isHLGish(&fn)) {
        // For HLG this curve is the inverse OETF and then a per-channel OOTF.
        x = skcms_TransferFunction_eval(&SkNamedTransferFn::kHLG, x) / 12.f;
        x *= std::pow(x, 0.2);
    } else if (skcms_TransferFunction_isPQish(&fn)) {
        // For PQ this is the EOTF, scaled so that 1,000 nits maps to 1.0.
        x = 10.f * skcms_TransferFunction_eval(&SkNamedTransferFn::kPQ, x);
        x = std::min(x, 1.f);
    }

    // Scale x so that 203 nits maps to 1.0.
    x *= kToneMapInputMax;
    return x;
}

// Write a lookup table based 1D curve.
sk_sp<SkData> write_trc_tag(const skcms_Curve& trc) {
    SkDynamicMemoryWStream s;
    if (trc.table_entries) {
        SkWStreamWriteU32BE(&s, kTAG_CurveType);     // Type
        SkWStreamWriteU32BE(&s, 0);                  // Reserved
        SkWStreamWriteU32BE(&s, trc.table_entries);  // Value count
        for (uint32_t i = 0; i < trc.table_entries; ++i) {
            uint16_t value = reinterpret_cast<const uint16_t*>(trc.table_16)[i];
            s.write16(value);
        }
    } else {
        SkWStreamWriteU32BE(&s, kTAG_ParaCurveType);       // Type
        s.write32(0);                                      // Reserved
        const auto& fn = trc.parametric;
        SkASSERT(skcms_TransferFunction_isSRGBish(&fn));
        if (fn.a == 1.f && fn.b == 0.f && fn.c == 0.f && fn.d == 0.f && fn.e == 0.f &&
            fn.f == 0.f) {
            SkWStreamWriteU16BE(&s, kExponential_ParaCurveType);
            SkWStreamWriteU16BE(&s, 0);
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.g));
        } else {
            SkWStreamWriteU16BE(&s, kGABCDEF_ParaCurveType);
            SkWStreamWriteU16BE(&s, 0);
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.g));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.a));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.b));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.c));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.d));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.e));
            SkWStreamWriteU32BE(&s, float_round_to_fixed(fn.f));
        }
    }
    s.padToAlign4();
    return s.detachAsData();
}

sk_sp<SkData> write_clut(const uint8_t* grid_points, const uint8_t* grid_16) {
    SkDynamicMemoryWStream s;
    for (size_t i = 0; i < 16; ++i) {
        s.write8(i < kNumChannels ? grid_points[i] : 0);  // Grid size
    }
    s.write8(2);  // Grid byte width (always 16-bit)
    s.write8(0);  // Reserved
    s.write8(0);  // Reserved
    s.write8(0);  // Reserved

    uint32_t value_count = kNumChannels;
    for (uint32_t i = 0; i < kNumChannels; ++i) {
        value_count *= grid_points[i];
    }
    for (uint32_t i = 0; i < value_count; ++i) {
        uint16_t value = reinterpret_cast<const uint16_t*>(grid_16)[i];
        s.write16(value);
    }
    s.padToAlign4();
    return s.detachAsData();
}

// Write an A2B or B2A tag.
sk_sp<SkData> write_mAB_or_mBA_tag(uint32_t type,
                                   const skcms_Curve* b_curves,
                                   const skcms_Curve* a_curves,
                                   const uint8_t* grid_points,
                                   const uint8_t* grid_16,
                                   const skcms_Curve* m_curves,
                                   const skcms_Matrix3x4* matrix) {
    size_t offset = 32;

    // The "B" curve is required.
    size_t b_curves_offset = offset;
    sk_sp<SkData> b_curves_data[kNumChannels];
    SkASSERT(b_curves);
    for (size_t i = 0; i < kNumChannels; ++i) {
        b_curves_data[i] = write_trc_tag(b_curves[i]);
        SkASSERT(b_curves_data[i]);
        offset += b_curves_data[i]->size();
    }

    // The CLUT.
    size_t clut_offset = 0;
    sk_sp<SkData> clut;
    if (grid_points) {
        SkASSERT(grid_16);
        clut_offset = offset;
        clut = write_clut(grid_points, grid_16);
        SkASSERT(clut);
        offset += clut->size();
    }

    // The "A" curves.
    size_t a_curves_offset = 0;
    sk_sp<SkData> a_curves_data[kNumChannels];
    if (a_curves) {
        SkASSERT(grid_points);
        SkASSERT(grid_16);
        a_curves_offset = offset;
        for (size_t i = 0; i < kNumChannels; ++i) {
            a_curves_data[i] = write_trc_tag(a_curves[i]);
            SkASSERT(a_curves_data[i]);
            offset += a_curves_data[i]->size();
        }
    }

    // The matrix.
    size_t matrix_offset = 0;
    sk_sp<SkData> matrix_data;
    if (matrix) {
        SkASSERT(m_curves);
        matrix_offset = offset;
        matrix_data = write_matrix(matrix);
        offset += matrix_data->size();
    }

    // The "M" curves.
    size_t m_curves_offset = 0;
    sk_sp<SkData> m_curves_data[kNumChannels];
    if (m_curves) {
        SkASSERT(matrix);
        m_curves_offset = offset;
        for (size_t i = 0; i < kNumChannels; ++i) {
            m_curves_data[i] = write_trc_tag(m_curves[i]);
            SkASSERT(a_curves_data[i]);
            offset += m_curves_data[i]->size();
        }
    }

    SkDynamicMemoryWStream s;
    SkWStreamWriteU32BE(&s, type);  // Type signature
    s.write32(0);                   // Reserved
    s.write8(kNumChannels);         // Input channels
    s.write8(kNumChannels);         // Output channels
    s.write16(0);                   // Reserved
    SkWStreamWriteU32BE(&s, b_curves_offset);  // B curve offset
    SkWStreamWriteU32BE(&s, matrix_offset);    // Matrix offset
    SkWStreamWriteU32BE(&s, m_curves_offset);  // M curve offset
    SkWStreamWriteU32BE(&s, clut_offset);      // CLUT offset
    SkWStreamWriteU32BE(&s, a_curves_offset);  // A curve offset
    SkASSERT(s.bytesWritten() == b_curves_offset);
    for (size_t i = 0; i < kNumChannels; ++i) {
        s.write(b_curves_data[i]->data(), b_curves_data[i]->size());
    }
    if (clut) {
        SkASSERT(s.bytesWritten() == clut_offset);
        s.write(clut->data(), clut->size());
    }
    if (a_curves) {
        SkASSERT(s.bytesWritten() == a_curves_offset);
        for (size_t i = 0; i < kNumChannels; ++i) {
            s.write(a_curves_data[i]->data(), a_curves_data[i]->size());
        }
    }
    if (matrix_data) {
        SkASSERT(s.bytesWritten() == matrix_offset);
        s.write(matrix_data->data(), matrix_data->size());
    }
    if (m_curves) {
        SkASSERT(s.bytesWritten() == m_curves_offset);
        for (size_t i = 0; i < kNumChannels; ++i) {
            s.write(m_curves_data[i]->data(), m_curves_data[i]->size());
        }
    }
    return s.detachAsData();
}

}  // namespace

sk_sp<SkData> SkWriteICCProfile(const skcms_ICCProfile* profile, const char* desc) {
    ICCHeader header;

    std::vector<std::pair<uint32_t, sk_sp<SkData>>> tags;

    // Compute primaries.
    if (profile->has_toXYZD50) {
        const auto& m = profile->toXYZD50;
        tags.emplace_back(kTAG_rXYZ, write_xyz_tag(m.vals[0][0], m.vals[1][0], m.vals[2][0]));
        tags.emplace_back(kTAG_gXYZ, write_xyz_tag(m.vals[0][1], m.vals[1][1], m.vals[2][1]));
        tags.emplace_back(kTAG_bXYZ, write_xyz_tag(m.vals[0][2], m.vals[1][2], m.vals[2][2]));
    }

    // Compute white point tag (must be D50)
    tags.emplace_back(kTAG_wtpt, write_xyz_tag(kD50_x, kD50_y, kD50_z));

    // Compute transfer curves.
    if (profile->has_trc) {
        tags.emplace_back(kTAG_rTRC, write_trc_tag(profile->trc[0]));

        // Use empty data to indicate that the entry should use the previous tag's
        // data.
        if (!memcmp(&profile->trc[1], &profile->trc[0], sizeof(profile->trc[0]))) {
            tags.emplace_back(kTAG_gTRC, SkData::MakeEmpty());
        } else {
            tags.emplace_back(kTAG_gTRC, write_trc_tag(profile->trc[1]));
        }

        if (!memcmp(&profile->trc[2], &profile->trc[1], sizeof(profile->trc[1]))) {
            tags.emplace_back(kTAG_bTRC, SkData::MakeEmpty());
        } else {
            tags.emplace_back(kTAG_bTRC, write_trc_tag(profile->trc[2]));
        }
    }

    // Compute CICP.
    if (profile->has_CICP) {
        // The CICP tag is present in ICC 4.4, so update the header's version.
        header.version = SkEndian_SwapBE32(0x04400000);
        tags.emplace_back(kTAG_cicp, write_cicp_tag(profile->CICP));
    }

    // Compute A2B0.
    if (profile->has_A2B) {
        const auto& a2b = profile->A2B;
        SkASSERT(a2b.output_channels == kNumChannels);
        auto a2b_data = write_mAB_or_mBA_tag(kTAG_mABType,
                                             a2b.output_curves,
                                             a2b.input_channels ? a2b.input_curves : nullptr,
                                             a2b.input_channels ? a2b.grid_points : nullptr,
                                             a2b.input_channels ? a2b.grid_16 : nullptr,
                                             a2b.matrix_channels ? a2b.matrix_curves : nullptr,
                                             a2b.matrix_channels ? &a2b.matrix : nullptr);
        tags.emplace_back(kTAG_A2B0, std::move(a2b_data));
    }

    // Compute B2A0.
    if (profile->has_B2A) {
        const auto& b2a = profile->B2A;
        SkASSERT(b2a.input_channels == kNumChannels);
        auto b2a_data = write_mAB_or_mBA_tag(kTAG_mBAType,
                                             b2a.input_curves,
                                             b2a.output_channels ? b2a.input_curves : nullptr,
                                             b2a.output_channels ? b2a.grid_points : nullptr,
                                             b2a.output_channels ? b2a.grid_16 : nullptr,
                                             b2a.matrix_channels ? b2a.matrix_curves : nullptr,
                                             b2a.matrix_channels ? &b2a.matrix : nullptr);
        tags.emplace_back(kTAG_B2A0, std::move(b2a_data));
    }

    // Compute copyright tag
    tags.emplace_back(kTAG_cprt, write_text_tag("Google Inc. 2016"));

    // Ensure that the desc isn't empty https://crbug.com/329032158
    std::string generatedDesc;
    if (!desc || *desc == '\0') {
        SkMD5 md5;
        for (const auto& tag : tags) {
            md5.write(&tag.first, sizeof(tag.first));
            md5.write(tag.second->bytes(), tag.second->size());
        }
        SkMD5::Digest digest = md5.finish();
        generatedDesc = std::string("Google/Skia/") + digest.toHexString().c_str();
        desc = generatedDesc.c_str();
    }
    // Compute profile description tag
    tags.emplace(tags.begin(), kTAG_desc, write_text_tag(desc));

    // Compute the size of the profile.
    size_t tag_data_size = 0;
    for (const auto& tag : tags) {
        tag_data_size += tag.second->size();
    }
    size_t tag_table_size = kICCTagTableEntrySize * tags.size();
    size_t profile_size = kICCHeaderSize + tag_table_size + tag_data_size;

    // Write the header.
    header.data_color_space = SkEndian_SwapBE32(profile->data_color_space);
    header.pcs = SkEndian_SwapBE32(profile->pcs);
    header.size = SkEndian_SwapBE32(profile_size);
    header.tag_count = SkEndian_SwapBE32(tags.size());

    SkAutoMalloc profile_data(profile_size);
    uint8_t* ptr = (uint8_t*)profile_data.get();
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

    SkASSERT(profile_size == static_cast<size_t>(ptr - (uint8_t*)profile_data.get()));
    return SkData::MakeFromMalloc(profile_data.release(), profile_size);
}

sk_sp<SkData> SkWriteICCProfile(const skcms_TransferFunction& fn, const skcms_Matrix3x3& toXYZD50) {
    skcms_ICCProfile profile;
    memset(&profile, 0, sizeof(profile));
    std::vector<uint16_t> trc_table;
    std::vector<uint16_t> a2b_grid;

    profile.data_color_space = skcms_Signature_RGB;
    profile.pcs = skcms_Signature_XYZ;

    // Populate toXYZD50.
    {
        profile.has_toXYZD50 = true;
        profile.toXYZD50 = toXYZD50;
    }

    // Populate the analytic TRC for sRGB-like curves.
    if (skcms_TransferFunction_isSRGBish(&fn)) {
        profile.has_trc = true;
        profile.trc[0].table_entries = 0;
        profile.trc[0].parametric = fn;
        memcpy(&profile.trc[1], &profile.trc[0], sizeof(profile.trc[0]));
        memcpy(&profile.trc[2], &profile.trc[0], sizeof(profile.trc[0]));
    }

    // Populate A2B (PQ and HLG only).
    if (skcms_TransferFunction_isPQish(&fn) || skcms_TransferFunction_isHLGish(&fn)) {
        // Populate a 1D curve to perform per-channel conversion to linear and tone mapping.
        constexpr uint32_t kTrcTableSize = 65;
        trc_table.resize(kTrcTableSize);
        for (uint32_t i = 0; i < kTrcTableSize; ++i) {
            float x = i / (kTrcTableSize - 1.f);
            x = hdr_trfn_eval(fn, x);
            x *= tone_map_gain(x);
            trc_table[i] = SkEndian_SwapBE16(float_to_uInt16Number(x, kOne16CurveType));
        }

        // Populate the grid with a 3D LUT to do cross-channel tone mapping.
        constexpr uint32_t kGridSize = 11;
        a2b_grid.resize(kGridSize * kGridSize * kGridSize * kNumChannels);
        size_t a2b_grid_index = 0;
        for (uint32_t r_index = 0; r_index < kGridSize; ++r_index) {
            for (uint32_t g_index = 0; g_index < kGridSize; ++g_index) {
                for (uint32_t b_index = 0; b_index < kGridSize; ++b_index) {
                    float rgb[3] = {
                            r_index / (kGridSize - 1.f),
                            g_index / (kGridSize - 1.f),
                            b_index / (kGridSize - 1.f),
                    };

                    // Un-apply the per-channel tone mapping.
                    for (auto& c : rgb) {
                        c = tone_map_inverse(c);
                    }

                    // For HLG, mix the channels according to the OOTF.
                    if (skcms_TransferFunction_isHLGish(&fn)) {
                        // Scale to [0, 1].
                        for (auto& c : rgb) {
                            c /= kToneMapInputMax;
                        }

                        // Un-apply the per-channel OOTF.
                        for (auto& c : rgb) {
                            c = std::pow(c, 1 / 1.2);
                        }

                        // Re-apply the cross-channel OOTF.
                        float Y = 0.2627f * rgb[0] + 0.6780f * rgb[1] + 0.0593f * rgb[2];
                        for (auto& c : rgb) {
                            c *= std::pow(Y, 0.2);
                        }

                        // Scale back up to 1.0 being 1,000/203.
                        for (auto& c : rgb) {
                            c *= kToneMapInputMax;
                        }
                    }

                    // Apply tone mapping to take 1,000/203 to 1.0.
                    {
                        float max_rgb = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
                        for (auto& c : rgb) {
                            c *= tone_map_gain(0.5 * (c + max_rgb));
                            c = std::min(c, 1.f);
                        }
                    }

                    // Write the result to the LUT.
                    for (const auto& c : rgb) {
                        a2b_grid[a2b_grid_index++] =
                                SkEndian_SwapBE16(float_to_uInt16Number(c, kOne16XYZ));
                    }
                }
            }
        }

        // Populate A2B as this tone mapping.
        profile.has_A2B = true;
        profile.A2B.input_channels = kNumChannels;
        profile.A2B.output_channels = kNumChannels;
        profile.A2B.matrix_channels = kNumChannels;
        for (size_t i = 0; i < kNumChannels; ++i) {
            profile.A2B.grid_points[i] = kGridSize;
            // Set the input curve to convert to linear pre-OOTF space.
            profile.A2B.input_curves[i].table_entries = kTrcTableSize;
            profile.A2B.input_curves[i].table_16 = reinterpret_cast<uint8_t*>(trc_table.data());
            // The output and matrix curves are the identity.
            profile.A2B.output_curves[i].parametric = SkNamedTransferFn::kLinear;
            profile.A2B.matrix_curves[i].parametric = SkNamedTransferFn::kLinear;
            // Set the matrix to convert from the primaries to XYZD50.
            for (size_t j = 0; j < 3; ++j) {
                profile.A2B.matrix.vals[i][j] = toXYZD50.vals[i][j];
            }
            profile.A2B.matrix.vals[i][3] = 0.f;
        }
        profile.A2B.grid_16 = reinterpret_cast<const uint8_t*>(a2b_grid.data());

        // Populate B2A as the identity.
        profile.has_B2A = true;
        profile.B2A.input_channels = kNumChannels;
        for (size_t i = 0; i < 3; ++i) {
            profile.B2A.input_curves[i].parametric = SkNamedTransferFn::kLinear;
        }
    }

    // Populate CICP.
    if (skcms_TransferFunction_isHLGish(&fn) || skcms_TransferFunction_isPQish(&fn)) {
        profile.has_CICP = true;
        profile.CICP.color_primaries = get_cicp_primaries(toXYZD50);
        profile.CICP.transfer_characteristics = get_cicp_trfn(fn);
        profile.CICP.matrix_coefficients = 0;
        profile.CICP.video_full_range_flag = 1;
        SkASSERT(profile.CICP.color_primaries);
        SkASSERT(profile.CICP.transfer_characteristics);
    }

    std::string description = get_desc_string(fn, toXYZD50);
    return SkWriteICCProfile(&profile, description.c_str());
}
