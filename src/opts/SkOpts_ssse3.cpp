/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#define SK_OPTS_NS sk_ssse3
#include "SkBlitMask_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_ssse3() {
        create_xfermode = sk_ssse3::create_xfermode;
        blit_mask_d32_a8 = sk_ssse3::blit_mask_d32_a8;
        color_cube_filter_span = sk_ssse3::color_cube_filter_span;
    }
}
