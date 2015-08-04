/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS neon
#include "SkBlurImageFilter_opts.h"
#include "SkFloatingPoint_opts.h"
#include "SkUtils_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_neon() {
        rsqrt           = neon::rsqrt;
        memset16        = neon::memset16;
        memset32        = neon::memset32;
        create_xfermode = SkCreate4pxXfermode;

        static const auto x = neon::kX, y = neon::kY;
        box_blur_xx = neon::box_blur<x,x>;
        box_blur_xy = neon::box_blur<x,y>;
        box_blur_yx = neon::box_blur<y,x>;
    }
}
