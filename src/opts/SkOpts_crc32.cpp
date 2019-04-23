/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS crc32
#include "src/opts/SkChecksum_opts.h"

namespace SkOpts {
    void Init_crc32() {
        hash_fn = crc32::hash_fn;
    }
}
