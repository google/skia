/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkFloatingPoint.h"

namespace SkOpts {
    void Init_neon() {
        rsqrt = sk_float_rsqrt;  // This copy of sk_float_rsqrt will take the NEON path.

    }
}
