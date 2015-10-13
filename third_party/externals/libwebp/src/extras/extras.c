// Copyright 2015 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  Additional WebP utilities.
//

#include "../webp/extras.h"

#include <string.h>

#define XTRA_MAJ_VERSION 0
#define XTRA_MIN_VERSION 0
#define XTRA_REV_VERSION 0

//------------------------------------------------------------------------------

int WebPGetExtrasVersion(void) {
  return (XTRA_MAJ_VERSION << 16) | (XTRA_MIN_VERSION << 8) | XTRA_REV_VERSION;
}

//------------------------------------------------------------------------------

int WebPImportGray(const uint8_t* gray_data, WebPPicture* pic) {
  int y, width, uv_width;
  if (pic == NULL || gray_data == NULL) return 0;
  pic->colorspace = WEBP_YUV420;
  if (!WebPPictureAlloc(pic)) return 0;
  width = pic->width;
  uv_width = (width + 1) >> 1;
  for (y = 0; y < pic->height; ++y) {
    memcpy(pic->y + y * pic->y_stride, gray_data, width);
    gray_data += width;    // <- we could use some 'data_stride' here if needed
    if ((y & 1) == 0) {
      memset(pic->u + (y >> 1) * pic->uv_stride, 128, uv_width);
      memset(pic->v + (y >> 1) * pic->uv_stride, 128, uv_width);
    }
  }
  return 1;
}

int WebPImportRGB565(const uint8_t* rgb565, WebPPicture* pic) {
  int x, y;
  if (pic == NULL || rgb565 == NULL) return 0;
  pic->colorspace = WEBP_YUV420;
  pic->use_argb = 1;
  if (!WebPPictureAlloc(pic)) return 0;
  for (y = 0; y < pic->height; ++y) {
    const int width = pic->width;
    uint32_t* dst = pic->argb + y * pic->argb_stride;
    for (x = 0; x < width; ++x) {
#ifdef WEBP_SWAP_16BIT_CSP
      const uint32_t rg = rgb565[2 * x + 1];
      const uint32_t gb = rgb565[2 * x + 0];
#else
      const uint32_t rg = rgb565[2 * x + 0];
      const uint32_t gb = rgb565[2 * x + 1];
#endif
      uint32_t r = rg & 0xf8;
      uint32_t g = ((rg << 5) | (gb >> 3)) & 0xfc;
      uint32_t b = (gb << 5);
      // dithering
      r = r | (r >> 5);
      g = g | (g >> 6);
      b = b | (b >> 5);
      dst[x] = (r << 16) | (g << 8) | b;
    }
    rgb565 += 2 * width;
  }
  return 1;
}

int WebPImportRGB4444(const uint8_t* rgb4444, WebPPicture* pic) {
  int x, y;
  if (pic == NULL || rgb4444 == NULL) return 0;
  pic->colorspace = WEBP_YUV420;
  pic->use_argb = 1;
  if (!WebPPictureAlloc(pic)) return 0;
  for (y = 0; y < pic->height; ++y) {
    const int width = pic->width;
    uint32_t* dst = pic->argb + y * pic->argb_stride;
    for (x = 0; x < width; ++x) {
#ifdef WEBP_SWAP_16BIT_CSP
      const uint32_t rg = rgb4444[2 * x + 1];
      const uint32_t ba = rgb4444[2 * x + 0];
#else
      const uint32_t rg = rgb4444[2 * x + 0];
      const uint32_t ba = rgb4444[2 * x + 1];
#endif
      uint32_t r = rg & 0xf0;
      uint32_t g = (rg << 4);
      uint32_t b = (ba & 0xf0);
      uint32_t a = (ba << 4);
      // dithering
      r = r | (r >> 4);
      g = g | (g >> 4);
      b = b | (b >> 4);
      a = a | (a >> 4);
      dst[x] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    rgb4444 += 2 * width;
  }
  return 1;
}

//------------------------------------------------------------------------------
