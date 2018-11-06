/*
 * Copyright 2010, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkImageEncoderPriv.h"

#ifdef SK_HAS_WEBP_LIBRARY

#include "SkBitmap.h"
#include "SkColorData.h"
#include "SkImageEncoderFns.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUnPreMultiply.h"
#include "SkUTF.h"
#include "SkWebpEncoder.h"

// A WebP encoder only, on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   http://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp_webp_image_decoder_library
//   http://review.webmproject.org/gitweb?p=libwebp.git

#include <stdio.h>
extern "C" {
// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/encode.h"
#include "webp/mux.h"
}

static transform_scanline_proc choose_proc(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_RGBX;
                case kUnpremul_SkAlphaType:
                    return transform_scanline_memcpy;
                case kPremul_SkAlphaType:
                    return transform_scanline_rgbA;
                default:
                    return nullptr;
            }
        case kBGRA_8888_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_BGRX;
                case kUnpremul_SkAlphaType:
                    return transform_scanline_BGRA;
                case kPremul_SkAlphaType:
                    return transform_scanline_bgrA;
                default:
                    return nullptr;
            }
        case kRGB_565_SkColorType:
            if (!info.isOpaque()) {
                return nullptr;
            }

            return transform_scanline_565;
        case kARGB_4444_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_444;
                case kPremul_SkAlphaType:
                    return transform_scanline_4444;
                default:
                    return nullptr;
            }
        case kGray_8_SkColorType:
            return transform_scanline_gray;
        case kRGBA_F16_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return transform_scanline_F16_to_8888;
                case kPremul_SkAlphaType:
                    return transform_scanline_F16_premul_to_8888;
                default:
                    return nullptr;
            }
        default:
            return nullptr;
    }
}

static int stream_writer(const uint8_t* data, size_t data_size,
                         const WebPPicture* const picture) {
  SkWStream* const stream = (SkWStream*)picture->custom_ptr;
  return stream->write(data, data_size) ? 1 : 0;
}

bool SkWebpEncoder::Encode(SkWStream* stream, const SkPixmap& pixmap, const Options& opts) {
    if (!SkPixmapIsValid(pixmap)) {
        return false;
    }

    const transform_scanline_proc proc = choose_proc(pixmap.info());
    if (!proc) {
        return false;
    }

    int bpp;
    if (kRGBA_F16_SkColorType == pixmap.colorType()) {
        bpp = 4;
    } else {
        bpp = pixmap.isOpaque() ? 3 : 4;
    }

    if (nullptr == pixmap.addr()) {
        return false;
    }

    WebPConfig webp_config;
    if (!WebPConfigPreset(&webp_config, WEBP_PRESET_DEFAULT, opts.fQuality)) {
        return false;
    }

    WebPPicture pic;
    WebPPictureInit(&pic);
    SkAutoTCallVProc<WebPPicture, WebPPictureFree> autoPic(&pic);
    pic.width = pixmap.width();
    pic.height = pixmap.height();
    pic.writer = stream_writer;

    // Set compression, method, and pixel format.
    // libwebp recommends using BGRA for lossless and YUV for lossy.
    // The choices of |webp_config.method| currently just match Chrome's defaults.  We
    // could potentially expose this decision to the client.
    if (Compression::kLossy == opts.fCompression) {
        webp_config.lossless = 0;
#ifndef SK_WEBP_ENCODER_USE_DEFAULT_METHOD
        webp_config.method = 3;
#endif
        pic.use_argb = 0;
    } else {
        webp_config.lossless = 1;
        webp_config.method = 0;
        pic.use_argb = 1;
    }

    // If there is no need to embed an ICC profile, we write directly to the input stream.
    // Otherwise, we will first encode to |tmp| and use a mux to add the ICC chunk.  libwebp
    // forces us to have an encoded image before we can add a profile.
    sk_sp<SkData> icc = icc_from_color_space(pixmap.info());
    SkDynamicMemoryWStream tmp;
    pic.custom_ptr = icc ? (void*)&tmp : (void*)stream;

    const uint8_t* src = (uint8_t*)pixmap.addr();
    const int rgbStride = pic.width * bpp;
    const size_t rowBytes = pixmap.rowBytes();

    // Import (for each scanline) the bit-map image (in appropriate color-space)
    // to RGB color space.
    std::unique_ptr<uint8_t[]> rgb(new uint8_t[rgbStride * pic.height]);
    for (int y = 0; y < pic.height; ++y) {
        proc((char*) &rgb[y * rgbStride],
             (const char*) &src[y * rowBytes],
             pic.width,
             bpp);
    }

    auto importProc = WebPPictureImportRGB;
    if (3 != bpp) {
        if (pixmap.isOpaque()) {
            importProc = WebPPictureImportRGBX;
        } else {
            importProc = WebPPictureImportRGBA;
        }
    }

    if (!importProc(&pic, &rgb[0], rgbStride)) {
        return false;
    }

    if (!WebPEncode(&webp_config, &pic)) {
        return false;
    }

    if (icc) {
        sk_sp<SkData> encodedData = tmp.detachAsData();
        WebPData encoded = { encodedData->bytes(), encodedData->size() };
        WebPData iccChunk = { icc->bytes(), icc->size() };

        SkAutoTCallVProc<WebPMux, WebPMuxDelete> mux(WebPMuxNew());
        if (WEBP_MUX_OK != WebPMuxSetImage(mux, &encoded, 0)) {
            return false;
        }

        if (WEBP_MUX_OK != WebPMuxSetChunk(mux, "ICCP", &iccChunk, 0)) {
            return false;
        }

        WebPData assembled;
        if (WEBP_MUX_OK != WebPMuxAssemble(mux, &assembled)) {
            return false;
        }

        stream->write(assembled.bytes, assembled.size);
        WebPDataClear(&assembled);
    }

    return true;
}

#endif
