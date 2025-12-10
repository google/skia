/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpRustCodec_DEFINED
#define SkBmpRustCodec_DEFINED

#include <memory>
#include <vector>

#include "experimental/rust_bmp/ffi/FFI.rs.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkSpan.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkStream;
class SkSwizzler;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party `image` crate (containing BMP decompression and decoding
//   implemented in Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space
//   transformations implemented in C++).
class SkBmpRustCodec final : public SkCodec {
public:
    static std::unique_ptr<SkBmpRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkBmpRustCodec() override;

protected:
    SkBmpRustCodec(SkEncodedInfo&&,
                   std::unique_ptr<SkStream>,
                   rust::Box<rust_bmp::Reader>,
                   size_t srcRowBytes);

    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kBMP; }

    bool onRewind() override;

private:
    // Codec implementation
    Result onGetPixels(const SkImageInfo& info,
                       void* dst,
                       size_t dstRowStride,
                       const Options& options,
                       int* rowsDecoded) override;

    // Helper methods
    Result performFullDecode(const SkImageInfo& dstInfo, void* dst, size_t dstRowStride);
    Result initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts);
    void swizzleRow(const uint8_t* srcRow, void* dstRow);

    rust::Box<rust_bmp::Reader> fReader;
    std::unique_ptr<SkStream> fPrivStream;
    std::unique_ptr<SkSwizzler> fSwizzler;

    // Using 4-bytes-wide `uint32_t` for each pixel, because
    // `kXformSrcColorType = kRGBA_8888_SkColorType`.
    std::unique_ptr<uint32_t[]> fXformBuffer;

    const size_t fSrcRowBytes;
};

#endif  // SkBmpRustCodec_DEFINED
