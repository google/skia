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
        box_blur_xx = sse41::box_blur_xx;
        box_blur_xy = sse41::box_blur_xy;
        box_blur_yx = sse41::box_blur_yx;
    }
}
