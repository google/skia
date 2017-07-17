/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCodecAnimation.h"
#include "SkCodecAnimationPriv.h"
#include "SkCodecPriv.h"
#include "SkColorSpaceXform.h"
#include "SkRasterPipeline.h"
#include "SkSampler.h"
#include "SkStreamPriv.h"
#include "SkTemplates.h"
#include "SkWebpCodec.h"

// A WebP decoder on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   https://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp-webp-image-library
//   https://chromium.googlesource.com/webm/libwebp

// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/decode.h"
#include "webp/demux.h"
#include "webp/encode.h"

bool SkWebpCodec::IsWebp(const void* buf, size_t bytesRead) {
    // WEBP starts with the following:
    // RIFFXXXXWEBPVP
    // Where XXXX is unspecified.
    const char* bytes = static_cast<const char*>(buf);
    return bytesRead >= 14 && !memcmp(bytes, "RIFF", 4) && !memcmp(&bytes[8], "WEBPVP", 6);
}

static SkAlphaType alpha_type(bool hasAlpha) {
    return hasAlpha ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
}

// Parse headers of RIFF container, and check for valid Webp (VP8) content.
// Returns an SkWebpCodec on success
SkCodec* SkWebpCodec::NewFromStream(SkStream* stream, Result* result) {
    std::unique_ptr<SkStream> streamDeleter(stream);

    // Webp demux needs a contiguous data buffer.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream);

        // If we are forced to copy the stream to a data, we can go ahead and delete the stream.
        streamDeleter.reset(nullptr);
    }

    // It's a little strange that the |demux| will outlive |webpData|, though it needs the
    // pointer in |webpData| to remain valid.  This works because the pointer remains valid
    // until the SkData is freed.
    WebPData webpData = { data->bytes(), data->size() };
    WebPDemuxState state;
    SkAutoTCallVProc<WebPDemuxer, WebPDemuxDelete> demux(WebPDemuxPartial(&webpData, &state));
    switch (state) {
        case WEBP_DEMUX_PARSE_ERROR:
            *result = kInvalidInput;
            return nullptr;
        case WEBP_DEMUX_PARSING_HEADER:
            *result = kIncompleteInput;
            return nullptr;
        case WEBP_DEMUX_PARSED_HEADER:
        case WEBP_DEMUX_DONE:
            SkASSERT(demux);
            break;
    }

    const int width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    const int height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

    // Sanity check for image size that's about to be decoded.
    {
        const int64_t size = sk_64_mul(width, height);
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (!sk_64_isS32(size) || sk_64_asS32(size) > (0x7FFFFFFF >> 2)) {
            *result = kInvalidInput;
            return nullptr;
        }
    }

    WebPChunkIterator chunkIterator;
    SkAutoTCallVProc<WebPChunkIterator, WebPDemuxReleaseChunkIterator> autoCI(&chunkIterator);
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (WebPDemuxGetChunk(demux, "ICCP", 1, &chunkIterator)) {
        colorSpace = SkColorSpace::MakeICC(chunkIterator.chunk.bytes, chunkIterator.chunk.size);
    }
    if (!colorSpace) {
        colorSpace = SkColorSpace::MakeSRGB();
    }

    // Get the first frame and its "features" to determine the color and alpha types.
    WebPIterator frame;
    SkAutoTCallVProc<WebPIterator, WebPDemuxReleaseIterator> autoFrame(&frame);
    if (!WebPDemuxGetFrame(demux, 1, &frame)) {
        *result = kIncompleteInput;
        return nullptr;
    }

    WebPBitstreamFeatures features;
    switch (WebPGetFeatures(frame.fragment.bytes, frame.fragment.size, &features)) {
        case VP8_STATUS_OK:
            break;
        case VP8_STATUS_SUSPENDED:
        case VP8_STATUS_NOT_ENOUGH_DATA:
            *result = kIncompleteInput;
            return nullptr;
        default:
            *result = kInvalidInput;
            return nullptr;
    }

    const bool hasAlpha = SkToBool(frame.has_alpha)
            || frame.width != width || frame.height != height;
    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    switch (features.format) {
        case 0:
            // This indicates a "mixed" format.  We could see this for
            // animated webps (multiple fragments).
            // We could also guess kYUV here, but I think it makes more
            // sense to guess kBGRA which is likely closer to the final
            // output.  Otherwise, we might end up converting
            // BGRA->YUVA->BGRA.
            // Fallthrough:
        case 2:
            // This is the lossless format (BGRA).
            if (hasAlpha) {
                color = SkEncodedInfo::kBGRA_Color;
                alpha = SkEncodedInfo::kUnpremul_Alpha;
            } else {
                color = SkEncodedInfo::kBGRX_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        case 1:
            // This is the lossy format (YUV).
            if (hasAlpha) {
                color = SkEncodedInfo::kYUVA_Color;
                alpha = SkEncodedInfo::kUnpremul_Alpha;
            } else {
                color = SkEncodedInfo::kYUV_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        default:
            *result = kInvalidInput;
            return nullptr;
    }

    *result = kSuccess;
    SkEncodedInfo info = SkEncodedInfo::Make(color, alpha, 8);
    return new SkWebpCodec(width, height, info, std::move(colorSpace),
                           streamDeleter.release(), demux.release(),
                           std::move(data));
}

SkISize SkWebpCodec::onGetScaledDimensions(float desiredScale) const {
    SkISize dim = this->getInfo().dimensions();
    // SkCodec treats zero dimensional images as errors, so the minimum size
    // that we will recommend is 1x1.
    dim.fWidth = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fWidth));
    dim.fHeight = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fHeight));
    return dim;
}

bool SkWebpCodec::onDimensionsSupported(const SkISize& dim) {
    const SkImageInfo& info = this->getInfo();
    return dim.width() >= 1 && dim.width() <= info.width()
            && dim.height() >= 1 && dim.height() <= info.height();
}

static WEBP_CSP_MODE webp_decode_mode(const SkImageInfo& info) {
    const bool premultiply = info.alphaType() == kPremul_SkAlphaType;
    switch (info.colorType()) {
        case kBGRA_8888_SkColorType:
            return premultiply ? MODE_bgrA : MODE_BGRA;
        case kRGBA_8888_SkColorType:
            return premultiply ? MODE_rgbA : MODE_RGBA;
        case kRGB_565_SkColorType:
            return MODE_RGB_565;
        default:
            return MODE_LAST;
    }
}

SkWebpCodec::Frame* SkWebpCodec::FrameHolder::appendNewFrame(bool hasAlpha) {
    const int i = this->size();
    fFrames.emplace_back(i, hasAlpha);
    return &fFrames[i];
}

bool SkWebpCodec::onGetValidSubset(SkIRect* desiredSubset) const {
    if (!desiredSubset) {
        return false;
    }

    SkIRect dimensions  = SkIRect::MakeSize(this->getInfo().dimensions());
    if (!dimensions.contains(*desiredSubset)) {
        return false;
    }

    // As stated below, libwebp snaps to even left and top. Make sure top and left are even, so we
    // decode this exact subset.
    // Leave right and bottom unmodified, so we suggest a slightly larger subset than requested.
    desiredSubset->fLeft = (desiredSubset->fLeft >> 1) << 1;
    desiredSubset->fTop = (desiredSubset->fTop >> 1) << 1;
    return true;
}

int SkWebpCodec::onGetRepetitionCount() {
    auto flags = WebPDemuxGetI(fDemux.get(), WEBP_FF_FORMAT_FLAGS);
    if (!(flags & ANIMATION_FLAG)) {
        return 0;
    }

    const int repCount = WebPDemuxGetI(fDemux.get(), WEBP_FF_LOOP_COUNT);
    if (0 == repCount) {
        return kRepetitionCountInfinite;
    }

    return repCount;
}

int SkWebpCodec::onGetFrameCount() {
    auto flags = WebPDemuxGetI(fDemux.get(), WEBP_FF_FORMAT_FLAGS);
    if (!(flags & ANIMATION_FLAG)) {
        return 1;
    }

    const uint32_t oldFrameCount = fFrameHolder.size();
    if (fFailed) {
        return oldFrameCount;
    }

    const uint32_t frameCount = WebPDemuxGetI(fDemux, WEBP_FF_FRAME_COUNT);
    if (oldFrameCount == frameCount) {
        // We have already parsed this.
        return frameCount;
    }

    fFrameHolder.reserve(frameCount);

    for (uint32_t i = oldFrameCount; i < frameCount; i++) {
        WebPIterator iter;
        SkAutoTCallVProc<WebPIterator, WebPDemuxReleaseIterator> autoIter(&iter);

        if (!WebPDemuxGetFrame(fDemux.get(), i + 1, &iter)) {
            fFailed = true;
            break;
        }

        // libwebp only reports complete frames of an animated image.
        SkASSERT(iter.complete);

        Frame* frame = fFrameHolder.appendNewFrame(iter.has_alpha);
        frame->setXYWH(iter.x_offset, iter.y_offset, iter.width, iter.height);
        frame->setDisposalMethod(iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND ?
                SkCodecAnimation::DisposalMethod::kRestoreBGColor :
                SkCodecAnimation::DisposalMethod::kKeep);
        frame->setDuration(iter.duration);
        if (WEBP_MUX_BLEND != iter.blend_method) {
            frame->setBlend(SkCodecAnimation::Blend::kBG);
        }
        fFrameHolder.setAlphaAndRequiredFrame(frame);
    }

    return fFrameHolder.size();

}

const SkFrame* SkWebpCodec::FrameHolder::onGetFrame(int i) const {
    return static_cast<const SkFrame*>(this->frame(i));
}

const SkWebpCodec::Frame* SkWebpCodec::FrameHolder::frame(int i) const {
    SkASSERT(i >= 0 && i < this->size());
    return &fFrames[i];
}

bool SkWebpCodec::onGetFrameInfo(int i, FrameInfo* frameInfo) const {
    if (i >= fFrameHolder.size()) {
        return false;
    }

    const Frame* frame = fFrameHolder.frame(i);
    if (!frame) {
        return false;
    }

    if (frameInfo) {
        frameInfo->fRequiredFrame = frame->getRequiredFrame();
        frameInfo->fDuration = frame->getDuration();
        // libwebp only reports fully received frames for an
        // animated image.
        frameInfo->fFullyReceived = true;
        frameInfo->fAlphaType = alpha_type(frame->hasAlpha());
        frameInfo->fDisposalMethod = frame->getDisposalMethod();
    }

    return true;
}

static bool is_8888(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        default:
            return false;
    }
}

static void pick_memory_stages(SkColorType ct, SkRasterPipeline::StockStage* load,
                                               SkRasterPipeline::StockStage* store) {
    switch(ct) {
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType:
        case kARGB_4444_SkColorType:
        case kGray_8_SkColorType:
            SkASSERT(false);
            break;
        case kRGB_565_SkColorType:
            if (load) *load = SkRasterPipeline::load_565;
            if (store) *store = SkRasterPipeline::store_565;
            break;
        case kRGBA_8888_SkColorType:
            if (load) *load = SkRasterPipeline::load_8888;
            if (store) *store = SkRasterPipeline::store_8888;
            break;
        case kBGRA_8888_SkColorType:
            if (load) *load = SkRasterPipeline::load_bgra;
            if (store) *store = SkRasterPipeline::store_bgra;
            break;
        case kRGBA_F16_SkColorType:
            if (load) *load = SkRasterPipeline::load_f16;
            if (store) *store = SkRasterPipeline::store_f16;
            break;
    }
}

static void blend_line(SkColorType dstCT, void* dst,
                       SkColorType srcCT, void* src,
                       bool needsSrgbToLinear, SkAlphaType at,
                       int width) {
    // Setup conversion from the source and dest, which will be the same.
    SkRasterPipeline_<256> convert_to_linear_premul;
    if (needsSrgbToLinear) {
        convert_to_linear_premul.append_from_srgb(at);
    }
    if (kUnpremul_SkAlphaType == at) {
        // srcover assumes premultiplied inputs.
        convert_to_linear_premul.append(SkRasterPipeline::premul);
    }

    SkRasterPipeline_<256> p;
    SkRasterPipeline::StockStage load_dst, store_dst;
    pick_memory_stages(dstCT, &load_dst, &store_dst);

    // Load the final dst.
    p.append(load_dst, dst);
    p.extend(convert_to_linear_premul);
    p.append(SkRasterPipeline::move_src_dst);

    // Load the src.
    SkRasterPipeline::StockStage load_src;
    pick_memory_stages(srcCT, &load_src, nullptr);
    p.append(load_src, src);
    p.extend(convert_to_linear_premul);

    p.append(SkRasterPipeline::srcover);

    // Convert back to dst.
    if (kUnpremul_SkAlphaType == at) {
        p.append(SkRasterPipeline::unpremul);
    }
    if (needsSrgbToLinear) {
        p.append(SkRasterPipeline::to_srgb);
    }
    p.append(store_dst, dst);

    p.run(0,0, width);
}

SkCodec::Result SkWebpCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
                                         const Options& options, int* rowsDecodedPtr) {
    const int index = options.fFrameIndex;
    SkASSERT(0 == index || index < fFrameHolder.size());

    const auto& srcInfo = this->getInfo();
    {
        auto info = srcInfo;
        if (index > 0) {
            auto alphaType = alpha_type(fFrameHolder.frame(index)->hasAlpha());
            info = info.makeAlphaType(alphaType);
        }
        if (!conversion_possible(dstInfo, info) ||
            !this->initializeColorXform(dstInfo, options.fPremulBehavior))
        {
            return kInvalidConversion;
        }
    }

    SkASSERT(0 == index || (!options.fSubset && dstInfo.dimensions() == srcInfo.dimensions()));

    WebPDecoderConfig config;
    if (0 == WebPInitDecoderConfig(&config)) {
        // ABI mismatch.
        // FIXME: New enum for this?
        return kInvalidInput;
    }

    // Free any memory associated with the buffer. Must be called last, so we declare it first.
    SkAutoTCallVProc<WebPDecBuffer, WebPFreeDecBuffer> autoFree(&(config.output));

    WebPIterator frame;
    SkAutoTCallVProc<WebPIterator, WebPDemuxReleaseIterator> autoFrame(&frame);
    // If this succeeded in onGetFrameCount(), it should succeed again here.
    SkAssertResult(WebPDemuxGetFrame(fDemux, index + 1, &frame));

    const bool independent = index == 0 ? true :
            (fFrameHolder.frame(index)->getRequiredFrame() == kNone);
    // Get the frameRect.  libwebp will have already signaled an error if this is not fully
    // contained by the canvas.
    auto frameRect = SkIRect::MakeXYWH(frame.x_offset, frame.y_offset, frame.width, frame.height);
    SkASSERT(srcInfo.bounds().contains(frameRect));
    const bool frameIsSubset = frameRect != srcInfo.bounds();
    if (independent && frameIsSubset) {
        SkSampler::Fill(dstInfo, dst, rowBytes, 0, options.fZeroInitialized);
    }

    int dstX = frameRect.x();
    int dstY = frameRect.y();
    int subsetWidth = frameRect.width();
    int subsetHeight = frameRect.height();
    if (options.fSubset) {
        SkIRect subset = *options.fSubset;
        SkASSERT(this->getInfo().bounds().contains(subset));
        SkASSERT(SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));
        SkASSERT(this->getValidSubset(&subset) && subset == *options.fSubset);

        if (!SkIRect::IntersectsNoEmptyCheck(subset, frameRect)) {
            return kSuccess;
        }

        int minXOffset = SkTMin(dstX, subset.x());
        int minYOffset = SkTMin(dstY, subset.y());
        dstX -= minXOffset;
        dstY -= minYOffset;
        frameRect.offset(-minXOffset, -minYOffset);
        subset.offset(-minXOffset, -minYOffset);

        // Just like we require that the requested subset x and y offset are even, libwebp
        // guarantees that the frame x and y offset are even (it's actually impossible to specify
        // an odd frame offset).  So we can still guarantee that the adjusted offsets are even.
        SkASSERT(SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));

        SkIRect intersection;
        SkAssertResult(intersection.intersect(frameRect, subset));
        subsetWidth = intersection.width();
        subsetHeight = intersection.height();

        config.options.use_cropping = 1;
        config.options.crop_left = subset.x();
        config.options.crop_top = subset.y();
        config.options.crop_width = subsetWidth;
        config.options.crop_height = subsetHeight;
    }

    // Ignore the frame size and offset when determining if scaling is necessary.
    int scaledWidth = subsetWidth;
    int scaledHeight = subsetHeight;
    SkISize srcSize = options.fSubset ? options.fSubset->size() : srcInfo.dimensions();
    if (srcSize != dstInfo.dimensions()) {
        config.options.use_scaling = 1;

        if (frameIsSubset) {
            float scaleX = ((float) dstInfo.width()) / srcSize.width();
            float scaleY = ((float) dstInfo.height()) / srcSize.height();

            // We need to be conservative here and floor rather than round.
            // Otherwise, we may find ourselves decoding off the end of memory.
            dstX = scaleX * dstX;
            scaledWidth = scaleX * scaledWidth;
            dstY = scaleY * dstY;
            scaledHeight = scaleY * scaledHeight;
            if (0 == scaledWidth || 0 == scaledHeight) {
                return kSuccess;
            }
        } else {
            scaledWidth = dstInfo.width();
            scaledHeight = dstInfo.height();
        }

        config.options.scaled_width = scaledWidth;
        config.options.scaled_height = scaledHeight;
    }

    const bool blendWithPrevFrame = !independent && frame.blend_method == WEBP_MUX_BLEND
        && frame.has_alpha;
    if (blendWithPrevFrame && options.fPremulBehavior == SkTransferFunctionBehavior::kRespect) {
        // Blending is done with SkRasterPipeline, which requires a color space that is valid for
        // rendering.
        const auto* cs = dstInfo.colorSpace();
        if (!cs || (!cs->gammaCloseToSRGB() && !cs->gammaIsLinear())) {
            return kInvalidConversion;
        }
    }

    SkBitmap webpDst;
    auto webpInfo = dstInfo;
    if (!frame.has_alpha) {
        webpInfo = webpInfo.makeAlphaType(kOpaque_SkAlphaType);
    }
    if (this->colorXform()) {
        // Swizzling between RGBA and BGRA is zero cost in a color transform.  So when we have a
        // color transform, we should decode to whatever is easiest for libwebp, and then let the
        // color transform swizzle if necessary.
        // Lossy webp is encoded as YUV (so RGBA and BGRA are the same cost).  Lossless webp is
        // encoded as BGRA. This means decoding to BGRA is either faster or the same cost as RGBA.
        webpInfo = webpInfo.makeColorType(kBGRA_8888_SkColorType);

        if (webpInfo.alphaType() == kPremul_SkAlphaType) {
            webpInfo = webpInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
    }

    if ((this->colorXform() && !is_8888(dstInfo.colorType())) || blendWithPrevFrame) {
        // We will decode the entire image and then perform the color transform.  libwebp
        // does not provide a row-by-row API.  This is a shame particularly when we do not want
        // 8888, since we will need to create another image sized buffer.
        webpDst.allocPixels(webpInfo);
    } else {
        // libwebp can decode directly into the output memory.
        webpDst.installPixels(webpInfo, dst, rowBytes);
    }

    config.output.colorspace = webp_decode_mode(webpInfo);
    config.output.is_external_memory = 1;

    config.output.u.RGBA.rgba = reinterpret_cast<uint8_t*>(webpDst.getAddr(dstX, dstY));
    config.output.u.RGBA.stride = static_cast<int>(webpDst.rowBytes());
    config.output.u.RGBA.size = webpDst.getSafeSize();

    SkAutoTCallVProc<WebPIDecoder, WebPIDelete> idec(WebPIDecode(nullptr, 0, &config));
    if (!idec) {
        return kInvalidInput;
    }

    int rowsDecoded;
    SkCodec::Result result;
    switch (WebPIUpdate(idec, frame.fragment.bytes, frame.fragment.size)) {
        case VP8_STATUS_OK:
            rowsDecoded = scaledHeight;
            result = kSuccess;
            break;
        case VP8_STATUS_SUSPENDED:
            WebPIDecGetRGB(idec, &rowsDecoded, nullptr, nullptr, nullptr);
            *rowsDecodedPtr = rowsDecoded + dstY;
            result = kIncompleteInput;
            break;
        default:
            return kInvalidInput;
    }

    // We're only transforming the new part of the frame, so no need to worry about the
    // final composited alpha.
    const auto srcAlpha = 0 == index ? srcInfo.alphaType() : alpha_type(frame.has_alpha);
    const auto xformAlphaType = select_xform_alpha(dstInfo.alphaType(), srcAlpha);
    const bool needsSrgbToLinear = dstInfo.gammaCloseToSRGB() &&
            options.fPremulBehavior == SkTransferFunctionBehavior::kRespect;

    const size_t dstBpp = SkColorTypeBytesPerPixel(dstInfo.colorType());
    dst = SkTAddOffset<void>(dst, dstBpp * dstX + rowBytes * dstY);
    const size_t srcRowBytes = config.output.u.RGBA.stride;

    const auto dstCT = dstInfo.colorType();
    if (this->colorXform()) {
        uint32_t* xformSrc = (uint32_t*) config.output.u.RGBA.rgba;
        SkBitmap tmp;
        void* xformDst;

        if (blendWithPrevFrame) {
            // Xform into temporary bitmap big enough for one row.
            tmp.allocPixels(dstInfo.makeWH(scaledWidth, 1));
            xformDst = tmp.getPixels();
        } else {
            xformDst = dst;
        }
        for (int y = 0; y < rowsDecoded; y++) {
            this->applyColorXform(xformDst, xformSrc, scaledWidth, xformAlphaType);
            if (blendWithPrevFrame) {
                blend_line(dstCT, &dst, dstCT, &xformDst, needsSrgbToLinear, xformAlphaType,
                        scaledWidth);
                dst = SkTAddOffset<void>(dst, rowBytes);
            } else {
                xformDst = SkTAddOffset<void>(xformDst, rowBytes);
            }
            xformSrc = SkTAddOffset<uint32_t>(xformSrc, srcRowBytes);
        }
    } else if (blendWithPrevFrame) {
        const uint8_t* src = config.output.u.RGBA.rgba;

        for (int y = 0; y < rowsDecoded; y++) {
            blend_line(dstCT, &dst, webpDst.colorType(), &src, needsSrgbToLinear,
                    xformAlphaType, scaledWidth);
            src = SkTAddOffset<const uint8_t>(src, srcRowBytes);
            dst = SkTAddOffset<void>(dst, rowBytes);
        }
    }

    return result;
}

SkWebpCodec::SkWebpCodec(int width, int height, const SkEncodedInfo& info,
                         sk_sp<SkColorSpace> colorSpace, SkStream* stream, WebPDemuxer* demux,
                         sk_sp<SkData> data)
    : INHERITED(width, height, info, SkColorSpaceXform::kBGRA_8888_ColorFormat, stream,
                std::move(colorSpace))
    , fDemux(demux)
    , fData(std::move(data))
    , fFailed(false)
{
    fFrameHolder.setScreenSize(width, height);
}
