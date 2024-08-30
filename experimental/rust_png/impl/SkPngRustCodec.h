/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustCodec_DEFINED
#define SkPngRustCodec_DEFINED

#include <memory>
#include <vector>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "src/codec/SkPngCodecBase.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkFrame;
class SkStream;
template <typename T> class SkSpan;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party `png` crate (PNG decompression and decoding implemented in
//   Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space
//   transformations implemented in C++).
class SkPngRustCodec : public SkPngCodecBase {
public:
    static std::unique_ptr<SkPngRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    // `public` to support `std::make_unique<SkPngRustCodec>(...)`.
    SkPngRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_png::Reader>);

    ~SkPngRustCodec() override;

private:
    struct DecodingState {
        SkSpan<uint8_t> dst;
        size_t dstRowSize;  // in bytes.
    };

    // Helper for validating parameters of `onGetPixels` and/or
    // `onStartIncrementalDecode`.  If `kSuccess` is returned then
    // `decodingState` output parameter got populated.
    Result startDecoding(const SkImageInfo& dstInfo,
                         void* pixels,
                         size_t rowBytes,
                         const Options& options,
                         DecodingState* decodingState);

    // Helper for row-by-row decoding which is used from `onGetPixels` and/or
    // `onIncrementalDecode`.
    Result incrementalDecode(DecodingState& decodingState, int* rowsDecoded);

    // Temporary helper for *non*-row-by-row decoding of interlaced images.
    //
    // TODO(https://crbug.com/356923435): Remove this method after implementing
    // row-by-row decoding of interlaced images (see WIP CL at
    // http://review.skia.org/894576).
    Result decodeInterlacedImage(DecodingState& decodingState);

    // SkCodec overrides:
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* pixels,
                       size_t rowBytes,
                       const Options&,
                       int* rowsDecoded) override;
    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* pixels,
                                    size_t rowBytes,
                                    const Options&) override;
    Result onIncrementalDecode(int* rowsDecoded) override;
    bool onGetFrameInfo(int, FrameInfo*) const override;

    // SkPngCodecBase overrides:
    std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() override;
    std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() override;

    rust::Box<rust_png::Reader> fReader;

    std::optional<DecodingState> fIncrementalDecodingState;

    class PngFrame;
    std::vector<PngFrame> fFrames;

    size_t fNumOfFullyReceivedFrames = 0;
};

#endif  // SkPngRustCodec_DEFINED
