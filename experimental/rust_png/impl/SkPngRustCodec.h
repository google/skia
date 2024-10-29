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
        // After decoding a non-interlaced row this moves (by `fDstRowStride`)
        // to the next row.
        SkSpan<uint8_t> fDst;

        // Size of a row (in bytes) in the full image.  Based on `rowBytes`
        // passed to `onGetPixels` or `onStartIncrementalDecode`.
        size_t fDstRowStride = 0;

        // Size of a row (in bytes) in the current frame.
        size_t fDstRowSize = 0;

        // Index (in `fFrameHolder`) of the frame being currently decoded.
        size_t fFrameIndex = 0;

        // Intermediate buffer that holds color-transformed pixels that are
        // ready to be blended with the destination.  Used only when this frame
        // uses `SkCodecAnimation::Blend::kSrcOver`.  For interlaced images this
        // buffer holds the whole frame; otherwise it holds only a single row.
        std::vector<uint8_t> fPreblendBuffer;

        // Stashed subset of `dstInfo`.
        SkColorType fDstColor = kUnknown_SkColorType;
        SkAlphaType fDstAlpha = kUnknown_SkAlphaType;
        uint8_t fDstBytesPerPixel = 0;
    };

    // Helper for validating parameters of `onGetPixels` and/or
    // `onStartIncrementalDecode`.  If `kSuccess` is returned then
    // `decodingState` output parameter got populated.
    Result startDecoding(const SkImageInfo& dstInfo,
                         void* pixels,
                         size_t rowBytes,
                         const Options& options,
                         DecodingState* decodingState);

    // Helper for taking a decoded interlaced `srcRow`, applying color
    // transformations, and then expanding it into the `frame`.
    void expandDecodedInterlacedRow(SkSpan<uint8_t> dstFrame,
                                    SkSpan<const uint8_t> srcRow,
                                    const DecodingState& decodingState);

    // Helper for row-by-row decoding which is used from `onGetPixels` and/or
    // `onIncrementalDecode`.
    Result incrementalDecode(DecodingState& decodingState, int* rowsDecoded);

    // Helper for reading until the start of the next `fdAT` sequence.
    Result readToStartOfNextFrame();

    // Helper for seeking to the start of image data for the given frame.
    Result seekToStartOfFrame(int index);

    // The number of frames calculated based on 1) the presence, and 2) the
    // contents of an `acTL` chunk.  "raw" in the sense that it reports all the
    // frames, while `SkCodec::getFrameCount` and
    // `SkPngRustCodec::onGetFrameCount` only report frames for which we have
    // successfully populated `fFrameHolder` with frame info parsed from `IHDR`
    // and/or `fcTL` chunks.
    int getRawFrameCount() const;

    // Attempts to read through the input stream to parse the additional `fcTL`
    // chunks.
    Result parseAdditionalFrameInfos();

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

    // `-1` means that `IDAT` is not part of animation and wasn't skipped yet.
    int fFrameAtCurrentStreamPosition = -1;
    bool fStreamIsPositionedAtStartOfFrameData = false;
    const std::unique_ptr<SkStream> fPrivStream;
    // TODO(https://crbug.com/371060427): Once fast seeking is available, we can
    // remove the field that tracks the stream length.
    std::optional<size_t> fStreamLengthDuringLastCallToParseAdditionalFrameInfos;

    std::optional<DecodingState> fIncrementalDecodingState;

    class FrameHolder final : public SkFrameHolder {
    public:
        FrameHolder(int width, int height);
        ~FrameHolder() override;

        FrameHolder(const FrameHolder&) = delete;
        FrameHolder(FrameHolder&&) = delete;
        FrameHolder& operator=(const FrameHolder&) = delete;
        FrameHolder& operator=(FrameHolder&&) = delete;

        // Returning an `int` (rather than `size_t`) for easier interop with
        // other parts of the SkCodec API.
        int size() const;

        Result appendNewFrame(const rust_png::Reader& reader, const SkEncodedInfo& info);
        void markFrameAsFullyReceived(size_t index);
        bool getFrameInfo(int index, FrameInfo* info) const;

    private:
        class PngFrame;

        const SkFrame* onGetFrame(int unverifiedIndex) const override;
        Result setFrameInfoFromCurrentFctlChunk(const rust_png::Reader& reader,
                                                PngFrame* out_frame);

        std::vector<PngFrame> fFrames;
    };
    FrameHolder fFrameHolder;

    // Whether there may still be additional `fcTL` chunks to discover and parse.
    //
    // `true` if the stream hasn't been fully received (i.e. only
    // `kIncompleteInput` errors so far, no hard errors) and `fFrameHolder`
    // doesn't yet contain frame info for all `num_frames` declared in an `acTL`
    // chunk.
    bool fCanParseAdditionalFrameInfos = true;
};

#endif  // SkPngRustCodec_DEFINED
