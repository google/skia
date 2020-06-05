/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS skx
#include "src/opts/SkBlitRow_opts.h"
#include "src/opts/SkVM_opts.h"

namespace SkOpts {
    void Init_skx() {
        blit_row_s32a_opaque = SK_OPTS_NS::blit_row_s32a_opaque;
        interpret_skvm = SK_OPTS_NS::interpret_skvm;
    }
}
