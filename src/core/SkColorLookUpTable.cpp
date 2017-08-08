/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorLookUpTable.h"
#include "SkColorSpaceXformPriv.h"
#include "SkFloatingPoint.h"

void SkColorLookUpTable::interp(float* dst, const float* src) const {
    if (fInputChannels == 3) {
        return this->interp3D(dst, src);
    }

    Sk4f rgb;
    switch (fInputChannels-1) {
        case 0: rgb = this->interpDimension<0>(src); break;
        case 1: rgb = this->interpDimension<1>(src); break;
        case 3: rgb = this->interpDimension<3>(src); break;
        default: SkDEBUGFAIL("oops");
    }

    rgb = Sk4f::Max(0, Sk4f::Min(rgb, 1));
    dst[0] = rgb[0];
    dst[1] = rgb[1];
    dst[2] = rgb[2];
}

template <int dim>
Sk4f SkColorLookUpTable::interpDimension(const float* src, int index, int stride) const {
    int limit = fLimits[dim];
    float x = src[dim] * (limit - 1);

    int lo = (int)(x          ),
        hi = (int)(x + 0.9999f);

    Sk4f L = this->interpDimension<dim-1>(src, stride*lo + index, stride*limit),
         H = this->interpDimension<dim-1>(src, stride*hi + index, stride*limit);

    float t = (x - lo);
    return (1 - t)*L + t*H;
}

template <>
Sk4f SkColorLookUpTable::interpDimension<-1>(const float* src, int index, int stride) const {
    return {
        this->table()[kOutputChannels*index+0],
        this->table()[kOutputChannels*index+1],
        this->table()[kOutputChannels*index+2],
        0.0f,
    };
}

void SkColorLookUpTable::interp3D(float* dst, const float* src) const {
    SkASSERT(3 == kOutputChannels);
    // Call the src components x, y, and z.
    const uint8_t maxX = fLimits[0] - 1;
    const uint8_t maxY = fLimits[1] - 1;
    const uint8_t maxZ = fLimits[2] - 1;

    // An approximate index into each of the three dimensions of the table.
    const float x = src[0] * maxX;
    const float y = src[1] * maxY;
    const float z = src[2] * maxZ;

    // This gives us the low index for our interpolation.
    int ix = sk_float_floor2int(x);
    int iy = sk_float_floor2int(y);
    int iz = sk_float_floor2int(z);

    // Make sure the low index is not also the max index.
    ix = (maxX == ix) ? ix - 1 : ix;
    iy = (maxY == iy) ? iy - 1 : iy;
    iz = (maxZ == iz) ? iz - 1 : iz;

    // Weighting factors for the interpolation.
    const float diffX = x - ix;
    const float diffY = y - iy;
    const float diffZ = z - iz;

    // Constants to help us navigate the 3D table.
    // Ex: Assume x = a, y = b, z = c.
    //     table[a * n001 + b * n010 + c * n100] logically equals table[a][b][c].
    const int n000 = 0;
    const int n001 = 3 * fLimits[1] * fLimits[2];
    const int n010 = 3 * fLimits[2];
    const int n011 = n001 + n010;
    const int n100 = 3;
    const int n101 = n100 + n001;
    const int n110 = n100 + n010;
    const int n111 = n110 + n001;

    // Base ptr into the table.
    const float* ptr = &(table()[ix*n001 + iy*n010 + iz*n100]);

    // The code below performs a tetrahedral interpolation for each of the three
    // dst components.  Once the tetrahedron containing the interpolation point is
    // identified, the interpolation is a weighted sum of grid values at the
    // vertices of the tetrahedron.  The claim is that tetrahedral interpolation
    // provides a more accurate color conversion.
    // blogs.mathworks.com/steve/2006/11/24/tetrahedral-interpolation-for-colorspace-conversion/
    //
    // I have one test image, and visually I can't tell the difference between
    // tetrahedral and trilinear interpolation.  In terms of computation, the
    // tetrahedral code requires more branches but less computation.  The
    // SampleICC library provides an option for the client to choose either
    // tetrahedral or trilinear.
    for (int i = 0; i < 3; i++) {
        if (diffZ < diffY) {
            if (diffZ > diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n110] - ptr[n010]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n011] - ptr[n001]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n011] - ptr[n010]));
            }
        } else {
            if (diffZ < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n101] - ptr[n001]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n101] - ptr[n100]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n110] - ptr[n100]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            }
        }

        // |src| is guaranteed to be in the 0-1 range as are all entries
        // in the table.  For "increasing" tables, outputs will also be
        // in the 0-1 range.  While this property is logical for color
        // look up tables, we don't check for it.
        // And for arbitrary, non-increasing tables, it is easy to see how
        // the output might not be 0-1.  So we clamp here.
        dst[i] = clamp_0_1(dst[i]);

        // Increment the table ptr in order to handle the next component.
        // Note that this is the how table is designed: all of nXXX
        // variables are multiples of 3 because there are 3 output
        // components.
        ptr++;
    }
}
