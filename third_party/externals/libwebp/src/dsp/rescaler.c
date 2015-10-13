// Copyright 2014 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Rescaling functions

#include <assert.h>

#include "./dsp.h"
#include "../utils/rescaler.h"

//------------------------------------------------------------------------------
// Implementations of critical functions ImportRow / ExportRow

#define ROUNDER (WEBP_RESCALER_ONE >> 1)
#define MULT_FIX(x, y) (((uint64_t)(x) * (y) + ROUNDER) >> WEBP_RESCALER_RFIX)

//------------------------------------------------------------------------------
// Row import

void WebPRescalerImportRowExpandC(WebPRescaler* const wrk,
                                  const uint8_t* const src, int channel) {
  const int x_stride = wrk->num_channels;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  int x_in = channel;
  int x_out;
  // simple bilinear interpolation
  int accum = wrk->x_add;
  int left = src[x_in];
  int right = (wrk->src_width > 1) ? src[x_in + x_stride] : left;
  x_in += x_stride;
  x_out = channel;

  assert(!WebPRescalerInputDone(wrk));
  assert(wrk->x_expand);
  while (1) {
    wrk->frow[x_out] = right * wrk->x_add + (left - right) * accum;
    x_out += x_stride;
    if (x_out >= x_out_max) break;
    accum -= wrk->x_sub;
    if (accum < 0) {
      left = right;
      x_in += x_stride;
      assert(x_in < wrk->src_width * x_stride);
      right = src[x_in];
      accum += wrk->x_add;
    }
  }
  assert(wrk->x_sub == 0 /* <- special case for src_width=1 */ || accum == 0);
}

void WebPRescalerImportRowShrinkC(WebPRescaler* const wrk,
                                  const uint8_t* const src, int channel) {
  const int x_stride = wrk->num_channels;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  int x_in = channel;
  int x_out;
  uint32_t sum = 0;
  int accum = 0;

  assert(!WebPRescalerInputDone(wrk));
  assert(!wrk->x_expand);
  for (x_out = channel; x_out < x_out_max; x_out += x_stride) {
    uint32_t base = 0;
    accum += wrk->x_add;
    while (accum > 0) {
      accum -= wrk->x_sub;
      assert(x_in < wrk->src_width * x_stride);
      base = src[x_in];
      sum += base;
      x_in += x_stride;
    }
    {        // Emit next horizontal pixel.
      const rescaler_t frac = base * (-accum);
      wrk->frow[x_out] = sum * wrk->x_sub - frac;
      // fresh fractional start for next pixel
      sum = (int)MULT_FIX(frac, wrk->fx_scale);
    }
  }
  assert(accum == 0);
}

//------------------------------------------------------------------------------
// Row export

void WebPRescalerExportRowExpandC(WebPRescaler* const wrk) {
  int x_out;
  uint8_t* const dst = wrk->dst;
  rescaler_t* const irow = wrk->irow;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  const rescaler_t* const frow = wrk->frow;
  assert(!WebPRescalerOutputDone(wrk));
  assert(wrk->y_accum <= 0);
  assert(wrk->y_expand);
  if (wrk->y_accum == 0) {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const int v = (int)MULT_FIX(frow[x_out], wrk->fy_scale);
      dst[x_out] = (!(v & ~0xff)) ? v : (v < 0) ? 0 : 255;
    }
  } else {
    const int64_t A = wrk->y_sub + wrk->y_accum;
    const int64_t B = -wrk->y_accum;
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const int64_t I = A * frow[x_out] + B * irow[x_out];
      const int v = (int)MULT_FIX(I, wrk->fxy_scale);
      dst[x_out] = (!(v & ~0xff)) ? v : (v < 0) ? 0 : 255;
    }
  }
}

void WebPRescalerExportRowShrinkC(WebPRescaler* const wrk) {
  int x_out;
  uint8_t* const dst = wrk->dst;
  rescaler_t* const irow = wrk->irow;
  const int x_out_max = wrk->dst_width * wrk->num_channels;
  const rescaler_t* const frow = wrk->frow;
  const int yscale = wrk->fy_scale * (-wrk->y_accum);
  assert(!WebPRescalerOutputDone(wrk));
  assert(wrk->y_accum <= 0);
  assert(!wrk->y_expand);
  if (yscale) {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const int frac = (int)MULT_FIX(frow[x_out], yscale);
      const int v = (int)MULT_FIX(irow[x_out] - frac, wrk->fxy_scale);
      dst[x_out] = (!(v & ~0xff)) ? v : (v < 0) ? 0 : 255;
      irow[x_out] = frac;   // new fractional start
    }
  } else {
    for (x_out = 0; x_out < x_out_max; ++x_out) {
      const int v = (int)MULT_FIX(irow[x_out], wrk->fxy_scale);
      dst[x_out] = (!(v & ~0xff)) ? v : (v < 0) ? 0 : 255;
      irow[x_out] = 0;
    }
  }
}

#undef MULT_FIX
#undef ROUNDER

//------------------------------------------------------------------------------
// Main entry calls

void WebPRescalerImportRow(WebPRescaler* const wrk,
                           const uint8_t* const src, int channel) {
  assert(!WebPRescalerInputDone(wrk));
  if (!wrk->x_expand) {
    WebPRescalerImportRowShrink(wrk, src, channel);
  } else {
    WebPRescalerImportRowExpand(wrk, src, channel);
  }
}

void WebPRescalerExportRow(WebPRescaler* const wrk) {
  if (wrk->y_accum <= 0) {
    assert(!WebPRescalerOutputDone(wrk));
    if (wrk->y_expand) {
      WebPRescalerExportRowExpand(wrk);
    } else {
      WebPRescalerExportRowShrink(wrk);
    }
    wrk->y_accum += wrk->y_add;
    wrk->dst += wrk->dst_stride;
    ++wrk->dst_y;
  }
}

//------------------------------------------------------------------------------

WebPRescalerImportRowFunc WebPRescalerImportRowExpand;
WebPRescalerImportRowFunc WebPRescalerImportRowShrink;

WebPRescalerExportRowFunc WebPRescalerExportRowExpand;
WebPRescalerExportRowFunc WebPRescalerExportRowShrink;

extern void WebPRescalerDspInitMIPS32(void);
extern void WebPRescalerDspInitMIPSdspR2(void);

static volatile VP8CPUInfo rescaler_last_cpuinfo_used =
    (VP8CPUInfo)&rescaler_last_cpuinfo_used;

WEBP_TSAN_IGNORE_FUNCTION void WebPRescalerDspInit(void) {
  if (rescaler_last_cpuinfo_used == VP8GetCPUInfo) return;

  WebPRescalerImportRowExpand = WebPRescalerImportRowExpandC;
  WebPRescalerImportRowShrink = WebPRescalerImportRowShrinkC;
  WebPRescalerExportRowExpand = WebPRescalerExportRowExpandC;
  WebPRescalerExportRowShrink = WebPRescalerExportRowShrinkC;

  if (VP8GetCPUInfo != NULL) {
#if defined(WEBP_USE_MIPS32)
    if (VP8GetCPUInfo(kMIPS32)) {
      WebPRescalerDspInitMIPS32();
    }
#endif
#if defined(WEBP_USE_MIPS_DSP_R2)
    if (VP8GetCPUInfo(kMIPSdspR2)) {
      WebPRescalerDspInitMIPSdspR2();
    }
#endif
  }
  rescaler_last_cpuinfo_used = VP8GetCPUInfo;
}
