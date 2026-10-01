/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkIcoRustCodec_DEFINED
#define SkIcoRustCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "src/codec/SkFrameHolder.h"

#include <cstddef>
#include <memory>
#include <vector>

class SkSampler;
class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;

/**
 * ICO codec implementation using Rust-based PNG and BMP decoders.
 *
 * This class mirrors SkIcoCodec but delegates embedded image decoding to:
 * - SkPngRustCodec for PNG images
 * - SkBmpRustCodec for BMP images
 *
 * ICO files are container formats that can hold multiple images at different
 * resolutions. This codec parses the ICO directory and creates appropriate
 * embedded codecs for each image.
 */
class SkIcoRustCodec : public SkCodec {
public:
    static bool IsIco(const void*, size_t);

    /**
     * Assumes IsIco was called and returned true.
     * Creates an ICO decoder using Rust-based codecs for embedded images.
     * Reads enough of the stream to determine the image format.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    SkISize onGetScaledDimensions(float desiredScale) const override;

    /**
     * Initiates the ICO decode.
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kICO;
    }

    // Chromium's IcoRustImageDecoder enumerates ICO entries through SkCodec's
    // frame APIs. Each entry is independent and updates its native-sized rect.
    int onGetFrameCount() override;
    bool onGetFrameInfo(int index, FrameInfo* info) const override;

    SkScanlineOrder onGetScanlineOrder() const override;

    // ICO entries are alternative representations of one logical image, so any
    // embedded entry's native dimensions are a supported decode size.
    bool onDimensionsSupported(const SkISize&) override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override {
        // This will be checked by the embedded codec.
        return true;
    }

    // Handled by the embedded codec.
    bool usesColorXform() const override { return false; }

private:
    // Incremental decoding is delegated to the embedded PNG/BMP codecs (both of
    // which support it) in `onStartIncrementalDecode`/`onIncrementalDecode`.
    // Without this override the base class defaults to `false` and
    // `SkCodec::startIncrementalDecode` returns `kUnimplemented`, which breaks
    // clients such as Blink's `SkiaImageDecoderBase`.
    bool onSupportsIncrementalDecode(const SkImageInfo&) override { return true; }

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
            const SkCodec::Options&) override;

    Result onIncrementalDecode(int* rowsDecoded) override;

    SkSampler* getSampler(bool createIfNecessary) override;

    /**
     * Searches fEmbeddedImages for a codec that matches requestedSize.
     * The search starts at startIndex and ends when an appropriate codec
     * is found, or we have reached the end of the array.
     *
     * @return the index of the matching codec or -1 if there is no
     *         matching codec between startIndex and the end of
     *         the array.
     */
    int chooseCodec(const SkISize& requestedSize, int startIndex);

    /**
     * Common codec selection logic for onGetPixels and onStartIncrementalDecode.
     *
     * A nonzero frame index selects that entry authoritatively. Frame zero also
     * serves as SkCodec's default option, so it retains dimension-based
     * selection for callers that choose an ICO representation by output size.
     *
     * fn signature: Result fn(SkCodec* codec, int codecIndex, const Options& embeddedOpts)
     * where embeddedOpts has fFrameIndex reset to 0.
     */
    template <typename Fn>
    Result selectAndDecode(const SkISize& dims, const Options& opts, Fn fn);

    /**
     * Bundles an embedded image's decoder with its optional AND-mask payload so
     * the two can never drift out of sync.
     */
    struct EmbeddedImage {
        std::unique_ptr<SkCodec> fCodec;
        // Raw BMP entry data for AND mask post-processing.
        // Non-null for BMP entries, null for PNG entries.
        sk_sp<const SkData> fBmpEntryData;
        SkISize fReportedFrameSize;
    };

    /**
     * Constructor called by MakeFromStream.
     * @param embeddedImages decoder + AND-mask payload for each embedded image,
     *        ordered by descending quality; takes ownership
     */
    SkIcoRustCodec(SkEncodedInfo&& info,
                   std::unique_ptr<SkStream>,
                   std::vector<EmbeddedImage> embeddedImages);

    const SkFrameHolder* getFrameHolder() const override { return &fFrameHolder; }

    class Frame final : public SkFrame {
    public:
        Frame(int index, const SkImageInfo& info, SkEncodedInfo::Alpha alpha);

    protected:
        SkEncodedInfo::Alpha onReportedAlpha() const override { return fReportedAlpha; }

    private:
        SkEncodedInfo::Alpha fReportedAlpha;
    };

    class FrameHolder final : public SkFrameHolder {
    public:
        void setScreenSize(int width, int height) {
            fScreenWidth = width;
            fScreenHeight = height;
        }

        void appendFrame(int index, const SkImageInfo& info, SkEncodedInfo::Alpha alpha) {
            fFrames.emplace_back(index, info, alpha);
        }

    protected:
        const SkFrame* onGetFrame(int index) const override {
            return index >= 0 && static_cast<size_t>(index) < fFrames.size()
                           ? &fFrames[index]
                           : nullptr;
        }

    private:
        std::vector<Frame> fFrames;
    };

    // One record per embedded image keeps the codec and its AND-mask payload in
    // lockstep.
    std::vector<EmbeddedImage> fEmbeddedImages;
    FrameHolder fFrameHolder;

    // fCurrCodec is owned by this class, but should not be an
    // std::unique_ptr. It will be deleted by the destructor of fEmbeddedImages.
    SkCodec* fCurrCodec;

    // Saved for AND mask application during incremental decode
    void* fIncrementalDst = nullptr;
    size_t fIncrementalRowBytes = 0;
    SkImageInfo fIncrementalDstInfo = SkImageInfo::MakeUnknown();
    // AND-mask payload for the in-progress incremental decode, or null when the
    // active entry is a PNG (or no mask applies). Held by ref so it stays valid
    // for the duration of the decode without indexing back into fEmbeddedImages.
    sk_sp<const SkData> fIncrementalBmpEntryData;

    using INHERITED = SkCodec;
};

#endif  // SkIcoRustCodec_DEFINED
