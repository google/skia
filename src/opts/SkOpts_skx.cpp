/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#define SK_OPTS_NS skx
#include "src/opts/SkVM_opts.h"

namespace SkOpts {
    void Init_skx() {
        interpret_skvm = SK_OPTS_NS::interpret_skvm;
    }
}  // namespace SkOpts

#endif // SK_ENABLE_OPTIMIZE_SIZE
