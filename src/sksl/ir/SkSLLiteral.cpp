/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLString.h"
#include "include/private/SkStringView.h"
#include "src/sksl/ir/SkSLLiteral.h"

#include <cstdlib>

namespace SkSL {

std::string Literal::description(OperatorPrecedence) const {
    if (this->type().isBoolean()) {
        return fValue ? "true" : "false";
    }
    if (this->type().isInteger()) {
        return std::to_string(this->intValue());
    }
    // Attempt to find a compact, but accurate, representation of this float value.
    std::string text;
    float value = this->floatValue();
    for (int width = 6; width <= 9; ++width) {
        text = SkSL::String::printf("%.*g", width, value);
        if (std::strtof(text.c_str(), nullptr) == value) {
            break;
        }
    }
    // %.9g should be enough precision to match any float input.
    // https://randomascii.wordpress.com/2013/02/07/float-precision-revisited-nine-digit-float-portability/
    SkASSERT(std::strtof(text.c_str(), nullptr) == value);

    // %g might not emit a decimal point, but we need one to distinguish floats from ints.
    if (!skstd::contains(text, '.') && !skstd::contains(text, 'e')) {
        text += ".0";
    }
    return text;
}

}  // namespace SkSL
