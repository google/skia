/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rust/exif/FFI.h"
#include "rust/exif/FFI.rs.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/private/SkExif.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cmath>

// Helpers: build minimal EXIF blobs (big- and little-endian) for unit tests.

// Writes a 2-byte big-endian u16 into `buf` at `off`.
static void write_u16_be(std::vector<uint8_t>& buf, size_t off, uint16_t v) {
    buf[off]     = static_cast<uint8_t>(v >> 8);
    buf[off + 1] = static_cast<uint8_t>(v);
}

// Writes a 4-byte big-endian u32 into `buf` at `off`.
static void write_u32_be(std::vector<uint8_t>& buf, size_t off, uint32_t v) {
    buf[off]     = static_cast<uint8_t>(v >> 24);
    buf[off + 1] = static_cast<uint8_t>(v >> 16);
    buf[off + 2] = static_cast<uint8_t>(v >> 8);
    buf[off + 3] = static_cast<uint8_t>(v);
}

// Writes a 2-byte little-endian u16 into `buf` at `off`.
static void write_u16_le(std::vector<uint8_t>& buf, size_t off, uint16_t v) {
    buf[off]     = static_cast<uint8_t>(v);
    buf[off + 1] = static_cast<uint8_t>(v >> 8);
}

// Writes a 4-byte little-endian u32 into `buf` at `off`.
static void write_u32_le(std::vector<uint8_t>& buf, size_t off, uint32_t v) {
    buf[off]     = static_cast<uint8_t>(v);
    buf[off + 1] = static_cast<uint8_t>(v >> 8);
    buf[off + 2] = static_cast<uint8_t>(v >> 16);
    buf[off + 3] = static_cast<uint8_t>(v >> 24);
}

// Returns a TIFF header (8 bytes, big-endian) pointing to IFD at `ifd_offset`.
static std::vector<uint8_t> tiff_header_be(uint32_t ifd_offset = 8) {
    return {'M', 'M', 0x00, 0x2a,
            static_cast<uint8_t>(ifd_offset >> 24),
            static_cast<uint8_t>(ifd_offset >> 16),
            static_cast<uint8_t>(ifd_offset >> 8),
            static_cast<uint8_t>(ifd_offset)};
}

// Returns a TIFF header (8 bytes, little-endian) pointing to IFD at `ifd_offset`.
static std::vector<uint8_t> tiff_header_le(uint32_t ifd_offset = 8) {
    return {'I', 'I', 0x2a, 0x00,
            static_cast<uint8_t>(ifd_offset),
            static_cast<uint8_t>(ifd_offset >> 8),
            static_cast<uint8_t>(ifd_offset >> 16),
            static_cast<uint8_t>(ifd_offset >> 24)};
}

// Builds a single-IFD little-endian EXIF blob containing only an Orientation tag.
static std::vector<uint8_t> make_orientation_exif_le(uint16_t orientation) {
    std::vector<uint8_t> buf(26, 0);
    auto hdr = tiff_header_le(8);
    std::copy(hdr.begin(), hdr.end(), buf.begin());
    write_u16_le(buf, 8, 1);          // IFD entry count = 1
    write_u16_le(buf, 10, 0x0112);    // tag: Orientation
    write_u16_le(buf, 12, 3);         // type: SHORT
    write_u32_le(buf, 14, 1);         // count
    write_u16_le(buf, 18, orientation); // value
    write_u32_le(buf, 22, 0);         // next IFD offset
    return buf;
}

// Builds a single-IFD EXIF blob containing only a SHORT Orientation tag.
static std::vector<uint8_t> make_orientation_exif_be(uint16_t orientation) {
    // Header (8) + entry count (2) + 1 entry (12) + next IFD offset (4) = 26 bytes
    std::vector<uint8_t> buf(26, 0);
    // TIFF header pointing to IFD at offset 8.
    auto hdr = tiff_header_be(8);
    std::copy(hdr.begin(), hdr.end(), buf.begin());
    // IFD entry count = 1.
    write_u16_be(buf, 8, 1);
    // Entry: tag=0x0112 (Orientation), type=3 (SHORT), count=1, value=orientation.
    write_u16_be(buf, 10, 0x0112);  // tag
    write_u16_be(buf, 12, 3);       // type SHORT
    write_u32_be(buf, 14, 1);       // count
    write_u16_be(buf, 18, orientation);  // value (fits in 4 bytes inline, BE)
    // next IFD = 0 (no more IFDs).
    write_u32_be(buf, 22, 0);
    return buf;
}

// Builds a single-IFD EXIF blob with XResolution, YResolution, ResolutionUnit.
static std::vector<uint8_t> make_resolution_exif_be(float xres, float yres, uint16_t unit) {
    // The RATIONAL values (8 bytes each) are stored after the IFD.
    // Header=8, count=2, entries=3×12=36, next_ifd=4 → data at offset 52.
    // XResolution → RATIONAL at offset 52, YResolution at 60.
    const uint32_t xres_off = 52, yres_off = 60;
    std::vector<uint8_t> buf(68, 0);

    auto hdr = tiff_header_be(8);
    std::copy(hdr.begin(), hdr.end(), buf.begin());
    write_u16_be(buf, 8, 3);  // 3 entries

    // Entry 0: ResolutionUnit (tag 0x0128, SHORT, 1, unit)
    write_u16_be(buf, 10, 0x0128);
    write_u16_be(buf, 12, 3);  // SHORT
    write_u32_be(buf, 14, 1);
    write_u16_be(buf, 18, unit);

    // Entry 1: XResolution (tag 0x011a, RATIONAL, 1, offset)
    write_u16_be(buf, 22, 0x011a);
    write_u16_be(buf, 24, 5);  // RATIONAL
    write_u32_be(buf, 26, 1);
    write_u32_be(buf, 30, xres_off);

    // Entry 2: YResolution (tag 0x011b, RATIONAL, 1, offset)
    write_u16_be(buf, 34, 0x011b);
    write_u16_be(buf, 36, 5);  // RATIONAL
    write_u32_be(buf, 38, 1);
    write_u32_be(buf, 42, yres_off);

    // next IFD offset = 0
    write_u32_be(buf, 46, 0);

    // XResolution rational: (xres * 1000) / 1000 → numerator/denominator
    auto xnum = static_cast<uint32_t>(xres * 1000.0f + 0.5f);
    write_u32_be(buf, xres_off, xnum);
    write_u32_be(buf, xres_off + 4, 1000);

    auto ynum = static_cast<uint32_t>(yres * 1000.0f + 0.5f);
    write_u32_be(buf, yres_off, ynum);
    write_u32_be(buf, yres_off + 4, 1000);

    return buf;
}

// Unit tests: parse_exif() + ToSkExifMetadata() on synthetic EXIF data.

DEF_TEST(RustExif_empty_data_returns_false, r) {
    rust_exif::ExifMetadata meta;
    bool ok = rust_exif::parse_exif(rust::Slice<const uint8_t>(nullptr, 0), meta);
    REPORTER_ASSERT(r, !ok);
}

DEF_TEST(RustExif_invalid_data_returns_false, r) {
    rust_exif::ExifMetadata meta;
    const uint8_t garbage[] = {'n', 'o', 't', ' ', 'e', 'x', 'i', 'f'};
    bool ok = rust_exif::parse_exif(
            rust::Slice<const uint8_t>(garbage, sizeof(garbage)), meta);
    REPORTER_ASSERT(r, !ok);
}

DEF_TEST(RustExif_orientation_parsing, r) {
    for (uint16_t orient = 1; orient <= 8; ++orient) {
        auto blob = make_orientation_exif_be(orient);
        rust_exif::ExifMetadata meta;
        bool ok = rust_exif::parse_exif(
                rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
        REPORTER_ASSERT(r, ok);
        REPORTER_ASSERT(r, meta.has_origin);
        REPORTER_ASSERT(r, meta.origin == static_cast<uint32_t>(orient));

        // Verify ToSkExifMetadata maps it to the right SkEncodedOrigin.
        SkExif::Metadata sk_meta;
        rust_exif::ToSkExifMetadata(meta, &sk_meta);
        REPORTER_ASSERT(r, sk_meta.fOrigin.has_value());
        REPORTER_ASSERT(r, sk_meta.fOrigin.value() == static_cast<SkEncodedOrigin>(orient));
    }
}

DEF_TEST(RustExif_orientation_out_of_range_ignored, r) {
    // Orientation value 0 is invalid (EXIF spec says 1-8).
    auto blob = make_orientation_exif_be(0);
    rust_exif::ExifMetadata meta;
    rust_exif::parse_exif(rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
    REPORTER_ASSERT(r, !meta.has_origin);

    // Orientation value 9 is out of range.
    blob = make_orientation_exif_be(9);
    rust_exif::parse_exif(rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
    REPORTER_ASSERT(r, !meta.has_origin);
}

DEF_TEST(RustExif_little_endian_parsing, r) {
    // EXIF supports both big-endian ('M','M') and little-endian ('I','I') byte order.
    // Verify little-endian blobs are parsed correctly.
    for (uint16_t orient = 1; orient <= 8; ++orient) {
        auto blob = make_orientation_exif_le(orient);
        rust_exif::ExifMetadata meta;
        bool ok = rust_exif::parse_exif(
                rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
        REPORTER_ASSERT(r, ok);
        REPORTER_ASSERT(r, meta.has_origin);
        REPORTER_ASSERT(r, meta.origin == static_cast<uint32_t>(orient));
    }
}

DEF_TEST(RustExif_resolution_parsing, r) {
    constexpr float kEpsilon = 0.0001f;
    auto blob = make_resolution_exif_be(72.0f, 72.0f, 2 /*inch*/);
    rust_exif::ExifMetadata meta;
    bool ok = rust_exif::parse_exif(
            rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
    REPORTER_ASSERT(r, ok);
    REPORTER_ASSERT(r, meta.has_resolution_unit);
    REPORTER_ASSERT(r, meta.resolution_unit == 2);
    REPORTER_ASSERT(r, meta.has_x_resolution);
    REPORTER_ASSERT(r, std::abs(meta.x_resolution - 72.0f) < kEpsilon);
    REPORTER_ASSERT(r, meta.has_y_resolution);
    REPORTER_ASSERT(r, std::abs(meta.y_resolution - 72.0f) < kEpsilon);

    // Verify ToSkExifMetadata conversion.
    SkExif::Metadata sk_meta;
    rust_exif::ToSkExifMetadata(meta, &sk_meta);
    REPORTER_ASSERT(r, sk_meta.fResolutionUnit.has_value());
    REPORTER_ASSERT(r, sk_meta.fResolutionUnit.value() == 2);
    REPORTER_ASSERT(r, sk_meta.fXResolution.has_value());
    REPORTER_ASSERT(r, std::abs(sk_meta.fXResolution.value() - 72.0f) < kEpsilon);
    REPORTER_ASSERT(r, sk_meta.fYResolution.has_value());
    REPORTER_ASSERT(r, std::abs(sk_meta.fYResolution.value() - 72.0f) < kEpsilon);
}

// Builds a single-IFD EXIF blob with PixelXDimension and PixelYDimension tags.
static std::vector<uint8_t> make_pixel_dimensions_exif_be(uint16_t width, uint16_t height) {
    // Header=8, count=2, entries=2×12=24, next_ifd=4 → 38 bytes.
    std::vector<uint8_t> buf(38, 0);
    auto hdr = tiff_header_be(8);
    std::copy(hdr.begin(), hdr.end(), buf.begin());
    write_u16_be(buf, 8, 2);  // 2 entries

    // Entry 0: PixelXDimension (tag 0xa002, SHORT, 1, width)
    write_u16_be(buf, 10, 0xa002);
    write_u16_be(buf, 12, 3);  // SHORT
    write_u32_be(buf, 14, 1);
    write_u16_be(buf, 18, width);

    // Entry 1: PixelYDimension (tag 0xa003, SHORT, 1, height)
    write_u16_be(buf, 22, 0xa003);
    write_u16_be(buf, 24, 3);  // SHORT
    write_u32_be(buf, 26, 1);
    write_u16_be(buf, 30, height);

    // next IFD = 0
    write_u32_be(buf, 34, 0);
    return buf;
}

DEF_TEST(RustExif_pixel_dimensions_parsing, r) {
    auto blob = make_pixel_dimensions_exif_be(1920, 1080);
    rust_exif::ExifMetadata meta;
    bool ok = rust_exif::parse_exif(
            rust::Slice<const uint8_t>(blob.data(), blob.size()), meta);
    REPORTER_ASSERT(r, ok);
    REPORTER_ASSERT(r, meta.has_pixel_x_dimension);
    REPORTER_ASSERT(r, meta.pixel_x_dimension == 1920u);
    REPORTER_ASSERT(r, meta.has_pixel_y_dimension);
    REPORTER_ASSERT(r, meta.pixel_y_dimension == 1080u);

    SkExif::Metadata sk_meta;
    rust_exif::ToSkExifMetadata(meta, &sk_meta);
    REPORTER_ASSERT(r, sk_meta.fPixelXDimension.has_value());
    REPORTER_ASSERT(r, sk_meta.fPixelXDimension.value() == 1920u);
    REPORTER_ASSERT(r, sk_meta.fPixelYDimension.has_value());
    REPORTER_ASSERT(r, sk_meta.fPixelYDimension.value() == 1080u);
}

// Equivalence tests: compare Rust parser against SkExif::Parse on real files.

static bool approx_eq_f(float a, float b, float eps) {
    return std::abs(a - b) < eps;
}

// Compares a Rust-parsed ExifMetadata (converted to SkExif::Metadata) against
// the reference C++ SkExif::Parse result for the given resource EXIF file.
static void check_equivalence(skiatest::Reporter* r, const char* resource_path) {
    sk_sp<SkData> data = GetResourceAsData(resource_path);
    if (!data) {
        INFOF(r, "Skipping equivalence test for %s: resource not available.", resource_path);
        return;
    }

    // C++ reference parse.
    SkExif::Metadata cpp_meta;
    SkExif::Parse(cpp_meta, data.get());

    // Rust parse + conversion.
    rust_exif::ExifMetadata rust_raw;
    bool rust_ok = rust_exif::parse_exif(
            rust::Slice<const uint8_t>(
                    reinterpret_cast<const uint8_t*>(data->data()), data->size()),
            rust_raw);
    SkExif::Metadata rust_meta;
    if (rust_ok) {
        rust_exif::ToSkExifMetadata(rust_raw, &rust_meta);
    }

    constexpr float kEpsilon = 0.0001f;

    // fOrigin
    REPORTER_ASSERT(r,
                    cpp_meta.fOrigin.has_value() == rust_meta.fOrigin.has_value(),
                    "[%s] fOrigin presence mismatch", resource_path);
    if (cpp_meta.fOrigin.has_value() && rust_meta.fOrigin.has_value()) {
        REPORTER_ASSERT(r,
                        cpp_meta.fOrigin.value() == rust_meta.fOrigin.value(),
                        "[%s] fOrigin value mismatch: cpp=%d rust=%d",
                        resource_path,
                        static_cast<int>(cpp_meta.fOrigin.value()),
                        static_cast<int>(rust_meta.fOrigin.value()));
    }

    // fHdrHeadroom
    REPORTER_ASSERT(r,
                    cpp_meta.fHdrHeadroom.has_value() == rust_meta.fHdrHeadroom.has_value(),
                    "[%s] fHdrHeadroom presence mismatch", resource_path);
    if (cpp_meta.fHdrHeadroom.has_value() && rust_meta.fHdrHeadroom.has_value()) {
        REPORTER_ASSERT(r,
                        approx_eq_f(cpp_meta.fHdrHeadroom.value(),
                                    rust_meta.fHdrHeadroom.value(),
                                    kEpsilon),
                        "[%s] fHdrHeadroom value mismatch: cpp=%f rust=%f",
                        resource_path,
                        cpp_meta.fHdrHeadroom.value(),
                        rust_meta.fHdrHeadroom.value());
    }

    // fResolutionUnit
    REPORTER_ASSERT(r,
                    cpp_meta.fResolutionUnit.has_value() == rust_meta.fResolutionUnit.has_value(),
                    "[%s] fResolutionUnit presence mismatch", resource_path);
    if (cpp_meta.fResolutionUnit.has_value() && rust_meta.fResolutionUnit.has_value()) {
        REPORTER_ASSERT(r,
                        cpp_meta.fResolutionUnit.value() == rust_meta.fResolutionUnit.value(),
                        "[%s] fResolutionUnit value mismatch: cpp=%d rust=%d",
                        resource_path,
                        cpp_meta.fResolutionUnit.value(),
                        rust_meta.fResolutionUnit.value());
    }

    // fXResolution
    REPORTER_ASSERT(r,
                    cpp_meta.fXResolution.has_value() == rust_meta.fXResolution.has_value(),
                    "[%s] fXResolution presence mismatch", resource_path);
    if (cpp_meta.fXResolution.has_value() && rust_meta.fXResolution.has_value()) {
        REPORTER_ASSERT(r,
                        approx_eq_f(cpp_meta.fXResolution.value(),
                                    rust_meta.fXResolution.value(),
                                    kEpsilon),
                        "[%s] fXResolution value mismatch: cpp=%f rust=%f",
                        resource_path,
                        cpp_meta.fXResolution.value(),
                        rust_meta.fXResolution.value());
    }

    // fYResolution
    REPORTER_ASSERT(r,
                    cpp_meta.fYResolution.has_value() == rust_meta.fYResolution.has_value(),
                    "[%s] fYResolution presence mismatch", resource_path);
    if (cpp_meta.fYResolution.has_value() && rust_meta.fYResolution.has_value()) {
        REPORTER_ASSERT(r,
                        approx_eq_f(cpp_meta.fYResolution.value(),
                                    rust_meta.fYResolution.value(),
                                    kEpsilon),
                        "[%s] fYResolution value mismatch: cpp=%f rust=%f",
                        resource_path,
                        cpp_meta.fYResolution.value(),
                        rust_meta.fYResolution.value());
    }

    // fPixelXDimension
    REPORTER_ASSERT(r,
                    cpp_meta.fPixelXDimension.has_value() ==
                            rust_meta.fPixelXDimension.has_value(),
                    "[%s] fPixelXDimension presence mismatch", resource_path);
    if (cpp_meta.fPixelXDimension.has_value() && rust_meta.fPixelXDimension.has_value()) {
        REPORTER_ASSERT(r,
                        cpp_meta.fPixelXDimension.value() ==
                                rust_meta.fPixelXDimension.value(),
                        "[%s] fPixelXDimension value mismatch: cpp=%u rust=%u",
                        resource_path,
                        cpp_meta.fPixelXDimension.value(),
                        rust_meta.fPixelXDimension.value());
    }

    // fPixelYDimension
    REPORTER_ASSERT(r,
                    cpp_meta.fPixelYDimension.has_value() ==
                            rust_meta.fPixelYDimension.has_value(),
                    "[%s] fPixelYDimension presence mismatch", resource_path);
    if (cpp_meta.fPixelYDimension.has_value() && rust_meta.fPixelYDimension.has_value()) {
        REPORTER_ASSERT(r,
                        cpp_meta.fPixelYDimension.value() ==
                                rust_meta.fPixelYDimension.value(),
                        "[%s] fPixelYDimension value mismatch: cpp=%u rust=%u",
                        resource_path,
                        cpp_meta.fPixelYDimension.value(),
                        rust_meta.fPixelYDimension.value());
    }
}

DEF_TEST(RustExif_equivalence_with_resource_files, r) {
    // Test against the same EXIF files used in ExifTest.cpp.
    check_equivalence(r, "images/test0-hdr.exif");
    check_equivalence(r, "images/test1-pixel32.exif");
    check_equivalence(r, "images/test2-nonuniform.exif");
    check_equivalence(r, "images/test3-little-endian.exif");
}
