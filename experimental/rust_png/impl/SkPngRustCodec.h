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
#include "src/codec/SkFrameHolder.h"
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
class SkPngRustCodec final : public SkPngCodecBase {
public:
    static std::unique_ptr<SkPngRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    // `public` to support `std::make_unique<SkPngRustCodec>(...)`.
    SkPngRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_png::Reader>);

    ~SkPngRustCodec() override;

private:
    struct DecodingState {
        // `fDst` is based on `pixels` passed to `onGetPixels` or
        // `onStartIncrementalDecode`.  For interlaced and non-interlaced
        // images, `startDecoding` initializes `fDst` to start at the (0,0)
        // (top-left) pixel of the current frame (which may be offset from
        // `pixels` if the current frame is a sub-rect of the full image).
        // After decoding a non-interlaced row this moves (by `fDstRowSize`) to
        // the next row.
        SkSpan<uint8_t> fDst;

        // Size of a row (in bytes) in the full image.  Based on `rowBytes`
        // passed to `onGetPixels` or `onStartIncrementalDecode`.
        size_t fDstRowSize = 0;

        // Stashed `dstInfo.bytesPerPixel()`
        size_t fBytesPerPixel = 0;

        // Index (in `fFrameHolder`) of the frame being currently decoded.
        size_t fFrameIndex = 0;
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
    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;
    const SkFrameHolder* getFrameHolder() const override;
    std::unique_ptr<SkStream> getEncodedData() const override;

    // SkPngCodecBase overrides:
    std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() override;
    std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() override;

    rust::Box<rust_png::Reader> fReader;
    const std::unique_ptr<SkStream> fPrivStream;

    std::optional<DecodingState> fIncrementalDecodingState;

    class FrameHolder final : public SkFrameHolder {
    public:
        FrameHolder(int width, int height);
        ~FrameHolder() override;

        FrameHolder(const FrameHolder&) = delete;
        FrameHolder(FrameHolder&&) = delete;
        FrameHolder& operator=(const FrameHolder&) = delete;
        FrameHolder& operator=(FrameHolder&&) = delete;

        size_t size() const;

        void appendNewFrame(const rust_png::Reader& reader, const SkEncodedInfo& info);
        void markFrameAsFullyReceived(size_t index);
        bool getFrameInfo(int index, FrameInfo* info) const;

    private:
        const SkFrame* onGetFrame(int i) const override;
        void setLastFrameInfoFromCurrentFctlChunk(const rust_png::Reader& reader);

        class PngFrame;
        std::vector<PngFrame> fFrames;
    };
    FrameHolder fFrameHolder;
};

#endif  // SkPngRustCodec_DEFINED
