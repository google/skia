/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"

#include "include/core/SkTypes.h"
#ifdef SK_DEBUG
#include "src/sksl/SkSLPool.h"
#endif

namespace SkSL {

Context::Context(ErrorReporter& errors, const ShaderCaps* caps, Mangler& mangler)
        : fCaps(caps)
        , fErrors(&errors)
        , fMangler(&mangler) {
    SkASSERT(!Pool::IsAttached());
}

Context::~Context() {
    SkASSERT(!Pool::IsAttached());
}

}  // namespace SkSL

