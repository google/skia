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
    /**
     * Initiates the ICO decode.
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kICO;
    }

    // Frame support - each embedded image is a separate frame
    int onGetFrameCount() override;
    bool onGetFrameInfo(int index, FrameInfo* info) const override;

    SkScanlineOrder onGetScanlineOrder() const override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override {
        // This will be checked by the embedded codec.
        return true;
    }

    // Handled by the embedded codec.
    bool usesColorXform() const override { return false; }

private:
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
     * If opts.fFrameIndex is valid and matches dims, calls fn on that codec and
     * returns unconditionally. Otherwise loops through dimension-matched codecs,
     * stopping on kSuccess or kIncompleteInput.
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
    };

    /**
     * Constructor called by MakeFromStream.
     * @param embeddedImages decoder + AND-mask payload for each embedded image,
     *        ordered by decreasing quality; takes ownership
     */
    SkIcoRustCodec(SkEncodedInfo&& info,
                   std::unique_ptr<SkStream>,
                   std::vector<EmbeddedImage> embeddedImages);

    const SkFrameHolder* getFrameHolder() const override {
        return &fFrameHolder;
    }

    /**
     * Frame class for ICO - each embedded image is a frame.
     * ICO frames are always independent (no inter-frame dependencies).
     */
    class Frame : public SkFrame {
    public:
        Frame(int index, int width, int height, SkEncodedInfo::Alpha alpha)
            : SkFrame(index)
            , fWidth(width)
            , fHeight(height)
            , fReportedAlpha(alpha) {
            // ICO frames don't depend on previous frames
            this->setRequiredFrame(SkCodec::kNoFrame);
            this->setHasAlpha(alpha != SkEncodedInfo::Alpha::kOpaque_Alpha);
            // Set the frame rect to cover the full image
            this->setXYWH(0, 0, width, height);
        }

        int width() const { return fWidth; }
        int height() const { return fHeight; }

    protected:
        SkEncodedInfo::Alpha onReportedAlpha() const override {
            return fReportedAlpha;
        }

    private:
        int fWidth;
        int fHeight;
        SkEncodedInfo::Alpha fReportedAlpha;
    };

    /**
     * FrameHolder for ICO - holds info about all embedded images.
     */
    class FrameHolder : public SkFrameHolder {
    public:
        FrameHolder() = default;
        ~FrameHolder() override = default;

        void setScreenSize(int w, int h) {
            fScreenWidth = w;
            fScreenHeight = h;
        }

        void appendFrame(int index, int width, int height, SkEncodedInfo::Alpha alpha) {
            fFrames.emplace_back(index, width, height, alpha);
        }

        int size() const { return static_cast<int>(fFrames.size()); }

        const Frame* frame(int i) const {
            if (i < 0 || i >= static_cast<int>(fFrames.size())) {
                return nullptr;
            }
            return &fFrames[i];
        }

    protected:
        const SkFrame* onGetFrame(int i) const override {
            return frame(i);
        }

    private:
        std::vector<Frame> fFrames;
    };

    // Single source of truth: one record per embedded image keeps the codec and
    // its AND-mask payload in lockstep, and fFrameHolder is derived from it.
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
