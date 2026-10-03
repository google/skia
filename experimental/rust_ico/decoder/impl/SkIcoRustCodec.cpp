/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"

#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "rust/common/SkStreamAdapter.h"
#include "src/codec/SkBmpRustCodec.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkPngRustCodec.h"
#include "src/core/SkStreamPriv.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include "modules/skcms/skcms.h"

using namespace skia_private;

class SkSampler;

static bool dimensions_fit_within(const SkISize& dimensions, const SkISize& bounds) {
    return dimensions.width() <= bounds.width() && dimensions.height() <= bounds.height();
}

static bool clear_canvas(const SkImageInfo& info,
                         void* dst,
                         size_t rowBytes,
                         SkCodec::ZeroInitialized zeroInitialized) {
    return zeroInitialized == SkCodec::kYes_ZeroInitialized ||
           SkPixmap(info, dst, rowBytes).erase(SK_ColorTRANSPARENT);
}

static bool apply_and_mask(void* dst,
                           size_t rowBytes,
                           const SkImageInfo& info,
                           const sk_sp<const SkData>& entryData) {
    const size_t pixelBytes = info.computeByteSize(rowBytes);
    if (SkImageInfo::ByteSizeOverflowed(pixelBytes)) {
        return false;
    }
    rust::Slice<uint8_t> pixelSlice(static_cast<uint8_t*>(dst), pixelBytes);
    rust::Slice<const uint8_t> dataSlice(static_cast<const uint8_t*>(entryData->data()),
                                         entryData->size());
    rust_ico::apply_and_mask(
            pixelSlice, dataSlice, info.width(), info.height(), info.bytesPerPixel(), rowBytes);
    return true;
}

// Checks the start of the stream to see if the image is an ICO or CUR.
bool SkIcoRustCodec::IsIco(const void* buffer, size_t bytesRead) {
    // Create a memory stream from the peeked buffer data
    auto data = SkData::MakeWithoutCopy(buffer, bytesRead);
    SkMemoryStream stream(std::move(data));
    rust::stream::SkStreamAdapter inputAdapter(&stream);
    return rust_ico::is_ico(inputAdapter);
}

std::unique_ptr<SkCodec> SkIcoRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                        Result* result) {
    // Handle nullptr result parameter by using local storage
    Result resultStorage;
    if (result == nullptr) {
        result = &resultStorage;
    }

    if (!stream) {
        SkCodecPrintf("Error: ICO stream is null.\n");
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    // ICO parsing needs random access (to detect PNG vs BMP per entry) and we
    // carve each embedded image out of the full buffer, so pull everything into
    // a single SkData up front. If the stream is already backed by SkData we
    // share it without copying; otherwise we read it out once. Every embedded
    // entry is then a zero-copy subset of this buffer (see shareSubset below),
    // and the subsets keep this SkData alive for as long as any entry codec.
    sk_sp<const SkData> data = stream->getData();
    if (!data) {
        data = SkStreamPriv::CopyStreamToData(stream.get());
        if (!data) {
            SkCodecPrintf("Error: CopyStreamToData returned null.\n");
            *result = kIncompleteInput;
            return nullptr;
        }
    }

    if (data->size() == 0) {
        SkCodecPrintf("Error: ICO stream is empty.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    const size_t totalSize = data->size();

    // Parse ICO directory using Rust via a seekable view over the buffer.
    SkMemoryStream dataStream(data);
    rust::stream::SkStreamAdapter inputAdapter(&dataStream);
    rust::Box<rust_ico::DirectoryResult> directoryResult = rust_ico::parse_directory(inputAdapter);

    // Map Rust parse result to SkCodec::Result
    switch (directoryResult->status()) {
        case rust_ico::DirectoryParseResult::Success:
            break;
        case rust_ico::DirectoryParseResult::InsufficientData:
        case rust_ico::DirectoryParseResult::TruncatedDirectory:
            SkCodecPrintf("Error: ICO data truncated.\n");
            *result = kIncompleteInput;
            return nullptr;
        case rust_ico::DirectoryParseResult::InvalidSignature:
            SkCodecPrintf("Error: Invalid ICO signature.\n");
            *result = kInvalidInput;
            return nullptr;
        case rust_ico::DirectoryParseResult::NoImages:
            SkCodecPrintf("Error: No images embedded in ICO.\n");
            *result = kInvalidInput;
            return nullptr;
    }

    const uint32_t numImages = directoryResult->image_count();

    // Determine a stable canvas that contains every directory entry (a width or
    // height byte of 0 means 256 per the ICO spec). Unlike legacy SkIcoCodec,
    // this codec exposes each entry as a frame, whose fFrameRect must be
    // contained by dimensions(). Taking the component-wise maxima preserves
    // that invariant for mixed-aspect entries where the largest-area entry
    // would not contain every other entry.
    //
    // Every parsed directory entry contributes even if its payload is skipped
    // later. The canvas must be established before payload decodability is
    // known and remain stable as additional data becomes available.
    //
    // This matters for clients that decode progressively. Blink's deferred image
    // pipeline locks the image size from the first successful parse and allocates
    // buffers at that size. If the size were derived from the largest entry we
    // can currently *decode*, it would grow as more data arrived (e.g. a small
    // entry is present first, a larger one only later), and the previously
    // allocated buffer would then be too small -- the later decode overflows it
    // and crashes the renderer. Unlike SkIcoCodec on partial input, we preserve
    // Blink's stable directory-derived size.
    //
    // selectAndDecode() always dimension-matches an embedded codec to the
    // requested dstInfo, so reporting a directory size that no decodable entry
    // matches can only cause a decode to fail gracefully -- never an overflow.
    int dirMaxWidth = 0, dirMaxHeight = 0;
    for (uint32_t i = 0; i < numImages; i++) {
        rust_ico::IcoEntry dirEntry = directoryResult->get_entry(i);
        int w = dirEntry.width == 0 ? 256 : dirEntry.width;
        int h = dirEntry.height == 0 ? 256 : dirEntry.height;
        dirMaxWidth = std::max(dirMaxWidth, w);
        dirMaxHeight = std::max(dirMaxHeight, h);
    }

    // Default Result, if no valid embedded codecs are found.
    *result = kInvalidInput;

    // Temporary storage while constructing each embedded codec.
    struct CodecEntry {
        std::unique_ptr<SkCodec> codec;
        sk_sp<const SkData> bmpEntryData;  // Raw entry data for BMP (nullptr for PNG)
        SkISize reportedFrameSize;
        SkIPoint hotSpot;
        uint16_t bitCount;
    };
    std::vector<CodecEntry> entries;
    entries.reserve(numImages);
    bool hasIncompleteEntry = false;

    struct PayloadBoundary {
        size_t size;
        bool truncated;
    };
    std::vector<PayloadBoundary> payloadBoundaries(numImages);
    for (uint32_t groupStart = 0; groupStart < numImages;) {
        const uint32_t offset = directoryResult->get_entry(groupStart).offset;
        uint32_t groupEnd = groupStart + 1;
        while (groupEnd < numImages && directoryResult->get_entry(groupEnd).offset == offset) {
            ++groupEnd;
        }

        const size_t availableSize = offset < totalSize ? totalSize - offset : 0;
        const bool hasNextOffset = groupEnd < numImages;
        const uint32_t nextOffset = hasNextOffset ? directoryResult->get_entry(groupEnd).offset : 0;
        const size_t size = hasNextOffset ? std::min<size_t>(nextOffset - offset, availableSize)
                                          : availableSize;
        const bool hasCompleteNextOffset = hasNextOffset && nextOffset <= totalSize;
        for (uint32_t i = groupStart; i < groupEnd; ++i) {
            const bool truncated =
                    !hasCompleteNextOffset && directoryResult->get_entry(i).size > availableSize;
            payloadBoundaries[i] = {size, truncated};
        }
        groupStart = groupEnd;
    }

    // Construct a candidate codec for each of the embedded images
    // Entries are already sorted by offset in Rust for proper size calculation
    for (uint32_t i = 0; i < numImages; i++) {
        rust_ico::IcoEntry entry = directoryResult->get_entry(i);
        uint32_t offset = entry.offset;

        if (offset >= totalSize) {
            SkCodecPrintf("Warning: could not skip to ico offset.\n");
            hasIncompleteEntry = true;
            break;
        }

        // Directory size fields are sometimes incorrect, so bound each payload
        // by the next distinct entry offset, or EOF for the final offset group.
        // Entries at the same offset share the same physical payload.
        const PayloadBoundary& boundary = payloadBoundaries[i];

        // Skip entries whose payload is only partially present. Handing a
        // truncated image to the embedded decoder would either fail or, worse,
        // silently produce a corrupt frame; instead we drop the entry and let
        // the caller see kIncompleteInput below.
        if (boundary.truncated) {
            SkCodecPrintf("Warning: ico entry truncated; skipping.\n");
            hasIncompleteEntry = true;
            continue;
        }

        // Carve this entry out of the buffer without copying; the shared subset
        // keeps the parent SkData alive for as long as the entry stream (and the
        // codec built from it) lives.
        sk_sp<const SkData> entryData = data->shareSubset(offset, boundary.size);
        auto entryStream = std::make_unique<SkMemoryStream>(entryData);

        // Create codec for the embedded image.
        // PNG entries delegate to SkPngRustCodec for full color profile and EXIF support.
        // BMP entries delegate to SkBmpRustCodec with ICO-aware metadata parsing
        // (no file header, halved height, alpha channel via image crate).
        Result entryResult;
        std::unique_ptr<SkCodec> codec;
        sk_sp<const SkData> bmpData;
        if (entry.format == rust_ico::EmbeddedFormat::Png) {
            codec = SkPngRustCodec::MakeFromStream(std::move(entryStream), &entryResult);
        } else {
            bmpData = entryData;
            codec = SkBmpRustCodec::MakeFromStream(
                    std::move(entryStream), &entryResult, SkBmpRustCodec::StreamType::kICO);
        }
        if (!codec && entryResult == kIncompleteInput) {
            hasIncompleteEntry = true;
        }

        if (nullptr != codec) {
            const SkISize directorySize = SkISize::Make(entry.width == 0 ? 256 : entry.width,
                                                        entry.height == 0 ? 256 : entry.height);
            const SkIPoint hotSpot = SkIPoint::Make(entry.hotspot_x, entry.hotspot_y);

            // Store codec with its bit count for sorting
            entries.push_back({
                    std::move(codec),
                    std::move(bmpData),
                    directorySize,
                    hotSpot,
                    entry.bit_count,
            });
        }
    }

    if (entries.empty()) {
        if (hasIncompleteEntry) {
            *result = kIncompleteInput;
        }
        SkCodecPrintf("Error: could not find any valid embedded ico codecs.\n");
        return nullptr;
    }

    const bool isCursor = directoryResult->file_type() == rust_ico::IcoFileType::Cursor;
    if (isCursor && hasIncompleteEntry) {
        *result = kIncompleteInput;
        return nullptr;
    }

    const auto fitsReportedFrame = [](const CodecEntry& entry) {
        return dimensions_fit_within(entry.codec->dimensions(), entry.reportedFrameSize);
    };

    // Blink treats entry zero as the default representation. Keep entries whose
    // payload fits their reported frame ahead of metadata-only oversized
    // entries so frame zero remains decodable, then preserve the existing
    // payload-area/bit-depth ordering within each group.
    std::stable_sort(entries.begin(),
                     entries.end(),
                     [fitsReportedFrame](const CodecEntry& a, const CodecEntry& b) {
                         const bool aFits = fitsReportedFrame(a);
                         const bool bFits = fitsReportedFrame(b);
                         if (aFits != bFits) {
                             return aFits;
                         }
                         const SkISize aDims = a.codec->dimensions();
                         const SkISize bDims = b.codec->dimensions();
                         const int64_t aArea = aDims.area();
                         const int64_t bArea = bDims.area();
                         return aArea != bArea ? aArea > bArea : a.bitCount > b.bitCount;
                     });

    if (!fitsReportedFrame(entries.front()) && hasIncompleteEntry) {
        *result = kIncompleteInput;
        return nullptr;
    }

    // Bundling the codec with its AND-mask payload keeps the two from drifting
    // out of sync when entries are reordered.
    std::vector<EmbeddedImage> embeddedImages;
    embeddedImages.reserve(entries.size());
    for (auto& entry : entries) {
        embeddedImages.push_back({std::move(entry.codec),
                                  std::move(entry.bmpEntryData),
                                  entry.reportedFrameSize,
                                  entry.hotSpot});
    }

    // Canvas-fitting entries sort first, so frame zero supplies the container's
    // decode metadata whenever decoding is possible. If all entries are
    // oversized, frame zero still supplies metadata but decoding remains
    // unsupported. In both cases, dimensions come from the stable directory
    // canvas rather than payload availability.
    auto maxInfo = embeddedImages.front().fCodec->getEncodedInfo().copy();
    if (dirMaxWidth != maxInfo.width() || dirMaxHeight != maxInfo.height()) {
        maxInfo = SkEncodedInfo::Make(dirMaxWidth,
                                      dirMaxHeight,
                                      maxInfo.color(),
                                      maxInfo.alpha(),
                                      maxInfo.bitsPerComponent());
    }
    // Report kSuccess once at least one embedded codec supplies stable metadata.
    // If no entry fits the canvas, decode calls return kInvalidScale; the
    // incomplete-input case returned above so clients can retry with more data.
    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkIcoRustCodec(
            std::move(maxInfo), std::move(stream), std::move(embeddedImages), isCursor));
}

SkIcoRustCodec::SkIcoRustCodec(SkEncodedInfo&& info,
                               std::unique_ptr<SkStream> stream,
                               std::vector<EmbeddedImage> embeddedImages,
                               bool isCursor)
        // The source skcms_PixelFormat will not be used. The embedded
        // codec's will be used instead.
        : INHERITED(std::move(info), skcms_PixelFormat(), std::move(stream))
        , fEmbeddedImages(std::move(embeddedImages))
        , fIsCursor(isCursor)
        , fCurrCodec(nullptr) {
    fFrameHolder.setScreenSize(this->dimensions().width(), this->dimensions().height());
    for (int i = 0; i < SkToInt(fEmbeddedImages.size()); ++i) {
        const EmbeddedImage& image = fEmbeddedImages[i];
        const SkImageInfo frameInfo =
                image.fCodec->getInfo().makeDimensions(image.fReportedFrameSize);
        fFrameHolder.appendFrame(i, frameInfo, image.fCodec->getEncodedInfo().alpha());
    }
}

bool SkIcoRustCodec::getHotSpot(int frameIndex, SkIPoint* hotSpot) const {
    if (!fIsCursor || !hotSpot || frameIndex < 0 || frameIndex >= SkToInt(fEmbeddedImages.size())) {
        return false;
    }
    *hotSpot = fEmbeddedImages[frameIndex].fHotSpot;
    return true;
}

SkIcoRustCodec::Frame::Frame(int index, const SkImageInfo& info, SkEncodedInfo::Alpha alpha)
        : SkFrame(index), fReportedAlpha(alpha) {
    this->setRequiredFrame(SkCodec::kNoFrame);
    this->setHasAlpha(alpha != SkEncodedInfo::Alpha::kOpaque_Alpha);
    this->setBlend(SkCodecAnimation::Blend::kSrc);
    this->setXYWH(0, 0, info.width(), info.height());
}

int SkIcoRustCodec::onGetFrameCount() { return SkToInt(fEmbeddedImages.size()); }

bool SkIcoRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    const SkFrame* frame = fFrameHolder.getFrame(index);
    if (!frame) {
        return false;
    }
    if (info) {
        frame->fillIn(info, true);
    }
    return true;
}

SkISize SkIcoRustCodec::onGetScaledDimensions(float desiredScale) const {
    const int origWidth = this->dimensions().width();
    const int origHeight = this->dimensions().height();
    const float desiredSize = desiredScale * origWidth * origHeight;
    float minError = std::numeric_limits<float>::max();
    size_t minIndex = fEmbeddedImages.size();
    for (size_t i = 0; i < fEmbeddedImages.size(); i++) {
        const EmbeddedImage& image = fEmbeddedImages[i];
        const SkISize dimensions = image.fCodec->dimensions();
        if (!dimensions_fit_within(dimensions, image.fReportedFrameSize)) {
            continue;
        }
        const float error = SkTAbs(static_cast<float>(dimensions.area()) - desiredSize);
        if (error < minError) {
            minError = error;
            minIndex = i;
        }
    }

    return minIndex < fEmbeddedImages.size() ? fEmbeddedImages[minIndex].fCodec->dimensions()
                                             : this->dimensions();
}

int SkIcoRustCodec::chooseCodec(const SkISize& requestedSize, int startIndex) {
    SkASSERT_RELEASE(startIndex >= 0);

    for (int i = startIndex; i < SkToInt(fEmbeddedImages.size()); i++) {
        const EmbeddedImage& image = fEmbeddedImages[i];
        const SkISize dimensions = image.fCodec->dimensions();
        if (dimensions_fit_within(dimensions, image.fReportedFrameSize) &&
            dimensions == requestedSize) {
            return i;
        }
    }

    return -1;
}

/*
 * Any embedded entry's dimensions are a valid decode size. ICO entries are
 * alternative representations, selected by the requested destination size.
 */
bool SkIcoRustCodec::onDimensionsSupported(const SkISize& dim) {
    return this->chooseCodec(dim, 0) >= 0;
}

/*
 * Common codec selection logic shared by onGetPixels and onStartIncrementalDecode.
 * See selectAndDecode declaration in the header for details.
 */
template <typename Fn>
SkCodec::Result SkIcoRustCodec::selectAndDecode(const SkISize& dims, const Options& opts, Fn fn) {
    // Each embedded image is a separate codec with its own single frame (index 0),
    // so reset fFrameIndex when delegating.
    Options embeddedOpts = opts;
    embeddedOpts.fFrameIndex = 0;
    embeddedOpts.fPriorFrame = kNoFrame;

    if (opts.fFrameIndex < 0) {
        return kInvalidParameters;
    }
    if (opts.fFrameIndex > 0) {
        if (opts.fFrameIndex >= SkToInt(fEmbeddedImages.size())) {
            return kIncompleteInput;
        }
        const EmbeddedImage& image = fEmbeddedImages[opts.fFrameIndex];
        const SkISize entryDims = image.fCodec->dimensions();
        if (!dimensions_fit_within(entryDims, image.fReportedFrameSize) ||
            !dimensions_fit_within(entryDims, dims)) {
            return kInvalidScale;
        }
        return fn(image.fCodec.get(), opts.fFrameIndex, embeddedOpts);
    }

    int index = this->chooseCodec(dims, 0);
    if (index < 0 && dims == this->dimensions()) {
        const EmbeddedImage& fallback = fEmbeddedImages.front();
        if (dimensions_fit_within(fallback.fCodec->dimensions(), fallback.fReportedFrameSize)) {
            index = 0;
        }
    }
    if (index < 0) {
        return kInvalidScale;
    }
    Result lastResult = kInvalidScale;
    while (index >= 0) {
        lastResult = fn(fEmbeddedImages[index].fCodec.get(), index, embeddedOpts);
        if (lastResult == kSuccess || lastResult == kIncompleteInput) {
            return lastResult;
        }
        index = this->chooseCodec(dims, index + 1);
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return lastResult;
}

/*
 * Initiates the ICO decode using an embedded codec matching the requested
 * dimensions.
 */
SkCodec::Result SkIcoRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* dst,
                                            size_t dstRowBytes,
                                            const Options& opts,
                                            int* rowsDecoded) {
    fCurrCodec = nullptr;
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    return selectAndDecode(
            dstInfo.dimensions(),
            opts,
            [&](SkCodec* codec, int codecIndex, const Options& embeddedOpts) -> Result {
                const SkImageInfo embeddedInfo = dstInfo.makeDimensions(codec->dimensions());
                if (embeddedInfo.dimensions() != dstInfo.dimensions() &&
                    !clear_canvas(dstInfo, dst, dstRowBytes, opts.fZeroInitialized)) {
                    return kInvalidConversion;
                }
                Result result = codec->getPixels(embeddedInfo, dst, dstRowBytes, &embeddedOpts);
                if (result == kSuccess || result == kIncompleteInput) {
                    // Apply AND mask for BMP entries (non-32-bit BMPs have a transparency mask)
                    if (codecIndex < SkToInt(fEmbeddedImages.size()) &&
                        fEmbeddedImages[codecIndex].fBmpEntryData) {
                        if (!apply_and_mask(dst,
                                            dstRowBytes,
                                            embeddedInfo,
                                            fEmbeddedImages[codecIndex].fBmpEntryData)) {
                            return kInvalidParameters;
                        }
                    }
                    *rowsDecoded = dstInfo.height();
                }
                return result;
            });
}

SkCodec::Result SkIcoRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                         void* pixels,
                                                         size_t rowBytes,
                                                         const SkCodec::Options& options) {
    fCurrCodec = nullptr;
    fIncrementalBmpEntryData.reset();
    return selectAndDecode(
            dstInfo.dimensions(),
            options,
            [&](SkCodec* codec, int codecIndex, const Options& embeddedOpts) -> Result {
                const SkImageInfo embeddedInfo = dstInfo.makeDimensions(codec->dimensions());
                if (embeddedInfo.dimensions() != dstInfo.dimensions() &&
                    !clear_canvas(dstInfo, pixels, rowBytes, options.fZeroInitialized)) {
                    return kInvalidConversion;
                }
                Result r = codec->startIncrementalDecode(
                        embeddedInfo, pixels, rowBytes, &embeddedOpts);
                if (r == kSuccess) {
                    fCurrCodec = codec;
                    fIncrementalDst = pixels;
                    fIncrementalRowBytes = rowBytes;
                    fIncrementalDstInfo = embeddedInfo;
                    fIncrementalBmpEntryData = fEmbeddedImages[codecIndex].fBmpEntryData;
                }
                return r;
            });
}

SkCodec::Result SkIcoRustCodec::onIncrementalDecode(int* rowsDecoded) {
    SkASSERT_RELEASE(fCurrCodec);
    Result result = fCurrCodec->incrementalDecode(rowsDecoded);
    if (result == kSuccess || result == kIncompleteInput) {
        // Apply AND mask for BMP entries after incremental decode completes
        if (fIncrementalBmpEntryData) {
            if (!apply_and_mask(fIncrementalDst,
                                fIncrementalRowBytes,
                                fIncrementalDstInfo,
                                fIncrementalBmpEntryData)) {
                return kInvalidParameters;
            }
        }
    }
    return result;
}

SkCodec::SkScanlineOrder SkIcoRustCodec::onGetScanlineOrder() const {
    if (fCurrCodec) {
        return fCurrCodec->getScanlineOrder();
    }

    return INHERITED::onGetScanlineOrder();
}

SkSampler* SkIcoRustCodec::getSampler(bool createIfNecessary) {
    if (fCurrCodec) {
        return fCurrCodec->getSampler(createIfNecessary);
    }

    return nullptr;
}
