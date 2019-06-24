/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace SkSL {

std::unique_ptr<Expression> Program::Settings::Value::literal(const Context& context,
                                                              int offset) const {
    switch (fKind) {
        case Program::Settings::Value::kBool_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(context,
                                                               offset,
                                                               fValue));
        case Program::Settings::Value::kInt_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(context,
                                                              offset,
                                                              fValue));
        case Program::Settings::Value::kFloat_Kind:
            return std::unique_ptr<Expression>(new FloatLiteral(context,
                                                              offset,
                                                              fValue));
        default:
            SkASSERT(false);
            return nullptr;
    }
}

} // namespace
