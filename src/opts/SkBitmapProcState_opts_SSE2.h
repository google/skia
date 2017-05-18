/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_opts_SSE2_DEFINED
#define SkBitmapProcState_opts_SSE2_DEFINED

#include "SkBitmapProcState.h"

void S32_opaque_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors);
void S32_alpha_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                  const uint32_t* xy,
                                  int count, uint32_t* colors);
void ClampX_ClampY_filter_scale_SSE2(const SkBitmapProcState& s, uint32_t xy[],
                                     int count, int x, int y);
void ClampX_ClampY_nofilter_scale_SSE2(const SkBitmapProcState& s,
                                       uint32_t xy[], int count, int x, int y);
void ClampX_ClampY_filter_affine_SSE2(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y);
void ClampX_ClampY_nofilter_affine_SSE2(const SkBitmapProcState& s,
                                        uint32_t xy[], int count, int x, int y);

#endif
