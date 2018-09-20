/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Define NAME_WRAP(x) before including this header to perform name-wrapping
// E.g. for ARM NEON, defined it as 'x ## _neon' to ensure all important
// identifiers have a _neon suffix.
#ifndef NAME_WRAP
#error "Please define NAME_WRAP() before including this file"
#endif

// SRC == 8888

#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        NAME_WRAP(S32_opaque_D32 ## suffix)
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_alpha)(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        NAME_WRAP(S32_alpha_D32 ## suffix)
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(src, alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef NAME_WRAP
