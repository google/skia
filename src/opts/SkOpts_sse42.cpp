/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS sse42
#include "src/opts/SkChecksum_opts.h"

namespace SkOpts {
    void Init_sse42() {
        hash_fn = sse42::hash_fn;
    }
}

