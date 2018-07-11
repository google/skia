/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXformPriv.h"
#include "SkData.h"
#include "SkFrameHolder.h"
#include "SkGifCodec.h"
#include "SkHalf.h"
#ifdef SK_HAS_HEIF_LIBRARY
#include "SkHeifCodec.h"
#endif
#include "SkIcoCodec.h"
#include "SkJpegCodec.h"
#ifdef SK_HAS_PNG_LIBRARY
#include "SkPngCodec.h"
#endif
#include "SkRawCodec.h"
#include "SkStream.h"
#include "SkWbmpCodec.h"
#include "SkWebpCodec.h"

struct DecoderProc {
    bool (*IsFormat)(const void*, size_t);
    std::unique_ptr<SkCodec> (*MakeFromStream)(std::unique_ptr<SkStream>, SkCodec::Result*);
};

static constexpr DecoderProc gDecoderProcs[] = {
#ifdef SK_HAS_JPEG_LIBRARY
    { SkJpegCodec::IsJpeg, SkJpegCodec::MakeFromStream },
#endif
#ifdef SK_HAS_WEBP_LIBRARY
    { SkWebpCodec::IsWebp, SkWebpCodec::MakeFromStream },
#endif
    { SkGifCodec::IsGif, SkGifCodec::MakeFromStream },
#ifdef SK_HAS_PNG_LIBRARY
    { SkIcoCodec::IsIco, SkIcoCodec::MakeFromStream },
#endif
    { SkBmpCodec::IsBmp, SkBmpCodec::MakeFromStream },
    { SkWbmpCodec::IsWbmp, SkWbmpCodec::MakeFromStream },
#ifdef SK_HAS_HEIF_LIBRARY
    { SkHeifCodec::IsHeif, SkHeifCodec::MakeFromStream },
#endif
};

std::unique_ptr<SkCodec> SkCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                 Result* outResult, SkPngChunkReader* chunkReader) {
    Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }

    if (!stream) {
        *outResult = kInvalidInput;
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

    // PNG is special, since we want to be able to supply an SkPngChunkReader.
    // But this code follows the same pattern as the loop.
#ifdef SK_HAS_PNG_LIBRARY
    if (SkPngCodec::IsPng(buffer, bytesRead)) {
        return SkPngCodec::MakeFromStream(std::move(stream), outResult, chunkReader);
    } else
#endif
    {
        for (DecoderProc proc : gDecoderProcs) {
            if (proc.IsFormat(buffer, bytesRead)) {
                return proc.MakeFromStream(std::move(stream), outResult);
            }
        }

#ifdef SK_CODEC_DECODES_RAW
        // Try to treat the input as RAW if all the other checks failed.
        return SkRawCodec::MakeFromStream(std::move(stream), outResult);
#endif
    }

    if (bytesRead < bytesToRead) {
        *outResult = kIncompleteInput;
    } else {
        *outResult = kUnimplemented;
    }

    return nullptr;
}

std::unique_ptr<SkCodec> SkCodec::MakeFromData(sk_sp<SkData> data, SkPngChunkReader* reader) {
    if (!data) {
        return nullptr;
    }
    return MakeFromStream(SkMemoryStream::Make(std::move(data)), nullptr, reader);
}

SkCodec::SkCodec(int width, int height, const SkEncodedInfo& info,
        XformFormat srcFormat, std::unique_ptr<SkStream> stream,
        sk_sp<SkColorSpace> colorSpace, SkEncodedOrigin origin)
    : fEncodedInfo(info)
    , fSrcInfo(info.makeImageInfo(width, height, std::move(colorSpace)))
    , fSrcXformFormat(srcFormat)
    , fStream(std::move(stream))
    , fNeedsRewind(false)
    , fOrigin(origin)
    , fDstInfo()
    , fOptions()
    , fCurrScanline(-1)
    , fStartedIncrementalDecode(false)
{}

SkCodec::SkCodec(const SkEncodedInfo& info, const SkImageInfo& imageInfo,
        XformFormat srcFormat, std::unique_ptr<SkStream> stream,
        SkEncodedOrigin origin)
    : fEncodedInfo(info)
    , fSrcInfo(imageInfo)
    , fSrcXformFormat(srcFormat)
    , fStream(std::move(stream))
    , fNeedsRewind(false)
    , fOrigin(origin)
    , fDstInfo()
    , fOptions()
    , fCurrScanline(-1)
    , fStartedIncrementalDecode(false)
{}

SkCodec::~SkCodec() {}

bool SkCodec::conversionSupported(const SkImageInfo& dst, SkColorType srcColor,
                                  bool srcIsOpaque, const SkColorSpace* srcCS) const {
    if (!valid_alpha(dst.alphaType(), srcIsOpaque)) {
        return false;
    }

    switch (dst.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        case kRGBA_F16_SkColorType:
            return dst.colorSpace();
        case kRGB_565_SkColorType:
            return srcIsOpaque;
        case kGray_8_SkColorType:
            return kGray_8_SkColorType == srcColor && srcIsOpaque &&
                   !needs_color_xform(dst, srcCS);
        case kAlpha_8_SkColorType:
            // conceptually we can convert anything into alpha_8, but we haven't actually coded
            // all of those other conversions yet, so only return true for the case we have codec.
            return fSrcInfo.colorType() == kAlpha_8_SkColorType;;
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

bool zero_rect(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
               SkISize srcDimensions, SkIRect prevRect) {
    const auto dimensions = dstInfo.dimensions();
    if (dimensions != srcDimensions) {
        SkRect src = SkRect::Make(srcDimensions);
        SkRect dst = SkRect::Make(dimensions);
        SkMatrix map = SkMatrix::MakeRectToRect(src, dst, SkMatrix::kCenter_ScaleToFit);
        SkRect asRect = SkRect::Make(prevRect);
        if (!map.mapRect(&asRect)) {
            return false;
        }
        asRect.roundIn(&prevRect);
        if (prevRect.isEmpty()) {
            // Down-scaling shrank the empty portion to nothing,
            // so nothing to zero.
            return true;
        }
    }

    if (!prevRect.intersect(dstInfo.bounds())) {
        SkCodecPrintf("rectangles do not intersect!");
        SkASSERT(false);
        return true;
    }

    const SkImageInfo info = dstInfo.makeWH(prevRect.width(), prevRect.height());
    const size_t bpp = dstInfo.bytesPerPixel();
    const size_t offset = prevRect.x() * bpp + prevRect.y() * rowBytes;
    void* eraseDst = SkTAddOffset<void>(pixels, offset);
    SkSampler::Fill(info, eraseDst, rowBytes, 0, SkCodec::kNo_ZeroInitialized);
    return true;
}

SkCodec::Result SkCodec::handleFrameIndex(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                          const Options& options) {
    const int index = options.fFrameIndex;
    if (0 == index) {
        if (!this->conversionSupported(info, fSrcInfo.colorType(), fEncodedInfo.opaque(),
                                      fSrcInfo.colorSpace())
            || !this->initializeColorXform(info, fEncodedInfo.alpha()))
        {
            return kInvalidConversion;
        }
        return kSuccess;
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

    if (!this->conversionSupported(info, fSrcInfo.colorType(), !frame->hasAlpha(),
                                   fSrcInfo.colorSpace())) {
        return kInvalidConversion;
    }

    const int requiredFrame = frame->getRequiredFrame();
    if (requiredFrame != kNone) {
        if (options.fPriorFrame != kNone) {
            // Check for a valid frame as a starting point. Alternatively, we could
            // treat an invalid frame as not providing one, but rejecting it will
            // make it easier to catch the mistake.
            if (options.fPriorFrame < requiredFrame || options.fPriorFrame >= index) {
                return kInvalidParameters;
            }
            const auto* prevFrame = frameHolder->getFrame(options.fPriorFrame);
            switch (prevFrame->getDisposalMethod()) {
                case SkCodecAnimation::DisposalMethod::kRestorePrevious:
                    return kInvalidParameters;
                case SkCodecAnimation::DisposalMethod::kRestoreBGColor:
                    // If a frame after the required frame is provided, there is no
                    // need to clear, since it must be covered by the desired frame.
                    if (options.fPriorFrame == requiredFrame) {
                        SkIRect prevRect = prevFrame->frameRect();
                        if (!zero_rect(info, pixels, rowBytes, fSrcInfo.dimensions(), prevRect)) {
                            return kInternalError;
                        }
                    }
                    break;
                default:
                    break;
            }
        } else {
            Options prevFrameOptions(options);
            prevFrameOptions.fFrameIndex = requiredFrame;
            prevFrameOptions.fZeroInitialized = kNo_ZeroInitialized;
            const Result result = this->getPixels(info, pixels, rowBytes, &prevFrameOptions);
            if (result != kSuccess) {
                return result;
            }
            const auto* prevFrame = frameHolder->getFrame(requiredFrame);
            const auto disposalMethod = prevFrame->getDisposalMethod();
            if (disposalMethod == SkCodecAnimation::DisposalMethod::kRestoreBGColor) {
                auto prevRect = prevFrame->frameRect();
                if (!zero_rect(info, pixels, rowBytes, fSrcInfo.dimensions(), prevRect)) {
                    return kInternalError;
                }
            }
        }
    }

    return this->initializeColorXform(info, frame->reportedAlpha()) ? kSuccess : kInvalidConversion;
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

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
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

SkCodec::Result SkCodec::startIncrementalDecode(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const SkCodec::Options* options) {
    fStartedIncrementalDecode = false;

    if (kUnknown_SkColorType == info.colorType()) {
        return kInvalidConversion;
    }
    if (nullptr == pixels) {
        return kInvalidParameters;
    }

    // FIXME: If the rows come after the rows of a previous incremental decode,
    // we might be able to skip the rewind, but only the implementation knows
    // that. (e.g. PNG will always need to rewind, since we called longjmp, but
    // a bottom-up BMP could skip rewinding if the new rows are above the old
    // rows.)
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
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

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

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
    SkASSERT(0 <= inputScanline && inputScanline < this->getInfo().height());
    return this->onOutputScanline(inputScanline);
}

int SkCodec::onOutputScanline(int inputScanline) const {
    switch (this->getScanlineOrder()) {
        case kTopDown_SkScanlineOrder:
            return inputScanline;
        case kBottomUp_SkScanlineOrder:
            return this->getInfo().height() - inputScanline - 1;
        default:
            // This case indicates an interlaced gif and is implemented by SkGifCodec.
            SkASSERT(false);
            return 0;
    }
}

uint64_t SkCodec::onGetFillValue(const SkImageInfo& dstInfo) const {
    switch (dstInfo.colorType()) {
        case kRGBA_F16_SkColorType: {
            static constexpr uint64_t transparentColor = 0;
            static constexpr uint64_t opaqueColor = ((uint64_t) SK_Half1) << 48;
            return (kOpaque_SkAlphaType == fSrcInfo.alphaType()) ? opaqueColor : transparentColor;
        }
        default: {
            // This not only handles the kN32 case, but also k565, kGray8, since
            // the low bits are zeros.
            return (kOpaque_SkAlphaType == fSrcInfo.alphaType()) ?
                    SK_ColorBLACK : SK_ColorTRANSPARENT;
        }
    }
}

static void fill_proc(const SkImageInfo& info, void* dst, size_t rowBytes,
        uint64_t colorOrIndex, SkCodec::ZeroInitialized zeroInit, SkSampler* sampler) {
    if (sampler) {
        sampler->fill(info, dst, rowBytes, colorOrIndex, zeroInit);
    } else {
        SkSampler::Fill(info, dst, rowBytes, colorOrIndex, zeroInit);
    }
}

void SkCodec::fillIncompleteImage(const SkImageInfo& info, void* dst, size_t rowBytes,
        ZeroInitialized zeroInit, int linesRequested, int linesDecoded) {

    void* fillDst;
    const uint64_t fillValue = this->getFillValue(info);
    const int linesRemaining = linesRequested - linesDecoded;
    SkSampler* sampler = this->getSampler(false);

    int fillWidth = info.width();
    if (fOptions.fSubset) {
        fillWidth = fOptions.fSubset->width();
    }

    switch (this->getScanlineOrder()) {
        case kTopDown_SkScanlineOrder: {
            const SkImageInfo fillInfo = info.makeWH(fillWidth, linesRemaining);
            fillDst = SkTAddOffset<void>(dst, linesDecoded * rowBytes);
            fill_proc(fillInfo, fillDst, rowBytes, fillValue, zeroInit, sampler);
            break;
        }
        case kBottomUp_SkScanlineOrder: {
            fillDst = dst;
            const SkImageInfo fillInfo = info.makeWH(fillWidth, linesRemaining);
            fill_proc(fillInfo, fillDst, rowBytes, fillValue, zeroInit, sampler);
            break;
        }
    }
}

static inline SkColorSpaceXform::ColorFormat select_xform_format_ct(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
        case kBGRA_8888_SkColorType:
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
        case kRGB_565_SkColorType:
#ifdef SK_PMCOLOR_IS_RGBA
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
#else
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
#endif
        default:
            SkASSERT(false);
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
    }
}

bool SkCodec::initializeColorXform(const SkImageInfo& dstInfo, SkEncodedInfo::Alpha encodedAlpha) {
    fColorXform = nullptr;
    fXformOnDecode = false;
    if (!this->usesColorXform()) {
        return true;
    }
    // FIXME: In SkWebpCodec, if a frame is blending with a prior frame, we don't need
    // a colorXform to do a color correct premul, since the blend step will handle
    // premultiplication. But there is no way to know whether we need to blend from
    // inside this call.
    if (needs_color_xform(dstInfo, fSrcInfo.colorSpace())) {
        fColorXform = SkMakeColorSpaceXform(fSrcInfo.colorSpace(), dstInfo.colorSpace());
        if (!fColorXform) {
            return false;
        }

        // We will apply the color xform when reading the color table unless F16 is requested.
        fXformOnDecode = SkEncodedInfo::kPalette_Color != fEncodedInfo.color()
            || kRGBA_F16_SkColorType == dstInfo.colorType();
        if (fXformOnDecode) {
            fDstXformFormat = select_xform_format(dstInfo.colorType());
        } else {
            fDstXformFormat = select_xform_format_ct(dstInfo.colorType());
        }
    }

    return true;
}

void SkCodec::applyColorXform(void* dst, const void* src, int count, SkAlphaType at) const {
    SkASSERT(fColorXform);
    SkAssertResult(fColorXform->apply(fDstXformFormat, dst,
                                      fSrcXformFormat, src,
                                      count, at));
}

void SkCodec::applyColorXform(void* dst, const void* src, int count) const {
    auto alphaType = select_xform_alpha(fDstInfo.alphaType(), fSrcInfo.alphaType());
    this->applyColorXform(dst, src, count, alphaType);
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
