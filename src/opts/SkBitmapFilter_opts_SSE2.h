
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBitmapFilter_opts_sse2_DEFINED
#define SkBitmapFilter_opts_sse2_DEFINED

#include "SkBitmapProcState.h"

void highQualityFilter_ScaleOnly_SSE2(const SkBitmapProcState &s, int x, int y,
                          SkPMColor *SK_RESTRICT colors, int count);
void highQualityFilter_SSE2(const SkBitmapProcState &s, int x, int y,
                SkPMColor *SK_RESTRICT colors, int count);

#endif
