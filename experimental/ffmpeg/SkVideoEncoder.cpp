/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAIndex.h"
#include "include/private/SkTDArray.h"

extern "C" {
#include "libswscale/swscale.h"
}

class SkRandomAccessWStream {
    SkTDArray<char> fStorage;
    size_t          fPos = 0;

public:
    SkRandomAccessWStream() {}

    size_t pos() const { return fPos; }

    size_t size() const { return fStorage.size(); }

    void write(const void* src, size_t bytes) {
        size_t len = fStorage.size();
        SkASSERT(fPos <= len);

        size_t overwrite = std::min(len - fPos, bytes);
        if (overwrite) {
            SkDebugf("overwrite %zu bytes at %zu offset with %zu remaining\n", overwrite, fPos, bytes - overwrite);
            memcpy(&fStorage[fPos], src, overwrite);
            fPos += overwrite;
            src = (const char*)src + overwrite;
            bytes -= overwrite;
        }
        // bytes now represents the amount to append
        if (bytes) {
            fStorage.append(bytes, (const char*)src);
            fPos += bytes;
        }
        SkASSERT(fPos <= fStorage.size());
    }

    void seek(size_t pos) {
        SkASSERT(pos <= fStorage.size());
        fPos = pos;
    }

    sk_sp<SkData> detachAsData() {
        // TODO: could add an efficient detach to SkTDArray if we wanted, w/o copy
        return SkData::MakeWithCopy(fStorage.begin(), fStorage.size());
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// returns true on error (and may dump the particular error message)
static bool check_err(int err, const int silentList[] = nullptr) {
    if (err >= 0) {
        return false;
    }

    if (silentList) {
        for (; *silentList; ++silentList) {
            if (*silentList == err) {
                return true;    // we still report the error, but we don't printf
            }
        }
    }

    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0) {
        errbuf_ptr = strerror(AVUNERROR(err));
    }
    SkDebugf("%s\n", errbuf_ptr);
    return true;
}

static int sk_write_packet(void* ctx, uint8_t* buffer, int size) {
    SkRandomAccessWStream* stream = (SkRandomAccessWStream*)ctx;
    stream->write(buffer, size);
    return size;
}

static int64_t sk_seek_packet(void* ctx, int64_t pos, int whence) {
    SkRandomAccessWStream* stream = (SkRandomAccessWStream*)ctx;
    switch (whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            pos = (int64_t)stream->pos() + pos;
            break;
        case SEEK_END:
            pos = (int64_t)stream->size() + pos;
            break;
        default:
            return -1;
    }
    if (pos < 0 || pos > (int64_t)stream->size()) {
        return -1;
    }
    stream->seek(SkToSizeT(pos));
    return pos;
}

SkVideoEncoder::SkVideoEncoder() {
    fInfo = SkImageInfo::MakeUnknown();
}

SkVideoEncoder::~SkVideoEncoder() {
    this->reset();

    if (fSWScaleCtx) {
        sws_freeContext(fSWScaleCtx);
    }
}

void SkVideoEncoder::reset() {
    if (fFrame) {
        av_frame_free(&fFrame);
        fFrame = nullptr;
    }
    if (fEncoderCtx) {
        avcodec_free_context(&fEncoderCtx);
        fEncoderCtx = nullptr;
    }
    if (fFormatCtx) {
        avformat_free_context(fFormatCtx);
        fFormatCtx = nullptr;
    }

    av_packet_free(&fPacket);
    fPacket = nullptr;

    fSurface.reset();
    fWStream.reset();
}

bool SkVideoEncoder::init(int fps) {
    // only support this for now
    AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

    this->reset();

    fWStream.reset(new SkRandomAccessWStream);

    int bufferSize = 4 * 1024;
    uint8_t* buffer = (uint8_t*)av_malloc(bufferSize);
    if (!buffer) {
        return false;
    }
    fStreamCtx = avio_alloc_context(buffer, bufferSize, AVIO_FLAG_WRITE, fWStream.get(),
                                    nullptr, sk_write_packet, sk_seek_packet);
    SkASSERT(fStreamCtx);

    avformat_alloc_output_context2(&fFormatCtx, nullptr, "mp4", nullptr);
    SkASSERT(fFormatCtx);
    fFormatCtx->pb = fStreamCtx;

    AVOutputFormat *output_format = fFormatCtx->oformat;

    if (output_format->video_codec == AV_CODEC_ID_NONE) {
        return false;
    }
    AVCodec* codec = avcodec_find_encoder(output_format->video_codec);
    SkASSERT(codec);

    fStream = avformat_new_stream(fFormatCtx, codec);
    SkASSERT(fStream);
    fStream->id = fFormatCtx->nb_streams-1;
    fStream->time_base = (AVRational){ 1, fps };

    fEncoderCtx = avcodec_alloc_context3(codec);
    SkASSERT(fEncoderCtx);

    fEncoderCtx->codec_id = output_format->video_codec;
    fEncoderCtx->width    = fInfo.width();
    fEncoderCtx->height   = fInfo.height();
    fEncoderCtx->time_base = fStream->time_base;
    fEncoderCtx->pix_fmt  = pix_fmt;

    /* Some formats want stream headers to be separate. */
    if (output_format->flags & AVFMT_GLOBALHEADER) {
        fEncoderCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (check_err(avcodec_open2(fEncoderCtx, codec, nullptr))) {
        return false;
    }
    fFrame = av_frame_alloc();
    SkASSERT(fFrame);
    fFrame->format = pix_fmt;
    fFrame->width = fEncoderCtx->width;
    fFrame->height = fEncoderCtx->height;
    if (check_err(av_frame_get_buffer(fFrame, 32))) {
        return false;
    }

    if (check_err(avcodec_parameters_from_context(fStream->codecpar, fEncoderCtx))) {
        return false;
    }
    if (check_err(avformat_write_header(fFormatCtx, nullptr))) {
        return false;
    }
    fPacket = av_packet_alloc();
    return true;
}

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkSurface.h"
#include "src/core/SkYUVMath.h"

static bool is_valid(SkISize dim) {
    if (dim.width() <= 0 || dim.height() <= 0) {
        return false;
    }
    // need the dimensions to be even for YUV 420
    return ((dim.width() | dim.height()) & 1) == 0;
}

bool SkVideoEncoder::beginRecording(SkISize dim, int fps) {
    if (!is_valid(dim)) {
        return false;
    }

    SkAlphaType alphaType = kOpaque_SkAlphaType;
    sk_sp<SkColorSpace> cs = nullptr;   // should we use this?
    fInfo = SkImageInfo::MakeN32(dim.width(), dim.height(), alphaType, cs);
    if (!this->init(fps)) {
        return false;
    }

    fCurrentPTS = 0;
    fDeltaPTS = 1;

    const auto fmt = kN32_SkColorType == kRGBA_8888_SkColorType ? AV_PIX_FMT_RGBA : AV_PIX_FMT_BGRA;
    SkASSERT(sws_isSupportedInput(fmt) > 0);
    SkASSERT(sws_isSupportedOutput(AV_PIX_FMT_YUV420P) > 0);
    // sws_getCachedContext takes in either null or a previous ctx. It returns either a new ctx,
    // or the same as the input if it is compatible with the inputs. Thus we never have to
    // explicitly release our ctx until the destructor, since sws_getCachedContext takes care
    // of freeing the old as needed if/when it returns a new one.
    fSWScaleCtx = sws_getCachedContext(fSWScaleCtx,
                                       dim.width(), dim.height(), fmt,
                                       dim.width(), dim.height(), AV_PIX_FMT_YUV420P,
                                       SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    return fSWScaleCtx != nullptr;
}

bool SkVideoEncoder::addFrame(const SkPixmap& pm) {
    if (!is_valid(pm.dimensions())) {
        return false;
    }
    if (pm.info().colorType() != fInfo.colorType()) {
        return false;
    }
    /* make sure the frame data is writable */
    if (check_err(av_frame_make_writable(fFrame))) {
        return false;
    }

    fFrame->pts = fCurrentPTS;
    fCurrentPTS += fDeltaPTS;

    const uint8_t* src[] = { (const uint8_t*)pm.addr() };
    const int strides[] = { SkToInt(pm.rowBytes()) };
    sws_scale(fSWScaleCtx, src, strides, 0, fInfo.height(), fFrame->data, fFrame->linesize);

    return this->sendFrame(fFrame);
}

bool SkVideoEncoder::sendFrame(AVFrame* frame) {
    if (check_err(avcodec_send_frame(fEncoderCtx, frame))) {
        return false;
    }

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(fEncoderCtx, fPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (check_err(ret)) {
            return false;
        }

        av_packet_rescale_ts(fPacket, fEncoderCtx->time_base, fStream->time_base);
        SkASSERT(fPacket->stream_index == fStream->index);

        if (check_err(av_interleaved_write_frame(fFormatCtx, fPacket))) {
            return false;
        }
    }
    return true;
}

SkCanvas* SkVideoEncoder::beginFrame() {
    if (!fSurface) {
        fSurface = SkSurface::MakeRaster(fInfo);
        if (!fSurface) {
            return nullptr;
        }
    }
    SkCanvas* canvas = fSurface->getCanvas();
    canvas->restoreToCount(1);
    canvas->clear(0);
    return canvas;
}

bool SkVideoEncoder::endFrame() {
    if (!fSurface) {
        return false;
    }
    SkPixmap pm;
    return fSurface->peekPixels(&pm) && this->addFrame(pm);
}

sk_sp<SkData> SkVideoEncoder::endRecording() {
    if (!fFormatCtx) {
        return nullptr;
    }

    this->sendFrame(nullptr);
    av_write_trailer(fFormatCtx);

    sk_sp<SkData> data = fWStream->detachAsData();
    this->reset();
    return data;
}
