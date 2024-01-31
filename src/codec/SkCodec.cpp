/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"

#include "include/codec/SkCodecAnimation.h"
#include "include/codec/SkPixmapUtils.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h" // IWYU pragma: keep
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkNoDestructor.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkPixmapUtilsPriv.h"
#include "src/codec/SkSampler.h"

#include <string>
#include <string_view>
#include <utility>

#if !defined(SK_DISABLE_LEGACY_INIT_DECODERS)
#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkWbmpDecoder.h"
#include "include/private/base/SkOnce.h"

#ifdef SK_CODEC_DECODES_AVIF
#include "include/codec/SkAvifDecoder.h"
#endif

#ifdef SK_HAS_WUFFS_LIBRARY
#include "include/codec/SkGifDecoder.h"
#endif

#ifdef SK_HAS_HEIF_LIBRARY
#include "include/android/SkHeifDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_JPEG
#include "include/codec/SkJpegDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_JPEGXL
#include "include/codec/SkJpegxlDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_PNG
#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkPngDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_RAW
#include "include/codec/SkRawDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_WEBP
#include "include/codec/SkWebpDecoder.h"
#endif
#endif // !defined(SK_DISABLE_LEGACY_INIT_DECODERS)

namespace SkCodecs {
// A static variable inside a function avoids a static initializer.
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/static_initializers.md#removing-static-initializers
static std::vector<Decoder>* get_decoders_for_editing() {
    static SkNoDestructor<std::vector<Decoder>> decoders;
#if !defined(SK_DISABLE_LEGACY_INIT_DECODERS)
    static SkOnce once;
    once([] {
        if (decoders->empty()) {
#ifdef SK_CODEC_DECODES_PNG
            decoders->push_back(SkPngDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_JPEG
            decoders->push_back(SkJpegDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_WEBP
            decoders->push_back(SkWebpDecoder::Decoder());
#endif
#ifdef SK_HAS_WUFFS_LIBRARY
            decoders->push_back(SkGifDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_PNG
            decoders->push_back(SkIcoDecoder::Decoder());
#endif
            decoders->push_back(SkBmpDecoder::Decoder());
            decoders->push_back(SkWbmpDecoder::Decoder());
#ifdef SK_CODEC_DECODES_AVIF
            decoders->push_back(SkAvifDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_JPEGXL
            decoders->push_back(SkJpegxlDecoder::Decoder());
#endif
#ifdef SK_HAS_HEIF_LIBRARY
            decoders->push_back(SkHeifDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_RAW
            decoders->push_back(SkRawDecoder::Decoder());
#endif
        }
    });
#endif // !defined(SK_DISABLE_LEGACY_INIT_DECODERS)
    return decoders.get();
}

const std::vector<Decoder>& get_decoders() {
    auto decoders = get_decoders_for_editing();
    return *decoders;
}

void Register(Decoder d) {
    auto decoders = get_decoders_for_editing();
    for (size_t i = 0; i < decoders->size(); i++) {
        if ((*decoders)[i].id == d.id) {
            (*decoders)[i] = d;
            return;
        }
    }
    decoders->push_back(d);
}

bool HasDecoder(std::string_view id) {
    for (const SkCodecs::Decoder& decoder : get_decoders()) {
        if (decoder.id == id) {
            return true;
        }
    }
    return false;
}

}  // namespace SkCodecs

std::unique_ptr<SkCodec> SkCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, Result* outResult,
        SkPngChunkReader* chunkReader, SelectionPolicy selectionPolicy) {
    return MakeFromStream(std::move(stream), SkCodecs::get_decoders(), outResult,
                          chunkReader, selectionPolicy);
}
std::unique_ptr<SkCodec> SkCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, SkSpan<const SkCodecs::Decoder> decoders,
        Result* outResult, SkPngChunkReader* chunkReader, SelectionPolicy selectionPolicy) {
    Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }

    if (!stream) {
        *outResult = kInvalidInput;
        return nullptr;
    }

    if (selectionPolicy != SelectionPolicy::kPreferStillImage
            && selectionPolicy != SelectionPolicy::kPreferAnimation) {
        *outResult = kInvalidParameters;
        return nullptr;
    }

    constexpr size_t bytesToRead = MinBufferedBytesNeeded();

    char buffer[bytesToRead];
    size_t bytesRead = stream->peek(buffer, bytesToRead);

    // It is also possible to have a complete image less than bytesToRead bytes
    // (e.g. a 1 x 1 wbmp), meaning peek() would return less than bytesToRead.
    // Assume that if bytesRead < bytesToRead, but > 0, the stream is shorter
    // than bytesToRead, so pass that directly to the decoder.
    // It also is possible the stream uses too small a buffer for peeking, but
    // we trust the caller to use a large enough buffer.

    if (0 == bytesRead) {
        // TODO: After implementing peek in CreateJavaOutputStreamAdaptor.cpp, this
        // printf could be useful to notice failures.
        // SkCodecPrintf("Encoded image data failed to peek!\n");

        // It is possible the stream does not support peeking, but does support
        // rewinding.
        // Attempt to read() and pass the actual amount read to the decoder.
        bytesRead = stream->read(buffer, bytesToRead);
        if (!stream->rewind()) {
            SkCodecPrintf("Encoded image data could not peek or rewind to determine format!\n");
            *outResult = kCouldNotRewind;
            return nullptr;
        }
    }

    SkCodecs::MakeFromStreamCallback rawFallback = nullptr;
    for (const SkCodecs::Decoder& proc : decoders) {
        if (proc.isFormat(buffer, bytesRead)) {
            // png and heif are special, since we want to be able to supply a SkPngChunkReader
            // or SelectionPolicy respectively
            if (proc.id == "png") {
                return proc.makeFromStream(std::move(stream), outResult, chunkReader);
            } else if (proc.id == "heif") {
                return proc.makeFromStream(std::move(stream), outResult, &selectionPolicy);
            } else if (proc.id == "raw") {
                rawFallback = proc.makeFromStream;
                continue;
            }
            return proc.makeFromStream(std::move(stream), outResult, nullptr);
        }
    }
    if (rawFallback != nullptr) {
        // Fallback to raw.
        return rawFallback(std::move(stream), outResult, nullptr);
    }

    if (bytesRead < bytesToRead) {
        *outResult = kIncompleteInput;
    } else {
        *outResult = kUnimplemented;
    }
    return nullptr;
}

std::unique_ptr<SkCodec> SkCodec::MakeFromData(sk_sp<SkData> data, SkPngChunkReader* reader) {
    return MakeFromData(std::move(data), SkCodecs::get_decoders(), reader);
}
std::unique_ptr<SkCodec> SkCodec::MakeFromData(sk_sp<SkData> data,
                                               SkSpan<const SkCodecs::Decoder> decoders,
                                               SkPngChunkReader* reader) {
    if (!data) {
        return nullptr;
    }
    return MakeFromStream(SkMemoryStream::Make(std::move(data)), decoders, nullptr, reader);
}

SkCodec::SkCodec(SkEncodedInfo&& info,
                 XformFormat srcFormat,
                 std::unique_ptr<SkStream> stream,
                 SkEncodedOrigin origin)
        : fEncodedInfo(std::move(info))
        , fSrcXformFormat(srcFormat)
        , fStream(std::move(stream))
        , fOrigin(origin)
        , fDstInfo()
        , fOptions() {}

SkCodec::~SkCodec() {}

void SkCodec::setSrcXformFormat(XformFormat pixelFormat) {
    fSrcXformFormat = pixelFormat;
}

bool SkCodec::queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                            SkYUVAPixmapInfo* yuvaPixmapInfo) const {
    if (!yuvaPixmapInfo) {
        return false;
    }
    return this->onQueryYUVAInfo(supportedDataTypes, yuvaPixmapInfo) &&
           yuvaPixmapInfo->isSupported(supportedDataTypes);
}

SkCodec::Result SkCodec::getYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) {
    if (!yuvaPixmaps.isValid()) {
        return kInvalidInput;
    }
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }
    return this->onGetYUVAPlanes(yuvaPixmaps);
}

bool SkCodec::conversionSupported(const SkImageInfo& dst, bool srcIsOpaque, bool needsColorXform) {
    if (!valid_alpha(dst.alphaType(), srcIsOpaque)) {
        return false;
    }

    switch (dst.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            return true;
        case kBGR_101010x_XR_SkColorType:
        case kRGB_565_SkColorType:
            return srcIsOpaque;
        case kGray_8_SkColorType:
            return SkEncodedInfo::kGray_Color == fEncodedInfo.color() && srcIsOpaque;
        case kAlpha_8_SkColorType:
            // conceptually we can convert anything into alpha_8, but we haven't actually coded
            // all of those other conversions yet.
            return SkEncodedInfo::kXAlpha_Color == fEncodedInfo.color();
        default:
            return false;
    }
}

bool SkCodec::rewindIfNeeded() {
    // Store the value of fNeedsRewind so we can update it. Next read will
    // require a rewind.
    const bool needsRewind = fNeedsRewind;
    fNeedsRewind = true;
    if (!needsRewind) {
        return true;
    }

    // startScanlineDecode will need to be called before decoding scanlines.
    fCurrScanline = -1;
    // startIncrementalDecode will need to be called before incrementalDecode.
    fStartedIncrementalDecode = false;

    // Some codecs do not have a stream.  They may hold onto their own data or another codec.
    // They must handle rewinding themselves.
    if (fStream && !fStream->rewind()) {
        return false;
    }

    return this->onRewind();
}

static SkIRect frame_rect_on_screen(SkIRect frameRect,
                                    const SkIRect& screenRect) {
    if (!frameRect.intersect(screenRect)) {
        return SkIRect::MakeEmpty();
    }

    return frameRect;
}

bool zero_rect(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
               SkISize srcDimensions, SkIRect prevRect) {
    const auto dimensions = dstInfo.dimensions();
    if (dimensions != srcDimensions) {
        SkRect src = SkRect::Make(srcDimensions);
        SkRect dst = SkRect::Make(dimensions);
        SkMatrix map = SkMatrix::RectToRect(src, dst);
        SkRect asRect = SkRect::Make(prevRect);
        if (!map.mapRect(&asRect)) {
            return false;
        }
        asRect.roundOut(&prevRect);
    }

    if (!prevRect.intersect(SkIRect::MakeSize(dimensions))) {
        // Nothing to zero, due to scaling or bad frame rect.
        return true;
    }

    const SkImageInfo info = dstInfo.makeDimensions(prevRect.size());
    const size_t bpp = dstInfo.bytesPerPixel();
    const size_t offset = prevRect.x() * bpp + prevRect.y() * rowBytes;
    void* eraseDst = SkTAddOffset<void>(pixels, offset);
    SkSampler::Fill(info, eraseDst, rowBytes, SkCodec::kNo_ZeroInitialized);
    return true;
}

SkCodec::Result SkCodec::handleFrameIndex(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                          const Options& options, GetPixelsCallback getPixelsFn) {
    if (getPixelsFn) {
        // If a callback is used, it handles the frame index, so calls from this SkCodec
        // should always short-circuit in the else case below.
        fUsingCallbackForHandleFrameIndex = true;
    } else if (fUsingCallbackForHandleFrameIndex) {
        return kSuccess;
    }

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

    const int index = options.fFrameIndex;
    if (0 == index) {
        return this->initializeColorXform(info, fEncodedInfo.alpha(), fEncodedInfo.opaque())
            ? kSuccess : kInvalidConversion;
    }

    if (index < 0) {
        return kInvalidParameters;
    }

    if (options.fSubset) {
        // If we add support for this, we need to update the code that zeroes
        // a kRestoreBGColor frame.
        return kInvalidParameters;
    }

    if (index >= this->onGetFrameCount()) {
        return kIncompleteInput;
    }

    const auto* frameHolder = this->getFrameHolder();
    SkASSERT(frameHolder);

    const auto* frame = frameHolder->getFrame(index);
    SkASSERT(frame);

    const int requiredFrame = frame->getRequiredFrame();
    if (requiredFrame != kNoFrame) {
        // Decode earlier frame if necessary
        const SkFrame* preppedFrame = nullptr;
        if (options.fPriorFrame == kNoFrame) {
            Result result = kInternalError;
            // getPixelsFn will be set when things like SkAndroidCodec are calling this function.
            // Thus, we call the provided function when recursively decoding previous frames,
            // but only when necessary (i.e. there is a required frame).
            if (getPixelsFn) {
                result = getPixelsFn(info, pixels, rowBytes, options, requiredFrame);
            } else {
                Options prevFrameOptions(options);
                prevFrameOptions.fFrameIndex = requiredFrame;
                result = this->getPixels(info, pixels, rowBytes, &prevFrameOptions);
            }
            if (result != kSuccess) {
                return result;
            }
            preppedFrame = frameHolder->getFrame(requiredFrame);
        } else {
            // Check for a valid frame as a starting point. Alternatively, we could
            // treat an invalid frame as not providing one, but rejecting it will
            // make it easier to catch the mistake.
            if (options.fPriorFrame < requiredFrame || options.fPriorFrame >= index) {
                return kInvalidParameters;
            }
            preppedFrame = frameHolder->getFrame(options.fPriorFrame);
        }

        SkASSERT(preppedFrame);
        switch (preppedFrame->getDisposalMethod()) {
            case SkCodecAnimation::DisposalMethod::kRestorePrevious:
                SkASSERT(options.fPriorFrame != kNoFrame);
                return kInvalidParameters;
            case SkCodecAnimation::DisposalMethod::kRestoreBGColor:
                // If a frame after the required frame is provided, there is no
                // need to clear, since it must be covered by the desired frame.
                // FIXME: If the required frame is kRestoreBGColor, we don't actually need to decode
                // it, since we'll just clear it to transparent. Instead, we could decode *its*
                // required frame and then clear.
                if (preppedFrame->frameId() == requiredFrame) {
                    SkIRect preppedRect = preppedFrame->frameRect();
                    if (!zero_rect(info, pixels, rowBytes, this->dimensions(), preppedRect)) {
                        return kInternalError;
                    }
                }
                break;
            default:
                break;
        }
    }

    return this->initializeColorXform(info, frame->reportedAlpha(), !frame->hasAlpha())
        ? kSuccess : kInvalidConversion;
}

SkCodec::Result SkCodec::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                   const Options* options) {
    if (kUnknown_SkColorType == info.colorType()) {
        return kInvalidConversion;
    }
    if (nullptr == pixels) {
        return kInvalidParameters;
    }
    if (rowBytes < info.minRowBytes()) {
        return kInvalidParameters;
    }

    // Default options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    } else {
        if (options->fSubset) {
            SkIRect subset(*options->fSubset);
            if (!this->onGetValidSubset(&subset) || subset != *options->fSubset) {
                // FIXME: How to differentiate between not supporting subset at all
                // and not supporting this particular subset?
                return kUnimplemented;
            }
        }
    }

    const Result frameIndexResult = this->handleFrameIndex(info, pixels, rowBytes,
                                                           *options);
    if (frameIndexResult != kSuccess) {
        return frameIndexResult;
    }

    // FIXME: Support subsets somehow? Note that this works for SkWebpCodec
    // because it supports arbitrary scaling/subset combinations.
    if (!this->dimensionsSupported(info.dimensions())) {
        return kInvalidScale;
    }

    fDstInfo = info;
    fOptions = *options;

    // On an incomplete decode, the subclass will specify the number of scanlines that it decoded
    // successfully.
    int rowsDecoded = 0;
    const Result result = this->onGetPixels(info, pixels, rowBytes, *options, &rowsDecoded);

    // A return value of kIncompleteInput indicates a truncated image stream.
    // In this case, we will fill any uninitialized memory with a default value.
    // Some subclasses will take care of filling any uninitialized memory on
    // their own.  They indicate that all of the memory has been filled by
    // setting rowsDecoded equal to the height.
    if ((kIncompleteInput == result || kErrorInInput == result) && rowsDecoded != info.height()) {
        // FIXME: (skbug.com/5772) fillIncompleteImage will fill using the swizzler's width, unless
        // there is a subset. In that case, it will use the width of the subset. From here, the
        // subset will only be non-null in the case of SkWebpCodec, but it treats the subset
        // differenty from the other codecs, and it needs to use the width specified by the info.
        // Set the subset to null so SkWebpCodec uses the correct width.
        fOptions.fSubset = nullptr;
        this->fillIncompleteImage(info, pixels, rowBytes, options->fZeroInitialized, info.height(),
                rowsDecoded);
    }

    return result;
}

std::tuple<sk_sp<SkImage>, SkCodec::Result> SkCodec::getImage(const SkImageInfo& info,
                                                              const Options* options) {
    SkBitmap bm;
    if (!bm.tryAllocPixels(info)) {
        return {nullptr, kInternalError};
    }

    Result result;
    auto decode = [this, options, &result](const SkPixmap& pm) {
        result = this->getPixels(pm, options);
        switch (result) {
            case SkCodec::kSuccess:
            case SkCodec::kIncompleteInput:
            case SkCodec::kErrorInInput:
                return true;
            default:
                return false;
        }
    };

    // If the codec reports this image is rotated, we will decode it into
    // a temporary buffer, then copy it (rotated) into the pixmap belonging
    // to bm that we allocated above. If the image is not rotated, we will
    // decode straight into that allocated pixmap.
    if (!SkPixmapUtils::Orient(bm.pixmap(), this->getOrigin(), decode)) {
        return {nullptr, result};
    }
    // Setting the bitmap to be immutable saves us from having to copy it.
    bm.setImmutable();
    return {SkImages::RasterFromBitmap(bm), kSuccess};
}

std::tuple<sk_sp<SkImage>, SkCodec::Result> SkCodec::getImage() {
    // If the codec reports that it is rotated, we need to rotate the image info
    // it says it is, so the output is what the user wants.
    SkImageInfo info = this->getInfo();
    if (SkEncodedOriginSwapsWidthHeight(this->getOrigin())) {
        info = SkPixmapUtils::SwapWidthHeight(info);
    }
    return this->getImage(info, nullptr);
}

SkCodec::Result SkCodec::startIncrementalDecode(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const SkCodec::Options* options) {
    fStartedIncrementalDecode = false;

    if (kUnknown_SkColorType == info.colorType()) {
        return kInvalidConversion;
    }
    if (nullptr == pixels) {
        return kInvalidParameters;
    }

    // Set options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    } else {
        if (options->fSubset) {
            SkIRect size = SkIRect::MakeSize(info.dimensions());
            if (!size.contains(*options->fSubset)) {
                return kInvalidParameters;
            }

            const int top = options->fSubset->top();
            const int bottom = options->fSubset->bottom();
            if (top < 0 || top >= info.height() || top >= bottom || bottom > info.height()) {
                return kInvalidParameters;
            }
        }
    }

    const Result frameIndexResult = this->handleFrameIndex(info, pixels, rowBytes,
                                                           *options);
    if (frameIndexResult != kSuccess) {
        return frameIndexResult;
    }

    if (!this->dimensionsSupported(info.dimensions())) {
        return kInvalidScale;
    }

    fDstInfo = info;
    fOptions = *options;

    const Result result = this->onStartIncrementalDecode(info, pixels, rowBytes, fOptions);
    if (kSuccess == result) {
        fStartedIncrementalDecode = true;
    } else if (kUnimplemented == result) {
        // FIXME: This is temporarily necessary, until we transition SkCodec
        // implementations from scanline decoding to incremental decoding.
        // SkAndroidCodec will first attempt to use incremental decoding, but
        // will fall back to scanline decoding if incremental returns
        // kUnimplemented. rewindIfNeeded(), above, set fNeedsRewind to true
        // (after potentially rewinding), but we do not want the next call to
        // startScanlineDecode() to do a rewind.
        fNeedsRewind = false;
    }
    return result;
}


SkCodec::Result SkCodec::startScanlineDecode(const SkImageInfo& info,
        const SkCodec::Options* options) {
    // Reset fCurrScanline in case of failure.
    fCurrScanline = -1;

    // Set options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    } else if (options->fSubset) {
        SkIRect size = SkIRect::MakeSize(info.dimensions());
        if (!size.contains(*options->fSubset)) {
            return kInvalidInput;
        }

        // We only support subsetting in the x-dimension for scanline decoder.
        // Subsetting in the y-dimension can be accomplished using skipScanlines().
        if (options->fSubset->top() != 0 || options->fSubset->height() != info.height()) {
            return kInvalidInput;
        }
    }

    // Scanline decoding only supports decoding the first frame.
    if (options->fFrameIndex != 0) {
        return kUnimplemented;
    }

    // The void* dst and rowbytes in handleFrameIndex or only used for decoding prior
    // frames, which is not supported here anyway, so it is safe to pass nullptr/0.
    const Result frameIndexResult = this->handleFrameIndex(info, nullptr, 0, *options);
    if (frameIndexResult != kSuccess) {
        return frameIndexResult;
    }

    // FIXME: Support subsets somehow?
    if (!this->dimensionsSupported(info.dimensions())) {
        return kInvalidScale;
    }

    const Result result = this->onStartScanlineDecode(info, *options);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    // FIXME: See startIncrementalDecode. That method set fNeedsRewind to false
    // so that when onStartScanlineDecode calls rewindIfNeeded it would not
    // rewind. But it also relies on that call to rewindIfNeeded to set
    // fNeedsRewind to true for future decodes. When
    // fUsingCallbackForHandleFrameIndex is true, that call to rewindIfNeeded is
    // skipped, so this method sets it back to true.
    SkASSERT(fUsingCallbackForHandleFrameIndex || fNeedsRewind);
    fNeedsRewind = true;

    fCurrScanline = 0;
    fDstInfo = info;
    fOptions = *options;
    return kSuccess;
}

int SkCodec::getScanlines(void* dst, int countLines, size_t rowBytes) {
    if (fCurrScanline < 0) {
        return 0;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if (countLines <= 0 || fCurrScanline + countLines > fDstInfo.height()) {
        return 0;
    }

    const int linesDecoded = this->onGetScanlines(dst, countLines, rowBytes);
    if (linesDecoded < countLines) {
        this->fillIncompleteImage(this->dstInfo(), dst, rowBytes, this->options().fZeroInitialized,
                countLines, linesDecoded);
    }
    fCurrScanline += countLines;
    return linesDecoded;
}

bool SkCodec::skipScanlines(int countLines) {
    if (fCurrScanline < 0) {
        return false;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if (countLines < 0 || fCurrScanline + countLines > fDstInfo.height()) {
        // Arguably, we could just skip the scanlines which are remaining,
        // and return true. We choose to return false so the client
        // can catch their bug.
        return false;
    }

    bool result = this->onSkipScanlines(countLines);
    fCurrScanline += countLines;
    return result;
}

int SkCodec::outputScanline(int inputScanline) const {
    SkASSERT(0 <= inputScanline && inputScanline < fEncodedInfo.height());
    return this->onOutputScanline(inputScanline);
}

int SkCodec::onOutputScanline(int inputScanline) const {
    switch (this->getScanlineOrder()) {
        case kTopDown_SkScanlineOrder:
            return inputScanline;
        case kBottomUp_SkScanlineOrder:
            return fEncodedInfo.height() - inputScanline - 1;
        default:
            // This case indicates an interlaced gif and is implemented by SkGifCodec.
            SkASSERT(false);
            return 0;
    }
}

void SkCodec::fillIncompleteImage(const SkImageInfo& info, void* dst, size_t rowBytes,
        ZeroInitialized zeroInit, int linesRequested, int linesDecoded) {
    if (kYes_ZeroInitialized == zeroInit) {
        return;
    }

    const int linesRemaining = linesRequested - linesDecoded;
    SkSampler* sampler = this->getSampler(false);

    const int fillWidth = sampler          ? sampler->fillWidth()      :
                          fOptions.fSubset ? fOptions.fSubset->width() :
                                             info.width()              ;
    void* fillDst = this->getScanlineOrder() == kBottomUp_SkScanlineOrder ? dst :
                        SkTAddOffset<void>(dst, linesDecoded * rowBytes);
    const auto fillInfo = info.makeWH(fillWidth, linesRemaining);
    SkSampler::Fill(fillInfo, fillDst, rowBytes, kNo_ZeroInitialized);
}

bool sk_select_xform_format(SkColorType colorType, bool forColorTable,
                            skcms_PixelFormat* outFormat) {
    SkASSERT(outFormat);

    switch (colorType) {
        case kRGBA_8888_SkColorType:
            *outFormat = skcms_PixelFormat_RGBA_8888;
            break;
        case kBGRA_8888_SkColorType:
            *outFormat = skcms_PixelFormat_BGRA_8888;
            break;
        case kRGB_565_SkColorType:
            if (forColorTable) {
#ifdef SK_PMCOLOR_IS_RGBA
                *outFormat = skcms_PixelFormat_RGBA_8888;
#else
                *outFormat = skcms_PixelFormat_BGRA_8888;
#endif
                break;
            }
            *outFormat = skcms_PixelFormat_BGR_565;
            break;
        case kRGBA_F16_SkColorType:
            *outFormat = skcms_PixelFormat_RGBA_hhhh;
            break;
        case kBGR_101010x_XR_SkColorType:
            *outFormat = skcms_PixelFormat_BGR_101010x_XR;
            break;
        case kGray_8_SkColorType:
            *outFormat = skcms_PixelFormat_G_8;
            break;
        default:
            return false;
    }
    return true;
}

bool SkCodec::initializeColorXform(const SkImageInfo& dstInfo, SkEncodedInfo::Alpha encodedAlpha,
                                   bool srcIsOpaque) {
    fXformTime = kNo_XformTime;
    bool needsColorXform = false;
    if (this->usesColorXform()) {
        if (kRGBA_F16_SkColorType == dstInfo.colorType() ||
                kBGR_101010x_XR_SkColorType == dstInfo.colorType()) {
            needsColorXform = true;
            if (dstInfo.colorSpace()) {
                dstInfo.colorSpace()->toProfile(&fDstProfile);
            } else {
                // Use the srcProfile to avoid conversion.
                const auto* srcProfile = fEncodedInfo.profile();
                fDstProfile = srcProfile ? *srcProfile : *skcms_sRGB_profile();
            }
        } else if (dstInfo.colorSpace()) {
            dstInfo.colorSpace()->toProfile(&fDstProfile);
            const auto* srcProfile = fEncodedInfo.profile();
            if (!srcProfile) {
                srcProfile = skcms_sRGB_profile();
            }
            if (!skcms_ApproximatelyEqualProfiles(srcProfile, &fDstProfile) ) {
                needsColorXform = true;
            }
        }
    }

    if (!this->conversionSupported(dstInfo, srcIsOpaque, needsColorXform)) {
        return false;
    }

    if (needsColorXform) {
        fXformTime = SkEncodedInfo::kPalette_Color != fEncodedInfo.color()
                          || kRGBA_F16_SkColorType == dstInfo.colorType()
                ? kDecodeRow_XformTime : kPalette_XformTime;
        if (!sk_select_xform_format(dstInfo.colorType(), fXformTime == kPalette_XformTime,
                                    &fDstXformFormat)) {
            return false;
        }
        if (encodedAlpha == SkEncodedInfo::kUnpremul_Alpha
                && dstInfo.alphaType() == kPremul_SkAlphaType) {
            fDstXformAlphaFormat = skcms_AlphaFormat_PremulAsEncoded;
        } else {
            fDstXformAlphaFormat = skcms_AlphaFormat_Unpremul;
        }
    }
    return true;
}

void SkCodec::applyColorXform(void* dst, const void* src, int count) const {
    // It is okay for srcProfile to be null. This will use sRGB.
    const auto* srcProfile = fEncodedInfo.profile();
    SkAssertResult(skcms_Transform(src, fSrcXformFormat, skcms_AlphaFormat_Unpremul, srcProfile,
                                   dst, fDstXformFormat, fDstXformAlphaFormat, &fDstProfile,
                                   count));
}

std::vector<SkCodec::FrameInfo> SkCodec::getFrameInfo() {
    const int frameCount = this->getFrameCount();
    SkASSERT(frameCount >= 0);
    if (frameCount <= 0) {
        return std::vector<FrameInfo>{};
    }

    if (frameCount == 1 && !this->onGetFrameInfo(0, nullptr)) {
        // Not animated.
        return std::vector<FrameInfo>{};
    }

    std::vector<FrameInfo> result(frameCount);
    for (int i = 0; i < frameCount; ++i) {
        SkAssertResult(this->onGetFrameInfo(i, &result[i]));
    }
    return result;
}

const char* SkCodec::ResultToString(Result result) {
    switch (result) {
        case kSuccess:
            return "success";
        case kIncompleteInput:
            return "incomplete input";
        case kErrorInInput:
            return "error in input";
        case kInvalidConversion:
            return "invalid conversion";
        case kInvalidScale:
            return "invalid scale";
        case kInvalidParameters:
            return "invalid parameters";
        case kInvalidInput:
            return "invalid input";
        case kCouldNotRewind:
            return "could not rewind";
        case kInternalError:
            return "internal error";
        case kUnimplemented:
            return "unimplemented";
        default:
            SkASSERT(false);
            return "bogus result value";
    }
}

void SkFrame::fillIn(SkCodec::FrameInfo* frameInfo, bool fullyReceived) const {
    SkASSERT(frameInfo);

    frameInfo->fRequiredFrame = fRequiredFrame;
    frameInfo->fDuration = fDuration;
    frameInfo->fFullyReceived = fullyReceived;
    frameInfo->fAlphaType = fHasAlpha ? kUnpremul_SkAlphaType
                                      : kOpaque_SkAlphaType;
    frameInfo->fHasAlphaWithinBounds = this->reportedAlpha() != SkEncodedInfo::kOpaque_Alpha;
    frameInfo->fDisposalMethod = fDisposalMethod;
    frameInfo->fBlend = fBlend;
    frameInfo->fFrameRect = fRect;
}

static bool independent(const SkFrame& frame) {
    return frame.getRequiredFrame() == SkCodec::kNoFrame;
}

static bool restore_bg(const SkFrame& frame) {
    return frame.getDisposalMethod() == SkCodecAnimation::DisposalMethod::kRestoreBGColor;
}

// As its name suggests, this method computes a frame's alpha (e.g. completely
// opaque, unpremul, binary) and its required frame (a preceding frame that
// this frame depends on, to draw the complete image at this frame's point in
// the animation stream), and calls this frame's setter methods with that
// computed information.
//
// A required frame of kNoFrame means that this frame is independent: drawing
// the complete image at this frame's point in the animation stream does not
// require first preparing the pixel buffer based on another frame. Instead,
// drawing can start from an uninitialized pixel buffer.
//
// "Uninitialized" is from the SkCodec's caller's point of view. In the SkCodec
// implementation, for independent frames, first party Skia code (in src/codec)
// will typically fill the buffer with a uniform background color (e.g.
// transparent black) before calling into third party codec-specific code (e.g.
// libjpeg or libpng). Pixels outside of the frame's rect will remain this
// background color after drawing this frame. For incomplete decodes, pixels
// inside that rect may be (at least temporarily) set to that background color.
// In an incremental decode, later passes may then overwrite that background
// color.
//
// Determining kNoFrame or otherwise involves testing a number of conditions
// sequentially. The first satisfied condition results in setting the required
// frame to kNoFrame (an "INDx" condition) or to a non-negative frame number (a
// "DEPx" condition), and the function returning early. Those "INDx" and "DEPx"
// labels also map to comments in the function body.
//
//  - IND1: this frame is the first frame.
//  - IND2: this frame fills out the whole image, and it is completely opaque
//          or it overwrites (not blends with) the previous frame.
//  - IND3: all preceding frames' disposals are kRestorePrevious.
//  - IND4: the prevFrame's disposal is kRestoreBGColor, and it fills out the
//          whole image or it is itself otherwise independent.
//  - DEP5: this frame reports alpha (it is not completely opaque) and it
//          blends with (not overwrites) the previous frame.
//  - IND6: this frame's rect covers the rects of all preceding frames back to
//          and including the most recent independent frame before this frame.
//  - DEP7: unconditional.
//
// The "prevFrame" variable initially points to the previous frame (also known
// as the prior frame), but that variable may iterate further backwards over
// the course of this computation.
void SkFrameHolder::setAlphaAndRequiredFrame(SkFrame* frame) {
    const bool reportsAlpha = frame->reportedAlpha() != SkEncodedInfo::kOpaque_Alpha;
    const auto screenRect = SkIRect::MakeWH(fScreenWidth, fScreenHeight);
    const auto frameRect = frame_rect_on_screen(frame->frameRect(), screenRect);

    const int i = frame->frameId();
    if (0 == i) {
        frame->setHasAlpha(reportsAlpha || frameRect != screenRect);
        frame->setRequiredFrame(SkCodec::kNoFrame);  // IND1
        return;
    }


    const bool blendWithPrevFrame = frame->getBlend() == SkCodecAnimation::Blend::kSrcOver;
    if ((!reportsAlpha || !blendWithPrevFrame) && frameRect == screenRect) {
        frame->setHasAlpha(reportsAlpha);
        frame->setRequiredFrame(SkCodec::kNoFrame);  // IND2
        return;
    }

    const SkFrame* prevFrame = this->getFrame(i-1);
    while (prevFrame->getDisposalMethod() == SkCodecAnimation::DisposalMethod::kRestorePrevious) {
        const int prevId = prevFrame->frameId();
        if (0 == prevId) {
            frame->setHasAlpha(true);
            frame->setRequiredFrame(SkCodec::kNoFrame);  // IND3
            return;
        }

        prevFrame = this->getFrame(prevId - 1);
    }

    const bool clearPrevFrame = restore_bg(*prevFrame);
    auto prevFrameRect = frame_rect_on_screen(prevFrame->frameRect(), screenRect);

    if (clearPrevFrame) {
        if (prevFrameRect == screenRect || independent(*prevFrame)) {
            frame->setHasAlpha(true);
            frame->setRequiredFrame(SkCodec::kNoFrame);  // IND4
            return;
        }
    }

    if (reportsAlpha && blendWithPrevFrame) {
        // Note: We could be more aggressive here. If prevFrame clears
        // to background color and covers its required frame (and that
        // frame is independent), prevFrame could be marked independent.
        // Would this extra complexity be worth it?
        frame->setRequiredFrame(prevFrame->frameId());  // DEP5
        frame->setHasAlpha(prevFrame->hasAlpha() || clearPrevFrame);
        return;
    }

    while (frameRect.contains(prevFrameRect)) {
        const int prevRequiredFrame = prevFrame->getRequiredFrame();
        if (prevRequiredFrame == SkCodec::kNoFrame) {
            frame->setRequiredFrame(SkCodec::kNoFrame);  // IND6
            frame->setHasAlpha(true);
            return;
        }

        prevFrame = this->getFrame(prevRequiredFrame);
        prevFrameRect = frame_rect_on_screen(prevFrame->frameRect(), screenRect);
    }

    frame->setRequiredFrame(prevFrame->frameId());  // DEP7
    if (restore_bg(*prevFrame)) {
        frame->setHasAlpha(true);
        return;
    }
    SkASSERT(prevFrame->getDisposalMethod() == SkCodecAnimation::DisposalMethod::kKeep);
    frame->setHasAlpha(prevFrame->hasAlpha() || (reportsAlpha && !blendWithPrevFrame));
}

