/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS skx
#include "src/opts/SkBlitRow_opts.h"
#include "src/opts/SkSwizzler_opts.h"
#include "src/opts/SkVM_opts.h"

namespace SkOpts {
    void Init_skx() {
        blit_row_s32a_opaque = SK_OPTS_NS::blit_row_s32a_opaque;
        interpret_skvm = SK_OPTS_NS::interpret_skvm;
        RGBA_to_BGRA          = SK_OPTS_NS::RGBA_to_BGRA;
        RGBA_to_rgbA          = SK_OPTS_NS::RGBA_to_rgbA;
        RGBA_to_bgrA          = SK_OPTS_NS::RGBA_to_bgrA;
        grayA_to_RGBA         = SK_OPTS_NS::grayA_to_RGBA;
        grayA_to_rgbA         = SK_OPTS_NS::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = SK_OPTS_NS::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = SK_OPTS_NS::inverted_CMYK_to_BGR1;
    }
}  // namespace SkOpts
