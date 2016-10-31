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

#include "SkBitmap.h"
#include "SkImageEncoder.h"
#include "SkColorPriv.h"
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
#include "webp/encode.h"
}

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
            return nullptr;
    }
}

static int stream_writer(const uint8_t* data, size_t data_size,
                         const WebPPicture* const picture) {
  SkWStream* const stream = (SkWStream*)picture->custom_ptr;
  return stream->write(data, data_size) ? 1 : 0;
}

class SkWEBPImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;

private:
    typedef SkImageEncoder INHERITED;
};

bool SkWEBPImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bm,
                                  int quality) {
    const bool hasAlpha = !bm.isOpaque();
    int bpp = -1;
    const ScanlineImporter scanline_import = ChooseImporter(bm.colorType(), hasAlpha, &bpp);
    if (nullptr == scanline_import) {
        return false;
    }
    if (-1 == bpp) {
        return false;
    }

    SkAutoLockPixels alp(bm);
    if (nullptr == bm.getPixels()) {
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

    const SkPMColor* colors = bm.getColorTable() ? bm.getColorTable()->readColors() : nullptr;
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
DEFINE_ENCODER_CREATOR(WEBPImageEncoder);
///////////////////////////////////////////////////////////////////////////////

static SkImageEncoder* sk_libwebp_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kWEBP_Type == t) ? new SkWEBPImageEncoder : nullptr;
}

static SkImageEncoder_EncodeReg gEReg(sk_libwebp_efactory);
