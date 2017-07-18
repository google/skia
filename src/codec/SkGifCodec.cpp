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
#include "SkRasterPipeline.h"
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

SkCodec* SkGifCodec::NewFromStream(SkStream* stream, Result* result) {
    std::unique_ptr<SkGifImageReader> reader(new SkGifImageReader(stream));
    *result = reader->parse(SkGifImageReader::SkGIFSizeQuery);
    if (*result != kSuccess) {
        return nullptr;
    }

    // If no images are in the data, or the first header is not yet defined, we cannot
    // create a codec. In either case, the width and height are not yet known.
    auto* frame = reader->frameContext(0);
    if (!frame || !frame->isHeaderDefined()) {
        *result = kInvalidInput;
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

    // The choice of unpremul versus premul is arbitrary, since all colors are either fully
    // opaque or fully transparent (i.e. kBinary), but we stored the transparent colors as all
    // zeroes, which is arguably premultiplied.
    const auto alphaType = reader->firstFrameHasAlpha() ? kUnpremul_SkAlphaType
                                                        : kOpaque_SkAlphaType;

    const auto imageInfo = SkImageInfo::Make(reader->screenWidth(), reader->screenHeight(),
                                             kN32_SkColorType, alphaType,
                                             SkColorSpace::MakeSRGB());
    return new SkGifCodec(encodedInfo, imageInfo, reader.release());
}

bool SkGifCodec::onRewind() {
    fReader->clearDecodeState();
    return true;
}

SkGifCodec::SkGifCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
                       SkGifImageReader* reader)
    : INHERITED(encodedInfo, imageInfo, SkColorSpaceXform::kRGBA_8888_ColorFormat, nullptr)
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

int SkGifCodec::onGetFrameCount() {
    fReader->parse(SkGifImageReader::SkGIFFrameCountQuery);
    return fReader->imagesCount();
}

bool SkGifCodec::onGetFrameInfo(int i, SkCodec::FrameInfo* frameInfo) const {
    if (i >= fReader->imagesCount()) {
        return false;
    }

    const SkGIFFrameContext* frameContext = fReader->frameContext(i);
    SkASSERT(frameContext->reachedStartOfData());

    if (frameInfo) {
        frameInfo->fDuration = frameContext->getDuration();
        frameInfo->fRequiredFrame = frameContext->getRequiredFrame();
        frameInfo->fFullyReceived = frameContext->isComplete();
        frameInfo->fAlphaType = frameContext->hasAlpha() ? kUnpremul_SkAlphaType
                                                         : kOpaque_SkAlphaType;
        frameInfo->fDisposalMethod = frameContext->getDisposalMethod();
    }
    return true;
}

int SkGifCodec::onGetRepetitionCount() {
    fReader->parse(SkGifImageReader::SkGIFLoopCountQuery);
    return fReader->loopCount();
}

static const SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;
static const SkAlphaType kXformAlphaType    = kUnpremul_SkAlphaType;

void SkGifCodec::initializeColorTable(const SkImageInfo& dstInfo, int frameIndex) {
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
    } else if (this->colorXform() && !this->xformOnDecode()) {
        SkPMColor dstColors[256];
        this->applyColorXform(dstColors, currColorTable->readColors(), currColorTable->count(),
                              kXformAlphaType);
        fCurrColorTable.reset(new SkColorTable(dstColors, currColorTable->count()));
    } else {
        fCurrColorTable = std::move(currColorTable);
    }
}


SkCodec::Result SkGifCodec::prepareToDecode(const SkImageInfo& dstInfo, const Options& opts) {
    if (opts.fSubset) {
        return gif_error("Subsets not supported.\n", kUnimplemented);
    }

    const int frameIndex = opts.fFrameIndex;
    if (frameIndex > 0 && kRGB_565_SkColorType == dstInfo.colorType()) {
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
    }

    const auto* frame = fReader->frameContext(frameIndex);
    SkASSERT(frame);
    if (0 == frameIndex) {
        // SkCodec does not have a way to just parse through frame 0, so we
        // have to do so manually, here.
        fReader->parse((SkGifImageReader::SkGIFParseQuery) 0);
        if (!frame->reachedStartOfData()) {
            // We have parsed enough to know that there is a color map, but cannot
            // parse the map itself yet. Exit now, so we do not build an incorrect
            // table.
            return gif_error("color map not available yet\n", kIncompleteInput);
        }
    } else {
        // Parsing happened in SkCodec::getPixels.
        SkASSERT(frameIndex < fReader->imagesCount());
        SkASSERT(frame->reachedStartOfData());
    }

    const auto at = frame->hasAlpha() ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
    const auto srcInfo = this->getInfo().makeAlphaType(at);
    if (!conversion_possible(dstInfo, srcInfo) ||
        !this->initializeColorXform(dstInfo, opts.fPremulBehavior))
    {
        return gif_error("Cannot convert input type to output type.\n", kInvalidConversion);
    }

    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[dstInfo.width()]);
        sk_bzero(fXformBuffer.get(), dstInfo.width() * sizeof(uint32_t));
    }

    fTmpBuffer.reset(new uint8_t[dstInfo.minRowBytes()]);

    this->initializeColorTable(dstInfo, frameIndex);
    this->initializeSwizzler(dstInfo, frameIndex);

    SkASSERT(fCurrColorTable);
    return kSuccess;
}

void SkGifCodec::initializeSwizzler(const SkImageInfo& dstInfo, int frameIndex) {
    const SkGIFFrameContext* frame = fReader->frameContext(frameIndex);
    // This is only called by prepareToDecode, which ensures frameIndex is in range.
    SkASSERT(frame);

    const int xBegin = frame->xOffset();
    const int xEnd = std::min(frame->frameRect().right(), fReader->screenWidth());

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
                                        int* rowsDecoded) {
    Result result = this->prepareToDecode(dstInfo, opts);
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
                                                     const SkCodec::Options& opts) {
    Result result = this->prepareToDecode(dstInfo, opts);
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
    const int frameIndex = options.fFrameIndex;
    fReader->parse((SkGifImageReader::SkGIFParseQuery) frameIndex);

    const bool firstCallToIncrementalDecode = fFirstCallToIncrementalDecode;
    fFirstCallToIncrementalDecode = false;
    return this->decodeFrame(firstCallToIncrementalDecode, options, rowsDecoded);
}

SkCodec::Result SkGifCodec::decodeFrame(bool firstAttempt, const Options& opts, int* rowsDecoded) {
    const SkImageInfo& dstInfo = this->dstInfo();
    const int frameIndex = opts.fFrameIndex;
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
            // Not independent.
            // SkCodec ensured that the prior frame has been decoded.
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

    bool frameDecoded = false;
    const bool fatalError = !fReader->decode(frameIndex, &frameDecoded);
    if (fatalError || !frameDecoded) {
        if (rowsDecoded) {
            *rowsDecoded = fRowsDecoded;
        }
        if (fatalError) {
            return kErrorInInput;
        }
        return kIncompleteInput;
    }

    return kSuccess;
}

uint64_t SkGifCodec::onGetFillValue(const SkImageInfo& dstInfo) const {
    // Using transparent as the fill value matches the behavior in Chromium,
    // which ignores the background color.
    return SK_ColorTRANSPARENT;
}

void SkGifCodec::applyXformRow(const SkImageInfo& dstInfo, void* dst, const uint8_t* src) const {
    if (this->xformOnDecode()) {
        SkASSERT(this->colorXform());
        fSwizzler->swizzle(fXformBuffer.get(), src);

        const int xformWidth = get_scaled_dimension(dstInfo.width(), fSwizzler->sampleX());
        this->applyColorXform(dst, fXformBuffer.get(), xformWidth, kXformAlphaType);
    } else {
        fSwizzler->swizzle(dst, src);
    }
}

bool SkGifCodec::haveDecodedRow(int frameIndex, const unsigned char* rowBegin,
                                int rowNumber, int repeatCount, bool writeTransparentPixels)
{
    const SkGIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    // The pixel data and coordinates supplied to us are relative to the frame's
    // origin within the entire image size, i.e.
    // (frameContext->xOffset, frameContext->yOffset). There is no guarantee
    // that width == (size().width() - frameContext->xOffset), so
    // we must ensure we don't run off the end of either the source data or the
    // row's X-coordinates.
    const int width = frameContext->width();
    const int xBegin = frameContext->xOffset();
    const int yBegin = frameContext->yOffset() + rowNumber;
    const int xEnd = std::min(xBegin + width, this->getInfo().width());
    const int yEnd = std::min(yBegin + rowNumber + repeatCount, this->getInfo().height());
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
        for (int i = 0; i < repeatCount; i++) {
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
                if (dstRow + repeatCount > scaledHeight) {
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
        repeatCount = SkTMin(repeatCount, this->dstInfo().height() - yBegin);
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
        this->applyXformRow(dstInfo, fTmpBuffer.get(), rowBegin);

        size_t offsetBytes = fSwizzler->swizzleOffsetBytes();
        if (dstInfo.colorType() == kRGBA_F16_SkColorType) {
            // Account for the fact that post-swizzling we converted to F16,
            // which is twice as wide.
            offsetBytes *= 2;
        }
        SkRasterPipeline_<256> p;
        SkRasterPipeline::StockStage storeDst;
        void* src = SkTAddOffset<void>(fTmpBuffer.get(), offsetBytes);
        void* dst = SkTAddOffset<void>(dstLine, offsetBytes);
        switch (dstInfo.colorType()) {
            case kBGRA_8888_SkColorType:
            case kRGBA_8888_SkColorType:
                p.append(SkRasterPipeline::load_8888_dst, &dst);
                p.append(SkRasterPipeline::load_8888, &src);
                storeDst = SkRasterPipeline::store_8888;
                break;
            case kRGBA_F16_SkColorType:
                p.append(SkRasterPipeline::load_f16_dst, &dst);
                p.append(SkRasterPipeline::load_f16, &src);
                storeDst = SkRasterPipeline::store_f16;
                break;
            default:
                SkASSERT(false);
                return false;
                break;
        }
        p.append(SkRasterPipeline::srcover);
        p.append(storeDst, &dst);
        p.run(0, 0, fSwizzler->swizzleWidth());
    }

    // Tell the frame to copy the row data if need be.
    if (repeatCount > 1) {
        const size_t bytesPerPixel = SkColorTypeBytesPerPixel(this->dstInfo().colorType());
        const size_t bytesToCopy = fSwizzler->swizzleWidth() * bytesPerPixel;
        void* copiedLine = SkTAddOffset<void>(dstLine, fSwizzler->swizzleOffsetBytes());
        void* dst = copiedLine;
        for (int i = 1; i < repeatCount; i++) {
            dst = SkTAddOffset<void>(dst, fDstRowBytes);
            memcpy(dst, copiedLine, bytesToCopy);
        }
    }

    return true;
}
