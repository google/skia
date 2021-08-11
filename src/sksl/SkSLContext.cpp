/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"

namespace SkSL {

Context::Context(ErrorReporter& errors, const ShaderCapsClass& caps)
        : fCaps(caps)
        , fErrors(errors) {
    SkASSERT(!Pool::IsAttached());
}

}  // namespace SkSL

