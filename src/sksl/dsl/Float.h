/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FLOAT
#define SKSL_DSL_FLOAT

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Type.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"

namespace SkSL {

class Expression;
class Statement;

} // namespace SkSL

namespace skslcode {

class Float : Type {
public:
    using Type = Float;

    Float() = default;

    Float(float value)
        : fValue(value) {}

    const SkSL::Type& type() const {
        return *DSLWriter::Instance().context().fFloat_Type;
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::FloatLiteral>(/*offset=*/-1, fValue, &this->type());
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    float fValue;
};

} // namespace skslcode

#endif
