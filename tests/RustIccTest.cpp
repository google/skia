/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rust/icc/FFI.h"
#include "rust/icc/FFI.rs.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecColorProfileRust.h"
#include "src/codec/SkCodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cmath>

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
    for (int i = 0; i < 3; i++) {
        rust_profile.trc[i].g = 2.2f;
        rust_profile.trc[i].a = 1.0f;
        rust_profile.trc[i].b = 0.0f;
        rust_profile.trc[i].c = 0.0f;
        rust_profile.trc[i].d = 0.0f;
        rust_profile.trc[i].e = 0.0f;
        rust_profile.trc[i].f = 0.0f;
    }

    // Convert to skcms
    skcms_ICCProfile skcms_profile;
    bool success = rust_icc::ToSkcmsIccProfile(rust_profile, &skcms_profile);

    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, skcms_profile.has_toXYZD50);
    REPORTER_ASSERT(r, skcms_profile.has_trc);

    // Verify matrix was copied correctly
    REPORTER_ASSERT(r, fabsf(skcms_profile.toXYZD50.vals[0][0] - 1.0f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.toXYZD50.vals[1][1] - 1.0f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.toXYZD50.vals[2][2] - 1.0f) < 0.0001f);

    // Verify transfer functions were copied (all channels should be gamma 2.2)
    REPORTER_ASSERT(r, skcms_profile.trc[0].table_entries == 0);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[0].parametric.g - 2.2f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[1].parametric.g - 2.2f) < 0.0001f);
    REPORTER_ASSERT(r, fabsf(skcms_profile.trc[2].parametric.g - 2.2f) < 0.0001f);
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
    for (int i = 0; i < 3; i++) {
        rust_profile.trc[i].g = 2.2f;
        rust_profile.trc[i].a = 1.0f;
        rust_profile.trc[i].b = 0.0f;
        rust_profile.trc[i].c = 0.0f;
        rust_profile.trc[i].d = 0.0f;
        rust_profile.trc[i].e = 0.0f;
        rust_profile.trc[i].f = 0.0f;
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
    for (int i = 0; i < 3; i++) {
        rust_profile.trc[i].g = 2.2f;
        rust_profile.trc[i].a = 1.0f;
        rust_profile.trc[i].b = 0.0f;
        rust_profile.trc[i].c = 0.0f;
        rust_profile.trc[i].d = 0.0f;
        rust_profile.trc[i].e = 0.0f;
        rust_profile.trc[i].f = 0.0f;
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

    // Set up minimal grid
    rust::Vec<uint8_t> grid_data;
    grid_data.push_back(0x80);
    rust_profile.a2b.grid_data = std::move(grid_data);
    rust_profile.a2b.grid_points[0] = 1;
    rust_profile.a2b.grid_points[1] = 1;
    rust_profile.a2b.grid_points[2] = 1;

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
    for (int i = 0; i < 3; i++) {
        rust_profile.trc[i].g = 2.2f;
        rust_profile.trc[i].a = 1.0f;
        rust_profile.trc[i].b = 0.0f;
        rust_profile.trc[i].c = 0.0f;
        rust_profile.trc[i].d = 0.0f;
        rust_profile.trc[i].e = 0.0f;
        rust_profile.trc[i].f = 0.0f;
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

    // Minimal grid (1x1x1 = 1 point, 3 output channels = 3 bytes)
    rust::Vec<uint8_t> grid_data;
    grid_data.push_back(0x80);
    grid_data.push_back(0x80);
    grid_data.push_back(0x80);
    rust_profile.a2b.grid_data = std::move(grid_data);
    rust_profile.a2b.grid_points[0] = 1;
    rust_profile.a2b.grid_points[1] = 1;
    rust_profile.a2b.grid_points[2] = 1;
    rust_profile.a2b.is_16bit_grid = false;

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
    for (int i = 0; i < 3; i++) {
        rust_profile.trc[i].g = 2.2f;
        rust_profile.trc[i].a = 1.0f;
        rust_profile.trc[i].b = 0.0f;
        rust_profile.trc[i].c = 0.0f;
        rust_profile.trc[i].d = 0.0f;
        rust_profile.trc[i].e = 0.0f;
        rust_profile.trc[i].f = 0.0f;
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

DEF_TEST(RustIcc_equivalence_with_skcms_resource_files, r) {
    // List of ICC profile files in resources/icc_profiles
    const char* icc_files[] = {
        "icc_profiles/AdobeRGB1998.icc",
        "icc_profiles/HP_Z32x.icc",
        "icc_profiles/HP_ZR30w.icc",
        "icc_profiles/srgb_lab_pcs.icc",
        "icc_profiles/upperLeft.icc",
        "icc_profiles/upperRight.icc"
    };

    for (const char* path : icc_files) {
        auto data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Failed to load ICC profile: %s", path);
            continue;
        }

        auto rust_profile = SkCodecs::MakeICCProfileWithRust(data);
        auto skcms_profile = SkCodecs::ColorProfile::MakeICCProfileWithSkCMS(data);

        if (!rust_profile) {
            ERRORF(r, "Rust parser failed for: %s", path);
            continue;
        }
        if (!skcms_profile) {
            ERRORF(r, "SkCMS parser failed for: %s", path);
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

        // Compare has_trc and transfer functions
        if (rust.has_trc != skcms.has_trc) {
            ERRORF(r, "[%s] has_trc mismatch: rust=%d, skcms=%d",
                   path, rust.has_trc, skcms.has_trc);
        }
        if (rust.has_trc && skcms.has_trc) {
            for (int c = 0; c < 3; ++c) {
                if (rust.trc[c].table_entries != skcms.trc[c].table_entries) {
                    ERRORF(r, "[%s] trc[%d].table_entries mismatch: rust=%u, skcms=%u",
                           path, c, rust.trc[c].table_entries, skcms.trc[c].table_entries);
                    continue;
                }
                if (rust.trc[c].table_entries == 0 && skcms.trc[c].table_entries == 0) {
                    // Parametric - compare transfer function parameters
                    compare_transfer_functions(r, path, c, rust.trc[c].parametric, skcms.trc[c].parametric);
                }
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
                if (rust.A2B.input_curves[i].table_entries != skcms.A2B.input_curves[i].table_entries) {
                    ERRORF(r, "[%s] A2B.input_curves[%u].table_entries mismatch: rust=%u, skcms=%u",
                           path, i, rust.A2B.input_curves[i].table_entries,
                           skcms.A2B.input_curves[i].table_entries);
                } else if (rust.A2B.input_curves[i].table_entries == 0) {
                    // Parametric curves - compare transfer functions
                    compare_transfer_functions(r, path, i,
                                             rust.A2B.input_curves[i].parametric,
                                             skcms.A2B.input_curves[i].parametric);
                }
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
                        if (fabsf(rust.A2B.matrix.vals[i][j] - skcms.A2B.matrix.vals[i][j]) >= 0.0001f) {
                            ERRORF(r, "[%s] A2B.matrix[%d][%d] mismatch: rust=%f, skcms=%f",
                                   path, i, j, rust.A2B.matrix.vals[i][j], skcms.A2B.matrix.vals[i][j]);
                        }
                    }
                }
                // Compare matrix bias (4th column of 3x4 matrix)
                for (int i = 0; i < 3; ++i) {
                    if (fabsf(rust.A2B.matrix.vals[i][3] - skcms.A2B.matrix.vals[i][3]) >= 0.0001f) {
                        ERRORF(r, "[%s] A2B.matrix_bias[%d] mismatch: rust=%f, skcms=%f",
                               path, i, rust.A2B.matrix.vals[i][3], skcms.A2B.matrix.vals[i][3]);
                    }
                }
                // Compare matrix curves (M curves)
                for (int i = 0; i < 3; ++i) {
                    if (rust.A2B.matrix_curves[i].table_entries != skcms.A2B.matrix_curves[i].table_entries) {
                        ERRORF(r, "[%s] A2B.matrix_curves[%d].table_entries mismatch: rust=%u, skcms=%u",
                               path, i, rust.A2B.matrix_curves[i].table_entries,
                               skcms.A2B.matrix_curves[i].table_entries);
                    } else if (rust.A2B.matrix_curves[i].table_entries == 0) {
                        compare_transfer_functions(r, path, i,
                                                 rust.A2B.matrix_curves[i].parametric,
                                                 skcms.A2B.matrix_curves[i].parametric);
                    }
                }
            }

            // Compare output curves (B curves)
            for (uint32_t i = 0; i < rust.A2B.output_channels && i < skcms.A2B.output_channels; ++i) {
                if (rust.A2B.output_curves[i].table_entries != skcms.A2B.output_curves[i].table_entries) {
                    ERRORF(r, "[%s] A2B.output_curves[%u].table_entries mismatch: rust=%u, skcms=%u",
                           path, i, rust.A2B.output_curves[i].table_entries,
                           skcms.A2B.output_curves[i].table_entries);
                } else if (rust.A2B.output_curves[i].table_entries == 0) {
                    compare_transfer_functions(r, path, i,
                                             rust.A2B.output_curves[i].parametric,
                                             skcms.A2B.output_curves[i].parametric);
                }
            }
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
                if (rust.B2A.input_curves[i].table_entries != skcms.B2A.input_curves[i].table_entries) {
                    ERRORF(r, "[%s] B2A.input_curves[%u].table_entries mismatch: rust=%u, skcms=%u",
                           path, i, rust.B2A.input_curves[i].table_entries,
                           skcms.B2A.input_curves[i].table_entries);
                } else if (rust.B2A.input_curves[i].table_entries == 0) {
                    compare_transfer_functions(r, path, i,
                                             rust.B2A.input_curves[i].parametric,
                                             skcms.B2A.input_curves[i].parametric);
                }
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
                        if (fabsf(rust.B2A.matrix.vals[i][j] - skcms.B2A.matrix.vals[i][j]) >= 0.0001f) {
                            ERRORF(r, "[%s] B2A.matrix[%d][%d] mismatch: rust=%f, skcms=%f",
                                   path, i, j, rust.B2A.matrix.vals[i][j], skcms.B2A.matrix.vals[i][j]);
                        }
                    }
                }
                // Compare matrix bias (4th column of 3x4 matrix)
                for (int i = 0; i < 3; ++i) {
                    if (fabsf(rust.B2A.matrix.vals[i][3] - skcms.B2A.matrix.vals[i][3]) >= 0.0001f) {
                        ERRORF(r, "[%s] B2A.matrix_bias[%d] mismatch: rust=%f, skcms=%f",
                               path, i, rust.B2A.matrix.vals[i][3], skcms.B2A.matrix.vals[i][3]);
                    }
                }
                // Compare matrix curves (M curves)
                for (int i = 0; i < 3; ++i) {
                    if (rust.B2A.matrix_curves[i].table_entries != skcms.B2A.matrix_curves[i].table_entries) {
                        ERRORF(r, "[%s] B2A.matrix_curves[%d].table_entries mismatch: rust=%u, skcms=%u",
                               path, i, rust.B2A.matrix_curves[i].table_entries,
                               skcms.B2A.matrix_curves[i].table_entries);
                    } else if (rust.B2A.matrix_curves[i].table_entries == 0) {
                        compare_transfer_functions(r, path, i,
                                                 rust.B2A.matrix_curves[i].parametric,
                                                 skcms.B2A.matrix_curves[i].parametric);
                    }
                }
            }

            // Compare output curves (A curves in B2A)
            for (uint32_t i = 0; i < rust.B2A.output_channels && i < skcms.B2A.output_channels; ++i) {
                if (rust.B2A.output_curves[i].table_entries != skcms.B2A.output_curves[i].table_entries) {
                    ERRORF(r, "[%s] B2A.output_curves[%u].table_entries mismatch: rust=%u, skcms=%u",
                           path, i, rust.B2A.output_curves[i].table_entries,
                           skcms.B2A.output_curves[i].table_entries);
                } else if (rust.B2A.output_curves[i].table_entries == 0) {
                    compare_transfer_functions(r, path, i,
                                             rust.B2A.output_curves[i].parametric,
                                             skcms.B2A.output_curves[i].parametric);
                }
            }
        }
    }
}

