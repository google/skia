/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sk_sse41
#include "SkBlurImageFilter_opts.h"

namespace SkOpts {
    void Init_sse41() {
        box_blur_xx = sk_sse41::box_blur_xx;
        box_blur_xy = sk_sse41::box_blur_xy;
        box_blur_yx = sk_sse41::box_blur_yx;
    }
}
