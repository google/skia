/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskBlurFilter_DEFINED
#define SkMaskBlurFilter_DEFINED

#include <algorithm>
#include <memory>
#include <tuple>

#include "include/core/SkTypes.h"
#include "src/core/SkMask.h"

// Implement a single channel Gaussian blur. The specifics for implementation are taken from:
// https://drafts.fxtf.org/filters/#feGaussianBlurElement
class SkMaskBlurFilter {
public:
    // Create an object suitable for filtering an SkMask using a filter with width sigmaW and
    // height sigmaH.
    SkMaskBlurFilter(double sigmaW, double sigmaH);

    // returns true iff the sigmas will result in an identity mask (no blurring)
    bool hasNoBlur() const;

    // Given a src SkMask, generate dst SkMask returning the border width and height.
    SkIPoint blur(const SkMask& src, SkMask* dst) const;

private:
    const double fSigmaW;
    const double fSigmaH;
};

#endif  // SkBlurMaskFilter_DEFINED
