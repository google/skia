/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJpegRustCodec_DEFINED
#define SkJpegRustCodec_DEFINED

#include <memory>
#include <optional>

#include "experimental/rust_jpeg/ffi/FFI.rs.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkSpan.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkStream;
class SkSwizzler;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party `zune-jpeg` crate (containing JPEG decompression and
//   decoding implemented in Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space
//   transformations implemented in C++).
class SkJpegRustCodec final : public SkCodec {
public:
    static std::unique_ptr<SkJpegRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkJpegRustCodec() override;

protected:
    SkJpegRustCodec(SkEncodedInfo&&,
                    std::unique_ptr<SkStream>,
                    rust::Box<rust_jpeg::Reader>,
                    SkEncodedOrigin origin = kDefault_SkEncodedOrigin);

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEG;
    }

    bool onRewind() override;

    bool onSupportsIncrementalDecode(const SkImageInfo&) override { return true; }

    bool onGetFrameInfo(int, FrameInfo*) const override;

    bool onGetGainmapInfo(SkGainmapInfo* info,
                          std::unique_ptr<SkStream>* gainmapImageStream) override;

    bool onGetGainmapCodec(SkGainmapInfo* info,
                           std::unique_ptr<SkCodec>* gainmapCodec) override;

private:
    sk_sp<const SkData> getEncodedData() const override;

    Result onGetPixels(const SkImageInfo& info,
                       void* dst,
                       size_t dstRowStride,
                       const Options& options,
                       int* rowsDecoded) override;

    // Incremental decoding exposes stable baseline scanlines as they become
    // available and replaceable full-frame previews after progressive scans.
    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                   void* dst,
                                   size_t dstRowBytes,
                                   const Options&) override;

    Result onIncrementalDecode(int* rowsDecoded) override;

    // Helper methods
    Result performFullDecode(const SkImageInfo& dstInfo, void* dst, size_t dstRowStride,
                             int* rowsDecoded = nullptr);
    Result initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts);
    void swizzleRow(const uint8_t* srcRow, void* dstRow);

    struct DecodingState {
        SkSpan<uint8_t> fDst;
        size_t fDstRowStride;
        int fTotalRowsInitialized = 0;
    };

    Result incrementalDecode(DecodingState& state, int* rowsDecoded);

    std::optional<DecodingState> fIncrementalDecodingState;

    // Stored separately; SkCodec base gets nullptr. Keep this before fReader so
    // the stream outlives the Rust reader's stream adapter.
    std::unique_ptr<SkStream> fPrivStream;
    rust::Box<rust_jpeg::Reader> fReader;
    std::unique_ptr<SkSwizzler> fSwizzler;

    // Using 4-bytes-wide `uint32_t` for each pixel, because
    // `kXformSrcColorType = kRGBA_8888_SkColorType`.
    std::unique_ptr<uint32_t[]> fXformBuffer;

    // TODO: Add onGetScaledDimensions for 1/2, 1/4, 1/8 IDCT scaling
    //   (requires zune-jpeg support or post-decode downscale).
    // TODO: Add onQueryYUVAInfo / onGetYUVAPlanes for GPU-accelerated YUV
    //   decode (requires zune-jpeg raw plane output).
    // NOTE: Progressive scan limit (100) is enforced by zune-jpeg's default
    //   DecoderOptions, matching Blink's ProgressMonitor.
    // NOTE: APP11 (C2PA/JUMBF) segments are captured by the Rust segment
    //   scanner and forwarded to SkJpegMetadataDecoder automatically.
};

#endif  // SkJpegRustCodec_DEFINED
