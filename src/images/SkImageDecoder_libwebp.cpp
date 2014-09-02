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

#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkColorPriv.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

// A WebP decoder only, on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   http://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp_webp_image_decoder_library
//   http://review.webmproject.org/gitweb?p=libwebp.git

#include <stdio.h>
extern "C" {
// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/decode.h"
#include "webp/encode.h"
}

// this enables timing code to report milliseconds for a decode
//#define TIME_DECODE

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Define VP8 I/O on top of Skia stream

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static const size_t WEBP_VP8_HEADER_SIZE = 64;
static const size_t WEBP_IDECODE_BUFFER_SZ = (1 << 16);

// Parse headers of RIFF container, and check for valid Webp (VP8) content.
static bool webp_parse_header(SkStream* stream, int* width, int* height, int* alpha) {
    unsigned char buffer[WEBP_VP8_HEADER_SIZE];
    size_t bytesToRead = WEBP_VP8_HEADER_SIZE;
    size_t totalBytesRead = 0;
    do {
        unsigned char* dst = buffer + totalBytesRead;
        const size_t bytesRead = stream->read(dst, bytesToRead);
        if (0 == bytesRead) {
            // Could not read any bytes. Check to see if we are at the end (exit
            // condition), and continue reading if not. Important for streams
            // that do not have all the data ready.
            continue;
        }
        bytesToRead -= bytesRead;
        totalBytesRead += bytesRead;
        SkASSERT(bytesToRead + totalBytesRead == WEBP_VP8_HEADER_SIZE);
    } while (!stream->isAtEnd() && bytesToRead > 0);

    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(buffer, totalBytesRead, &features);
    if (VP8_STATUS_OK != status) {
        return false; // Invalid WebP file.
    }
    *width = features.width;
    *height = features.height;
    *alpha = features.has_alpha;

    // sanity check for image size that's about to be decoded.
    {
        int64_t size = sk_64_mul(*width, *height);
        if (!sk_64_isS32(size)) {
            return false;
        }
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (sk_64_asS32(size) > (0x7FFFFFFF >> 2)) {
            return false;
        }
    }
    return true;
}

class SkWEBPImageDecoder: public SkImageDecoder {
public:
    SkWEBPImageDecoder() {
        fInputStream = NULL;
        fOrigWidth = 0;
        fOrigHeight = 0;
        fHasAlpha = 0;
    }
    virtual ~SkWEBPImageDecoder() {
        SkSafeUnref(fInputStream);
    }

    virtual Format getFormat() const SK_OVERRIDE {
        return kWEBP_Format;
    }

protected:
    virtual bool onBuildTileIndex(SkStreamRewindable *stream, int *width, int *height) SK_OVERRIDE;
    virtual bool onDecodeSubset(SkBitmap* bitmap, const SkIRect& rect) SK_OVERRIDE;
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode) SK_OVERRIDE;

private:
    /**
     *  Called when determining the output config to request to webp.
     *  If the image does not have alpha, there is no need to premultiply.
     *  If the caller wants unpremultiplied colors, that is respected.
     */
    bool shouldPremultiply() const {
        return SkToBool(fHasAlpha) && !this->getRequireUnpremultipliedColors();
    }

    bool setDecodeConfig(SkBitmap* decodedBitmap, int width, int height);

    SkStream* fInputStream;
    int fOrigWidth;
    int fOrigHeight;
    int fHasAlpha;

    typedef SkImageDecoder INHERITED;
};

//////////////////////////////////////////////////////////////////////////

#ifdef TIME_DECODE

#include "SkTime.h"

class AutoTimeMillis {
public:
    AutoTimeMillis(const char label[]) :
        fLabel(label) {
        if (NULL == fLabel) {
            fLabel = "";
        }
        fNow = SkTime::GetMSecs();
    }
    ~AutoTimeMillis() {
        SkDebugf("---- Time (ms): %s %d\n", fLabel, SkTime::GetMSecs() - fNow);
    }
private:
    const char* fLabel;
    SkMSec fNow;
};

#endif

///////////////////////////////////////////////////////////////////////////////

// This guy exists just to aid in debugging, as it allows debuggers to just
// set a break-point in one place to see all error exists.
static bool return_false(const SkBitmap& bm, const char msg[]) {
    SkDEBUGF(("libwebp error %s [%d %d]", msg, bm.width(), bm.height()));
    return false; // must always return false
}

static WEBP_CSP_MODE webp_decode_mode(const SkBitmap* decodedBitmap, bool premultiply) {
    WEBP_CSP_MODE mode = MODE_LAST;
    const SkColorType ct = decodedBitmap->colorType();

    if (ct == kN32_SkColorType) {
        #if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
            mode = premultiply ? MODE_bgrA : MODE_BGRA;
        #elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
            mode = premultiply ? MODE_rgbA : MODE_RGBA;
        #else
            #error "Skia uses BGRA or RGBA byte order"
        #endif
    } else if (ct == kARGB_4444_SkColorType) {
        mode = premultiply ? MODE_rgbA_4444 : MODE_RGBA_4444;
    } else if (ct == kRGB_565_SkColorType) {
        mode = MODE_RGB_565;
    }
    SkASSERT(MODE_LAST != mode);
    return mode;
}

// Incremental WebP image decoding. Reads input buffer of 64K size iteratively
// and decodes this block to appropriate color-space as per config object.
static bool webp_idecode(SkStream* stream, WebPDecoderConfig* config) {
    WebPIDecoder* idec = WebPIDecode(NULL, 0, config);
    if (NULL == idec) {
        WebPFreeDecBuffer(&config->output);
        return false;
    }

    if (!stream->rewind()) {
        SkDebugf("Failed to rewind webp stream!");
        return false;
    }
    const size_t readBufferSize = stream->hasLength() ?
            SkTMin(stream->getLength(), WEBP_IDECODE_BUFFER_SZ) : WEBP_IDECODE_BUFFER_SZ;
    SkAutoMalloc srcStorage(readBufferSize);
    unsigned char* input = (uint8_t*)srcStorage.get();
    if (NULL == input) {
        WebPIDelete(idec);
        WebPFreeDecBuffer(&config->output);
        return false;
    }

    bool success = true;
    VP8StatusCode status = VP8_STATUS_SUSPENDED;
    do {
        const size_t bytesRead = stream->read(input, readBufferSize);
        if (0 == bytesRead) {
            success = false;
            break;
        }

        status = WebPIAppend(idec, input, bytesRead);
        if (VP8_STATUS_OK != status && VP8_STATUS_SUSPENDED != status) {
            success = false;
            break;
        }
    } while (VP8_STATUS_OK != status);
    srcStorage.free();
    WebPIDelete(idec);
    WebPFreeDecBuffer(&config->output);

    return success;
}

static bool webp_get_config_resize(WebPDecoderConfig* config,
                                   SkBitmap* decodedBitmap,
                                   int width, int height, bool premultiply) {
    WEBP_CSP_MODE mode = webp_decode_mode(decodedBitmap, premultiply);
    if (MODE_LAST == mode) {
        return false;
    }

    if (0 == WebPInitDecoderConfig(config)) {
        return false;
    }

    config->output.colorspace = mode;
    config->output.u.RGBA.rgba = (uint8_t*)decodedBitmap->getPixels();
    config->output.u.RGBA.stride = (int) decodedBitmap->rowBytes();
    config->output.u.RGBA.size = decodedBitmap->getSize();
    config->output.is_external_memory = 1;

    if (width != decodedBitmap->width() || height != decodedBitmap->height()) {
        config->options.use_scaling = 1;
        config->options.scaled_width = decodedBitmap->width();
        config->options.scaled_height = decodedBitmap->height();
    }

    return true;
}

static bool webp_get_config_resize_crop(WebPDecoderConfig* config,
                                        SkBitmap* decodedBitmap,
                                        const SkIRect& region, bool premultiply) {

    if (!webp_get_config_resize(config, decodedBitmap, region.width(),
                                region.height(), premultiply)) {
      return false;
    }

    config->options.use_cropping = 1;
    config->options.crop_left = region.fLeft;
    config->options.crop_top = region.fTop;
    config->options.crop_width = region.width();
    config->options.crop_height = region.height();

    return true;
}

bool SkWEBPImageDecoder::setDecodeConfig(SkBitmap* decodedBitmap, int width, int height) {
    SkColorType colorType = this->getPrefColorType(k32Bit_SrcDepth, SkToBool(fHasAlpha));

    // YUV converter supports output in RGB565, RGBA4444 and RGBA8888 formats.
    if (fHasAlpha) {
        if (colorType != kARGB_4444_SkColorType) {
            colorType = kN32_SkColorType;
        }
    } else {
        if (colorType != kRGB_565_SkColorType && colorType != kARGB_4444_SkColorType) {
            colorType = kN32_SkColorType;
        }
    }

#ifdef SK_SUPPORT_LEGACY_IMAGEDECODER_CHOOSER
    if (!this->chooseFromOneChoice(colorType, width, height)) {
        return false;
    }
#endif

    SkAlphaType alphaType = kOpaque_SkAlphaType;
    if (SkToBool(fHasAlpha)) {
        if (this->getRequireUnpremultipliedColors()) {
            alphaType = kUnpremul_SkAlphaType;
        } else {
            alphaType = kPremul_SkAlphaType;
        }
    }
    return decodedBitmap->setInfo(SkImageInfo::Make(width, height, colorType, alphaType));
}

bool SkWEBPImageDecoder::onBuildTileIndex(SkStreamRewindable* stream,
                                          int *width, int *height) {
    int origWidth, origHeight, hasAlpha;
    if (!webp_parse_header(stream, &origWidth, &origHeight, &hasAlpha)) {
        return false;
    }

    if (!stream->rewind()) {
        SkDebugf("Failed to rewind webp stream!");
        return false;
    }

    *width = origWidth;
    *height = origHeight;

    SkRefCnt_SafeAssign(this->fInputStream, stream);
    this->fOrigWidth = origWidth;
    this->fOrigHeight = origHeight;
    this->fHasAlpha = hasAlpha;

    return true;
}

static bool is_config_compatible(const SkBitmap& bitmap) {
    const SkColorType ct = bitmap.colorType();
    return ct == kARGB_4444_SkColorType || ct == kRGB_565_SkColorType || ct == kN32_SkColorType;
}

bool SkWEBPImageDecoder::onDecodeSubset(SkBitmap* decodedBitmap,
                                        const SkIRect& region) {
    SkIRect rect = SkIRect::MakeWH(fOrigWidth, fOrigHeight);

    if (!rect.intersect(region)) {
        // If the requested region is entirely outsides the image, return false
        return false;
    }

    const int sampleSize = this->getSampleSize();
    SkScaledBitmapSampler sampler(rect.width(), rect.height(), sampleSize);
    const int width = sampler.scaledWidth();
    const int height = sampler.scaledHeight();

    // The image can be decoded directly to decodedBitmap if
    //   1. the region is within the image range
    //   2. bitmap's config is compatible
    //   3. bitmap's size is same as the required region (after sampled)
    bool directDecode = (rect == region) &&
                        (decodedBitmap->isNull() ||
                         (is_config_compatible(*decodedBitmap) &&
                         (decodedBitmap->width() == width) &&
                         (decodedBitmap->height() == height)));

    SkBitmap tmpBitmap;
    SkBitmap *bitmap = decodedBitmap;

    if (!directDecode) {
        bitmap = &tmpBitmap;
    }

    if (bitmap->isNull()) {
        if (!setDecodeConfig(bitmap, width, height)) {
            return false;
        }
        // alloc from native heap if it is a temp bitmap. (prevent GC)
        bool allocResult = (bitmap == decodedBitmap)
                               ? allocPixelRef(bitmap, NULL)
                               : bitmap->tryAllocPixels();
        if (!allocResult) {
            return return_false(*decodedBitmap, "allocPixelRef");
        }
#ifdef SK_SUPPORT_LEGACY_IMAGEDECODER_CHOOSER
    } else {
        // This is also called in setDecodeConfig in above block.
        // i.e., when bitmap->isNull() is true.
        if (!chooseFromOneChoice(bitmap->colorType(), width, height)) {
            return false;
        }
#endif
    }

    SkAutoLockPixels alp(*bitmap);
    WebPDecoderConfig config;
    if (!webp_get_config_resize_crop(&config, bitmap, rect,
                                     this->shouldPremultiply())) {
        return false;
    }

    // Decode the WebP image data stream using WebP incremental decoding for
    // the specified cropped image-region.
    if (!webp_idecode(this->fInputStream, &config)) {
        return false;
    }

    if (!directDecode) {
        cropBitmap(decodedBitmap, bitmap, sampleSize, region.x(), region.y(),
                   region.width(), region.height(), rect.x(), rect.y());
    }
    return true;
}

bool SkWEBPImageDecoder::onDecode(SkStream* stream, SkBitmap* decodedBitmap,
                                  Mode mode) {
#ifdef TIME_DECODE
    AutoTimeMillis atm("WEBP Decode");
#endif

    int origWidth, origHeight, hasAlpha;
    if (!webp_parse_header(stream, &origWidth, &origHeight, &hasAlpha)) {
        return false;
    }
    this->fHasAlpha = hasAlpha;

    const int sampleSize = this->getSampleSize();
    SkScaledBitmapSampler sampler(origWidth, origHeight, sampleSize);
    if (!setDecodeConfig(decodedBitmap, sampler.scaledWidth(),
                         sampler.scaledHeight())) {
        return false;
    }

    // If only bounds are requested, done
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }

    if (!this->allocPixelRef(decodedBitmap, NULL)) {
        return return_false(*decodedBitmap, "allocPixelRef");
    }

    SkAutoLockPixels alp(*decodedBitmap);

    WebPDecoderConfig config;
    if (!webp_get_config_resize(&config, decodedBitmap, origWidth, origHeight,
                                this->shouldPremultiply())) {
        return false;
    }

    // Decode the WebP image data stream using WebP incremental decoding.
    return webp_idecode(stream, &config);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkUnPreMultiply.h"

typedef void (*ScanlineImporter)(const uint8_t* in, uint8_t* out, int width,
                                 const SkPMColor* SK_RESTRICT ctable);

static void ARGB_8888_To_RGB(const uint8_t* in, uint8_t* rgb, int width,
                             const SkPMColor*) {
  const uint32_t* SK_RESTRICT src = (const uint32_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint32_t c = *src++;
      rgb[0] = SkGetPackedR32(c);
      rgb[1] = SkGetPackedG32(c);
      rgb[2] = SkGetPackedB32(c);
      rgb += 3;
  }
}

static void ARGB_8888_To_RGBA(const uint8_t* in, uint8_t* rgb, int width,
                              const SkPMColor*) {
  const uint32_t* SK_RESTRICT src = (const uint32_t*)in;
  const SkUnPreMultiply::Scale* SK_RESTRICT table =
      SkUnPreMultiply::GetScaleTable();
  for (int i = 0; i < width; ++i) {
      const uint32_t c = *src++;
      uint8_t a = SkGetPackedA32(c);
      uint8_t r = SkGetPackedR32(c);
      uint8_t g = SkGetPackedG32(c);
      uint8_t b = SkGetPackedB32(c);
      if (0 != a && 255 != a) {
        SkUnPreMultiply::Scale scale = table[a];
        r = SkUnPreMultiply::ApplyScale(scale, r);
        g = SkUnPreMultiply::ApplyScale(scale, g);
        b = SkUnPreMultiply::ApplyScale(scale, b);
      }
      rgb[0] = r;
      rgb[1] = g;
      rgb[2] = b;
      rgb[3] = a;
      rgb += 4;
  }
}

static void RGB_565_To_RGB(const uint8_t* in, uint8_t* rgb, int width,
                           const SkPMColor*) {
  const uint16_t* SK_RESTRICT src = (const uint16_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint16_t c = *src++;
      rgb[0] = SkPacked16ToR32(c);
      rgb[1] = SkPacked16ToG32(c);
      rgb[2] = SkPacked16ToB32(c);
      rgb += 3;
  }
}

static void ARGB_4444_To_RGB(const uint8_t* in, uint8_t* rgb, int width,
                             const SkPMColor*) {
  const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)in;
  for (int i = 0; i < width; ++i) {
      const SkPMColor16 c = *src++;
      rgb[0] = SkPacked4444ToR32(c);
      rgb[1] = SkPacked4444ToG32(c);
      rgb[2] = SkPacked4444ToB32(c);
      rgb += 3;
  }
}

static void ARGB_4444_To_RGBA(const uint8_t* in, uint8_t* rgb, int width,
                              const SkPMColor*) {
  const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)in;
  const SkUnPreMultiply::Scale* SK_RESTRICT table =
      SkUnPreMultiply::GetScaleTable();
  for (int i = 0; i < width; ++i) {
      const SkPMColor16 c = *src++;
      uint8_t a = SkPacked4444ToA32(c);
      uint8_t r = SkPacked4444ToR32(c);
      uint8_t g = SkPacked4444ToG32(c);
      uint8_t b = SkPacked4444ToB32(c);
      if (0 != a && 255 != a) {
        SkUnPreMultiply::Scale scale = table[a];
        r = SkUnPreMultiply::ApplyScale(scale, r);
        g = SkUnPreMultiply::ApplyScale(scale, g);
        b = SkUnPreMultiply::ApplyScale(scale, b);
      }
      rgb[0] = r;
      rgb[1] = g;
      rgb[2] = b;
      rgb[3] = a;
      rgb += 4;
  }
}

static void Index8_To_RGB(const uint8_t* in, uint8_t* rgb, int width,
                          const SkPMColor* SK_RESTRICT ctable) {
  const uint8_t* SK_RESTRICT src = (const uint8_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint32_t c = ctable[*src++];
      rgb[0] = SkGetPackedR32(c);
      rgb[1] = SkGetPackedG32(c);
      rgb[2] = SkGetPackedB32(c);
      rgb += 3;
  }
}

static ScanlineImporter ChooseImporter(SkColorType ct, bool  hasAlpha, int*  bpp) {
    switch (ct) {
        case kN32_SkColorType:
            if (hasAlpha) {
                *bpp = 4;
                return ARGB_8888_To_RGBA;
            } else {
                *bpp = 3;
                return ARGB_8888_To_RGB;
            }
        case kARGB_4444_SkColorType:
            if (hasAlpha) {
                *bpp = 4;
                return ARGB_4444_To_RGBA;
            } else {
                *bpp = 3;
                return ARGB_4444_To_RGB;
            }
        case kRGB_565_SkColorType:
            *bpp = 3;
            return RGB_565_To_RGB;
        case kIndex_8_SkColorType:
            *bpp = 3;
            return Index8_To_RGB;
        default:
            return NULL;
    }
}

static int stream_writer(const uint8_t* data, size_t data_size,
                         const WebPPicture* const picture) {
  SkWStream* const stream = (SkWStream*)picture->custom_ptr;
  return stream->write(data, data_size) ? 1 : 0;
}

class SkWEBPImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) SK_OVERRIDE;

private:
    typedef SkImageEncoder INHERITED;
};

bool SkWEBPImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bm,
                                  int quality) {
    const bool hasAlpha = !bm.isOpaque();
    int bpp = -1;
    const ScanlineImporter scanline_import = ChooseImporter(bm.colorType(), hasAlpha, &bpp);
    if (NULL == scanline_import) {
        return false;
    }
    if (-1 == bpp) {
        return false;
    }

    SkAutoLockPixels alp(bm);
    SkAutoLockColors ctLocker;
    if (NULL == bm.getPixels()) {
        return false;
    }

    WebPConfig webp_config;
    if (!WebPConfigPreset(&webp_config, WEBP_PRESET_DEFAULT, (float) quality)) {
        return false;
    }

    WebPPicture pic;
    WebPPictureInit(&pic);
    pic.width = bm.width();
    pic.height = bm.height();
    pic.writer = stream_writer;
    pic.custom_ptr = (void*)stream;

    const SkPMColor* colors = ctLocker.lockColors(bm);
    const uint8_t* src = (uint8_t*)bm.getPixels();
    const int rgbStride = pic.width * bpp;

    // Import (for each scanline) the bit-map image (in appropriate color-space)
    // to RGB color space.
    uint8_t* rgb = new uint8_t[rgbStride * pic.height];
    for (int y = 0; y < pic.height; ++y) {
        scanline_import(src + y * bm.rowBytes(), rgb + y * rgbStride,
                        pic.width, colors);
    }

    bool ok;
    if (bpp == 3) {
        ok = SkToBool(WebPPictureImportRGB(&pic, rgb, rgbStride));
    } else {
        ok = SkToBool(WebPPictureImportRGBA(&pic, rgb, rgbStride));
    }
    delete[] rgb;

    ok = ok && WebPEncode(&webp_config, &pic);
    WebPPictureFree(&pic);

    return ok;
}


///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(WEBPImageDecoder);
DEFINE_ENCODER_CREATOR(WEBPImageEncoder);
///////////////////////////////////////////////////////////////////////////////

static SkImageDecoder* sk_libwebp_dfactory(SkStreamRewindable* stream) {
    int width, height, hasAlpha;
    if (!webp_parse_header(stream, &width, &height, &hasAlpha)) {
        return NULL;
    }

    // Magic matches, call decoder
    return SkNEW(SkWEBPImageDecoder);
}

static SkImageDecoder::Format get_format_webp(SkStreamRewindable* stream) {
    int width, height, hasAlpha;
    if (webp_parse_header(stream, &width, &height, &hasAlpha)) {
        return SkImageDecoder::kWEBP_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageEncoder* sk_libwebp_efactory(SkImageEncoder::Type t) {
      return (SkImageEncoder::kWEBP_Type == t) ? SkNEW(SkWEBPImageEncoder) : NULL;
}

static SkImageDecoder_DecodeReg gDReg(sk_libwebp_dfactory);
static SkImageDecoder_FormatReg gFormatReg(get_format_webp);
static SkImageEncoder_EncodeReg gEReg(sk_libwebp_efactory);
