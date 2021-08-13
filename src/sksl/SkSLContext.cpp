/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"

#include "include/sksl/DSLCore.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

Context::Context(ErrorReporter& errors, const ShaderCapsClass& caps)
        : fCaps(caps)
        , fErrors(errors) {
    SkASSERT(!Pool::IsAttached());
}

ErrorReporter& Context::errors() const {
    if (dsl::DSLWriter::IsActive()) {
        return dsl::GetErrorReporter();
    }
    return fErrors;
}

}  // namespace SkSL

