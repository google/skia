/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_INT
#define SKSL_DSL_INT

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Type.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

class Expression;
class Statement;

} // namespace SkSL

namespace skslcode {

class Int : Type {
public:
    Int() = default;

    Int(int value)
        : fValue(value) {}

    const SkSL::Type& type() const {
        return *writer().context().fInt_Type;
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::IntLiteral>(writer().context(), /*offset=*/-1, fValue);
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    int fValue;
};

} // namespace skslcode

#endif
