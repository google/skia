/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_SHORT
#define SKSL_DSL_SHORT

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

class Short : Type {
public:
    using Type = Short;

    Short() = default;

    Short(int value)
        : fValue(value) {}

    const SkSL::Type& type() const {
        return *DSLWriter::Instance().context().fShort_Type;
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::IntLiteral>(/*offset=*/-1, fValue, &this->type());
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    int fValue;
};

} // namespace skslcode

#endif
