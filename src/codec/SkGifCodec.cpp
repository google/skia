/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SkCodecAnimation.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkGifCodec.h"
#include "SkStream.h"
#include "SkSwizzler.h"

#include <algorithm>

#define GIF87_STAMP "GIF87a"
#define GIF89_STAMP "GIF89a"
#define GIF_STAMP_LEN 6

/*
 * Checks the start of the stream to see if the image is a gif
 */
bool SkGifCodec::IsGif(const void* buf, size_t bytesRead) {
    if (bytesRead >= GIF_STAMP_LEN) {
        if (memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
            memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0)
        {
            return true;
        }
    }
    return false;
}

/*
 * Error function
 */
static SkCodec::Result gif_error(const char* msg, SkCodec::Result result = SkCodec::kInvalidInput) {
    SkCodecPrintf("Gif Error: %s\n", msg);
    return result;
}

/*
 * Assumes IsGif was called and returned true
 * Creates a gif decoder
 * Reads enough of the stream to determine the image format
 */
SkCodec* SkGifCodec::NewFromStream(SkStream* stream) {
    std::unique_ptr<SkGifImageReader> reader(new SkGifImageReader(stream));
    if (!reader->parse(SkGifImageReader::SkGIFSizeQuery)) {
        // Fatal error occurred.
        return nullptr;
    }

    // If no images are in the data, or the first header is not yet defined, we cannot
    // create a codec. In either case, the width and height are not yet known.
    if (0 == reader->imagesCount() || !reader->frameContext(0)->isHeaderDefined()) {
        return nullptr;
    }

    // isHeaderDefined() will not return true if the screen size is empty.
    SkASSERT(reader->screenHeight() > 0 && reader->screenWidth() > 0);

    const auto alpha = reader->firstFrameHasAlpha() ? SkEncodedInfo::kBinary_Alpha
                                                    : SkEncodedInfo::kOpaque_Alpha;
    // Use kPalette since Gifs are encoded with a color table.
    // FIXME: Gifs can actually be encoded with 4-bits per pixel. Using 8 works, but we could skip
    //        expanding to 8 bits and take advantage of the SkSwizzler to work from 4.
    const auto encodedInfo = SkEncodedInfo::Make(SkEncodedInfo::kPalette_Color, alpha, 8);

    // Although the encodedInfo is always kPalette_Color, it is possible that kIndex_8 is
    // unsupported if the frame is subset and there is no transparent pixel.
    const auto colorType = reader->firstFrameSupportsIndex8() ? kIndex_8_SkColorType
                                                              : kN32_SkColorType;
    // The choice of unpremul versus premul is arbitrary, since all colors are either fully
    // opaque or fully transparent (i.e. kBinary), but we stored the transparent colors as all
    // zeroes, which is arguably premultiplied.
    const auto alphaType = reader->firstFrameHasAlpha() ? kUnpremul_SkAlphaType
                                                        : kOpaque_SkAlphaType;

    const auto imageInfo = SkImageInfo::Make(reader->screenWidth(), reader->screenHeight(),
                                             colorType, alphaType,
                                             SkColorSpace::MakeSRGB());
    return new SkGifCodec(encodedInfo, imageInfo, reader.release());
}

bool SkGifCodec::onRewind() {
    fReader->clearDecodeState();
    return true;
}

SkGifCodec::SkGifCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
                       SkGifImageReader* reader)
    : INHERITED(encodedInfo, imageInfo, nullptr)
    , fReader(reader)
    , fTmpBuffer(nullptr)
    , fSwizzler(nullptr)
    , fCurrColorTable(nullptr)
    , fCurrColorTableIsReal(false)
    , fFilledBackground(false)
    , fFirstCallToIncrementalDecode(false)
    , fDst(nullptr)
    , fDstRowBytes(0)
    , fRowsDecoded(0)
{
    reader->setClient(this);
}

size_t SkGifCodec::onGetFrameCount() {
    fReader->parse(SkGifImageReader::SkGIFFrameCountQuery);
    return fReader->imagesCount();
}

bool SkGifCodec::onGetFrameInfo(size_t i, SkCodec::FrameInfo* frameInfo) const {
    if (i >= fReader->imagesCount()) {
        return false;
    }

    const SkGIFFrameContext* frameContext = fReader->frameContext(i);
    if (!frameContext->reachedStartOfData()) {
        return false;
    }

    if (frameInfo) {
        frameInfo->fDuration = frameContext->delayTime();
        frameInfo->fRequiredFrame = frameContext->getRequiredFrame();
        frameInfo->fFullyReceived = frameContext->isComplete();
        frameInfo->fAlphaType = frameContext->hasAlpha() ? kUnpremul_SkAlphaType
                                                         : kOpaque_SkAlphaType;
    }
    return true;
}

int SkGifCodec::onGetRepetitionCount() {
    fReader->parse(SkGifImageReader::SkGIFLoopCountQuery);
    return fReader->loopCount();
}

static const SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

void SkGifCodec::initializeColorTable(const SkImageInfo& dstInfo, size_t frameIndex) {
    SkColorType colorTableColorType = dstInfo.colorType();
    if (this->colorXform()) {
        colorTableColorType = kXformSrcColorType;
    }

    sk_sp<SkColorTable> currColorTable = fReader->getColorTable(colorTableColorType, frameIndex);
    fCurrColorTableIsReal = currColorTable;
    if (!fCurrColorTableIsReal) {
        // This is possible for an empty frame. Create a dummy with one value (transparent).
        SkPMColor color = SK_ColorTRANSPARENT;
        fCurrColorTable.reset(new SkColorTable(&color, 1));
    } else if (this->colorXform() && !fXformOnDecode) {
        SkPMColor dstColors[256];
        const SkColorSpaceXform::ColorFormat dstFormat =
                select_xform_format_ct(dstInfo.colorType());
        const SkColorSpaceXform::ColorFormat srcFormat = select_xform_format(kXformSrcColorType);
        const SkAlphaType xformAlphaType = select_xform_alpha(dstInfo.alphaType(),
                                                              this->getInfo().alphaType());
        SkAssertResult(this->colorXform()->apply(dstFormat, dstColors, srcFormat,
                                                 currColorTable->readColors(),
                                                 currColorTable->count(), xformAlphaType));
        fCurrColorTable.reset(new SkColorTable(dstColors, currColorTable->count()));
    } else {
        fCurrColorTable = std::move(currColorTable);
    }
}


SkCodec::Result SkGifCodec::prepareToDecode(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
        int* inputColorCount, const Options& opts) {
    // Check for valid input parameters
    if (!conversion_possible(dstInfo, this->getInfo()) ||
        !this->initializeColorXform(dstInfo, opts.fPremulBehavior))
    {
        return gif_error("Cannot convert input type to output type.\n", kInvalidConversion);
    }

    fXformOnDecode = false;
    if (this->colorXform()) {
        fXformOnDecode = apply_xform_on_decode(dstInfo.colorType(), this->getEncodedInfo().color());
        if (fXformOnDecode) {
            fXformBuffer.reset(new uint32_t[dstInfo.width()]);
            sk_bzero(fXformBuffer.get(), dstInfo.width() * sizeof(uint32_t));
        }
    }

    if (opts.fSubset) {
        return gif_error("Subsets not supported.\n", kUnimplemented);
    }

    const size_t frameIndex = opts.fFrameIndex;
    if (frameIndex > 0) {
        switch (dstInfo.colorType()) {
            case kIndex_8_SkColorType:
                // FIXME: It is possible that a later frame can be decoded to index8, if it does one
                // of the following:
                // - Covers the entire previous frame
                // - Shares a color table (and transparent index) with any prior frames that are
                //   showing.
                // We must support index8 for the first frame to be backwards compatible on Android,
                // but we do not (currently) need to support later frames as index8.
                return gif_error("Cannot decode multiframe gif (except frame 0) as index 8.\n",
                                 kInvalidConversion);
            case kRGB_565_SkColorType:
                // FIXME: In theory, we might be able to support this, but it's not clear that it
                // is necessary (Chromium does not decode to 565, and Android does not decode
                // frames beyond the first). Disabling it because it is somewhat difficult:
                // - If there is a transparent pixel, and this frame draws on top of another frame
                //   (if the frame is independent with a transparent pixel, we should not decode to
                //   565 anyway, since it is not opaque), we need to skip drawing the transparent
                //   pixels (see writeTransparentPixels in haveDecodedRow). We currently do this by
                //   first swizzling into temporary memory, then copying into the destination. (We
                //   let the swizzler handle it first because it may need to sample.) After
                //   swizzling to 565, we do not know which pixels in our temporary memory
                //   correspond to the transparent pixel, so we do not know what to skip. We could
                //   special case the non-sampled case (no need to swizzle), but as this is
                //   currently unused we can just not support it.
                return gif_error("Cannot decode multiframe gif (except frame 0) as 565.\n",
                                 kInvalidConversion);
            default:
                break;
        }
    }

    fReader->parse((SkGifImageReader::SkGIFParseQuery) frameIndex);

    if (frameIndex >= fReader->imagesCount()) {
        return gif_error("frame index out of range!\n", kIncompleteInput);
    }

    if (!fReader->frameContext(frameIndex)->reachedStartOfData()) {
        // We have parsed enough to know that there is a color map, but cannot
        // parse the map itself yet. Exit now, so we do not build an incorrect
        // table.
        return gif_error("color map not available yet\n", kIncompleteInput);
    }

    fTmpBuffer.reset(new uint8_t[dstInfo.minRowBytes()]);

    this->initializeColorTable(dstInfo, frameIndex);
    this->initializeSwizzler(dstInfo, frameIndex);

    SkASSERT(fCurrColorTable);
    if (inputColorCount) {
        *inputColorCount = fCurrColorTable->count();
    }
    copy_color_table(dstInfo, fCurrColorTable.get(), inputColorPtr, inputColorCount);

    return kSuccess;
}

void SkGifCodec::initializeSwizzler(const SkImageInfo& dstInfo, size_t frameIndex) {
    const SkGIFFrameContext* frame = fReader->frameContext(frameIndex);
    // This is only called by prepareToDecode, which ensures frameIndex is in range.
    SkASSERT(frame);

    const int xBegin = frame->xOffset();
    const int xEnd = std::min(static_cast<int>(frame->xOffset() + frame->width()),
                              static_cast<int>(fReader->screenWidth()));

    // CreateSwizzler only reads left and right of the frame. We cannot use the frame's raw
    // frameRect, since it might extend beyond the edge of the frame.
    SkIRect swizzleRect = SkIRect::MakeLTRB(xBegin, 0, xEnd, 0);

    SkImageInfo swizzlerInfo = dstInfo;
    if (this->colorXform()) {
        swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
    }

    // The default Options should be fine:
    // - we'll ignore if the memory is zero initialized - unless we're the first frame, this won't
    //   matter anyway.
    // - subsets are not supported for gif
    // - the swizzler does not need to know about the frame.
    // We may not be able to use the real Options anyway, since getPixels does not store it (due to
    // a bug).
    fSwizzler.reset(SkSwizzler::CreateSwizzler(this->getEncodedInfo(),
                    fCurrColorTable->readColors(), swizzlerInfo, Options(), &swizzleRect));
    SkASSERT(fSwizzler.get());
}

/*
 * Initiates the gif decode
 */
SkCodec::Result SkGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* pixels, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount,
                                        int* rowsDecoded) {
    Result result = this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, opts);
    switch (result) {
        case kSuccess:
            break;
        case kIncompleteInput:
            // onStartIncrementalDecode treats this as incomplete, since it may
            // provide more data later, but in this case, no more data will be
            // provided, and there is nothing to draw. We also cannot return
            // kIncompleteInput, which will make SkCodec attempt to fill
            // remaining rows, but that requires an SkSwizzler, which we have
            // not created.
            return kInvalidInput;
        default:
            return result;
    }

    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        return gif_error("Scaling not supported.\n", kInvalidScale);
    }

    fDst = pixels;
    fDstRowBytes = dstRowBytes;

    return this->decodeFrame(true, opts, rowsDecoded);
}

SkCodec::Result SkGifCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                     void* pixels, size_t dstRowBytes,
                                                     const SkCodec::Options& opts,
                                                     SkPMColor* inputColorPtr,
                                                     int* inputColorCount) {
    Result result = this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, opts);
    if (result != kSuccess) {
        return result;
    }

    fDst = pixels;
    fDstRowBytes = dstRowBytes;

    fFirstCallToIncrementalDecode = true;

    return kSuccess;
}

SkCodec::Result SkGifCodec::onIncrementalDecode(int* rowsDecoded) {
    // It is possible the client has appended more data. Parse, if needed.
    const auto& options = this->options();
    const size_t frameIndex = options.fFrameIndex;
    fReader->parse((SkGifImageReader::SkGIFParseQuery) frameIndex);

    const bool firstCallToIncrementalDecode = fFirstCallToIncrementalDecode;
    fFirstCallToIncrementalDecode = false;
    return this->decodeFrame(firstCallToIncrementalDecode, options, rowsDecoded);
}

SkCodec::Result SkGifCodec::decodeFrame(bool firstAttempt, const Options& opts, int* rowsDecoded) {
    const SkImageInfo& dstInfo = this->dstInfo();
    const size_t frameIndex = opts.fFrameIndex;
    SkASSERT(frameIndex < fReader->imagesCount());
    const SkGIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    if (firstAttempt) {
        // rowsDecoded reports how many rows have been initialized, so a layer above
        // can fill the rest. In some cases, we fill the background before decoding
        // (or it is already filled for us), so we report rowsDecoded to be the full
        // height.
        bool filledBackground = false;
        if (frameContext->getRequiredFrame() == kNone) {
            // We may need to clear to transparent for one of the following reasons:
            // - The frameRect does not cover the full bounds. haveDecodedRow will
            //   only draw inside the frameRect, so we need to clear the rest.
            // - The frame is interlaced. There is no obvious way to fill
            //   afterwards for an incomplete image. (FIXME: Does the first pass
            //   cover all rows? If so, we do not have to fill here.)
            // - There is no color table for this frame. In that case will not
            //   draw anything, so we need to fill.
            if (frameContext->frameRect() != this->getInfo().bounds()
                    || frameContext->interlaced() || !fCurrColorTableIsReal) {
                // fill ignores the width (replaces it with the actual, scaled width).
                // But we need to scale in Y.
                const int scaledHeight = get_scaled_dimension(dstInfo.height(),
                                                              fSwizzler->sampleY());
                auto fillInfo = dstInfo.makeWH(0, scaledHeight);
                fSwizzler->fill(fillInfo, fDst, fDstRowBytes, this->getFillValue(dstInfo),
                                opts.fZeroInitialized);
                filledBackground = true;
            }
        } else {
            // Not independent
            if (!opts.fHasPriorFrame) {
                // Decode that frame into pixels.
                Options prevFrameOpts(opts);
                prevFrameOpts.fFrameIndex = frameContext->getRequiredFrame();
                prevFrameOpts.fHasPriorFrame = false;
                // The prior frame may have a different color table, so update it and the
                // swizzler.
                this->initializeColorTable(dstInfo, prevFrameOpts.fFrameIndex);
                this->initializeSwizzler(dstInfo, prevFrameOpts.fFrameIndex);

                const Result prevResult = this->decodeFrame(true, prevFrameOpts, nullptr);
                switch (prevResult) {
                    case kSuccess:
                        // Prior frame succeeded. Carry on.
                        break;
                    case kIncompleteInput:
                        // Prior frame was incomplete. So this frame cannot be decoded.
                        return kInvalidInput;
                    default:
                        return prevResult;
                }

                // Go back to using the correct color table for this frame.
                this->initializeColorTable(dstInfo, frameIndex);
                this->initializeSwizzler(dstInfo, frameIndex);
            }
            const auto* prevFrame = fReader->frameContext(frameContext->getRequiredFrame());
            if (prevFrame->getDisposalMethod() == SkCodecAnimation::RestoreBGColor_DisposalMethod) {
                SkIRect prevRect = prevFrame->frameRect();
                if (prevRect.intersect(this->getInfo().bounds())) {
                    // Do the divide ourselves for left and top, since we do not want
                    // get_scaled_dimension to upgrade 0 to 1. (This is similar to SkSampledCodec's
                    // sampling of the subset.)
                    auto left = prevRect.fLeft / fSwizzler->sampleX();
                    auto top = prevRect.fTop / fSwizzler->sampleY();
                    void* const eraseDst = SkTAddOffset<void>(fDst, top * fDstRowBytes
                            + left * SkColorTypeBytesPerPixel(dstInfo.colorType()));
                    auto width = get_scaled_dimension(prevRect.width(), fSwizzler->sampleX());
                    auto height = get_scaled_dimension(prevRect.height(), fSwizzler->sampleY());
                    // fSwizzler->fill() would fill to the scaled width of the frame, but we want to
                    // fill to the scaled with of the width of the PRIOR frame, so we do all the
                    // scaling ourselves and call the static version.
                    SkSampler::Fill(dstInfo.makeWH(width, height), eraseDst,
                                    fDstRowBytes, this->getFillValue(dstInfo), kNo_ZeroInitialized);
                }
            }
            filledBackground = true;
        }

        fFilledBackground = filledBackground;
        if (filledBackground) {
            // Report the full (scaled) height, since the client will never need to fill.
            fRowsDecoded = get_scaled_dimension(dstInfo.height(), fSwizzler->sampleY());
        } else {
            // This will be updated by haveDecodedRow.
            fRowsDecoded = 0;
        }
    }

    if (!fCurrColorTableIsReal) {
        // Nothing to draw this frame.
        return kSuccess;
    }

    // Note: there is a difference between the following call to SkGifImageReader::decode
    // returning false and leaving frameDecoded false:
    // - If the method returns false, there was an error in the stream. We still treat this as
    //   incomplete, since we have already decoded some rows.
    // - If frameDecoded is false, that just means that we do not have enough data. If more data
    //   is supplied, we may be able to continue decoding this frame. We also treat this as
    //   incomplete.
    // FIXME: Ensure that we do not attempt to continue decoding if the method returns false and
    // more data is supplied.
    bool frameDecoded = false;
    if (!fReader->decode(frameIndex, &frameDecoded) || !frameDecoded) {
        if (rowsDecoded) {
            *rowsDecoded = fRowsDecoded;
        }
        return kIncompleteInput;
    }

    return kSuccess;
}

uint64_t SkGifCodec::onGetFillValue(const SkImageInfo& dstInfo) const {
    // Note: Using fCurrColorTable relies on having called initializeColorTable already.
    // This is (currently) safe because this method is only called when filling, after
    // initializeColorTable has been called.
    // FIXME: Is there a way to make this less fragile?
    if (dstInfo.colorType() == kIndex_8_SkColorType && fCurrColorTableIsReal) {
        // We only support index 8 for the first frame, for backwards
        // compatibity on Android, so we are using the color table for the first frame.
        SkASSERT(this->options().fFrameIndex == 0);
        // Use the transparent index for the first frame.
        const size_t transPixel = fReader->frameContext(0)->transparentPixel();
        if (transPixel < (size_t) fCurrColorTable->count()) {
            return transPixel;
        }
        // Fall through to return SK_ColorTRANSPARENT (i.e. 0). This choice is arbitrary,
        // but we have to pick something inside the color table, and this one is as good
        // as any.
    }
    // Using transparent as the fill value matches the behavior in Chromium,
    // which ignores the background color.
    // If the colorType is kIndex_8, and there was no color table (i.e.
    // fCurrColorTableIsReal is false), this value (zero) corresponds to the
    // only entry in the dummy color table provided to the client.
    return SK_ColorTRANSPARENT;
}

void SkGifCodec::applyXformRow(const SkImageInfo& dstInfo, void* dst, const uint8_t* src) const {
    if (this->colorXform() && fXformOnDecode) {
        fSwizzler->swizzle(fXformBuffer.get(), src);

        const SkColorSpaceXform::ColorFormat dstFormat = select_xform_format(dstInfo.colorType());
        const SkColorSpaceXform::ColorFormat srcFormat = select_xform_format(kXformSrcColorType);
        const SkAlphaType xformAlphaType = select_xform_alpha(dstInfo.alphaType(),
                                                              this->getInfo().alphaType());
        const int xformWidth = get_scaled_dimension(dstInfo.width(), fSwizzler->sampleX());
        SkAssertResult(this->colorXform()->apply(dstFormat, dst, srcFormat, fXformBuffer.get(),
                                                 xformWidth, xformAlphaType));
    } else {
        fSwizzler->swizzle(dst, src);
    }
}

bool SkGifCodec::haveDecodedRow(size_t frameIndex, const unsigned char* rowBegin,
                                size_t rowNumber, unsigned repeatCount, bool writeTransparentPixels)
{
    const SkGIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    // The pixel data and coordinates supplied to us are relative to the frame's
    // origin within the entire image size, i.e.
    // (frameContext->xOffset, frameContext->yOffset). There is no guarantee
    // that width == (size().width() - frameContext->xOffset), so
    // we must ensure we don't run off the end of either the source data or the
    // row's X-coordinates.
    const size_t width = frameContext->width();
    const int xBegin = frameContext->xOffset();
    const int yBegin = frameContext->yOffset() + rowNumber;
    const int xEnd = std::min(static_cast<int>(frameContext->xOffset() + width),
                              this->getInfo().width());
    const int yEnd = std::min(static_cast<int>(frameContext->yOffset() + rowNumber + repeatCount),
                              this->getInfo().height());
    // FIXME: No need to make the checks on width/xBegin/xEnd for every row. We could instead do
    // this once in prepareToDecode.
    if (!width || (xBegin < 0) || (yBegin < 0) || (xEnd <= xBegin) || (yEnd <= yBegin))
        return true;

    // yBegin is the first row in the non-sampled image. dstRow will be the row in the output,
    // after potentially scaling it.
    int dstRow = yBegin;

    const int sampleY = fSwizzler->sampleY();
    if (sampleY > 1) {
        // Check to see whether this row or one that falls in the repeatCount is needed in the
        // output.
        bool foundNecessaryRow = false;
        for (unsigned i = 0; i < repeatCount; i++) {
            const int potentialRow = yBegin + i;
            if (fSwizzler->rowNeeded(potentialRow)) {
                dstRow = potentialRow / sampleY;
                const int scaledHeight = get_scaled_dimension(this->dstInfo().height(), sampleY);
                if (dstRow >= scaledHeight) {
                    return true;
                }

                foundNecessaryRow = true;
                repeatCount -= i;

                repeatCount = (repeatCount - 1) / sampleY + 1;

                // Make sure the repeatCount does not take us beyond the end of the dst
                if (dstRow + (int) repeatCount > scaledHeight) {
                    repeatCount = scaledHeight - dstRow;
                    SkASSERT(repeatCount >= 1);
                }
                break;
            }
        }

        if (!foundNecessaryRow) {
            return true;
        }
    } else {
        // Make sure the repeatCount does not take us beyond the end of the dst
        SkASSERT(this->dstInfo().height() >= yBegin);
        repeatCount = SkTMin(repeatCount, (unsigned) (this->dstInfo().height() - yBegin));
    }

    if (!fFilledBackground) {
        // At this point, we are definitely going to write the row, so count it towards the number
        // of rows decoded.
        // We do not consider the repeatCount, which only happens for interlaced, in which case we
        // have already set fRowsDecoded to the proper value (reflecting that we have filled the
        // background).
        fRowsDecoded++;
    }

    // decodeFrame will early exit if this is false, so this method will not be
    // called.
    SkASSERT(fCurrColorTableIsReal);

    // The swizzler takes care of offsetting into the dst width-wise.
    void* dstLine = SkTAddOffset<void>(fDst, dstRow * fDstRowBytes);

    // We may or may not need to write transparent pixels to the buffer.
    // If we're compositing against a previous image, it's wrong, but if
    // we're decoding an interlaced gif and displaying it "Haeberli"-style,
    // we must write these for passes beyond the first, or the initial passes
    // will "show through" the later ones.
    const auto dstInfo = this->dstInfo();
    if (writeTransparentPixels) {
        this->applyXformRow(dstInfo, dstLine, rowBegin);
    } else {
        sk_bzero(fTmpBuffer.get(), dstInfo.minRowBytes());
        this->applyXformRow(dstInfo, fTmpBuffer.get(), rowBegin);

        const size_t offsetBytes = fSwizzler->swizzleOffsetBytes();
        switch (dstInfo.colorType()) {
            case kBGRA_8888_SkColorType:
            case kRGBA_8888_SkColorType: {
                uint32_t* dstPixel = SkTAddOffset<uint32_t>(dstLine, offsetBytes);
                uint32_t* srcPixel = SkTAddOffset<uint32_t>(fTmpBuffer.get(), offsetBytes);
                for (int i = 0; i < fSwizzler->swizzleWidth(); i++) {
                    // Technically SK_ColorTRANSPARENT is an SkPMColor, and srcPixel would have
                    // the opposite swizzle for the non-native swizzle, but TRANSPARENT is all
                    // zeroes, which is the same either way.
                    if (*srcPixel != SK_ColorTRANSPARENT) {
                        *dstPixel = *srcPixel;
                    }
                    dstPixel++;
                    srcPixel++;
                }
                break;
            }
            case kRGBA_F16_SkColorType: {
                uint64_t* dstPixel = SkTAddOffset<uint64_t>(dstLine, offsetBytes);
                uint64_t* srcPixel = SkTAddOffset<uint64_t>(fTmpBuffer.get(), offsetBytes);
                for (int i = 0; i < fSwizzler->swizzleWidth(); i++) {
                    if (*srcPixel != 0) {
                        *dstPixel = *srcPixel;
                    }
                    dstPixel++;
                    srcPixel++;
                }
                break;
            }
            default:
                SkASSERT(false);
                break;
        }
    }

    // Tell the frame to copy the row data if need be.
    if (repeatCount > 1) {
        const size_t bytesPerPixel = SkColorTypeBytesPerPixel(this->dstInfo().colorType());
        const size_t bytesToCopy = fSwizzler->swizzleWidth() * bytesPerPixel;
        void* copiedLine = SkTAddOffset<void>(dstLine, fSwizzler->swizzleOffsetBytes());
        void* dst = copiedLine;
        for (unsigned i = 1; i < repeatCount; i++) {
            dst = SkTAddOffset<void>(dst, fDstRowBytes);
            memcpy(dst, copiedLine, bytesToCopy);
        }
    }

    return true;
}
