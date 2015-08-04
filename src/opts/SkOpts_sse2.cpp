/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse2
#include "SkBlurImageFilter_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkUtils_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_sse2() {
        memset16        = sse2::memset16;
        memset32        = sse2::memset32;
        create_xfermode = SkCreate4pxXfermode;

        box_blur_xx = sse2::box_blur_xx;
        box_blur_xy = sse2::box_blur_xy;
        box_blur_yx = sse2::box_blur_yx;

        dilate_x = sse2::dilate_x;
        dilate_y = sse2::dilate_y;
         erode_x = sse2::erode_x;
         erode_y = sse2::erode_y;
    }
}
