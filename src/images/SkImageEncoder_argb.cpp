/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoder.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"

class SkARGBImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;

private:
    typedef SkImageEncoder INHERITED;
};

typedef void (*ScanlineImporter)(const uint8_t* in, uint8_t* argb, int width,
                                 const SkPMColor* SK_RESTRICT ctable);

static void ARGB_8888_To_ARGB(const uint8_t* in, uint8_t* argb, int width, const SkPMColor*) {
  const uint32_t* SK_RESTRICT src = (const uint32_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint32_t c = *src++;
      argb[0] = SkGetPackedA32(c);
      argb[1] = SkGetPackedR32(c);
      argb[2] = SkGetPackedG32(c);
      argb[3] = SkGetPackedB32(c);
      argb += 4;
  }
}

static void RGB_565_To_ARGB(const uint8_t* in, uint8_t* argb, int width, const SkPMColor*) {
  const uint16_t* SK_RESTRICT src = (const uint16_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint16_t c = *src++;
      argb[0] = 0xFF;
      argb[1] = SkPacked16ToR32(c);
      argb[2] = SkPacked16ToG32(c);
      argb[3] = SkPacked16ToB32(c);
      argb += 4;
  }
}

static void ARGB_4444_To_ARGB(const uint8_t* in, uint8_t* argb, int width, const SkPMColor*) {
  const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)in;
  for (int i = 0; i < width; ++i) {
      const SkPMColor16 c = *src++;
      argb[0] = SkPacked4444ToA32(c);
      argb[1] = SkPacked4444ToR32(c);
      argb[2] = SkPacked4444ToG32(c);
      argb[3] = SkPacked4444ToB32(c);
      argb += 4;
  }
}

static void Index8_To_ARGB(const uint8_t* in, uint8_t* argb, int width,
                           const SkPMColor* SK_RESTRICT ctable) {
  const uint8_t* SK_RESTRICT src = (const uint8_t*)in;
  for (int i = 0; i < width; ++i) {
      const uint32_t c = ctable[*src++];
      argb[0] = SkGetPackedA32(c);
      argb[1] = SkGetPackedR32(c);
      argb[2] = SkGetPackedG32(c);
      argb[3] = SkGetPackedB32(c);
      argb += 4;
  }
}

static ScanlineImporter ChooseImporter(SkColorType ct) {
    switch (ct) {
        case kN32_SkColorType:
            return ARGB_8888_To_ARGB;
        case kRGB_565_SkColorType:
            return RGB_565_To_ARGB;
        case kARGB_4444_SkColorType:
            return ARGB_4444_To_ARGB;
        case kIndex_8_SkColorType:
            return Index8_To_ARGB;
        default:
            return NULL;
    }
}

bool SkARGBImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bitmap, int) {
    const ScanlineImporter scanline_import = ChooseImporter(bitmap.colorType());
    if (NULL == scanline_import) {
        return false;
    }

    SkAutoLockPixels alp(bitmap);
    const uint8_t* src = (uint8_t*)bitmap.getPixels();
    if (NULL == bitmap.getPixels()) {
        return false;
    }

    const SkPMColor* colors = bitmap.getColorTable() ? bitmap.getColorTable()->readColors() : NULL;

    const int argbStride = bitmap.width() * 4;
    SkAutoTDeleteArray<uint8_t> ada(new uint8_t[argbStride]);
    uint8_t* argb = ada.get();
    for (int y = 0; y < bitmap.height(); ++y) {
        scanline_import(src + y * bitmap.rowBytes(), argb, bitmap.width(), colors);
        stream->write(argb, argbStride);
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
DEFINE_ENCODER_CREATOR(ARGBImageEncoder);
///////////////////////////////////////////////////////////////////////////////
