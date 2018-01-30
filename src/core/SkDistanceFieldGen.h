/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDistanceFieldGen_DEFINED
#define SkDistanceFieldGen_DEFINED

#include "SkTypes.h"

// the max magnitude for the distance field
// distance values are limited to the range (-SK_DistanceFieldMagnitude, SK_DistanceFieldMagnitude]
#define SK_DistanceFieldMagnitude   4
// we need to pad around the original glyph to allow our maximum distance of
// SK_DistanceFieldMagnitude texels away from any edge
#define SK_DistanceFieldPad         4
// the rect we render with is inset from the distance field glyph size to allow for bilerp
#define SK_DistanceFieldInset       2

// For the fragment shader:
//   The distance field is constructed as unsigned char values,
//   so that the zero value is at 128, and the supported range of distances is [-4 * 127/128, 4].
//   Hence our multiplier (width of the range) is 4 * 255/128 and zero threshold is 128/255.
#define SK_DistanceFieldMultiplier   "7.96875"
#define SK_DistanceFieldThreshold    "0.50196078431"

/** Given 8-bit mask data, generate the associated distance field

 *  @param distanceField     The distance field to be generated. Should already be allocated
 *                           by the client with the padding above.
 *  @param image             8-bit mask we're using to generate the distance field.
 *  @param w                 Width of the original image.
 *  @param h                 Height of the original image.
 *  @param rowBytes          Size of each row in the image, in bytes
 */
bool SkGenerateDistanceFieldFromA8Image(unsigned char* distanceField,
                                        const unsigned char* image,
                                        int w, int h, size_t rowBytes);

/** Given 1-bit mask data, generate the associated distance field

 *  @param distanceField     The distance field to be generated. Should already be allocated
 *                           by the client with the padding above.
 *  @param image             1-bit mask we're using to generate the distance field.
 *  @param w                 Width of the original image.
 *  @param h                 Height of the original image.
 *  @param rowBytes          Size of each row in the image, in bytes
 */
bool SkGenerateDistanceFieldFromBWImage(unsigned char* distanceField,
                                        const unsigned char* image,
                                        int w, int h, size_t rowBytes);

/** Given width and height of original image, return size (in bytes) of distance field
 *  @param w                 Width of the original image.
 *  @param h                 Height of the original image.
 */
inline size_t SkComputeDistanceFieldSize(int w, int h) {
    return (w + 2*SK_DistanceFieldPad) * (h + 2*SK_DistanceFieldPad) * sizeof(unsigned char);
}

#endif
