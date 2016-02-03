/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScaleToSides_DEFINED
#define SkScaleToSides_DEFINED

#include <cmath>
#include "SkScalar.h"
#include "SkTypes.h"

class ScaleToSides {
public:
    // This code assumes that a and b fit in in a float, and therefore the resulting smaller value
    // of a and b will fit in a float. The side of the rectangle may be larger than a float.
    // Scale must be less than or equal to the ratio limit / (*a + *b).
    // This code assumes that NaN and Inf are never passed in.
    static void AdjustRadii(double limit, double scale, SkScalar* a, SkScalar* b) {
        SkASSERTF(scale < 1.0 && scale > 0.0, "scale: %g", scale);

        *a = (float)((double)*a * scale);
        *b = (float)((double)*b * scale);

        // This check is conservative. (double)*a + (double)*b >= (double)(*a + *b)
        if ((double)*a + (double)*b > limit) {
            float* minRadius = a;
            float* maxRadius = b;

            // Force minRadius to be the smaller of the two.
            if (*minRadius > *maxRadius) {
                SkTSwap(minRadius, maxRadius);
            }

            // newMinRadius must be float in order to give the actual value of the radius.
            // The newMinRadius will always be smaller than limit. The largest that minRadius can be
            // is 1/2 the ratio of minRadius : (minRadius + maxRadius), therefore in the resulting
            // division, minRadius can be no larger than 1/2 limit + ULP.
            float newMinRadius = *minRadius;

            // Because newMaxRadius is the result of a double to float conversion, it can be larger
            // than limit, but only by one ULP.
            float newMaxRadius = (float)(limit - newMinRadius);

            // If newMaxRadius forces the total over the limit, then it needs to be
            // reduced by one ULP to be less than limit - newMinRadius.
            // Note: nexttowardf is a c99 call and should be std::nexttoward, but this is not
            // implemented in the ARM compiler.
            if ((double)newMaxRadius + (double)newMinRadius > limit) {
                newMaxRadius = nexttowardf(newMaxRadius, 0.0);
            }
            *maxRadius = newMaxRadius;
        }

        SkASSERTF(*a >= 0.0f && *b >= 0.0f, "a: %g, b: %g", *a, *b);
        SkASSERTF((*a + *b) <= limit, "limit: %g, a: %g, b: %g", limit, *a, *b);
    }
};
#endif // ScaleToSides_DEFINED
