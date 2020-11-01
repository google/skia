/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BOOL
#define SKSL_DSL_BOOL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Type.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"

namespace SkSL {

class Expression;
class Statement;

} // namespace SkSL

namespace skslcode {

class Bool : Type {
public:
    Bool() = default;

    Bool(bool value)
        : fValue(value) {}

    const SkSL::Type& type() const {
        return *writer().context().fBool_Type;
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::BoolLiteral>(writer().context(), /*offset=*/-1, fValue);
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    bool fValue;
};

} // namespace skslcode

#endif
