/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariable.h"

#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

const Expression* Variable::initialValue() const {
    return fDeclaration ? fDeclaration->value().get() : nullptr;
}

} // namespace SkSL
