/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorLookUpTable.h"
#include "SkColorSpaceXformPriv.h"
#include "SkFloatingPoint.h"

SkColorLookUpTable::SkColorLookUpTable(uint8_t inputChannels, const uint8_t limits[]) {
    fInputChannels = inputChannels;
    SkASSERT(inputChannels >= 1 && inputChannels <= kMaxColorChannels);
    memcpy(fLimits, limits, fInputChannels * sizeof(uint8_t));

    for (int i = 0; i < inputChannels; i++) {
        SkASSERT(fLimits[i] > 1);
    }
}

// Our general strategy is to recursively interpolate each dimension,
// accumulating the index to sample at, and our current pixel stride to help accumulate the index.
template <int dim>
static Sk4f interp_dimension(const float* table, const uint8_t* limits,
                             const float* src, int index, int stride) {
    // We'd logically like to sample this dimension at x.
    int limit = limits[dim];
    float x = src[dim] * (limit - 1);

    // We can't index an array by a float (darn) so we have to snap to nearby integers lo and hi.
    int lo = (int)(x          ),
        hi = (int)(x + 0.9999f);

    // Recursively sample at lo and hi.
    Sk4f L = interp_dimension<dim-1>(table,limits,src, stride*lo + index, stride*limit),
         H = interp_dimension<dim-1>(table,limits,src, stride*hi + index, stride*limit);

    // Linearly interpolate those colors based on their distance to x.
    float t = (x - lo);
    return (1 - t)*L + t*H;
}

// Bottom out our recursion at 0 dimensions, i.e. just return the color at index.
template <>
Sk4f interp_dimension<-1>(const float* table, const uint8_t* limits,
                          const float* src, int index, int stride) {
    return {
        table[3*index+0],
        table[3*index+1],
        table[3*index+2],
        0.0f,
    };
}

template <int dim>
static Sk4f interp_dimension(const float* table, const uint8_t* limits, const float* src) {
    // Start our accumulated index and stride off at their identity values, 0 and 1.
    return interp_dimension<dim>(table, limits, src, 0,1);
}

void SkColorLookUpTable::interp(float* dst, const float* src) const {
    Sk4f rgb;
    switch (fInputChannels-1) {
        case 0: rgb = interp_dimension<0>(this->table(), fLimits, src); break;
        case 1: rgb = interp_dimension<1>(this->table(), fLimits, src); break;
        case 2: rgb = interp_dimension<2>(this->table(), fLimits, src); break;
        case 3: rgb = interp_dimension<3>(this->table(), fLimits, src); break;
        default: SkDEBUGFAIL("oops"); return;
    }

    rgb = Sk4f::Max(0, Sk4f::Min(rgb, 1));
    dst[0] = rgb[0];
    dst[1] = rgb[1];
    dst[2] = rgb[2];
}
