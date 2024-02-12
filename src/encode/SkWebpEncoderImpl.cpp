/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkWebpEncoder.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class GrDirectContext;
class SkImage;

// A WebP encoder only, on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   http://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp_webp_image_decoder_library
//   http://review.webmproject.org/gitweb?p=libwebp.git

extern "C" {
// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/encode.h"  // NO_G3_REWRITE
#include "webp/mux.h"  // NO_G3_REWRITE
#include "webp/mux_types.h"  // NO_G3_REWRITE
}

static int stream_writer(const uint8_t* data, size_t data_size, const WebPPicture* const picture) {
    SkWStream* const stream = (SkWStream*)picture->custom_ptr;
    return stream->write(data, data_size) ? 1 : 0;
}

using WebPPictureImportProc = int (*)(WebPPicture* picture, const uint8_t* pixels, int stride);

static bool preprocess_webp_picture(WebPPicture* pic,
                                    WebPConfig* webp_config,
                                    const SkPixmap& pixmap,
                                    const SkWebpEncoder::Options& opts) {
    if (!SkPixmapIsValid(pixmap)) {
        return false;
    }

    if (SkColorTypeIsAlphaOnly(pixmap.colorType())) {
        // Maintain the existing behavior of not supporting encoding alpha-only images.
        // TODO: Support encoding alpha only to an image with alpha but no color?
        return false;
    }

    if (nullptr == pixmap.addr()) {
        return false;
    }

    pic->width = pixmap.width();
    pic->height = pixmap.height();

    // Set compression, method, and pixel format.
    // libwebp recommends using BGRA for lossless and YUV for lossy.
    // The choices of |webp_config.method| currently just match Chrome's defaults.  We
    // could potentially expose this decision to the client.
    if (SkWebpEncoder::Compression::kLossy == opts.fCompression) {
        webp_config->lossless = 0;
#ifndef SK_WEBP_ENCODER_USE_DEFAULT_METHOD
        webp_config->method = 3;
#endif
        pic->use_argb = 0;
    } else {
        webp_config->lossless = 1;
        webp_config->method = 0;
        pic->use_argb = 1;
    }

    {
        const SkColorType ct = pixmap.colorType();
        const bool premul = pixmap.alphaType() == kPremul_SkAlphaType;

        SkBitmap tmpBm;
        WebPPictureImportProc importProc = nullptr;
        const SkPixmap* src = &pixmap;
        if (ct == kRGB_888x_SkColorType) {
            importProc = WebPPictureImportRGBX;
        } else if (!premul && ct == kRGBA_8888_SkColorType) {
            importProc = WebPPictureImportRGBA;
        }
#ifdef WebPPictureImportBGRA
        else if (!premul && ct == kBGRA_8888_SkColorType) {
            importProc = WebPPictureImportBGRA;
        }
#endif
        else {
            importProc = WebPPictureImportRGBA;
            auto info = pixmap.info()
                                .makeColorType(kRGBA_8888_SkColorType)
                                .makeAlphaType(kUnpremul_SkAlphaType);
            if (!tmpBm.tryAllocPixels(info) ||
                !pixmap.readPixels(tmpBm.info(), tmpBm.getPixels(), tmpBm.rowBytes())) {
                return false;
            }
            src = &tmpBm.pixmap();
        }

        if (!importProc(pic, reinterpret_cast<const uint8_t*>(src->addr()), src->rowBytes())) {
            return false;
        }
    }

    return true;
}

namespace SkWebpEncoder {

bool Encode(SkWStream* stream, const SkPixmap& pixmap, const Options& opts) {
    if (!stream) {
        return false;
    }

    WebPConfig webp_config;
    if (!WebPConfigPreset(&webp_config, WEBP_PRESET_DEFAULT, opts.fQuality)) {
        return false;
    }

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) {
        return false;
    }
    SkAutoTCallVProc<WebPPicture, WebPPictureFree> autoPic(&pic);

    if (!preprocess_webp_picture(&pic, &webp_config, pixmap, opts)) {
        return false;
    }

    // If there is no need to embed an ICC profile, we write directly to the input stream.
    // Otherwise, we will first encode to |tmp| and use a mux to add the ICC chunk.  libwebp
    // forces us to have an encoded image before we can add a profile.
    sk_sp<SkData> icc =
            icc_from_color_space(pixmap.info(), opts.fICCProfile, opts.fICCProfileDescription);
    SkDynamicMemoryWStream tmp;
    pic.custom_ptr = icc ? (void*)&tmp : (void*)stream;
    pic.writer = stream_writer;

    if (!WebPEncode(&webp_config, &pic)) {
        return false;
    }

    if (icc) {
        sk_sp<SkData> encodedData = tmp.detachAsData();
        WebPData encoded = {encodedData->bytes(), encodedData->size()};
        WebPData iccChunk = {icc->bytes(), icc->size()};

        SkAutoTCallVProc<WebPMux, WebPMuxDelete> mux(WebPMuxNew());
        if (WEBP_MUX_OK != WebPMuxSetImage(mux, &encoded, 0)) {
            return false;
        }

        if (WEBP_MUX_OK != WebPMuxSetChunk(mux, "ICCP", &iccChunk, 0)) {
            return false;
        }

        WebPData assembled;
        SkAutoTCallVProc<WebPData, WebPDataClear> autoWebPData(&assembled);
        if (WEBP_MUX_OK != WebPMuxAssemble(mux, &assembled)) {
            return false;
        }

        if (!stream->write(assembled.bytes, assembled.size)) {
            return false;
        }
    }

    return true;
}

bool EncodeAnimated(SkWStream* stream, SkSpan<const SkEncoder::Frame> frames, const Options& opts) {
    if (!stream || !frames.size()) {
        return false;
    }

    const int canvasWidth = frames.front().pixmap.width();
    const int canvasHeight = frames.front().pixmap.height();
    int timestamp = 0;

    std::unique_ptr<WebPAnimEncoder, void (*)(WebPAnimEncoder*)> enc(
            WebPAnimEncoderNew(canvasWidth, canvasHeight, nullptr), WebPAnimEncoderDelete);
    if (!enc) {
        return false;
    }

    for (const auto& frame : frames) {
        const auto& pixmap = frame.pixmap;

        if (pixmap.width() != canvasWidth || pixmap.height() != canvasHeight) {
            return false;
        }

        WebPConfig webp_config;
        if (!WebPConfigPreset(&webp_config, WEBP_PRESET_DEFAULT, opts.fQuality)) {
            return false;
        }

        WebPPicture pic;
        if (!WebPPictureInit(&pic)) {
            return false;
        }
        SkAutoTCallVProc<WebPPicture, WebPPictureFree> autoPic(&pic);

        if (!preprocess_webp_picture(&pic, &webp_config, pixmap, opts)) {
            return false;
        }

        if (!WebPEncode(&webp_config, &pic)) {
            return false;
        }

        if (!WebPAnimEncoderAdd(enc.get(), &pic, timestamp, &webp_config)) {
            return false;
        }

        timestamp += frame.duration;
    }

    // Add a last fake frame to signal the last duration.
    if (!WebPAnimEncoderAdd(enc.get(), nullptr, timestamp, nullptr)) {
        return false;
    }

    WebPData assembled;
    SkAutoTCallVProc<WebPData, WebPDataClear> autoWebPData(&assembled);
    if (!WebPAnimEncoderAssemble(enc.get(), &assembled)) {
        return false;
    }

    enc.reset();

    return stream->write(assembled.bytes, assembled.size);
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }
    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (Encode(&stream, bm.pixmap(), options)) {
        return stream.detachAsData();
    }
    return nullptr;
}

}  // namespace SkWebpEncoder
