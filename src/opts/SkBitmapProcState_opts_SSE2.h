
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapProcState.h"

void S32_opaque_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors);
void S32_alpha_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                  const uint32_t* xy,
                                  int count, uint32_t* colors);
void Color32_SSE2(SkPMColor dst[], const SkPMColor src[], int count,
                  SkPMColor color);
void ClampX_ClampY_filter_scale_SSE2(const SkBitmapProcState& s, uint32_t xy[],
                                     int count, int x, int y);
void ClampX_ClampY_nofilter_scale_SSE2(const SkBitmapProcState& s,
                                       uint32_t xy[], int count, int x, int y);
void ClampX_ClampY_filter_affine_SSE2(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y);
void ClampX_ClampY_nofilter_affine_SSE2(const SkBitmapProcState& s,
                                       uint32_t xy[], int count, int x, int y);
void S32_D16_filter_DX_SSE2(const SkBitmapProcState& s,
                                  const uint32_t* xy,
                                  int count, uint16_t* colors);
