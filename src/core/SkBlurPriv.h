/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurPriv_DEFINED
#define SkBlurPriv_DEFINED

#include "include/core/SkRRect.h"
#include "include/core/SkSize.h"

static const int kSkBlurRRectMaxDivisions = 6;

// This method computes all the parameters for drawing a partially occluded nine-patched
// blurred rrect mask:
//   rrectToDraw - the integerized rrect to draw in the mask
//   widthHeight - how large to make the mask (rrectToDraw will be centered in this coord sys)
//   rectXs, rectYs - the x & y coordinates of the covering geometry lattice
//   texXs, texYs - the texture coordinate at each point in rectXs & rectYs
// It returns true if 'devRRect' is nine-patchable
bool SkComputeBlurredRRectParams(const SkRRect& srcRRect, const SkRRect& devRRect,
                                 SkScalar sigma, SkScalar xformedSigma,
                                 SkRRect* rrectToDraw,
                                 SkISize* widthHeight,
                                 SkScalar rectXs[kSkBlurRRectMaxDivisions],
                                 SkScalar rectYs[kSkBlurRRectMaxDivisions],
                                 SkScalar texXs[kSkBlurRRectMaxDivisions],
                                 SkScalar texYs[kSkBlurRRectMaxDivisions]);

int SkCreateIntegralTable(float sixSigma, SkBitmap* table);

void SkFillIn1DGaussianKernel(float* kernel, float gaussianSigma, int radius);

extern void sk_register_blur_maskfilter_createproc();

#endif
