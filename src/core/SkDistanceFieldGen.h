/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDistanceFieldGen_DEFINED
#define SkDistanceFieldGen_DEFINED

/** Given 8-bit mask data, generate the associated distance field

 *  @param distanceField     The distance field to be generated. Should already be allocated
 *                           by the client with the padding below.
 *  @param image             8-bit mask we're using to generate the distance field.
 *  @param w                 Width of the image.
 *  @param h                 Height of the image.
 *  @param distanceMagnitude Largest possible absolute value for the distance. The distance field
 *                           will be padded to w + 2*distanceMagnitude, h + 2*distanceMagnitude.
 */
bool SkGenerateDistanceFieldFromImage(unsigned char* distanceField,
                                      const unsigned char* image,
                                      int w, int h,
                                      int distanceMagnitude);

#endif
