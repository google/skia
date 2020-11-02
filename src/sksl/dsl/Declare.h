/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_DECLARE
#define SKSL_DSL_DECLARE

#include "src/sksl/dsl/Var.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Statement.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace skslcode {

template<class T>
class Declare : Statement {
public:
    Declare(const Var<T>& var)
        : fVar(var) {}

    std::unique_ptr<SkSL::Statement> statement() const {
        const SkSL::Variable* var = fVar.variable();
        return std::make_unique<SkSL::VarDeclaration>(var, &var->type(), SkSL::ExpressionArray(),
                                                      nullptr);
    }

private:
    const Var<T>& fVar;
};

template<class T>
Declare(Var<T>) -> Declare<T>;

} // namespace skslcode

#endif
