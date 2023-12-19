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

Context::Context(const BuiltinTypes& types, ErrorReporter& errors)
        : fTypes(types)
        , fErrors(&errors) {
    SkASSERT(!Pool::IsAttached());
}

Context::~Context() {
    SkASSERT(!Pool::IsAttached());
}

}  // namespace SkSL

