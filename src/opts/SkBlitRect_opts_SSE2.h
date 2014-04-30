/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRect_opts_SSE2_DEFINED
#define SkBlitRect_opts_SSE2_DEFINED

#include "SkColor.h"

/* These functions' implementations copy sections of both
 * SkBlitRow_opts_SSE2 and SkUtils_opts_SSE2.
 */
void ColorRect32_SSE2(SkPMColor* SK_RESTRICT dst,
                      int width, int height,
                      size_t rowBytes, uint32_t color);


#endif
