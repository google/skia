/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShadowParams_DEFINED
#define SkShadowParams_DEFINED

#include "SkScalar.h"

/** \struct SkShadowParams

    This struct holds information needed for drawing shadows.

    fShadowRadius - radius of the shadow blur

    fBiasingConstant - A constant used in variance shadow mapping to directly
    0.0 - 1.0          reduce light bleeding. Essentially sets all shadows
    ~.25               below a certain brightness equal to no light, and does
                       a linear step on the rest. Essentially makes shadows
                       darker and more rounded at higher values.

    fMinVariance - Too low of a variance (near the outer edges of blurry
    ~512, 1024     shadows) will lead to ugly sharp shadow brightness
                   distortions. This enforces a minimum amount of variance
                   in the calculation to smooth out the outside edges of
                   blurry shadows. However, too high of a value for this will
                   cause all shadows to be lighter by visibly different
                   amounts varying on depth.

    fType - Decides which algorithm to use to draw shadows.
*/
struct SkShadowParams {
    SkScalar fShadowRadius;
    SkScalar fBiasingConstant;
    SkScalar fMinVariance;

    enum ShadowType {
        kNoBlur_ShadowType,
        kVariance_ShadowType,

        kLast_ShadowType = kVariance_ShadowType
    };
    static const int kShadowTypeCount = kLast_ShadowType + 1;

    ShadowType fType;
};

#endif
