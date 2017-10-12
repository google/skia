/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageBlurFilter_DEFINED
#define SkImageBlurFilter_DEFINED

#include <tuple>
#include "SkPoint.h"
#include "SkRect.h"
#include "SkSpecialImage.h"

class SkImageBlurFilter {
public:
    SkImageBlurFilter(SkVector sigma);

    sk_sp<SkSpecialImage> blur(SkSpecialImage* source,
                               const sk_sp<SkSpecialImage>& input,
                               SkIRect inputBounds, SkIRect dstBounds) const;

    static SkRect  OutsetRect( const SkRect& src,  SkVector sigma);
    static SkIRect OutsetIRect(const SkIRect& src, SkVector sigma);

private:
    const SkVector      fSigma;
};


#endif  //SkImageBlurFilter_DEFINED
