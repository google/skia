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

// For a 3D lookup table, we'll use tetrahedral interpolation to halve the number of samples.
template <>
Sk4f interp_dimension<2>(const float* table, const uint8_t* limits,
                         const float* src, int index, int stride) {
    // Let's call the src components x, y, and z.
    int limitX = limits[0],
        limitY = limits[1],
        limitZ = limits[2];

    // We'd logically like to sample at (x,y,z).
    float x = src[0] * (limitX - 1),
          y = src[1] * (limitY - 1),
          z = src[2] * (limitZ - 1);

    // Snap to a nearby integer.
    int loX = (int)x,
        loY = (int)y,
        loZ = (int)z;

    // It's important that we're not right up at the limits, so tick down if we are.  (TODO: why?)
    if (loX == limitX - 1) { loX = limitX - 2; }
    if (loY == limitY - 1) { loY = limitY - 2; }
    if (loZ == limitZ - 1) { loZ = limitZ - 2; }

    // We'll interpolate each dimension by its t.
    float tX = x - loX,
          tY = y - loY,
          tZ = z - loZ;

    // These constants help us navigate the 3D table.
    // table[x*n001 + y*n010 + z*n100] == table[x][y][z]
    int n000 = 0,
        n001 = limitY * limitZ,
        n010 = limitZ,
        n011 = n001 + n010,
        n100 = 1,
        n101 = n100 + n001,
        n110 = n100 + n010,
        n111 = n100 + n010 + n001;

    // We'll load everything below relative to this base pointer to the lower corner.
    auto load = [&](int index) {
        const float* base = table + (loX*n001 + loY*n010 + loZ*n100);
        return Sk4f{
            base[3*index+0],
            base[3*index+1],
            base[3*index+2],
            0.0f,
        };
    };

    // Now we'll do the tetrahedral interpolation, which should get us similar
    // quality to trilinear interpolation from half the samples.
    //
    //   1) Logically split the 3D table into 6 tetrahedra that all share the x=y=z axis.
    //   2) Figure out which of those 6 contains our sample.
    //   3) Interpolate between that tetrahedron's four corners.
    //
    // blogs.mathworks.com/steve/2006/11/24/tetrahedral-interpolation-for-colorspace-conversion
    // is vaguely useful, but it'd be nice to get a better link here.

    if (tZ < tY) {
        if (tZ > tX) {
            return load(n000) + tZ * (load(n110) - load(n010))
                              + tY * (load(n010) - load(n000))
                              + tX * (load(n111) - load(n110));
        } else if (tY < tX) {
            return load(n000) + tZ * (load(n111) - load(n011))
                              + tY * (load(n011) - load(n001))
                              + tX * (load(n001) - load(n000));
        } else {
            return load(n000) + tZ * (load(n111) - load(n011))
                              + tY * (load(n010) - load(n000))
                              + tX * (load(n011) - load(n010));
        }
    } else {
        if (tZ < tX) {
            return load(n000) + tZ * (load(n101) - load(n001))
                              + tY * (load(n111) - load(n101))
                              + tX * (load(n001) - load(n000));
        } else if (tY < tX) {
            return load(n000) + tZ * (load(n100) - load(n000))
                              + tY * (load(n111) - load(n101))
                              + tX * (load(n101) - load(n100));
        } else {
            return load(n000) + tZ * (load(n100) - load(n000))
                              + tY * (load(n110) - load(n100))
                              + tX * (load(n111) - load(n110));
        }
    }
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
