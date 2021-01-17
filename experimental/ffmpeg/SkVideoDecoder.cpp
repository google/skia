/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoDecoder.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAPixmaps.h"

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
    const av_transfer_characteristics* av = &gTransfer[AVCOL_TRC_BT709];
    if ((unsigned)t < AVCOL_TRC_NB) {
        av = &gTransfer[t];
    }
    if (av->alpha == 0) {
        av = &gTransfer[AVCOL_TRC_BT709];
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

sk_sp<SkColorSpace> make_colorspace(AVColorPrimaries primaries,
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

static int skstream_read_packet(void* ctx, uint8_t* dstBuffer, int dstSize) {
    SkStream* stream = (SkStream*)ctx;
    int result = (int)stream->read(dstBuffer, dstSize);
    if (result == 0) {
        result = AVERROR_EOF;
    }
    return result;
}

static int64_t skstream_seek_packet(void* ctx, int64_t pos, int whence) {
    SkStream* stream = (SkStream*)ctx;
    switch (whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            pos = (int64_t)stream->getPosition() + pos;
            break;
        case SEEK_END:
            pos = (int64_t)stream->getLength() + pos;
            break;
        default:
            return -1;
    }
    return stream->seek(SkToSizeT(pos)) ? pos : -1;
}

static sk_sp<SkImage> make_yuv_420(GrRecordingContext* rContext,
                                   int w, int h,
                                   uint8_t* const data[],
                                   int const strides[],
                                   SkYUVColorSpace yuvSpace,
                                   sk_sp<SkColorSpace> cs) {
    SkYUVAInfo yuvaInfo({w, h},
                        SkYUVAInfo::PlaneConfig::kY_U_V,
                        SkYUVAInfo::Subsampling::k420,
                        yuvSpace);
    SkPixmap pixmaps[3];
    pixmaps[0].reset(SkImageInfo::MakeA8(w, h), data[0], strides[0]);
    w = (w + 1)/2;
    h = (h + 1)/2;
    pixmaps[1].reset(SkImageInfo::MakeA8(w, h), data[1], strides[1]);
    pixmaps[2].reset(SkImageInfo::MakeA8(w, h), data[2], strides[2]);
    auto yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pixmaps);

    return SkImage::MakeFromYUVAPixmaps(
            rContext, yuvaPixmaps, GrMipMapped::kNo, false, std::move(cs));
}

// Init with illegal values, so our first compare will fail, forcing us to compute
// the skcolorspace.
SkVideoDecoder::ConvertedColorSpace::ConvertedColorSpace()
    : fPrimaries(AVCOL_PRI_NB), fTransfer(AVCOL_TRC_NB)
{}

void SkVideoDecoder::ConvertedColorSpace::update(AVColorPrimaries primaries,
            AVColorTransferCharacteristic transfer) {
    if (fPrimaries != primaries || fTransfer != transfer) {
        fPrimaries = primaries;
        fTransfer  = transfer;
        fCS = make_colorspace(primaries, transfer);
    }
}

double SkVideoDecoder::computeTimeStamp(const AVFrame* frame) const {
    AVRational base = fFormatCtx->streams[fStreamIndex]->time_base;
    return 1.0 * frame->pts * base.num / base.den;
}

sk_sp<SkImage> SkVideoDecoder::convertFrame(const AVFrame* frame) {
    auto yuv_space = get_yuvspace(frame->colorspace);

    // we have a 1-entry cache for converting colorspaces
    fCSCache.update(frame->color_primaries, frame->color_trc);

    // Are these always true? If so, we don't need to check our "cache" on each frame...
    SkASSERT(fDecoderCtx->colorspace == frame->colorspace);
    SkASSERT(fDecoderCtx->color_primaries == frame->color_primaries);
    SkASSERT(fDecoderCtx->color_trc == frame->color_trc);

    // Is this always true? If so, we might take advantage of it, knowing up-front if we support
    // the format for the whole stream, in which case we might have to ask ffmpeg to convert it
    // to something more reasonable (for us)...
    SkASSERT(fDecoderCtx->pix_fmt == frame->format);

    switch (frame->format) {
        case AV_PIX_FMT_YUV420P:
            if (auto image = make_yuv_420(fRecordingContext, frame->width, frame->height,
                                          frame->data, frame->linesize, yuv_space, fCSCache.fCS)) {
                return image;
            }
            break;
        default:
            break;
    }

    // General N32 fallback.
    const auto info = SkImageInfo::MakeN32(frame->width, frame->height,
                                           SkAlphaType::kOpaque_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(info, info.minRowBytes());

    constexpr auto fmt = SK_PMCOLOR_BYTE_ORDER(R,G,B,A) ? AV_PIX_FMT_RGBA : AV_PIX_FMT_BGRA;

    // TODO: should we cache these?
    auto* ctx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                               info.width(), info.height(), fmt,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t*   dst[] = { (uint8_t*)bm.pixmap().writable_addr() };
    int dst_stride[] = { SkToInt(bm.pixmap().rowBytes()) };

    sws_scale(ctx, frame->data, frame->linesize, 0, frame->height, dst, dst_stride);

    sws_freeContext(ctx);

    bm.setImmutable();

    return SkImage::MakeFromBitmap(bm);
}

sk_sp<SkImage> SkVideoDecoder::nextImage(double* timeStamp) {
    double dummyTimeStampStorage = 0;
    if (!timeStamp) {
        timeStamp = &dummyTimeStampStorage;
    }

    if (fFormatCtx == nullptr) {
        return nullptr;
    }

    if (fMode == kProcessing_Mode) {
        // We sit in a loop, waiting for the codec to have received enough data (packets)
        // to have at least one frame available.
        // Treat non-zero return as EOF (or error, which we will decide is also EOF)
        while (!av_read_frame(fFormatCtx, &fPacket)) {
            if (fPacket.stream_index != fStreamIndex) {
                // got a packet for a stream other than our (video) stream, so continue
                continue;
            }

            int ret = avcodec_send_packet(fDecoderCtx, &fPacket);
            if (ret == AVERROR(EAGAIN)) {
                // may signal that we have plenty already, encouraging us to call receive_frame
                // so we don't treat this as an error.
                ret = 0;
            }
            (void)check_err(ret);   // we try to continue if there was an error

            int silentList[] = {
                -35,    // Resource temporarily unavailable (need more packets)
                0,
            };
            if (check_err(avcodec_receive_frame(fDecoderCtx, fFrame), silentList)) {
                // this may be just "needs more input", so we try to continue
            } else {
                *timeStamp = this->computeTimeStamp(fFrame);
                return this->convertFrame(fFrame);
            }
        }

        fMode = kDraining_Mode;
        (void)avcodec_send_packet(fDecoderCtx, nullptr);    // signal to start draining
    }
    if (fMode == kDraining_Mode) {
        if (avcodec_receive_frame(fDecoderCtx, fFrame) >= 0) {
            *timeStamp = this->computeTimeStamp(fFrame);
            return this->convertFrame(fFrame);
        }
        // else we decide we're done
        fMode = kDone_Mode;
    }
    return nullptr;
}

SkVideoDecoder::SkVideoDecoder(GrRecordingContext* rContext) : fRecordingContext(rContext) {}

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
    if (fStreamCtx) {
        av_freep(&fStreamCtx->buffer);
        avio_context_free(&fStreamCtx);
        fStreamCtx = nullptr;
    }

    fStream.reset(nullptr);
    fStreamIndex = -1;
    fMode = kDone_Mode;
}

bool SkVideoDecoder::loadStream(std::unique_ptr<SkStream> stream) {
    this->reset();
    if (!stream) {
        return false;
    }

    int bufferSize = 4 * 1024;
    uint8_t* buffer = (uint8_t*)av_malloc(bufferSize);
    if (!buffer) {
        return false;
    }

    fStream = std::move(stream);
    fStreamCtx = avio_alloc_context(buffer, bufferSize, 0, fStream.get(),
                                    skstream_read_packet, nullptr, skstream_seek_packet);
    if (!fStreamCtx) {
        av_freep(buffer);
        this->reset();
        return false;
    }

    fFormatCtx = avformat_alloc_context();
    if (!fFormatCtx) {
        this->reset();
        return false;
    }
    fFormatCtx->pb = fStreamCtx;

    int err = avformat_open_input(&fFormatCtx, nullptr, nullptr, nullptr);
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

    fMode = kProcessing_Mode;

    return true;
}

SkISize SkVideoDecoder::dimensions() const {
    if (!fFormatCtx) {
        return {0, 0};
    }

    AVStream* strm = fFormatCtx->streams[fStreamIndex];
    return {strm->codecpar->width, strm->codecpar->height};
}

double SkVideoDecoder::duration() const {
    if (!fFormatCtx) {
        return 0;
    }

    AVStream* strm = fFormatCtx->streams[fStreamIndex];
    AVRational base = strm->time_base;
    return 1.0 * strm->duration * base.num / base.den;
}

bool SkVideoDecoder::rewind() {
    auto stream = std::move(fStream);
    this->reset();
    if (stream) {
        stream->rewind();
    }
    return this->loadStream(std::move(stream));
}
