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
#if 0
            static size_t total;
            total += bytes;
            SkDebugf("write %zu %zu\n", bytes, total);
#endif

            memcpy(fStorage.append(bytes), src, bytes);
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

static SkYUVColorSpace get_yuvspace(AVColorSpace space) {
    // this is pretty incomplete -- TODO: look to convert more AVColorSpaces
    switch (space) {
        case AVCOL_SPC_RGB:     return kIdentity_SkYUVColorSpace;
        case AVCOL_SPC_BT709:   return kRec709_SkYUVColorSpace;
        case AVCOL_SPC_SMPTE170M:
        case AVCOL_SPC_SMPTE240M:
        case AVCOL_SPC_BT470BG: return kRec601_SkYUVColorSpace;
        default: break;
    }
    return kRec709_SkYUVColorSpace;
}

struct av_transfer_characteristics {
    // if x < beta     delta * x
    //    else         alpha * (x^gama)
    float alpha, beta, gamma, delta;
};

// Tables extracted from vf_colorspace.c

const av_transfer_characteristics gTransfer[AVCOL_TRC_NB] = {
    [AVCOL_TRC_BT709]     = { 1.099,  0.018,  0.45, 4.5 },
    [AVCOL_TRC_GAMMA22]   = { 1.0,    0.0,    1.0 / 2.2, 0.0 },
    [AVCOL_TRC_GAMMA28]   = { 1.0,    0.0,    1.0 / 2.8, 0.0 },
    [AVCOL_TRC_SMPTE170M] = { 1.099,  0.018,  0.45, 4.5 },
    [AVCOL_TRC_SMPTE240M] = { 1.1115, 0.0228, 0.45, 4.0 },
    [AVCOL_TRC_IEC61966_2_1] = { 1.055, 0.0031308, 1.0 / 2.4, 12.92 },
    [AVCOL_TRC_IEC61966_2_4] = { 1.099, 0.018, 0.45, 4.5 },
    [AVCOL_TRC_BT2020_10] = { 1.099,  0.018,  0.45, 4.5 },
    [AVCOL_TRC_BT2020_12] = { 1.0993, 0.0181, 0.45, 4.5 },
};

static skcms_TransferFunction compute_transfer(AVColorTransferCharacteristic t) {
    const av_transfer_characteristics* av = &gTransfer[0];
    if ((unsigned)t < AVCOL_TRC_NB) {
        av = &gTransfer[t];
    }

    skcms_TransferFunction linear_to_encoded = {
        av->gamma, sk_float_pow(av->alpha, 1/av->gamma), 0, av->delta, av->beta, 1 - av->alpha, 0,
    };
    skcms_TransferFunction encoded_to_linear;
    bool success = skcms_TransferFunction_invert(&linear_to_encoded, &encoded_to_linear);
    SkASSERT(success);

    return encoded_to_linear;
}

enum Whitepoint {
    WP_D65,
    WP_C,
    WP_DCI,
    WP_E,
    WP_NB,
};

const SkPoint gWP[WP_NB] = {
    [WP_D65] = { 0.3127f, 0.3290f },
    [WP_C]   = { 0.3100f, 0.3160f },
    [WP_DCI] = { 0.3140f, 0.3510f },
    [WP_E]   = { 1/3.0f, 1/3.0f },
};

#define ExpandWP(index) gWP[index].fX, gWP[index].fY

const SkColorSpacePrimaries gPrimaries[AVCOL_PRI_NB] = {
    [AVCOL_PRI_BT709]     = { 0.640f, 0.330f, 0.300f, 0.600f, 0.150f, 0.060f, ExpandWP(WP_D65) },
    [AVCOL_PRI_BT470M]    = { 0.670f, 0.330f, 0.210f, 0.710f, 0.140f, 0.080f, ExpandWP(WP_C)   },
    [AVCOL_PRI_BT470BG]   = { 0.640f, 0.330f, 0.290f, 0.600f, 0.150f, 0.060f, ExpandWP(WP_D65) },
    [AVCOL_PRI_SMPTE170M] = { 0.630f, 0.340f, 0.310f, 0.595f, 0.155f, 0.070f, ExpandWP(WP_D65) },
    [AVCOL_PRI_SMPTE240M] = { 0.630f, 0.340f, 0.310f, 0.595f, 0.155f, 0.070f, ExpandWP(WP_D65) },
    [AVCOL_PRI_SMPTE428]  = { 0.735f, 0.265f, 0.274f, 0.718f, 0.167f, 0.009f, ExpandWP(WP_E)   },
    [AVCOL_PRI_SMPTE431]  = { 0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, ExpandWP(WP_DCI) },
    [AVCOL_PRI_SMPTE432]  = { 0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, ExpandWP(WP_D65) },
    [AVCOL_PRI_FILM]      = { 0.681f, 0.319f, 0.243f, 0.692f, 0.145f, 0.049f, ExpandWP(WP_C)   },
    [AVCOL_PRI_BT2020]    = { 0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f, ExpandWP(WP_D65) },
    [AVCOL_PRI_JEDEC_P22] = { 0.630f, 0.340f, 0.295f, 0.605f, 0.155f, 0.077f, ExpandWP(WP_D65) },
};

static sk_sp<SkColorSpace> make_colorspace(AVColorPrimaries primaries,
                                    AVColorTransferCharacteristic transfer) {
    if (primaries == AVCOL_PRI_BT709 && transfer == AVCOL_TRC_BT709) {
        return SkColorSpace::MakeSRGB();
    }

    const SkColorSpacePrimaries* p = &gPrimaries[0];
    if ((unsigned)primaries < (unsigned)AVCOL_PRI_NB) {
        p = &gPrimaries[primaries];
    }

    skcms_Matrix3x3 matrix;
    p->toXYZD50(&matrix);
    return SkColorSpace::MakeRGB(compute_transfer(transfer), matrix);
}

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

// Init with illegal values, so our first compare will fail, forcing us to compute
// the skcolorspace.
SkVideoEncoder::ConvertedColorSpace::ConvertedColorSpace()
    : fPrimaries(AVCOL_PRI_NB), fTransfer(AVCOL_TRC_NB)
{}

void SkVideoEncoder::ConvertedColorSpace::update(AVColorPrimaries primaries,
            AVColorTransferCharacteristic transfer) {
    if (fPrimaries != primaries || fTransfer != transfer) {
        fPrimaries = primaries;
        fTransfer  = transfer;
        fCS = make_colorspace(primaries, transfer);
    }
}

SkVideoEncoder::SkVideoEncoder() {}

SkVideoEncoder::~SkVideoEncoder() {
    this->reset();
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

    av_packet_free(&fPacket);
    fPacket = nullptr;

    if (fWStream) {
        delete fWStream;
        fWStream = nullptr;
    }
}

static AVPixelFormat colortype_to_format(SkColorType ct) {
    return AV_PIX_FMT_YUV420P;
    switch (ct) {
        case kRGBA_8888_SkColorType: return AV_PIX_FMT_RGBA;
        case kBGRA_8888_SkColorType: return AV_PIX_FMT_BGRA;
        case kRGB_565_SkColorType:   return AV_PIX_FMT_RGB565LE;
        default: break;
    }
    return AV_PIX_FMT_NONE;
}

bool SkVideoEncoder::init(const SkImageInfo& info, int fps) {
    if ((info.width() & 1) || (info.height() & 1)) {
        SkDebugf("dimensinos must be even (it appears)\n");
        return false;
    }
    AVPixelFormat pix_fmt = colortype_to_format(info.colorType());
    if (pix_fmt == AV_PIX_FMT_NONE) {
        return false;
    }

    this->reset();

    fWStream = new SkRandomAccessWStream;

    int bufferSize = 4 * 1024;
    uint8_t* buffer = (uint8_t*)av_malloc(bufferSize);
    if (!buffer) {
        return false;
    }
    fStreamCtx = avio_alloc_context(buffer, bufferSize, AVIO_FLAG_WRITE, fWStream,
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

    AVStream* stream = avformat_new_stream(fFormatCtx, codec);
    SkASSERT(stream);
    stream->time_base = (AVRational){ 1, fps };

    fEncoderCtx = avcodec_alloc_context3(codec);
    SkASSERT(fEncoderCtx);

    fEncoderCtx->codec_id = output_format->video_codec;
    fEncoderCtx->bit_rate = 400000;             // ???
    fEncoderCtx->width    = info.width();
    fEncoderCtx->height   = info.height();
    fEncoderCtx->time_base = stream->time_base;
    fEncoderCtx->gop_size = 12;                 // ???
    fEncoderCtx->pix_fmt  = pix_fmt;

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

    if (check_err(avcodec_parameters_from_context(stream->codecpar, fEncoderCtx))) {
        return false;
    }

    if (check_err(avformat_write_header(fFormatCtx, nullptr))) {
        return false;
    }

    fPacket = av_packet_alloc();

    return true;
}

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkColorFilter.h"
#include "src/core/SkYUVMath.h"

bool SkVideoEncoder::beginRecording(const SkImageInfo& info, int fps) {
    if (!this->init(info, fps)) {
        return false;
    }
    fSurface = SkSurface::MakeRaster(info);

    fCurrentPTS = 0;
    fDeltaPTS = 1;
    return true;
}

SkCanvas* SkVideoEncoder::beginFrame() {
    if (!fSurface) {
        return nullptr;
    }
    SkCanvas* canvas = fSurface->getCanvas();
    canvas->restoreToCount(1);
    return canvas;
}

static void draw_into_alpha(SkImage* img, sk_sp<SkColorFilter> cf,
                            int w, int h, uint8_t* data, size_t rb) {
    SkImageInfo info = SkImageInfo::Make(w, h, kAlpha_8_SkColorType, kPremul_SkAlphaType);
    auto canvas = SkCanvas::MakeRasterDirect(info, data, rb);
    canvas->scale(1.0f * w / img->width(), 1.0f * h / img->height());
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    paint.setColorFilter(cf);
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawImage(img, 0, 0, &paint);
}

static void copy_to_frame(SkImage* img, AVFrame* frame) {
    SkYUVColorSpace cs = kRec709_SkYUVColorSpace;

    float m[20];
    SkColorMatrix_RGB2YUV(cs, m);

    memcpy(m + 15, m + 0, 5 * sizeof(float));   // copy Y into A
    auto cfy = SkColorFilters::Matrix(m);

    memcpy(m + 15, m + 5, 5 * sizeof(float));   // copy U into A
    auto cfu = SkColorFilters::Matrix(m);

    memcpy(m + 15, m + 10, 5 * sizeof(float));   // copy V into A
    auto cfv = SkColorFilters::Matrix(m);

    SkASSERT(frame->data[0] && frame->data[1] && frame->data[2]);
    draw_into_alpha(img, cfy, img->width(),   img->height(),   frame->data[0], frame->linesize[0]);
    draw_into_alpha(img, cfu, img->width()/2, img->height()/2, frame->data[1], frame->linesize[1]);
    draw_into_alpha(img, cfv, img->width()/2, img->height()/2, frame->data[2], frame->linesize[2]);
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
        fWStream->write(fPacket->data, fPacket->size);
        av_packet_unref(fPacket);
    }
    return true;
}

bool SkVideoEncoder::endFrame() {
    /* make sure the frame data is writable */
    if (check_err(av_frame_make_writable(fFrame))) {
        return false;
    }

    fFrame->pts = fCurrentPTS;
    fCurrentPTS += fDeltaPTS;

    auto img = fSurface->makeImageSnapshot();
    copy_to_frame(img.get(), fFrame);

    return this->sendFrame(fFrame);
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
