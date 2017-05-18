/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_opts_SSSE3_DEFINED
#define SkBitmapProcState_opts_SSSE3_DEFINED

#include "SkBitmapProcState.h"

void S32_opaque_D32_filter_DX_SSSE3(const SkBitmapProcState& s,
                                    const uint32_t* xy,
                                    int count, uint32_t* colors);
void S32_alpha_D32_filter_DX_SSSE3(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors);
void S32_opaque_D32_filter_DXDY_SSSE3(const SkBitmapProcState& s,
                                    const uint32_t* xy,
                                    int count, uint32_t* colors);
void S32_alpha_D32_filter_DXDY_SSSE3(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors);
#endif
