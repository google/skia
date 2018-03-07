/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse41
#include "SkBlitRow_opts.h"

namespace SkOpts {
    void Init_sse41() {
        blit_row_s32a_opaque = sse41::blit_row_s32a_opaque;
    }
}
