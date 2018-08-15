/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSRGB_DEFINED
#define SkSRGB_DEFINED

#include "SkTypes.h"

/** Components for building our canonical sRGB -> linear and linear -> sRGB transformations.
 *
 *  Current best practices:
 *      - for sRGB -> linear, lookup R,G,B in sk_linear_from_srgb;
 *      - for linear -> sRGB, call sk_linear_to_srgb() for R,G,B;
 *      - the alpha channel is linear in both formats, needing at most *(1/255.0f) or *255.0f.
 *
 *  sk_linear_to_srgb() will run a little faster than usual when compiled with SSE4.1+.
 */

extern const uint16_t sk_linear12_from_srgb[256];
extern const uint8_t  sk_linear12_to_srgb[4096];

#endif//SkSRGB_DEFINED
