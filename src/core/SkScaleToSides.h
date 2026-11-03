/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScaleToSides_DEFINED
#define SkScaleToSides_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"

#include <algorithm>
#include <cmath>
#include <utility>

class SkScaleToSides {
public:
    // This code assumes that a and b fit in a float, and therefore the resulting smaller value
    // of a and b will fit in a float. The side of the rectangle may be larger than a float.
    // Scale must be less than or equal to the ratio limit / (*a + *b).
    // This code assumes that NaN and Inf are never passed in.
    static void AdjustRadii(
            double scale, SkScalar* a, SkScalar* b, SkScalar minCoord, SkScalar maxCoord) {
        SkASSERTF(scale <= 1.0 && scale > 0.0, "scale: %g", scale);
        const double limit =
                std::min((double)maxCoord - (double)minCoord, (double)(maxCoord - minCoord));

        if (scale < 1.0) {
            *a = (float)((double)*a * scale);
            *b = (float)((double)*b * scale);
        }

        // If a corner radius is large enough that subtracting it from maxCoord underflows past
        // minCoord (or adding it to minCoord overflows past maxCoord) due to floating-point
        // precision differences between the two coordinates, step it down
        if (maxCoord - *b < minCoord) {
            *b = nextafterf(maxCoord - minCoord, 0.0f);
        }
        if (minCoord + *a > maxCoord) {
            *a = nextafterf(maxCoord - minCoord, 0.0f);
        }

        if (*a + *b > limit) {
            float* minRadius = a;
            float* maxRadius = b;

            // Force minRadius to be the smaller of the two.
            if (*minRadius > *maxRadius) {
                using std::swap;
                swap(minRadius, maxRadius);
            }

            // newMinRadius must be float in order to give the actual value of the radius.
            // The newMinRadius will always be smaller than limit. The largest that minRadius can be
            // is 1/2 the ratio of minRadius : (minRadius + maxRadius), therefore in the resulting
            // division, minRadius can be no larger than 1/2 limit + ULP.
            float newMinRadius = *minRadius;

            float newMaxRadius = (float)(limit - newMinRadius);

            // Reduce newMaxRadius an ulp at a time until it fits. This usually never happens,
            // but if it does it could be 1 or 2 times. In certain pathological cases it could be
            // more. Max iterations seen so far is 17.
            while (newMaxRadius + newMinRadius > limit && newMaxRadius > 0.0f) {
                newMaxRadius = nextafterf(newMaxRadius, 0.0f);
            }
            *maxRadius = newMaxRadius;
        }

        SkASSERTF(*a >= 0.0f && *b >= 0.0f, "a: %g, b: %g, limit: %g, scale: %g", *a, *b, limit,
                  scale);

        SkASSERTF(*a + *b <= limit,
                  "\nlimit: %.17f, sum: %.17f, a: %.10f, b: %.10f, scale: %.20f",
                  limit,
                  *a + *b,
                  *a,
                  *b,
                  scale);
    }
};
#endif // ScaleToSides_DEFINED
