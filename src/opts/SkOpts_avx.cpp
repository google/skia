/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#define SK_OPTS_NS sk_avx
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_avx() {
        create_xfermode = sk_avx::create_xfermode;
    }
}

