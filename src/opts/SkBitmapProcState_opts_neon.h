// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE
// file.

#ifndef SkBitmapProcState_opts_neon_DEFINED
#define SkBitmapProcState_opts_neon_DEFINED

#include "SkBitmapProcState.h"

void S32_opaque_D32_filter_DX_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                      unsigned* colors);

void S32_alpha_D32_filter_DX_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                     unsigned* colors);

void S32_opaque_D32_filter_DXDY_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                        unsigned* colors);

void S32_alpha_D32_filter_DXDY_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                       unsigned* colors);
#endif