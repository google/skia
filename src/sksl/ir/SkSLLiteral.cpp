/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLLiteral.h"

namespace SkSL {

std::string Literal::description(OperatorPrecedence) const {
    if (this->type().isBoolean()) {
        return fValue ? "true" : "false";
    }
    if (this->type().isInteger()) {
        return std::to_string(this->intValue());
    }
    return skstd::to_string(this->floatValue());
}

}  // namespace SkSL
