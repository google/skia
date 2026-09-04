/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkICC.h"
#include "include/private/SkHdrMetadata.h"
#include "modules/skcms/skcms.h"
#include "rust/icc/FFI.h"
#include "rust/icc/FFI.rs.h"
#include "src/codec/SkCodecColorProfileRust.h"
#include "src/codec/SkCodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <array>
#include <cmath>
#include <cstring>

// Helper to compare skcms_Matrix3x3 against expected rust_icc::Matrix3x3
static void assert_matrix_eq(skiatest::Reporter* r,
                              const skcms_Matrix3x3& skcms_matrix,
                              const rust_icc::Matrix3x3& expected,
                              float tolerance = 0.0001f) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            REPORTER_ASSERT(r,
                            fabsf(skcms_matrix.vals[row][col] -
                                  expected.vals[row][col]) < tolerance);
        }
    }
}

// Helper to compare skcms_TransferFunction against expected rust_icc::TransferFunction
static void assert_transfer_function_eq(skiatest::Reporter* r,
                                        const skcms_TransferFunction& skcms_tf,
                                        const rust_icc::TransferFunction& expected,
                                        float tolerance = 0.0001f) {
    REPORTER_ASSERT(r, fabsf(skcms_tf.g - expected.g) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.a - expected.a) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.b - expected.b) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.c - expected.c) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.d - expected.d) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.e - expected.e) < tolerance);
    REPORTER_ASSERT(r, fabsf(skcms_tf.f - expected.f) < tolerance);
}

// Helper to compare two skcms_TransferFunction objects
static void compare_transfer_functions(skiatest::Reporter* r,
                                       const char* path,
                                       int channel,
                                       const skcms_TransferFunction& rust_tf,
                                       const skcms_TransferFunction& skcms_tf,
                                       float tolerance = 0.0001f) {
    if (fabsf(rust_tf.g - skcms_tf.g) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.g mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.g, skcms_tf.g);
    }
    if (fabsf(rust_tf.a - skcms_tf.a) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.a mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.a, skcms_tf.a);
    }
    if (fabsf(rust_tf.b - skcms_tf.b) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.b mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.b, skcms_tf.b);
    }
    if (fabsf(rust_tf.c - skcms_tf.c) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.c mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.c, skcms_tf.c);
    }
    if (fabsf(rust_tf.d - skcms_tf.d) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.d mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.d, skcms_tf.d);
    }
    if (fabsf(rust_tf.e - skcms_tf.e) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.e mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.e, skcms_tf.e);
    }
    if (fabsf(rust_tf.f - skcms_tf.f) > tolerance) {
        ERRORF(r, "[%s] trc[%d].parametric.f mismatch: rust=%f, skcms=%f",
               path, channel, rust_tf.f, skcms_tf.f);
    }
}


DEF_TEST(RustIcc_matrix_conversion, r) {
    // Create a rust_icc::Matrix3x3
    rust_icc::Matrix3x3 rust_matrix;
    rust_matrix.vals[0][0] = 0.4124f;
    rust_matrix.vals[0][1] = 0.3576f;
    rust_matrix.vals[0][2] = 0.1805f;
    rust_matrix.vals[1][0] = 0.2126f;
    rust_matrix.vals[1][1] = 0.7152f;
    rust_matrix.vals[1][2] = 0.0722f;
    rust_matrix.vals[2][0] = 0.0193f;
    rust_matrix.vals[2][1] = 0.1192f;
    rust_matrix.vals[2][2] = 0.9505f;

    // Convert to skcms
    skcms_Matrix3x3 skcms_matrix;
    rust_icc::ToSkcmsMatrix3x3(rust_matrix, &skcms_matrix);

    // Verify binary compatibility - values should match exactly
    assert_matrix_eq(r, skcms_matrix, rust_matrix);
}

DEF_TEST(RustIcc_transfer_function_conversion, r) {
    // Create a rust_icc::TransferFunction (sRGB parameters)
    rust_icc::TransferFunction rust_tf;
    rust_tf.g = 2.4f;
    rust_tf.a = 1.0f / 1.055f;
    rust_tf.b = 0.055f / 1.055f;
    rust_tf.c = 1.0f / 12.92f;
    rust_tf.d = 0.04045f;
    rust_tf.e = 0.0f;
    rust_tf.f = 1.0f;

    // Convert to skcms
    skcms_TransferFunction skcms_tf;
    rust_icc::ToSkcmsTransferFunction(rust_tf, &skcms_tf);

    // Verify binary compatibility
    assert_transfer_function_eq(r, skcms_tf, rust_tf);
}

DEF_TEST(RustIcc_profile_conversion, r) {
    // Create a minimal rust_icc::IccProfile
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up a simple identity matrix
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[0][1] = 0.0f;
    rust_profile.to_xyzd50.vals[0][2] = 0.0f;
    rust_profile.to_xyzd50.vals[1][0] = 0.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[1][2] = 0.0f;
    rust_profile.to_xyzd50.vals[2][0] = 0.0f;
    rust_profile.to_xyzd50.vals[2][1] = 0.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    // Set up simple gamma curves
    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;  // parametric curve (no table)
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
        ch.parametric.b = 0.0f;
        ch.parametric.c = 0.0f;
        ch.parametric.d = 0.0f;
        ch.parametric.e = 0.0f;
        ch.parametric.f = 0.0f;
    }

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_toXYZD50);
    REPORTER_ASSERT(r, skcms_profile.has_trc);
    REPORTER_ASSERT(r, !skcms_profile.has_CICP);
    REPORTER_ASSERT(r, !skcms_profile.has_HAGC);
    REPORTER_ASSERT(r, !skcms_profile.has_A2B);

    // Verify matrix
    REPORTER_ASSERT(r, skcms_profile.toXYZD50.vals[0][0] == 1.0f);
    REPORTER_ASSERT(r, skcms_profile.toXYZD50.vals[1][1] == 1.0f);
    REPORTER_ASSERT(r, skcms_profile.toXYZD50.vals[2][2] == 1.0f);

    // Verify TRC curves
    REPORTER_ASSERT(r, skcms_profile.trc[0].table_entries == 0);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[0].parametric.g - 2.2f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[1].parametric.g - 2.2f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[2].parametric.g - 2.2f) < 0.0001f);

    // Verify null pointer handling
    REPORTER_ASSERT(r, !rust_icc::ToSkcmsIccProfile(rust_profile, nullptr));

    // Verify false return when disabled
    rust_profile.has_to_xyzd50 = false;
    rust_profile.has_trc = false;
    success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);
    REPORTER_ASSERT(r, !success);
}

DEF_TEST(RustIcc_profile_conversion_fails_without_data, r) {
    // Profile without matrix or TRC should fail conversion
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;
    rust_profile.has_to_xyzd50 = false;
    rust_profile.has_trc = false;

    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, !success);
}

DEF_TEST(RustIcc_profile_conversion_fails_trc_without_matrix, r) {
    // Profile with TRC curves but no toXYZD50 matrix should fail conversion,
    // matching skcms_Parse's usable_as_src() validation. Without both TRC and
    // toXYZD50, skcms_Transform would return false.
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;
    rust_profile.has_to_xyzd50 = false;
    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
    }

    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, !success);
}

DEF_TEST(RustIcc_profile_conversion_fails_matrix_without_trc, r) {
    // Profile with toXYZD50 matrix but no TRC curves (and no A2B) should fail
    // conversion, matching skcms_Parse's usable_as_src() validation.
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;
    rust_profile.has_trc = false;

    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, !success);
}

DEF_TEST(RustIcc_cicp_conversion, r) {
    // Create a profile with CICP data
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up minimal matrix to make conversion succeed
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    // Set up TRC curves (required alongside toXYZD50 for usable_as_src)
    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
    }

    // Set CICP data (e.g., BT.709 primaries, BT.709 transfer, BT.709 matrix, full range)
    rust_profile.has_cicp = true;
    rust_profile.cicp.color_primaries = 1;  // BT.709
    rust_profile.cicp.transfer_characteristics = 1;  // BT.709
    rust_profile.cicp.matrix_coefficients = 1;  // BT.709
    rust_profile.cicp.video_full_range_flag = 1;  // Full range

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_CICP);
    REPORTER_ASSERT(r, skcms_profile.CICP.color_primaries == 1);
    REPORTER_ASSERT(r, skcms_profile.CICP.transfer_characteristics == 1);
    REPORTER_ASSERT(r, skcms_profile.CICP.matrix_coefficients == 1);
    REPORTER_ASSERT(r, skcms_profile.CICP.video_full_range_flag == 1);
}

DEF_TEST(RustIcc_a2b_b2a_flags, r) {
    // Create a minimal profile to test A2B/B2A flags
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up minimal matrix
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    // Simulate a profile with A2B but no B2A transforms
    rust_profile.has_a2b = true;
    rust_profile.has_b2a = false;

    // A2B requires exactly 3 output channels with matching curves.
    rust_profile.a2b.output_channels = 3;
    rust_icc::Curve id_curve;
    id_curve.table_entries = 0;
    id_curve.parametric = {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 3; i++) {
        rust_profile.a2b.output_curves.push_back(id_curve);
    }

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_A2B);
    REPORTER_ASSERT(r, !skcms_profile.has_B2A);
}

DEF_TEST(RustIcc_profile_with_a2b_curves, r) {
    // Create a profile with A2B transform including curves
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up basic matrix/TRC for compatibility
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;  // parametric curve (no table)
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
        ch.parametric.b = 0.0f;
        ch.parametric.c = 0.0f;
        ch.parametric.d = 0.0f;
        ch.parametric.e = 0.0f;
        ch.parametric.f = 0.0f;
    }

    // Add A2B transform with input curves
    rust_profile.has_a2b = true;
    rust_profile.a2b.input_channels = 3;
    rust_profile.a2b.output_channels = 3;

    // Set up 3 input curves (RGB) - parametric with table_entries = 0
    rust::Vec<rust_icc::Curve> input_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;
        curve.table_entries = 0;  // 0 = parametric
        curve.parametric.g = 2.4f;
        curve.parametric.a = 1.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;
        input_curves.push_back(std::move(curve));
    }
    rust_profile.a2b.input_curves = std::move(input_curves);

    // Simple 2x2x2 grid (3 input channels, 3 output channels)
    rust::Vec<uint8_t> grid_data;
    // 8 grid points × 3 output channels = 24 values
    for (int i = 0; i < 24; i++) {
        grid_data.push_back(static_cast<uint8_t>(i * 10));
    }
    rust_profile.a2b.grid_data = std::move(grid_data);
    rust_profile.a2b.grid_points[0] = 2;
    rust_profile.a2b.grid_points[1] = 2;
    rust_profile.a2b.grid_points[2] = 2;

    // Set up 3 output curves
    rust::Vec<rust_icc::Curve> output_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;
        curve.table_entries = 0;  // 0 = parametric
        curve.parametric.g = 1.0f;
        curve.parametric.a = 1.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;
        output_curves.push_back(std::move(curve));
    }
    rust_profile.a2b.output_curves = std::move(output_curves);

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_A2B);
    REPORTER_ASSERT(r, skcms_profile.A2B.input_channels == 3);
    REPORTER_ASSERT(r, skcms_profile.A2B.output_channels == 3);

    // Verify input curves were set up
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.input_curves[0].parametric.g - 2.4f) < 0.0001f);

    // Verify grid
    REPORTER_ASSERT(r, skcms_profile.A2B.grid_points[0] == 2);
    REPORTER_ASSERT(r, skcms_profile.A2B.grid_points[1] == 2);
    REPORTER_ASSERT(r, skcms_profile.A2B.grid_points[2] == 2);
    REPORTER_ASSERT(r, skcms_profile.A2B.grid_8 != nullptr);

    // Verify output curves
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.output_curves[0].parametric.g - 1.0f) < 0.0001f);
}

DEF_TEST(RustIcc_profile_with_a2b_matrix, r) {
    // Create a profile with A2B transform including matrix
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up basic matrix/TRC
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;  // parametric curve (no table)
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
        ch.parametric.b = 0.0f;
        ch.parametric.c = 0.0f;
        ch.parametric.d = 0.0f;
        ch.parametric.e = 0.0f;
        ch.parametric.f = 0.0f;
    }

    // Add A2B transform with matrix
    rust_profile.has_a2b = true;
    rust_profile.a2b.input_channels = 3;
    rust_profile.a2b.output_channels = 3;

    // Set up matrix curves
    rust::Vec<rust_icc::Curve> matrix_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;
        curve.table_entries = 0;  // 0 = parametric
        curve.parametric.g = 1.8f;
        curve.parametric.a = 1.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;
        matrix_curves.push_back(std::move(curve));
    }
    rust_profile.a2b.matrix_curves = std::move(matrix_curves);

    // Set up a color conversion matrix
    rust_profile.a2b.matrix.vals[0][0] = 0.4124f;
    rust_profile.a2b.matrix.vals[0][1] = 0.3576f;
    rust_profile.a2b.matrix.vals[0][2] = 0.1805f;
    rust_profile.a2b.matrix.vals[1][0] = 0.2126f;
    rust_profile.a2b.matrix.vals[1][1] = 0.7152f;
    rust_profile.a2b.matrix.vals[1][2] = 0.0722f;
    rust_profile.a2b.matrix.vals[2][0] = 0.0193f;
    rust_profile.a2b.matrix.vals[2][1] = 0.1192f;
    rust_profile.a2b.matrix.vals[2][2] = 0.9505f;

    // Set up minimal grid (2x2x2 = 8 points, 1 byte per output = 8 bytes)
    rust::Vec<uint8_t> grid_data;
    for (int i = 0; i < 8; i++) {
        grid_data.push_back(0x80);
    }
    rust_profile.a2b.grid_data = std::move(grid_data);
    rust_profile.a2b.grid_points[0] = 2;
    rust_profile.a2b.grid_points[1] = 2;
    rust_profile.a2b.grid_points[2] = 2;

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_A2B);

    // Verify matrix curves
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix_curves[0].parametric.g - 1.8f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix_curves[1].parametric.g - 1.8f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix_curves[2].parametric.g - 1.8f) < 0.0001f);

    // Verify matrix values
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix.vals[0][0] - 0.4124f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix.vals[1][1] - 0.7152f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.A2B.matrix.vals[2][2] - 0.9505f) < 0.0001f);
}

// Test that verifies table-based curves work through the FFI layer.
// NOTE: The rust_profile must remain alive while skcms_profile is in use because
// skcms_profile contains pointers into rust_profile's table_data vectors.
DEF_TEST(RustIcc_profile_with_table_curves, r) {
    // Create a profile with A2B transform using table-based curves
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up basic matrix/TRC
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;  // parametric curve (no table)
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
        ch.parametric.b = 0.0f;
        ch.parametric.c = 0.0f;
        ch.parametric.d = 0.0f;
        ch.parametric.e = 0.0f;
        ch.parametric.f = 0.0f;
    }

    // Add A2B transform with table-based curves
    rust_profile.has_a2b = true;
    rust_profile.a2b.input_channels = 3;
    rust_profile.a2b.output_channels = 3;

    // Set up input curves with table data
    // Table data stores u16 values as little-endian bytes
    rust::Vec<rust_icc::Curve> input_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;

        // Create a simple linear table with 3 u16 entries
        rust::Vec<uint8_t> table_data;
        // Entry 0: 0x0000 (0.0)
        table_data.push_back(0x00);
        table_data.push_back(0x00);
        // Entry 1: 0x8000 (0.5)
        table_data.push_back(0x00);
        table_data.push_back(0x80);
        // Entry 2: 0xFFFF (1.0)
        table_data.push_back(0xFF);
        table_data.push_back(0xFF);

        curve.table_entries = 3;
        curve.table_data = std::move(table_data);
        curve.table_format = rust_icc::TableFormat::U16;

        // Parametric fields unused for table curves
        curve.parametric.g = 0.0f;
        curve.parametric.a = 0.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;

        input_curves.push_back(std::move(curve));
    }
    rust_profile.a2b.input_curves = std::move(input_curves);

    // Minimal grid (2x2x2 = 8 points, 3 output channels = 24 bytes)
    rust::Vec<uint8_t> grid_data;
    for (int i = 0; i < 24; i++) {
        grid_data.push_back(0x80);
    }
    rust_profile.a2b.grid_data = std::move(grid_data);
    rust_profile.a2b.grid_points[0] = 2;
    rust_profile.a2b.grid_points[1] = 2;
    rust_profile.a2b.grid_points[2] = 2;
    rust_profile.a2b.grid_format = rust_icc::TableFormat::U8;

    // Convert to skcms (rust_profile must remain alive after this!)
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_A2B);
    REPORTER_ASSERT(r, skcms_profile.A2B.input_channels == 3);

    // Verify table curves were set up correctly
    // FFI.cpp sets table_16 (which points to u16 data stored as bytes)
    REPORTER_ASSERT(r, skcms_profile.A2B.input_curves[0].table_entries == 3);
    REPORTER_ASSERT(r, skcms_profile.A2B.input_curves[0].table_8 == nullptr);
    REPORTER_ASSERT(r, skcms_profile.A2B.input_curves[0].table_16 != nullptr);

    // Verify the table data values (interpret as u16 little-endian)
    const uint8_t* data = skcms_profile.A2B.input_curves[0].table_16;
    if (data) {
        uint16_t val0 = data[0] | (data[1] << 8);
        uint16_t val1 = data[2] | (data[3] << 8);
        uint16_t val2 = data[4] | (data[5] << 8);

        REPORTER_ASSERT(r, val0 == 0x0000);
        REPORTER_ASSERT(r, val1 == 0x8000);
        REPORTER_ASSERT(r, val2 == 0xFFFF);
    }

    // rust_profile goes out of scope here, invalidating pointers in skcms_profile
}

// Regression tests for crbug.com/504160794 and crbug.com/504103236:
// ToSkcmsIccProfile must reject A2B structs with out-of-range channel counts
// or zero in an active grid_points dimension, or too many total grid points.
DEF_TEST(RustIcc_reject_malformed_a2b, r) {
    // Identity output curve reused by every case.
    rust_icc::Curve id_curve;
    id_curve.table_entries = 0;
    id_curve.table_format = rust_icc::TableFormat::U8;
    id_curve.parametric = {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    struct Case {
        const char* label;
        uint32_t    input_channels;
        uint32_t    output_channels;
        std::array<uint8_t, 4> grid_points;
        bool        has_grid;     // whether to attach CLUT data
    };

    const Case cases[] = {
        // b/504160794 – channel count > 4
        {"input_channels=64", 64, 3, {0,0,0,0}, false},
        {"input_channels=5",   5, 3, {0,0,0,0}, false},
        {"output_channels=5",  3, 5, {0,0,0,0}, false},
        // b/504103236 – zero in active grid dimension
        {"grid_points[1]=0",   2, 3, {2,0,0,0}, true},
        // b/513702971 – total CLUT grid points exceed skcms SIMD index limits
        {"grid_points overflow", 4, 3, {255,255,255,255}, true},
        // b/506010945 – output_channels != 3 causes OOB in clut()
        {"output_channels=1",  1, 1, {2,0,0,0}, true},
        {"output_channels=2",  3, 2, {0,0,0,0}, false},
        {"output_channels=4",  3, 4, {0,0,0,0}, false},
    };

    for (const auto& tc : cases) {
        rust_icc::IccProfile prof;
        prof.data_color_space = skcms_Signature_RGB;
        prof.connection_space  = skcms_Signature_XYZ;
        prof.has_a2b = true;
        prof.a2b.input_channels  = tc.input_channels;
        prof.a2b.output_channels = tc.output_channels;
        prof.a2b.grid_points = tc.grid_points;

        // Output curves (always 3 for PCS)
        rust::Vec<rust_icc::Curve> out;
        for (int i = 0; i < 3; i++) out.push_back(id_curve);
        prof.a2b.output_curves = std::move(out);

        if (tc.has_grid) {
            rust::Vec<uint8_t> grid;
            for (int i = 0; i < 3; i++) grid.push_back(0x80);
            prof.a2b.grid_data = std::move(grid);
            prof.a2b.grid_format = rust_icc::TableFormat::U8;

            rust::Vec<rust_icc::Curve> in_c;
            for (uint32_t i = 0; i < tc.input_channels && i < 4; i++) {
                in_c.push_back(id_curve);
            }
            prof.a2b.input_curves = std::move(in_c);
        }

        skcms_ICCProfile skcms_profile;
        bool ok = rust_icc::ToSkcmsIccProfile(prof, &skcms_profile);
        REPORTER_ASSERT(r, !ok, "Expected rejection for: %s", tc.label);
    }
}

DEF_TEST(RustIcc_profile_with_b2a, r) {
    // Create a profile with B2A transform (inverse)
    rust_icc::IccProfile rust_profile;
    rust_profile.data_color_space = skcms_Signature_RGB;
    rust_profile.connection_space = skcms_Signature_XYZ;

    // Set up basic matrix/TRC
    rust_profile.has_to_xyzd50 = true;
    rust_profile.to_xyzd50.vals[0][0] = 1.0f;
    rust_profile.to_xyzd50.vals[1][1] = 1.0f;
    rust_profile.to_xyzd50.vals[2][2] = 1.0f;

    rust_profile.has_trc = true;
    for (auto& ch : rust_profile.trc) {
        ch.table_entries = 0;  // parametric curve (no table)
        ch.parametric.g = 2.2f;
        ch.parametric.a = 1.0f;
        ch.parametric.b = 0.0f;
        ch.parametric.c = 0.0f;
        ch.parametric.d = 0.0f;
        ch.parametric.e = 0.0f;
        ch.parametric.f = 0.0f;
    }

    // Add B2A transform
    rust_profile.has_b2a = true;
    rust_profile.b2a.input_channels = 3;
    rust_profile.b2a.output_channels = 3;

    // Set up input curves with inverse gamma
    rust::Vec<rust_icc::Curve> input_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;
        curve.table_entries = 0;  // 0 = parametric
        curve.parametric.g = 1.0f / 2.2f;  // Inverse gamma
        curve.parametric.a = 1.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;
        input_curves.push_back(std::move(curve));
    }
    rust_profile.b2a.input_curves = std::move(input_curves);

    // Set up a 2x2x2 grid
    rust::Vec<uint8_t> grid_data;
    for (int i = 0; i < 24; i++) {
        grid_data.push_back(static_cast<uint8_t>(255 - i * 10));
    }
    rust_profile.b2a.grid_data = std::move(grid_data);
    rust_profile.b2a.grid_points[0] = 2;
    rust_profile.b2a.grid_points[1] = 2;
    rust_profile.b2a.grid_points[2] = 2;

    // Set up output curves
    rust::Vec<rust_icc::Curve> output_curves;
    for (int i = 0; i < 3; i++) {
        rust_icc::Curve curve;
        curve.table_entries = 0;  // 0 = parametric
        curve.parametric.g = 1.0f;
        curve.parametric.a = 1.0f;
        curve.parametric.b = 0.0f;
        curve.parametric.c = 0.0f;
        curve.parametric.d = 0.0f;
        curve.parametric.e = 0.0f;
        curve.parametric.f = 0.0f;
        output_curves.push_back(std::move(curve));
    }
    rust_profile.b2a.output_curves = std::move(output_curves);

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_B2A);
    REPORTER_ASSERT(r, skcms_profile.B2A.input_channels == 3);
    REPORTER_ASSERT(r, skcms_profile.B2A.output_channels == 3);

    // Verify input curves (inverse gamma)
    REPORTER_ASSERT(r,
                    fabsf(skcms_profile.B2A.input_curves[0].parametric.g -
                          1.0f / 2.2f) < 0.0001f);

    // Verify grid
    REPORTER_ASSERT(r, skcms_profile.B2A.grid_points[0] == 2);
    REPORTER_ASSERT(r, skcms_profile.B2A.grid_8 != nullptr);
    REPORTER_ASSERT(r, skcms_profile.B2A.grid_8[0] == 255);

    // Verify output curves
    REPORTER_ASSERT(r, fabsf(skcms_profile.B2A.output_curves[0].parametric.g - 1.0f) < 0.0001f);
}

// End-to-end: Rust-parsed ICC profiles with 3-channel CLUTs through skcms_Transform.
// Validates that the suffix padding added by convert_grid_data() in FFI.rs prevents
// the skcms gather over-read (b/498869813) from causing ASAN failures.
DEF_TEST(RustIcc_clut_transform_exercises_gather_overread, r) {
    skcms_DisableRuntimeCPUDetection();
    const char* profiles_with_cluts[] = {
        "icc_profiles/apng19.icc",
        "icc_profiles/srgb_lab_pcs.icc",
        "icc_profiles/upperRight.icc",
    };

    for (const char* path : profiles_with_cluts) {
        auto data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Failed to load: %s", path);
            continue;
        }

        // Parse through the Rust FFI path.
        auto profile = SkCodecs::MakeICCProfileWithRust(data);
        if (!profile) {
            ERRORF(r, "Rust parser failed for: %s", path);
            continue;
        }

        const skcms_ICCProfile* src = profile->profile();
        if (!src->has_A2B) {
            ERRORF(r, "[%s] Expected has_A2B but got false", path);
            continue;
        }

        // Transform pixels to exercise the CLUT gather path.
        uint8_t src_pixels[] = {0, 0, 0,  128, 128, 128,  255, 255, 255};
        uint8_t dst_pixels[9] = {};
        bool transformed = skcms_Transform(
                src_pixels, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Opaque, src,
                dst_pixels, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Opaque,
                skcms_sRGB_profile(),
                3);
        if (!transformed) {
            ERRORF(r, "[%s] skcms_Transform failed", path);
        }
    }
}

// Helper to compare two skcms_Curve objects by evaluating them at sample points.
// Works for both parametric and table-based curves. For table curves, reads the
// raw big-endian bytes the same way skcms_Transform would.
// Evaluate a skcms_Curve at a normalised x in [0, 1].
// Handles both parametric (table_entries == 0) and table-based curves
// (table_16 stores big-endian uint16_t pairs).
static float eval_skcms_curve(const skcms_Curve& c, float x) {
    if (c.table_entries == 0) {
        return skcms_TransferFunction_eval(&c.parametric, x);
    }
    x = x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
    float idx = x * (float)(c.table_entries - 1);
    int lo = (int)idx;
    int hi = lo + 1 < (int)c.table_entries ? lo + 1 : lo;
    float frac = idx - (float)lo;
    auto read_val = [&](int i) -> float {
        if (c.table_16) {
            uint16_t v = (uint16_t)((c.table_16[2*i] << 8) | c.table_16[2*i+1]);
            return (float)v / 65535.0f;
        } else if (c.table_8) {
            return (float)c.table_8[i] / 255.0f;
        }
        return 0.0f;
    };
    return read_val(lo) * (1.0f - frac) + read_val(hi) * frac;
}

static bool is_identity_parametric(const skcms_Curve& c) {
    return c.table_entries == 0 &&
           c.parametric.g == 1.0f &&
           c.parametric.a == 1.0f &&
           c.parametric.b == 0.0f &&
           c.parametric.c == 0.0f &&
           c.parametric.d == 0.0f &&
           c.parametric.e == 0.0f &&
           c.parametric.f == 0.0f;
}

static void compare_curves_by_evaluation(
        skiatest::Reporter* r,
        const char* path,
        const char* stage,
        int channel,
        const skcms_Curve& rust_curve,
        const skcms_Curve& skcms_curve) {
    // If both are parametric, compare the transfer function parameters directly.
    if (rust_curve.table_entries == 0 && skcms_curve.table_entries == 0) {
        compare_transfer_functions(r, path, channel,
                                  rust_curve.parametric, skcms_curve.parametric);
        return;
    }

    // If both are table-based with the same number of entries, compare table values directly.
    if (rust_curve.table_entries != 0 && rust_curve.table_entries == skcms_curve.table_entries) {
        const uint32_t n = rust_curve.table_entries;
        for (uint32_t i = 0; i < n; ++i) {
            uint16_t rv =
                    rust_curve.table_16
                            ? (uint16_t)(rust_curve.table_16[2 * i] << 8 |
                                         rust_curve.table_16[2 * i + 1])
                            : (uint16_t)(rust_curve.table_8[i] << 8 | rust_curve.table_8[i]);
            uint16_t sv =
                    skcms_curve.table_16
                            ? (uint16_t)(skcms_curve.table_16[2 * i] << 8 |
                                         skcms_curve.table_16[2 * i + 1])
                            : (uint16_t)(skcms_curve.table_8[i] << 8 | skcms_curve.table_8[i]);
            if (rv != sv) {
                ERRORF(r, "[%s] %s[%d] table entry %u mismatch: rust=%u, skcms=%u",
                       path, stage, channel, i, rv, sv);
                return;
            }
        }
        return;
    }

    // If skcms converted an identity table to an identity parametric curve via
    // canonicalize_identity() while Rust retained the table, compare by evaluation.
    // canonicalize_identity() will be removed from skcms, so this fallback is temporary.
    if (is_identity_parametric(skcms_curve) && rust_curve.table_entries != 0) {
        static const float kSamples[] = {0.0f, 0.05f, 0.1f, 0.25f,
                                         0.5f, 0.75f, 0.9f, 1.0f};
        for (float x : kSamples) {
            float rv = eval_skcms_curve(rust_curve,  x);
            float sv = eval_skcms_curve(skcms_curve, x);
            if (fabsf(rv - sv) > 0.01f) {
                ERRORF(r, "[%s] %s[%d] curve eval mismatch at x=%.2f: rust=%f, skcms=%f",
                       path, stage, channel, x, rv, sv);
                return;
            }
        }
        return;
    }

    // For any other difference in curve structure/entries, report mismatch.
    ERRORF(r, "[%s] %s[%d] table_entries mismatch: rust=%u, skcms=%u",
           path, stage, channel, rust_curve.table_entries, skcms_curve.table_entries);
}

// Helper to compare A2B grid data (CLUT) between rust and skcms parsed profiles.
static void compare_a2b_grid_data(
        skiatest::Reporter* r,
        const char* path,
        const skcms_A2B& rust_a2b,
        const skcms_A2B& skcms_a2b) {
    // Compute grid size
    uint64_t grid_size = rust_a2b.output_channels;
    for (uint32_t i = 0; i < rust_a2b.input_channels; ++i) {
        grid_size *= rust_a2b.grid_points[i];
    }
    if (grid_size == 0) return;

    if (!rust_a2b.grid_8 && !rust_a2b.grid_16 && !skcms_a2b.grid_8 && !skcms_a2b.grid_16) {
        // Both parsers found no CLUT - consistent, no error.
        return;
    }
    if (rust_a2b.grid_16 && skcms_a2b.grid_16) {
        // 16-bit grid: compare big-endian bytes
        for (uint64_t i = 0; i < grid_size; ++i) {
            uint16_t rv = (uint16_t)(rust_a2b.grid_16[2*i] << 8 |
                                     rust_a2b.grid_16[2*i+1]);
            uint16_t sv = (uint16_t)(skcms_a2b.grid_16[2*i] << 8 |
                                     skcms_a2b.grid_16[2*i+1]);
            if (rv != sv) {
                ERRORF(r, "[%s] A2B grid_16 entry %llu mismatch: rust=%u, skcms=%u",
                       path, (unsigned long long)i, rv, sv);
                return;
            }
        }
    } else if (rust_a2b.grid_8 && skcms_a2b.grid_8) {
        // 8-bit grid
        if (memcmp(rust_a2b.grid_8, skcms_a2b.grid_8, grid_size) != 0) {
            ERRORF(r, "[%s] A2B grid_8 data mismatch", path);
        }
    } else {
        // One parser has grid data, the other doesn't, or they use mismatched formats.
        ERRORF(r, "[%s] A2B grid format mismatch (8 vs 16 bit)", path);
    }
}

DEF_TEST(RustIcc_equivalence_with_skcms_resource_files, r) {
    // List of ICC profile files in resources/icc_profiles.
    // - apng19.icc is an ICCv4 scanner profile with only A2B tags (no TRC/XYZ),
    //   including 256-entry M curve tables and 16-bit CLUT grid data.
    // - swapped.icc is an ICCv2 display profile with 1024-entry table TRC curves;
    //   exercises the TRC table pass-through path (raw big-endian bytes, no approximation).
    // - mu_gray.icc is a GRAY/prtr printer profile with kTRC (256-entry curv) and
    //   no colorant matrix; exercises the GRAY toXYZD50 synthesis path.
    // - tiles.icc is the Apple RGB monitor profile (RGB/mntr, 1024-entry curv TRCs)
    //   embedded in tiles.png, which is used by the svg/as-border-image Blink layout
    //   test.  Same TRC structure as swapped.icc but a distinct profile.
    const char* icc_files[] = {
        "icc_profiles/AdobeRGB1998.icc",
        "icc_profiles/HP_Z32x.icc",
        "icc_profiles/HP_ZR30w.icc",
        "icc_profiles/apng19.icc",
        "icc_profiles/chromium/colorspin.icc",
        "icc_profiles/chromium/generic_rgb.icc",
        "icc_profiles/color.org/Lower_Left.icc",
        "icc_profiles/color.org/Lower_Right.icc",
        "icc_profiles/color.org/sRGB2014.icc",
        "icc_profiles/color.org/sRGB_D65_MAT.icc",
        "icc_profiles/color.org/sRGB_D65_colorimetric.icc",
        "icc_profiles/color.org/sRGB_ICC_v4_Appearance.icc",
        "icc_profiles/color.org/sRGB_ISO22028.icc",
        "icc_profiles/fuzz/a2b_too_many_input_channels.icc",
        "icc_profiles/fuzz/a2b_too_many_input_channels2.icc",
        "icc_profiles/fuzz/b2a_no_clut.icc",
        "icc_profiles/fuzz/b2a_too_few_output_channels.icc",
        // "icc_profiles/fuzz/clut_overflow.icc",
        // Note: clut_overflow.icc tests skcms's rejection of CLUTs that end without trailing
        // slack bytes in the raw input buffer. skcms required this slack to prevent SIMD gather
        // overread on unmanaged caller buffers. The Rust parser allocates and zero-pads CLUT
        // data in Vec<u8>, safely supporting profiles that skcms rejected.
        "icc_profiles/fuzz/curv_size_overflow.icc",
        "icc_profiles/fuzz/direct_fit_negative_a.icc",
        "icc_profiles/fuzz/direct_fit_not_invertible.icc",
        "icc_profiles/fuzz/fit_pq.icc",
        "icc_profiles/fuzz/inf_a.icc",
        "icc_profiles/fuzz/infinite_roundtrip.icc",
        "icc_profiles/fuzz/inverse_tf_adb_negative.icc",
        "icc_profiles/fuzz/inverse_tf_huge_g.icc",
        "icc_profiles/fuzz/inverse_tf_not_invertible.icc",
        "icc_profiles/fuzz/large_g.icc",
        "icc_profiles/fuzz/last_tag_too_small.icc",
        "icc_profiles/fuzz/mangled_trc_tags.icc",
        "icc_profiles/fuzz/named_tag_too_small.icc",
        "icc_profiles/fuzz/nan_s.icc",
        "icc_profiles/fuzz/negative_a_plus_b.icc",
        "icc_profiles/fuzz/negative_a_when_inverted.icc",
        "icc_profiles/fuzz/negative_g_para.icc",
        "icc_profiles/fuzz/one_d_clut.icc",
        "icc_profiles/fuzz/polytf_big_float_to_int_cast.icc",
        "icc_profiles/fuzz/polytf_nan_after_update.icc",
        "icc_profiles/fuzz/truncated_curv_tag.icc",
        "icc_profiles/fuzz/zero_a.icc",
        "icc_profiles/fuzz/zero_g.icc",
        "icc_profiles/misc/AdobeColorSpin.icc",
        "icc_profiles/misc/AdobeRGB.icc",
        "icc_profiles/misc/AppleHagc.icc",
        "icc_profiles/misc/Apple_Color_LCD.icc",
        "icc_profiles/misc/Apple_Wide_Color.icc",
        "icc_profiles/misc/Apple_pq_hagc_hdgm2.icc",
        "icc_profiles/misc/BenQ_GL2450.icc",
        "icc_profiles/misc/BenQ_RL2455.icc",
        "icc_profiles/misc/Calibrated_A2B_XYZ_Mismatch.icc",
        "icc_profiles/misc/Coated_FOGRA27_CMYK.icc",
        "icc_profiles/misc/Coated_FOGRA39_CMYK.icc",
        "icc_profiles/misc/ColorGATE_Sihl_PhotoPaper.icc",
        "icc_profiles/misc/ColorLogic_ISO_Coated_CMYK.icc",
        "icc_profiles/misc/Color_Spin_Gamma_18.icc",
        "icc_profiles/misc/DisplayCal_ASUS_NonMonotonic.icc",
        "icc_profiles/misc/Generic_RGB_Gamma_18.icc",
        "icc_profiles/misc/Gray_Gamma_22.icc",
        "icc_profiles/misc/HD_709.icc",
        "icc_profiles/misc/Japan_Color_2001_Coated.icc",
        "icc_profiles/misc/Kodak_sRGB.icc",
        "icc_profiles/misc/Lexmark_X110.icc",
        "icc_profiles/misc/MR2416GSDF.icc",
        "icc_profiles/misc/MartiMaria_browsertest_A2B.icc",
        "icc_profiles/misc/MartiMaria_browsertest_HARD.icc",
        "icc_profiles/misc/P3_PQ_cicp.icc",
        "icc_profiles/misc/Phase_One_P25.icc",
        "icc_profiles/misc/PrintOpen_ISO_Coated_CMYK.icc",
        "icc_profiles/misc/Rec2020_HLG_cicp.icc",
        "icc_profiles/misc/Rec2020_PQ_cicp.icc",
        "icc_profiles/misc/SM245B.icc",
        "icc_profiles/misc/SWOP_Coated_20_GCR_CMYK.icc",
        "icc_profiles/misc/ThinkpadX1YogaV2.icc",
        "icc_profiles/misc/US_Web_Coated_SWOP_CMYK.icc",
        "icc_profiles/misc/XPS13_9360.icc",
        "icc_profiles/misc/XRite_GRACol7_340_CMYK.icc",
        "icc_profiles/misc/bad_pcs.icc",
        "icc_profiles/misc/calibrated_nonzero_black.icc",
        "icc_profiles/misc/crbug_1017960_19.icc",
        "icc_profiles/misc/crbug_976551.icc",
        "icc_profiles/misc/sRGB_Calibrated_Heterogeneous.icc",
        "icc_profiles/misc/sRGB_Calibrated_Homogeneous.icc",
        "icc_profiles/misc/sRGB_Facebook.icc",
        "icc_profiles/misc/sRGB_HP.icc",
        "icc_profiles/misc/sRGB_HP_2.icc",
        "icc_profiles/misc/sRGB_ICC_v4_beta.icc",
        "icc_profiles/misc/sRGB_black_scaled.icc",
        "icc_profiles/misc/sRGB_lcms.icc",
        "icc_profiles/mobile/Display_P3_LUT.icc",
        "icc_profiles/mobile/Display_P3_parametric.icc",
        "icc_profiles/mobile/iPhone7p.icc",
        "icc_profiles/mobile/sRGB_LUT.icc",
        "icc_profiles/mobile/sRGB_parametric.icc",
        "icc_profiles/mu_gray.icc",
        "icc_profiles/pq_hdr.icc",
        "icc_profiles/srgb_lab_pcs.icc",
        "icc_profiles/swapped.icc",
        "icc_profiles/tiles.icc",
        "icc_profiles/upperLeft.icc",
        "icc_profiles/upperRight.icc",
    };

    for (const char* path : icc_files) {
        auto data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Failed to load ICC profile: %s", path);
            continue;
        }

        auto rust_profile = SkCodecs::MakeICCProfileWithRust(data);
        auto skcms_profile = SkCodecs::ColorProfile::MakeICCProfileWithSkCMS(data);

        if ((rust_profile == nullptr) != (skcms_profile == nullptr)) {
            ERRORF(r, "[%s] Parse success mismatch: rust=%s, skcms=%s",
                   path,
                   rust_profile ? "true" : "false",
                   skcms_profile ? "true" : "false");
            continue;
        }
        if (!rust_profile && !skcms_profile) {
            // Both parsers agree the profile is malformed/unusable.
            continue;
        }

        const auto& rust = *rust_profile->profile();
        const auto& skcms = *skcms_profile->profile();

        // Compare has_toXYZD50 and matrix values
        if (rust.has_toXYZD50 != skcms.has_toXYZD50) {
            ERRORF(r, "[%s] has_toXYZD50 mismatch: rust=%d, skcms=%d",
                   path, rust.has_toXYZD50, skcms.has_toXYZD50);
        }
        if (rust.has_toXYZD50 && skcms.has_toXYZD50) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (fabsf(rust.toXYZD50.vals[i][j] - skcms.toXYZD50.vals[i][j]) >= 0.0001f) {
                        ERRORF(r, "[%s] toXYZD50[%d][%d] mismatch: rust=%f, skcms=%f",
                               path, i, j, rust.toXYZD50.vals[i][j], skcms.toXYZD50.vals[i][j]);
                    }
                }
            }
        }

        // Compare has_trc and transfer functions.
        // Note: the Rust path approximates multi-entry table TRC curves as
        // parametric functions via skcms_ApproximateCurve, so trc[c].table_entries
        // may be 0 in the Rust profile and non-zero in the skcms profile.  Use
        // compare_curves_by_evaluation which handles both representations.
        if (rust.has_trc != skcms.has_trc) {
            ERRORF(r, "[%s] has_trc mismatch: rust=%d, skcms=%d",
                   path, rust.has_trc, skcms.has_trc);
        }
        if (rust.has_trc && skcms.has_trc) {
            for (int c = 0; c < 3; ++c) {
                compare_curves_by_evaluation(r, path, "trc", c,
                                             rust.trc[c], skcms.trc[c]);
            }
        }

        // Compare CICP if present
        if (rust.has_CICP != skcms.has_CICP) {
            ERRORF(r, "[%s] has_CICP mismatch: rust=%d, skcms=%d",
                   path, rust.has_CICP, skcms.has_CICP);
        }
        if (rust.has_CICP && skcms.has_CICP) {
            if (rust.CICP.color_primaries != skcms.CICP.color_primaries ||
                rust.CICP.transfer_characteristics != skcms.CICP.transfer_characteristics ||
                rust.CICP.matrix_coefficients != skcms.CICP.matrix_coefficients ||
                rust.CICP.video_full_range_flag != skcms.CICP.video_full_range_flag) {
                ERRORF(r, "[%s] CICP mismatch", path);
            }
        }

        // Compare HAGC if present
        if (rust.has_HAGC != skcms.has_HAGC) {
            ERRORF(r, "[%s] has_HAGC mismatch: rust=%d, skcms=%d",
                   path, rust.has_HAGC, skcms.has_HAGC);
        }
        if (rust.has_HAGC && skcms.has_HAGC) {
            if (rust.HAGC.size != skcms.HAGC.size) {
                ERRORF(r, "[%s] HAGC size mismatch: rust=%u, skcms=%u",
                       path, rust.HAGC.size, skcms.HAGC.size);
            } else if (memcmp(rust.HAGC.buffer, skcms.HAGC.buffer, rust.HAGC.size) != 0) {
                ERRORF(r, "[%s] HAGC buffer mismatch", path);
            }
        }

        // Compare data_color_space
        if (rust.data_color_space != skcms.data_color_space) {
            ERRORF(r, "[%s] data_color_space mismatch: rust=%u, skcms=%u",
                   path, rust.data_color_space, skcms.data_color_space);
        }

        // Compare A2B transform if present
        if (rust.has_A2B != skcms.has_A2B) {
            ERRORF(r, "[%s] has_A2B mismatch: rust=%d, skcms=%d",
                   path, rust.has_A2B, skcms.has_A2B);
        }
        if (rust.has_A2B && skcms.has_A2B) {
            if (rust.A2B.input_channels != skcms.A2B.input_channels) {
                ERRORF(r, "[%s] A2B.input_channels mismatch: rust=%u, skcms=%u",
                       path, rust.A2B.input_channels, skcms.A2B.input_channels);
            }
            if (rust.A2B.output_channels != skcms.A2B.output_channels) {
                ERRORF(r, "[%s] A2B.output_channels mismatch: rust=%u, skcms=%u",
                       path, rust.A2B.output_channels, skcms.A2B.output_channels);
            }
            // Compare grid points
            for (int i = 0; i < 4; ++i) {
                if (rust.A2B.grid_points[i] != skcms.A2B.grid_points[i]) {
                    ERRORF(r, "[%s] A2B.grid_points[%d] mismatch: rust=%u, skcms=%u",
                           path, i, rust.A2B.grid_points[i], skcms.A2B.grid_points[i]);
                }
            }

            // Compare input curves (A curves)
            for (uint32_t i = 0; i < rust.A2B.input_channels && i < skcms.A2B.input_channels; ++i) {
                compare_curves_by_evaluation(r, path, "A2B.input_curves",
                                             i, rust.A2B.input_curves[i],
                                             skcms.A2B.input_curves[i]);
            }

            // Compare matrix stage
            if (rust.A2B.matrix_channels != skcms.A2B.matrix_channels) {
                ERRORF(r, "[%s] A2B.matrix_channels mismatch: rust=%u, skcms=%u",
                       path, rust.A2B.matrix_channels, skcms.A2B.matrix_channels);
            }
            if (rust.A2B.matrix_channels > 0 && skcms.A2B.matrix_channels > 0) {
                // Compare matrix values
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        float diff = fabsf(rust.A2B.matrix.vals[i][j] -
                                           skcms.A2B.matrix.vals[i][j]);
                        if (diff >= 0.0001f) {
                            ERRORF(r, "[%s] A2B.matrix[%d][%d] mismatch: rust=%f, skcms=%f",
                                   path, i, j, rust.A2B.matrix.vals[i][j],
                                   skcms.A2B.matrix.vals[i][j]);
                        }
                    }
                }
                // Compare matrix bias (4th column of 3x4 matrix)
                for (int i = 0; i < 3; ++i) {
                    float diff = fabsf(rust.A2B.matrix.vals[i][3] -
                                       skcms.A2B.matrix.vals[i][3]);
                    if (diff >= 0.0001f) {
                        ERRORF(r, "[%s] A2B.matrix_bias[%d] mismatch: rust=%f, skcms=%f",
                               path, i, rust.A2B.matrix.vals[i][3],
                               skcms.A2B.matrix.vals[i][3]);
                    }
                }
                // Compare matrix curves (M curves)
                for (int i = 0; i < 3; ++i) {
                    compare_curves_by_evaluation(r, path, "A2B.matrix_curves",
                                                 i, rust.A2B.matrix_curves[i],
                                                 skcms.A2B.matrix_curves[i]);
                }
            }

            // Compare output curves (B curves)
            for (uint32_t i = 0;
                 i < rust.A2B.output_channels && i < skcms.A2B.output_channels;
                 ++i) {
                compare_curves_by_evaluation(r, path, "A2B.output_curves",
                                             i, rust.A2B.output_curves[i],
                                             skcms.A2B.output_curves[i]);
            }

            // Compare grid data (CLUT) byte-by-byte
            compare_a2b_grid_data(r, path, rust.A2B, skcms.A2B);
        }

        // Compare B2A transform if present
        if (rust.has_B2A != skcms.has_B2A) {
            ERRORF(r, "[%s] has_B2A mismatch: rust=%d, skcms=%d",
                   path, rust.has_B2A, skcms.has_B2A);
        }
        if (rust.has_B2A && skcms.has_B2A) {
            if (rust.B2A.input_channels != skcms.B2A.input_channels) {
                ERRORF(r, "[%s] B2A.input_channels mismatch: rust=%u, skcms=%u",
                       path, rust.B2A.input_channels, skcms.B2A.input_channels);
            }
            if (rust.B2A.output_channels != skcms.B2A.output_channels) {
                ERRORF(r, "[%s] B2A.output_channels mismatch: rust=%u, skcms=%u",
                       path, rust.B2A.output_channels, skcms.B2A.output_channels);
            }
            // Compare grid points
            for (int i = 0; i < 4; ++i) {
                if (rust.B2A.grid_points[i] != skcms.B2A.grid_points[i]) {
                    ERRORF(r, "[%s] B2A.grid_points[%d] mismatch: rust=%u, skcms=%u",
                           path, i, rust.B2A.grid_points[i], skcms.B2A.grid_points[i]);
                }
            }

            // Compare input curves (B curves in B2A)
            for (uint32_t i = 0; i < rust.B2A.input_channels && i < skcms.B2A.input_channels; ++i) {
                compare_curves_by_evaluation(r, path, "B2A.input_curves",
                                             i, rust.B2A.input_curves[i],
                                             skcms.B2A.input_curves[i]);
            }

            // Compare matrix stage
            if (rust.B2A.matrix_channels != skcms.B2A.matrix_channels) {
                ERRORF(r, "[%s] B2A.matrix_channels mismatch: rust=%u, skcms=%u",
                       path, rust.B2A.matrix_channels, skcms.B2A.matrix_channels);
            }
            if (rust.B2A.matrix_channels > 0 && skcms.B2A.matrix_channels > 0) {
                // Compare matrix values
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        float diff = fabsf(rust.B2A.matrix.vals[i][j] -
                                           skcms.B2A.matrix.vals[i][j]);
                        if (diff >= 0.0001f) {
                            ERRORF(r, "[%s] B2A.matrix[%d][%d] mismatch: rust=%f, skcms=%f",
                                   path, i, j, rust.B2A.matrix.vals[i][j],
                                   skcms.B2A.matrix.vals[i][j]);
                        }
                    }
                }
                // Compare matrix bias (4th column of 3x4 matrix)
                for (int i = 0; i < 3; ++i) {
                    float diff = fabsf(rust.B2A.matrix.vals[i][3] -
                                       skcms.B2A.matrix.vals[i][3]);
                    if (diff >= 0.0001f) {
                        ERRORF(r, "[%s] B2A.matrix_bias[%d] mismatch: rust=%f, skcms=%f",
                               path, i, rust.B2A.matrix.vals[i][3],
                               skcms.B2A.matrix.vals[i][3]);
                    }
                }
                // Compare matrix curves (M curves)
                for (int i = 0; i < 3; ++i) {
                    compare_curves_by_evaluation(r, path, "B2A.matrix_curves",
                                                 i, rust.B2A.matrix_curves[i],
                                                 skcms.B2A.matrix_curves[i]);
                }
            }

            // Compare output curves (A curves in B2A)
            for (uint32_t i = 0;
                 i < rust.B2A.output_channels && i < skcms.B2A.output_channels;
                 ++i) {
                compare_curves_by_evaluation(r, path, "B2A.output_curves",
                                             i, rust.B2A.output_curves[i],
                                             skcms.B2A.output_curves[i]);
            }
        }
    }
}

// Regression test for multi-entry TRC table pass-through.
//
// swapped.icc is a real ICC v2 display profile with 1024-entry curv TRCs.
// The Rust bridge must pass the raw big-endian u16 table bytes through to
// skcms unchanged (table_entries > 0, table_16 set) rather than approximating
// the curve as a parametric function.  Passing the table exactly eliminates
// the ±1 ULP rounding difference that previously caused a max_difference=1
// pixel error across the entire image in svg/as-border-image.
DEF_TEST(RustIcc_trc_table_passthrough, r) {
    auto data = GetResourceAsData("icc_profiles/swapped.icc");
    if (!data) {
        ERRORF(r, "Failed to load icc_profiles/swapped.icc");
        return;
    }

    // skcms reference: must parse and expose table-based TRC.
    skcms_ICCProfile skcms_prof;
    if (!skcms_Parse(data->data(), data->size(), &skcms_prof)) {
        ERRORF(r, "skcms_Parse failed on swapped.icc");
        return;
    }
    REPORTER_ASSERT(r, skcms_prof.has_trc,
                    "skcms should report has_trc=true for swapped.icc");

    // Rust path: must expose the 1024-entry table TRC bit-exactly.
    auto rust_profile = SkCodecs::MakeICCProfileWithRust(data);
    if (!rust_profile) {
        ERRORF(r, "Rust ICC parser failed to parse swapped.icc");
        return;
    }
    const skcms_ICCProfile& rp = *rust_profile->profile();

    REPORTER_ASSERT(r, rp.has_trc,
                    "Rust ICC path must set has_trc=true for swapped.icc; "
                    "failure indicates the TRC table pass-through is broken");

    if (!rp.has_trc || !skcms_prof.has_trc) {
        return;
    }

    // The Rust path now passes the table through bit-exactly.  Both profiles
    // should have table_entries > 0 and identical big-endian byte content.
    for (int c = 0; c < 3; ++c) {
        compare_curves_by_evaluation(r, "swapped.icc", "trc", c,
                                     rp.trc[c], skcms_prof.trc[c]);
    }
}

// Regression test for the non-fatal B2A rejection path.
//
// pq_hdr.icc is an ICCv4 PQ HDR profile with an A2B tag and a curve-only B2A
// Tests that curve-only B2A tags are correctly parsed and match skcms.
// pq_hdr.icc is an ICCv4 PQ HDR profile with an A2B tag and a curve-only B2A tag.
DEF_TEST(RustIcc_pq_hdr_b2a_present, r) {
    auto data = GetResourceAsData("icc_profiles/pq_hdr.icc");
    if (!data) {
        ERRORF(r, "Failed to load icc_profiles/pq_hdr.icc");
        return;
    }

    auto rust_profile  = SkCodecs::MakeICCProfileWithRust(data);
    auto skcms_profile = SkCodecs::ColorProfile::MakeICCProfileWithSkCMS(data);

    if (!rust_profile) {
        ERRORF(r, "Rust parser failed for pq_hdr.icc");
        return;
    }
    if (!skcms_profile) {
        ERRORF(r, "SkCMS parser failed for pq_hdr.icc");
        return;
    }

    const auto& rust  = *rust_profile->profile();
    const auto& skcms = *skcms_profile->profile();

    // Both parsers must expose the A2B tag.
    REPORTER_ASSERT(r, rust.has_A2B,  "Rust must expose A2B for pq_hdr.icc");
    REPORTER_ASSERT(r, skcms.has_A2B, "skcms must expose A2B for pq_hdr.icc");

    // Both parsers must expose the curve-only B2A tag.
    REPORTER_ASSERT(r, rust.has_B2A,  "Rust must expose B2A for pq_hdr.icc");
    REPORTER_ASSERT(r, skcms.has_B2A, "skcms must expose B2A for pq_hdr.icc");

    if (!skcms_ApproximatelyEqualProfiles(&rust, &skcms)) {
        ERRORF(r, "[pq_hdr.icc] profiles not approximately equal");
    }
}

DEF_TEST(RustIcc_writer_hagc_matches_parsed, r) {
    auto data = GetResourceAsData("icc_profiles/misc/AppleHagc.icc");
    if (!data) {
        ERRORF(r, "Failed to load icc_profiles/misc/AppleHagc.icc");
        return;
    }

    auto parsed_rust = SkCodecs::MakeICCProfileWithRust(data);
    REPORTER_ASSERT(r, parsed_rust != nullptr);
    if (!parsed_rust) {
        return;
    }

    const skcms_ICCProfile* rust_prof = parsed_rust->profile();
    REPORTER_ASSERT(r, rust_prof->has_HAGC);
    REPORTER_ASSERT(r, rust_prof->HAGC.size > 0);
    REPORTER_ASSERT(r, rust_prof->HAGC.buffer != nullptr);

    // Populate HDR metadata with the parsed HAGC payload.
    skhdr::Metadata hdr_metadata;
    auto hagc_data = SkData::MakeWithCopy(rust_prof->HAGC.buffer, rust_prof->HAGC.size);
    hdr_metadata.setSerializedAgtm(hagc_data);
    REPORTER_ASSERT(r, hdr_metadata.getSerializedAgtm() != nullptr);

    // Create a color space and write an ICC profile with the HDR metadata.
    auto color_space = SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ, SkNamedGamut::kDisplayP3);
    auto written_data = SkWriteICCProfile(color_space.get(), &hdr_metadata);
    REPORTER_ASSERT(r, written_data != nullptr);
    if (!written_data) {
        return;
    }

    // Parse the written ICC profile with the Rust parser and verify HAGC matches.
    auto written_rust = SkCodecs::MakeICCProfileWithRust(written_data);
    REPORTER_ASSERT(r, written_rust != nullptr);
    if (written_rust) {
        const skcms_ICCProfile* written_rust_prof = written_rust->profile();
        REPORTER_ASSERT(r, written_rust_prof->has_HAGC);
        REPORTER_ASSERT(r, written_rust_prof->HAGC.size == rust_prof->HAGC.size);
        REPORTER_ASSERT(r, memcmp(written_rust_prof->HAGC.buffer,
                                  rust_prof->HAGC.buffer,
                                  rust_prof->HAGC.size) == 0);
    }

    // Also verify that SkCMS parses the written ICC profile and matches.
    auto written_skcms = SkCodecs::ColorProfile::MakeICCProfileWithSkCMS(written_data);
    REPORTER_ASSERT(r, written_skcms != nullptr);
    if (written_skcms) {
        const skcms_ICCProfile* written_skcms_prof = written_skcms->profile();
        REPORTER_ASSERT(r, written_skcms_prof->has_HAGC);
        REPORTER_ASSERT(r, written_skcms_prof->HAGC.size == rust_prof->HAGC.size);
        REPORTER_ASSERT(r, memcmp(written_skcms_prof->HAGC.buffer,
                                  rust_prof->HAGC.buffer,
                                  rust_prof->HAGC.size) == 0);
    }
}
