/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapFilter_opts_sse2_DEFINED
#define SkBitmapFilter_opts_sse2_DEFINED

#include "SkBitmapProcState.h"
#include "SkConvolver.h"

void convolveVertically_SSE2(const SkConvolutionFilter1D::ConvolutionFixed* filter_values,
                             int filter_length,
                             unsigned char* const* source_data_rows,
                             int pixel_width,
                             unsigned char* out_row,
                             bool has_alpha);
void convolve4RowsHorizontally_SSE2(const unsigned char* src_data[4],
                                    const SkConvolutionFilter1D& filter,
                                    unsigned char* out_row[4],
                                    size_t outRowBytes);
void convolveHorizontally_SSE2(const unsigned char* src_data,
                               const SkConvolutionFilter1D& filter,
                               unsigned char* out_row,
                               bool has_alpha);
void applySIMDPadding_SSE2(SkConvolutionFilter1D* filter);

#endif
