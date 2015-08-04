/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse41
#include "SkBlurImageFilter_opts.h"

namespace SkOpts {
    void Init_sse41() {
        static const auto x = sse41::kX, y = sse41::kY;
        box_blur_xx = sse41::box_blur<x,x>;
        box_blur_xy = sse41::box_blur<x,y>;
        box_blur_yx = sse41::box_blur<y,x>;
    }
}
