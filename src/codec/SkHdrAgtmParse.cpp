/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "src/core/SkStreamPriv.h"

namespace {

// Return x clamped to [a, b].
template<typename T, typename U, typename V>
T clamp(T x, U a, V b) {
  return std::min(std::max(x, static_cast<T>(a)), static_cast<T>(b));
}

// Convert f by scaling by `scale`, offsetting by `offset`, and then clamping the result to
// [`clamp_min`, `clamp_max`].
uint16_t float_to_uint16(float f,
                                uint16_t clamp_min, uint16_t clamp_max,
                                uint16_t offset, float scale) {
    int32_t v = static_cast<int32_t>(std::lround(f * scale)) + offset;
    return clamp(v, clamp_min, clamp_max);
}

// The inverse of float_to_uint16's conversion.
float uint16_to_float(uint16_t v,
                             uint16_t clamp_min, uint16_t clamp_max,
                             uint16_t offset, float scale) {
  return (static_cast<int32_t>(clamp(v, clamp_min, clamp_max)) - offset) / scale;
}

// Helper class to read individual fields of a uint8_t bitfield.
class BitfieldReader {
  public:
    bool readFromStream(SkMemoryStream& s) {
        return s.readU8(&fBits);
    }
    uint8_t readBits(uint8_t bits) {
        // Read the topmost `bits` bits, then shift them off.
        uint8_t result = fBits >> (8 - bits);
        fBits <<= bits;
        fBitsRead += bits;
        return result;
    }
  private:
    uint8_t fBits = 0;
    uint8_t fBitsRead = 0;
};

// Helper class to write individual fields of a uint8_t bitfield.
class BitfieldWriter {
  public:
    void writeBits(uint8_t value, uint8_t bits) {
        // Ensure the value fits in `bits`.
        SkASSERT(value <= (1 << bits) - 1);
        fBits <<= bits;
        fBits |= value;
        fBitsWritten += bits;
    }
    void padAndWriteToStream(SkDynamicMemoryWStream& s) {
        // Write the remaining bits as zero, then write the result.
        SkASSERT(fBitsWritten <= 8);
        fBits <<= (8 - fBitsWritten);
        fBitsWritten = 8;
        s.write8(fBits);
    }
  private:
    uint8_t fBits = 0;
    uint8_t fBitsWritten = 0;
};

// Wrapper to return false if the specified call returns false. Used to keep syntax parsing
// shorter.
#define RETURN_ON_FALSE(x) \
    do { \
        if (!(x)) { \
            SkDebugf("AGTM parsing failed %s at %d\n", #x, __LINE__); \
            return false; \
        } \
    } while (0)

// All syntax elements listed in Annex C.2 (excluding reserved_zero). Parsing and serializing of
// an skhdr::Agtm is done in two steps:
// * converting from the metadata items to/from syntax elements (from Annex C.3)
// * serializing the syntax elements to/from a stream (from Annex C.2)
struct AgtmSyntax {
    static constexpr uint8_t kNumChromaticityValues = 8;
    static constexpr uint8_t kMaxNumAlternateImages = 4;
    static constexpr uint8_t kMaxNumControlPoints = 32;
    static constexpr uint8_t kNumMixCoefficients = 6;

    // Parse or write the syntax elements according to Annex C.2.
    bool parse_application_info(SkMemoryStream& s);
    void write_application_info(SkDynamicMemoryWStream& s);

    // Parse or write according to Table C.1.
    bool parse_color_volume_transform(SkMemoryStream& s);
    void write_color_volume_transform(SkDynamicMemoryWStream& s);

    // Parse or write according to Table C.3.
    bool parse_adaptive_tone_map(SkMemoryStream& s);
    void write_adaptive_tone_map(SkDynamicMemoryWStream& s);

    // Parse or write according to Table C.4.
    bool parse_component_mixing(uint8_t a, SkMemoryStream& s);
    void write_component_mixing(uint8_t a, SkDynamicMemoryWStream& s);

    // Parse or write according to Table C.5.
    bool parse_gain_curve(uint8_t a, SkMemoryStream& s);
    void write_gain_curve(uint8_t a, SkDynamicMemoryWStream& s);

    // syntax elements of smpte_st_2094_50_application_info()
    uint8_t application_version:3;
    uint8_t minimum_application_version:3;

    // syntax elements of smpte_st_2094_50_color_volume_transform()
    uint8_t has_custom_hdr_reference_white_flag:1;
    uint8_t has_adaptive_tone_map_flag:1;
    uint16_t hdr_reference_white;

    // syntax elements of smpte_st_2094_50_adaptive_tone_map()
    uint16_t baseline_hdr_headroom;
    uint8_t use_reference_white_tone_mapping_flag:1;
    uint8_t num_alternate_images:3;
    uint8_t gain_application_space_chromaticities_flag:2;
    uint8_t has_common_component_mix_params_flag:1;
    uint8_t has_common_curve_params_flag:1;
    uint16_t gain_application_space_chromaticities[kNumChromaticityValues];
    uint16_t alternate_hdr_headrooms[kMaxNumAlternateImages];

    // syntax elements of smpte_st_2094_50_component_mixing()
    uint8_t component_mixing_type[kMaxNumAlternateImages];
    uint8_t has_component_mixing_coefficient_flag[kMaxNumAlternateImages][kNumMixCoefficients];
    uint16_t component_mixing_coefficient[kMaxNumAlternateImages][kNumMixCoefficients];

    // syntax elements of smpte_st_2094_50_gain_curve()
    uint8_t gain_curve_num_control_points_minus_1[kMaxNumAlternateImages];
    uint8_t gain_curve_use_pchip_slope_flag[kMaxNumAlternateImages];
    uint16_t gain_curve_control_points_x[kMaxNumAlternateImages][kMaxNumControlPoints];
    uint16_t gain_curve_control_points_y[kMaxNumAlternateImages][kMaxNumControlPoints];
    uint16_t gain_curve_control_points_theta[kMaxNumAlternateImages][kMaxNumControlPoints];
};

static_assert(
    AgtmSyntax::kMaxNumAlternateImages ==
    skhdr::AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap::kMaxNumAlternateImages);
static_assert(
    AgtmSyntax::kMaxNumControlPoints ==
    skhdr::AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints);

// Parse according to Table C.1.
bool AgtmSyntax::parse_application_info(SkMemoryStream& s) {
    BitfieldReader flags;
    RETURN_ON_FALSE(flags.readFromStream(s));
    application_version = flags.readBits(3);
    minimum_application_version = flags.readBits(3);
    if (minimum_application_version > 0) {
        return false;
    }
    const auto reserved_zero = flags.readBits(2);
    RETURN_ON_FALSE(reserved_zero == 0);
    RETURN_ON_FALSE(parse_color_volume_transform(s));
    return true;
}

void AgtmSyntax::write_application_info(SkDynamicMemoryWStream& s) {
    BitfieldWriter flags;
    flags.writeBits(application_version, 3);
    flags.writeBits(minimum_application_version, 3);
    flags.padAndWriteToStream(s);

    write_color_volume_transform(s);
}

// Parse according to Table C.2.
bool AgtmSyntax::parse_color_volume_transform(SkMemoryStream& s) {
    BitfieldReader flags;
    RETURN_ON_FALSE(flags.readFromStream(s));
    has_custom_hdr_reference_white_flag = flags.readBits(1);
    has_adaptive_tone_map_flag = flags.readBits(1);
    const auto reserved_zero = flags.readBits(6);
    RETURN_ON_FALSE(reserved_zero == 0);
    if (has_custom_hdr_reference_white_flag == 1) {
        RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &hdr_reference_white));
    }
    if (has_adaptive_tone_map_flag == 1) {
        RETURN_ON_FALSE(parse_adaptive_tone_map(s));
    }
    return true;
}

void AgtmSyntax::write_color_volume_transform(SkDynamicMemoryWStream& s) {
    BitfieldWriter flags;
    flags.writeBits(has_custom_hdr_reference_white_flag, 1);
    flags.writeBits(has_adaptive_tone_map_flag, 1);
    flags.padAndWriteToStream(s);
    if (has_custom_hdr_reference_white_flag) {
        SkStreamPriv::WriteU16BE(&s, hdr_reference_white);
    }
    if (has_adaptive_tone_map_flag == 1) {
        write_adaptive_tone_map(s);
    }
}

bool AgtmSyntax::parse_adaptive_tone_map(SkMemoryStream& s) {
    RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &baseline_hdr_headroom));
    BitfieldReader flags;
    RETURN_ON_FALSE(flags.readFromStream(s) );
    use_reference_white_tone_mapping_flag = flags.readBits(1);
    if (use_reference_white_tone_mapping_flag == 0) {
        num_alternate_images = flags.readBits(3);
        gain_application_space_chromaticities_flag = flags.readBits(2);
        has_common_component_mix_params_flag = flags.readBits(1);
        has_common_curve_params_flag = flags.readBits(1);
        if (gain_application_space_chromaticities_flag == 3) {
            for (uint8_t r = 0; r < kNumChromaticityValues; ++r) {
                RETURN_ON_FALSE(
                        SkStreamPriv::ReadU16BE(&s, &gain_application_space_chromaticities[r]));
            }
        }
        for (uint8_t a = 0; a < clamp(num_alternate_images, 0u, kMaxNumAlternateImages); ++a) {
            RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &alternate_hdr_headrooms[a]));
            RETURN_ON_FALSE(parse_component_mixing(a, s));
            RETURN_ON_FALSE(parse_gain_curve(a, s));
        }
    } else {
        const auto reserved_zero = flags.readBits(7);
        RETURN_ON_FALSE(reserved_zero == 0);
    }
    return true;
}

void AgtmSyntax::write_adaptive_tone_map(SkDynamicMemoryWStream& s) {
    SkStreamPriv::WriteU16BE(&s, baseline_hdr_headroom);
    BitfieldWriter flags;
    flags.writeBits(use_reference_white_tone_mapping_flag, 1);
    if (use_reference_white_tone_mapping_flag == 0) {
        flags.writeBits(num_alternate_images, 3);
        flags.writeBits(gain_application_space_chromaticities_flag, 2);
        flags.writeBits(has_common_component_mix_params_flag, 1);
        flags.writeBits(has_common_curve_params_flag, 1);
        flags.padAndWriteToStream(s);
        if (gain_application_space_chromaticities_flag == 3) {
            for (uint8_t r = 0; r < kNumChromaticityValues; ++r) {
                SkStreamPriv::WriteU16BE(&s, gain_application_space_chromaticities[r]);
            }
        }
        for (uint8_t a = 0; a < num_alternate_images; ++a) {
            SkStreamPriv::WriteU16BE(&s, alternate_hdr_headrooms[a]);
            write_component_mixing(a, s);
            write_gain_curve(a, s);
        }
    } else {
        flags.padAndWriteToStream(s);
    }
}

bool AgtmSyntax::parse_component_mixing(uint8_t a, SkMemoryStream& s) {
    if (a == 0 || has_common_component_mix_params_flag == 0) {
        BitfieldReader flags;
        RETURN_ON_FALSE(flags.readFromStream(s));
        component_mixing_type[a] = flags.readBits(2);
        if (component_mixing_type[a] != 3) {
            const auto reserved_zero = flags.readBits(kNumMixCoefficients);
            RETURN_ON_FALSE(reserved_zero == 0);
        } else {
            for (uint8_t k = 0; k < kNumMixCoefficients; ++k) {
                has_component_mixing_coefficient_flag[a][k] = flags.readBits(1);
            }
            for (uint8_t k = 0; k < kNumMixCoefficients; ++k) {
                if (has_component_mixing_coefficient_flag[a][k] == 1) {
                    RETURN_ON_FALSE(
                            SkStreamPriv::ReadU16BE(&s, &component_mixing_coefficient[a][k]));
                } else {
                    component_mixing_coefficient[a][k] = 0;
                }
            }
        }
    } else {
        component_mixing_type[a] = component_mixing_type[0];
        if (component_mixing_type[a] == 3) {
            for (uint8_t k = 0; k < kNumMixCoefficients; ++k) {
                component_mixing_coefficient[a][k] = component_mixing_coefficient[0][k];
            }
        }
    }
    return true;
}

void AgtmSyntax::write_component_mixing(uint8_t a, SkDynamicMemoryWStream& s) {
    if (a == 0 || has_common_component_mix_params_flag == 0) {
        BitfieldWriter flags;
        flags.writeBits(component_mixing_type[a], 2);
        if (component_mixing_type[a] == 3) {
            for (uint8_t k = 0; k < kNumMixCoefficients; ++k) {
                flags.writeBits(has_component_mixing_coefficient_flag[a][k], 1);
            }
            flags.padAndWriteToStream(s);
            for (uint8_t k = 0; k < kNumMixCoefficients; ++k) {
                if (has_component_mixing_coefficient_flag[a][k] == 1) {
                    SkStreamPriv::WriteU16BE(&s, component_mixing_coefficient[a][k]);
                }
            }
        } else {
            flags.padAndWriteToStream(s);
        }
    }
}

bool AgtmSyntax::parse_gain_curve(uint8_t a, SkMemoryStream& s) {
    if (a == 0 || has_common_curve_params_flag == 0) {
        BitfieldReader flags;
        RETURN_ON_FALSE(flags.readFromStream(s));
        gain_curve_num_control_points_minus_1[a] = flags.readBits(5);
        gain_curve_use_pchip_slope_flag[a] = flags.readBits(1);
        const auto reserved_zero = flags.readBits(2);
        RETURN_ON_FALSE(reserved_zero == 0);
        for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
            RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &gain_curve_control_points_x[a][c]));
        }
    } else {
        gain_curve_num_control_points_minus_1[a] = gain_curve_num_control_points_minus_1[0];
        gain_curve_use_pchip_slope_flag[a] = gain_curve_use_pchip_slope_flag[0];
        for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
            gain_curve_control_points_x[a][c] = gain_curve_control_points_x[0][c];
        }
    }
    for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
        RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &gain_curve_control_points_y[a][c]));
    }
    if (gain_curve_use_pchip_slope_flag[a] == 0) {
        for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
            RETURN_ON_FALSE(SkStreamPriv::ReadU16BE(&s, &gain_curve_control_points_theta[a][c]));
        }
    }
    return true;
}

void AgtmSyntax::write_gain_curve(uint8_t a, SkDynamicMemoryWStream& s) {
    if (a == 0 || has_common_component_mix_params_flag == 0) {
        BitfieldWriter flags;
        flags.writeBits(gain_curve_num_control_points_minus_1[a], 5);
        flags.writeBits(gain_curve_use_pchip_slope_flag[a], 1);
        flags.padAndWriteToStream(s);
        for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
            SkStreamPriv::WriteU16BE(&s, gain_curve_control_points_x[a][c]);
        }
    }
    for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
        SkStreamPriv::WriteU16BE(&s, gain_curve_control_points_y[a][c]);
    }
    if (gain_curve_use_pchip_slope_flag[a] == 0) {
        for (uint8_t c = 0; c < gain_curve_num_control_points_minus_1[a] + 1u; ++c) {
            SkStreamPriv::WriteU16BE(&s, gain_curve_control_points_theta[a][c]);
        }
    }
}

}  // namespace

namespace skhdr {

bool AdaptiveGlobalToneMap::parse(const SkData* data) {
    if (data == nullptr) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    // Parse the syntax according to Clause C.2.
    AgtmSyntax syntax;
    memset(&syntax, 0, sizeof(syntax));
    if (!syntax.parse_application_info(s)) {
        return false;
    }

    // Apply the semantics to map syntax elements to metadata items according to Clause C.3.3.
    if (syntax.has_custom_hdr_reference_white_flag == 1) {
        fHdrReferenceWhite = uint16_to_float(syntax.hdr_reference_white, 1u, 50000u, 0u, 5.f);
    } else {
        fHdrReferenceWhite = kDefaultHdrReferenceWhite;
    }
    if (syntax.has_adaptive_tone_map_flag == 0) {
        return true;
    }
    auto& hatm = fHeadroomAdaptiveToneMap.emplace(HeadroomAdaptiveToneMap{});

    // Semantics from Clause C.3.4.
    hatm.fBaselineHdrHeadroom =
        uint16_to_float(syntax.baseline_hdr_headroom, 0u, 60000u, 0u, 10000.f);
    if (syntax.use_reference_white_tone_mapping_flag == 1) {
        AgtmHelpers::PopulateUsingRwtmo(hatm);
        return true;
    }

    const uint8_t numAlternateImages = clamp(
        syntax.num_alternate_images, 0u, HeadroomAdaptiveToneMap::kMaxNumAlternateImages);
    hatm.fAlternateImages.resize(numAlternateImages);
    for (size_t a = 0; a < numAlternateImages; ++a) {
        hatm.fAlternateImages[a].fHdrHeadroom = uint16_to_float(
            syntax.alternate_hdr_headrooms[a], 0u, 60000u, 0u, 10000.f);
    }

    // Semantics from Clause C.3.5.
    switch (syntax.gain_application_space_chromaticities_flag) {
        case 0:
            hatm.fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec709;
            break;
        case 1:
            hatm.fGainApplicationSpacePrimaries = SkNamedPrimaries::kSMPTE_EG_432_1;
            break;
        case 2:
            hatm.fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;
            break;
        case 3: {
            hatm.fGainApplicationSpacePrimaries = {
                .fRX = uint16_to_float(
                    syntax.gain_application_space_chromaticities[0], 0u, 50000u, 0u, 50000.f),
                .fRY = uint16_to_float(
                    syntax.gain_application_space_chromaticities[1], 0u, 50000u, 0u, 50000.f),
                .fGX = uint16_to_float(
                    syntax.gain_application_space_chromaticities[2], 0u, 50000u, 0u, 50000.f),
                .fGY = uint16_to_float(
                    syntax.gain_application_space_chromaticities[3], 0u, 50000u, 0u, 50000.f),
                .fBX = uint16_to_float(
                    syntax.gain_application_space_chromaticities[4], 0u, 50000u, 0u, 50000.f),
                .fBY = uint16_to_float(
                    syntax.gain_application_space_chromaticities[5], 0u, 50000u, 0u, 50000.f),
                .fWX = uint16_to_float(
                    syntax.gain_application_space_chromaticities[6], 0u, 50000u, 0u, 50000.f),
                .fWY = uint16_to_float(
                    syntax.gain_application_space_chromaticities[7], 0u, 50000u, 0u, 50000.f),
            };
            break;
        }
        default:
            // This should never be hit because syntax.gain_application_space_chromaticities_flag
            // is read as 2 bits.
            SkUNREACHABLE;
            break;
    }

    // Semantics from Clause C.3.6.
    for (uint8_t a = 0; a < numAlternateImages; ++a) {
        auto& mix = hatm.fAlternateImages[a].fColorGainFunction.fComponentMixing;
        switch (syntax.component_mixing_type[a]) {
            case 0:
                mix = {.fMax = 1.f};
                break;
            case 1:
                mix = {.fComponent = 1.f};
                break;
            case 2:
                mix = {
                    .fRed = 1.f / 6.f,
                    .fGreen = 1.f / 6.f,
                    .fBlue = 1.f / 6.f,
                    .fMax = 1.f / 2.f,
                };
                break;
            case 3: {
                float p[AgtmSyntax::kNumMixCoefficients];
                float p_sum = 0.f;
                for (uint8_t k = 0; k < AgtmSyntax::kNumMixCoefficients; ++k) {
                    p[k] = uint16_to_float(
                        syntax.component_mixing_coefficient[a][k], 0u, 50000u, 0u, 50000.f);
                    p_sum += p[k];
                }
                if (p_sum == 0.f) {
                    return false;
                }
                mix.fRed       = p[0] / p_sum;
                mix.fGreen     = p[1] / p_sum;
                mix.fBlue      = p[2] / p_sum;
                mix.fMax       = p[3] / p_sum;
                mix.fMin       = p[4] / p_sum;
                mix.fComponent = p[5] / p_sum;
                break;
            }
        }
    }

    // Semantics from Clause C.3.7.
    for (uint8_t a = 0; a < numAlternateImages; ++a) {
        auto& cubic = hatm.fAlternateImages[a].fColorGainFunction.fGainCurve;
        const uint8_t numControlPoints = syntax.gain_curve_num_control_points_minus_1[a] + 1u;
        cubic.fControlPoints.resize(numControlPoints);
        const float sign = syntax.baseline_hdr_headroom < syntax.alternate_hdr_headrooms[a] ?
                           1.f : -1.f;
        for (uint8_t c = 0; c < numControlPoints; ++c) {
            cubic.fControlPoints[c].fX = uint16_to_float(
                syntax.gain_curve_control_points_x[a][c], 0u, 64000u, 0u, 1000.f);
            cubic.fControlPoints[c].fY = sign * uint16_to_float(
                syntax.gain_curve_control_points_y[a][c], 0u, 60000u, 0, 10000.f);
        }
        if (syntax.gain_curve_use_pchip_slope_flag[a] == 0) {
            for (uint8_t c = 0; c < numControlPoints; ++c) {
                const float theta = uint16_to_float(
                    syntax.gain_curve_control_points_theta[a][c],
                    1u, 35999u, 18000u, 36000.f / SK_FloatPI);
                cubic.fControlPoints[c].fM = std::tan(theta);
            }
        } else {
            AgtmHelpers::PopulateSlopeFromPCHIP(cubic);
        }
    }

    // Reject any metadata that violates the normative constraints.
    if (!AgtmHelpers::Validate(*this)) {
        return false;
    }

    return true;
}

sk_sp<SkData> AdaptiveGlobalToneMap::serialize() const {
    // Reject requests to serialize invalid metadata.
    if (!AgtmHelpers::Validate(*this)) {
        return nullptr;
    }

    AgtmSyntax syntax;
    memset(&syntax, 0, sizeof(syntax));

    // Populate the rest of `syntax` according to the semantics in Clause C.3.
    syntax.application_version = 0;
    syntax.minimum_application_version = 0;
    syntax.has_custom_hdr_reference_white_flag = fHdrReferenceWhite != kDefaultHdrReferenceWhite;
    if (syntax.has_custom_hdr_reference_white_flag) {
        syntax.hdr_reference_white = float_to_uint16(fHdrReferenceWhite, 1u, 50000u, 0u, 5.f);
    }

    if (fHeadroomAdaptiveToneMap.has_value()) {
        const auto& hatm = fHeadroomAdaptiveToneMap.value();
        syntax.has_adaptive_tone_map_flag = true;

        // Semantics from Clause C.3.4.
        syntax.baseline_hdr_headroom =
            float_to_uint16(hatm.fBaselineHdrHeadroom, 0u, 60000u, 0u, 10000.f);

        // If the headroom-adaptive tone map is equal to RWTMO, then mark it as such.
        syntax.use_reference_white_tone_mapping_flag = false;
        {
            auto hatm_rwtmo = hatm;
            AgtmHelpers::PopulateUsingRwtmo(hatm_rwtmo);
            syntax.use_reference_white_tone_mapping_flag = hatm == hatm_rwtmo;
        }
        SkASSERT(hatm.fAlternateImages.size() <= HeadroomAdaptiveToneMap::kMaxNumAlternateImages);
        syntax.num_alternate_images = hatm.fAlternateImages.size();
        for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
            syntax.alternate_hdr_headrooms[a] = float_to_uint16(
                hatm.fAlternateImages[a].fHdrHeadroom, 0u, 60000u, 0u, 10000.f);
        }

        // Semantics from Clause C.3.5.
        if (hatm.fGainApplicationSpacePrimaries == SkNamedPrimaries::kRec709) {
            syntax.gain_application_space_chromaticities_flag = 0;
        } else if (hatm.fGainApplicationSpacePrimaries == SkNamedPrimaries::kSMPTE_EG_432_1) {
            syntax.gain_application_space_chromaticities_flag = 1;
        } else if (hatm.fGainApplicationSpacePrimaries == SkNamedPrimaries::kRec2020) {
            syntax.gain_application_space_chromaticities_flag = 2;
        } else {
            syntax.gain_application_space_chromaticities_flag = 3;
        }
        if (syntax.gain_application_space_chromaticities_flag == 3) {
            syntax.gain_application_space_chromaticities[0] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fRX, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[1] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fRY, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[2] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fGX, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[3] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fGY, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[4] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fBX, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[5] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fBY, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[6] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fWX, 0u, 50000u, 0u, 50000.f);
            syntax.gain_application_space_chromaticities[7] = float_to_uint16(
                hatm.fGainApplicationSpacePrimaries.fWY, 0u, 50000u, 0u, 50000.f);
        }

        // Semantics from Clause C.3.6.
        syntax.has_common_component_mix_params_flag = 0;
        for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
            auto& mix = hatm.fAlternateImages[a].fColorGainFunction.fComponentMixing;
            if (mix.fRed == 0.f && mix.fGreen == 0.f && mix.fBlue == 0.f &&
                mix.fMax == 1.f && mix.fMin == 0.f && mix.fComponent == 0.f) {
                syntax.component_mixing_type[a] = 0;
            } else if (mix.fRed == 0.f && mix.fGreen == 0.f && mix.fBlue == 0.f &&
                       mix.fMax == 0.f && mix.fMin == 0.f && mix.fComponent == 1.f) {
                syntax.component_mixing_type[a] = 1;
            } else if (mix.fRed == 1.f/6.f && mix.fGreen == 1.f/6.f && mix.fBlue == 1.f/6.f &&
                       mix.fMax == 1.f/2.f && mix.fMin == 0.f && mix.fComponent == 0.f) {
                syntax.component_mixing_type[a] = 2;
            } else {
                syntax.component_mixing_type[a] = 3;
                syntax.component_mixing_coefficient[a][0] =
                    float_to_uint16(mix.fRed, 0u, 50000u, 0u, 50000.f);
                syntax.component_mixing_coefficient[a][1] =
                    float_to_uint16(mix.fGreen, 0u, 50000u, 0u, 50000.f);
                syntax.component_mixing_coefficient[a][2] =
                    float_to_uint16(mix.fBlue, 0u, 50000u, 0u, 50000.f);
                syntax.component_mixing_coefficient[a][3] =
                    float_to_uint16(mix.fMax, 0u, 50000u, 0u, 50000.f);
                syntax.component_mixing_coefficient[a][4] =
                    float_to_uint16(mix.fMin, 0u, 50000u, 0u, 50000.f);
                syntax.component_mixing_coefficient[a][5] =
                    float_to_uint16(mix.fComponent, 0u, 50000u, 0u, 50000.f);
                for (uint8_t k = 0; k < AgtmSyntax::kNumMixCoefficients; ++k) {
                    syntax.has_component_mixing_coefficient_flag[a][k] =
                        syntax.component_mixing_coefficient[a][k] != 0;
                }
            }
        }

        // Semantics from Clause C.3.7.
        syntax.has_common_curve_params_flag = 0;
        for (size_t a = 0; a < hatm.fAlternateImages.size(); ++a) {
            const auto& alt = hatm.fAlternateImages[a];
            const float sign = hatm.fBaselineHdrHeadroom < alt.fHdrHeadroom ? 1.f : -1.f;

            const auto& cubic = alt.fColorGainFunction.fGainCurve;
            SkASSERT(GainCurve::kMinNumControlPoints <= cubic.fControlPoints.size());
            SkASSERT(cubic.fControlPoints.size() <= GainCurve::kMaxNumControlPoints);
            syntax.gain_curve_num_control_points_minus_1[a] = cubic.fControlPoints.size() - 1u;
            // TODO(https://crbug.com/395659818): Identify when slope is equal to PCHIP to further
            // compress the serialization.
            syntax.gain_curve_use_pchip_slope_flag[a] = 0;
            for (size_t c = 0; c < cubic.fControlPoints.size(); ++c) {
                syntax.gain_curve_control_points_x[a][c] =
                    float_to_uint16(cubic.fControlPoints[c].fX, 0u, 64000u, 0u, 1000.f);
            }
            for (size_t c = 0; c < cubic.fControlPoints.size(); ++c) {
                syntax.gain_curve_control_points_y[a][c] =
                    float_to_uint16(sign * cubic.fControlPoints[c].fY, 0u, 60000u, 0u, 10000.f);
            }
            for (size_t c = 0; c < cubic.fControlPoints.size(); ++c) {
                float theta = std::atan(cubic.fControlPoints[c].fM);
                syntax.gain_curve_control_points_theta[a][c] =
                    float_to_uint16(theta, 1u, 35999u, 18000u, 36000.f / SK_FloatPI);
            }
        }
    }

    // Write the syntax according to Clause C.2.
    SkDynamicMemoryWStream s;
    syntax.write_application_info(s);
    return s.detachAsData();
}

}  // namespace skhdr

