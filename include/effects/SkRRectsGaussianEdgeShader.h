/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRRectsGaussianEdgeShader_DEFINED
#define SkRRectsGaussianEdgeShader_DEFINED

#include "SkShader.h"

class SkRRect;

class SK_API SkRRectsGaussianEdgeShader {
public:
    /** Returns a shader that applies a Gaussian blur depending on distance to the edge
     *  of the intersection of two round rects.
     *  Currently this is only useable with round rects that have the same radii at
     *  all the corners and for which the x & y radii are equal.
     *  Raster will draw nothing.
     *  
     *  The coverage geometry that should be drawn should be no larger than the intersection
     *  of the bounding boxes of the two round rects. Ambitious users can omit the center
     *  area of the coverage geometry if it is known to be occluded.
     */
    static sk_sp<SkShader> Make(const SkRRect& first,
                                const SkRRect& second,
                                SkScalar radius, SkScalar unused = 0.0f);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkRRectsGaussianEdgeShader(); // can't be instantiated
};

#endif
