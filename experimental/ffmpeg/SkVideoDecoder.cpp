/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVideoDecoder.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAIndex.h"

static SkCanvas* gCanvas;

template <typename T> class CPtr {
public:
    CPtr(T* value, void (*deleter)(T*)) : fV(value), fD(deleter) {}
    ~CPtr() { if (fV) fD(fV); }

    T* get() const { return fV; }
    T* operator->() const { return fV; }

private:
    T* fV;
    void (*fD)(T*);
};

static void check_err(int err) {
    if (err >= 0) return;

    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0) {
        errbuf_ptr = strerror(AVUNERROR(err));
    }
    SkDebugf("%s\n", errbuf_ptr);
    SkASSERT(err >= 0);
}

static sk_sp<SkImage> make_yuv_420(GrContext* gr, int w, int h,
                                   uint8_t* const data[], int const strides[]) {
    SkImageInfo info[3];
    info[0] = SkImageInfo::Make(w, h, kGray_8_SkColorType, kOpaque_SkAlphaType);
    info[1] = SkImageInfo::Make(w/2, h/2, kGray_8_SkColorType, kOpaque_SkAlphaType);
    info[2] = SkImageInfo::Make(w/2, h/2, kGray_8_SkColorType, kOpaque_SkAlphaType);

    SkPixmap pm[4];
    for (int i = 0; i < 3; ++i) {
        pm[i] = SkPixmap(info[i], data[i], strides[i]);
    }
    pm[3].reset();  // no alpha

    SkYUVAIndex indices[4];
    indices[SkYUVAIndex::kY_Index] = {0, SkColorChannel::kR};
    indices[SkYUVAIndex::kU_Index] = {1, SkColorChannel::kR};
    indices[SkYUVAIndex::kV_Index] = {2, SkColorChannel::kR};
    indices[SkYUVAIndex::kA_Index] = {-1, SkColorChannel::kR};

    return SkImage::MakeFromYUVAPixmaps(gr, kRec709_SkYUVColorSpace, pm, indices, {w, h},
                                        kTopLeft_GrSurfaceOrigin, false, false);
}

static sk_sp<SkImage> convert_frame(GrContext* gr, const AVFrame* frame) {
    if (0) {
        SkDebugf("frame %d x %d format=%d %s\n", frame->width, frame->height,
                 frame->format, av_get_pix_fmt_name((AVPixelFormat)frame->format));
    }

    switch (frame->format) {
        case AV_PIX_FMT_YUV420P:
            return make_yuv_420(gr, frame->width, frame->height, frame->data, frame->linesize);
            break;
        default:
            SkDebugf("unsupported format (for now)\n");
    }
    return nullptr;
}

sk_sp<SkImage> SkVideoDecoder::nextImage() {
    if (fFormatCtx == nullptr) {
        return nullptr;
    }

    while (!av_read_frame(fFormatCtx, &fPacket)) {
        if (fPacket.stream_index != fStreamIndex) {
            continue;
        }

        int ret = avcodec_send_packet(fDecoderCtx, &fPacket);
        if (ret == AVERROR(EAGAIN)) {
            ret = 0;
        }

        ret = avcodec_receive_frame(fDecoderCtx, fFrame);
        if (ret >= 0) {
            return convert_frame(fGr, fFrame);
        }
    }
    return nullptr;
}

SkVideoDecoder::SkVideoDecoder(GrContext* gr) : fGr(gr) {}

SkVideoDecoder::~SkVideoDecoder() {
    this->reset();
}

void SkVideoDecoder::reset() {
    if (fFrame) {
        av_frame_free(&fFrame);
        fFrame = nullptr;
    }
    if (fDecoderCtx) {
        avcodec_free_context(&fDecoderCtx);
        fDecoderCtx = nullptr;
    }
    if (fFormatCtx) {
        avformat_close_input(&fFormatCtx);
        fFormatCtx = nullptr;
    }
}

bool SkVideoDecoder::loadFile(const char filename[]) {
    this->reset();

    int err = avformat_open_input(&fFormatCtx, filename, nullptr, nullptr);
    if (err < 0) {
        SkDebugf("avformat_open_input failed %d\n", err);
        return false;
    }

    AVCodec* codec;
    fStreamIndex = av_find_best_stream(fFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (fStreamIndex < 0) {
        SkDebugf("av_find_best_stream failed %d\n", fStreamIndex);
        this->reset();
        return false;
    }

    SkASSERT(codec);
    fDecoderCtx = avcodec_alloc_context3(codec);

    AVStream* strm = fFormatCtx->streams[fStreamIndex];
    if ((err = avcodec_parameters_to_context(fDecoderCtx, strm->codecpar)) < 0) {
        SkDebugf("avcodec_parameters_to_context failed %d\n", err);
        this->reset();
        return false;
    }

    if ((err = avcodec_open2(fDecoderCtx, codec, nullptr)) < 0) {
        SkDebugf("avcodec_open2 failed %d\n", err);
        this->reset();
        return false;
    }

    fFrame = av_frame_alloc();
    SkASSERT(fFrame);

    av_init_packet(&fPacket);   // is there a "free" call?

    return true;
}

SkISize SkVideoDecoder::dimensions() const {
    if (!fFormatCtx) {
        return {0, 0};
    }

    AVStream* strm = fFormatCtx->streams[fStreamIndex];
    return {strm->codecpar->width, strm->codecpar->height};
}
