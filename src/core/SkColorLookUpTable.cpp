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
        interp3D(dst, src);
    } else {
        SkASSERT(dst != src);
        // index gets initialized as the algorithm proceeds by interpDimension.
        // It's just there to store the choice of low/high so far.
        int index[kMaxColorChannels];
        for (uint8_t outputDimension = 0; outputDimension < kOutputChannels; ++outputDimension) {
            dst[outputDimension] = interpDimension(src, fInputChannels - 1, outputDimension,
                                                   index);
        }
    }
}

void SkColorLookUpTable::interp3D(float* dst, const float* src) const {
    SkASSERT(3 == kOutputChannels);
    // Call the src components x, y, and z.
    const uint8_t maxX = fGridPoints[0] - 1;
    const uint8_t maxY = fGridPoints[1] - 1;
    const uint8_t maxZ = fGridPoints[2] - 1;

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
    const int n001 = 3 * fGridPoints[1] * fGridPoints[2];
    const int n010 = 3 * fGridPoints[2];
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

float SkColorLookUpTable::interpDimension(const float* src, int inputDimension,
                                          int outputDimension,
                                          int index[kMaxColorChannels]) const {
    // Base case. We've already decided whether to use the low or high point for each dimension
    // which is stored inside of index[] where index[i] gives the point in the CLUT to use for
    // input dimension i.
    if (inputDimension < 0) {
        // compute index into CLUT and look up the colour
        int outputIndex = outputDimension;
        int indexMultiplier = kOutputChannels;
        for (int i = fInputChannels - 1; i >= 0; --i) {
            outputIndex += index[i] * indexMultiplier;
            indexMultiplier *= fGridPoints[i];
        }
        return table()[outputIndex];
    }
    // for each dimension (input channel), try both the low and high point for it
    // and then do the same recursively for the later dimensions.
    // Finally, we need to LERP the results. ie LERP X then LERP Y then LERP Z.
    const float x = src[inputDimension] * (fGridPoints[inputDimension] - 1);
    // try the low point for this dimension
    index[inputDimension] = sk_float_floor2int(x);
    const float diff = x - index[inputDimension];
    // and recursively LERP all sub-dimensions with the current dimension fixed to the low point
    const float lo = interpDimension(src, inputDimension - 1, outputDimension, index);
    // now try the high point for this dimension
    index[inputDimension] = sk_float_ceil2int(x);
    // and recursively LERP all sub-dimensions with the current dimension fixed to the high point
    const float hi = interpDimension(src, inputDimension - 1, outputDimension, index);
    // then LERP the results based on the current dimension
    return clamp_0_1((1 - diff) * lo + diff * hi);
}
