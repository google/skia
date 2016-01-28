/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngFilters_DEFINED
#define SkPngFilters_DEFINED

#include "png.h"
#include "SkTypes.h"

// We don't bother specializing Up...
// it's so simple it's usually already perfectly autovectorized.

// These all require bpp=3 (i.e. RGB).
void   sk_sub3_sse2(png_row_infop, uint8_t*, const uint8_t*);
void   sk_avg3_sse2(png_row_infop, uint8_t*, const uint8_t*);
void sk_paeth3_sse2(png_row_infop, uint8_t*, const uint8_t*);

// These all require bpp=4 (i.e. RGBA).
void   sk_sub4_sse2(png_row_infop, uint8_t*, const uint8_t*);
void   sk_avg4_sse2(png_row_infop, uint8_t*, const uint8_t*);
void sk_paeth4_sse2(png_row_infop, uint8_t*, const uint8_t*);

#endif//SkPngFilterOpts_DEFINED
