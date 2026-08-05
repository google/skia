/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/encoder/SkJpegRustEncoder.h"

#include "experimental/rust_jpeg/ffi/FFI.rs.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkTPin.h"
#include "include/private/SkTo.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkSafeMath.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace SkJpegRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src,
            const SkJpegEncoder::Options& options) {
    if (!dst) {
        return false;
    }

    const SkImageInfo& info = src.info();
    if (!SkImageInfoIsValid(info) || !src.addr() || src.rowBytes() < info.minRowBytes()) {
        return false;
    }

    SkSafeMath safe;
    const uint32_t width = safe.castTo<uint32_t>(info.width());
    const uint32_t height = safe.castTo<uint32_t>(info.height());
    const uint32_t srcRowBytes = safe.castTo<uint32_t>(src.rowBytes());
    const size_t totalBytes = safe.mul(static_cast<size_t>(height), src.rowBytes());
    if (!safe.ok()) {
        return false;
    }

    rust_jpeg::JpegEncodeColor colorType;
    switch (info.colorType()) {
        case kRGB_888x_SkColorType:
            // kRGB_888x is 4 bytes/pixel (R, G, B, X) — same layout as RGBA_8888
            // but the 4th byte is ignored.  Map to RGBA with Ignore alpha so the
            // Rust encoder uses the correct 4-byte row stride.
            colorType = rust_jpeg::JpegEncodeColor::RGBA;
            break;
        case kRGBA_8888_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::RGBA;
            break;
        case kBGRA_8888_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::BGRA;
            break;
        case kGray_8_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::Grayscale;
            break;
        default:
            return false;
    }

    const bool hasUnpremulAlpha = (info.colorType() == kRGBA_8888_SkColorType ||
                                   info.colorType() == kBGRA_8888_SkColorType) &&
                                  info.alphaType() == kUnpremul_SkAlphaType;
    const rust_jpeg::JpegEncodeAlpha alphaOption =
            options.fAlphaOption == SkJpegEncoder::AlphaOption::kBlendOnBlack && hasUnpremulAlpha
                    ? rust_jpeg::JpegEncodeAlpha::BlendOnBlack
                    : rust_jpeg::JpegEncodeAlpha::Ignore;

    const uint32_t quality = SkToU32(SkTPin(options.fQuality, 0, 100));

    const uint8_t* pixels = static_cast<const uint8_t*>(src.addr());
    rust::Slice<const uint8_t> pixelSlice(pixels, totalBytes);

    rust::Vec<uint8_t> output;
    rust_jpeg::EncodingResult result = rust_jpeg::encode_jpeg(
        pixelSlice, width, height, srcRowBytes, colorType, alphaOption,
        quality, output);

    if (result != rust_jpeg::EncodingResult::Success) {
        return false;
    }

    // Write encoded JPEG data, then insert XMP if provided.
    // The jpeg-encoder crate writes a complete JPEG. For XMP, we insert an
    // APP1 marker segment right after the SOI + APP0 (JFIF) markers.
    if (options.xmpMetadata && options.xmpMetadata->size() > 0) {
        // JPEG structure: SOI (2 bytes), then APP0/JFIF segment, then rest.
        // We find the end of the first APP marker and inject APP1 XMP there.
        const uint8_t* data = output.data();
        const size_t size = output.size();

        // Find insertion point after SOI + first APP marker.
        size_t insertPos = 2;  // After SOI (FF D8)
        if (size > 5 && data[2] == 0xFF && (data[3] & 0xF0) == 0xE0) {
            // The JPEG length field at data[4..5] includes its own 2 bytes.
            // Total bytes consumed by the APP segment: 2 (marker) + segLen.
            uint16_t segLen = (static_cast<uint16_t>(data[4]) << 8) | data[5];
            size_t candidate = static_cast<size_t>(2) + 2 + segLen;
            if (candidate <= size) {
                insertPos = candidate;
            }
        }

        // Build APP1 XMP segment:
        // Marker: FF E1
        // Length: 2 + namespace.size() + 1 + xmp.size()
        static const char kXmpNamespace[] = "http://ns.adobe.com/xap/1.0/";
        const size_t nsLen = sizeof(kXmpNamespace);  // includes null terminator
        const size_t xmpSize = options.xmpMetadata->size();
        SkSafeMath app1Safe;
        const size_t app1PayloadSize = app1Safe.add(app1Safe.add(2, nsLen), xmpSize);
        if (!app1Safe.ok() || app1PayloadSize > 0xFFFF) {
            // APP1 segment length field is 16-bit; extended XMP is not
            // supported yet.
            return false;
        }
        const uint16_t app1Len = static_cast<uint16_t>(app1PayloadSize);

        // Write: before insertion, APP1 marker, rest
        if (!dst->write(data, insertPos)) {
            return false;
        }
        uint8_t app1Header[4] = {
            0xFF, 0xE1,
            static_cast<uint8_t>(app1Len >> 8),
            static_cast<uint8_t>(app1Len & 0xFF)
        };
        if (!dst->write(app1Header, 4) || !dst->write(kXmpNamespace, nsLen) ||
            !dst->write(options.xmpMetadata->data(), xmpSize) ||
            !dst->write(data + insertPos, size - insertPos)) {
            return false;
        }
    } else {
        if (!dst->write(output.data(), output.size())) {
            return false;
        }
    }

    return true;
}

}  // namespace SkJpegRustEncoder
