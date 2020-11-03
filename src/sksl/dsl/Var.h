/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/sksl/SkSLString.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/dsl/Expression.h"

namespace SkSL {

class Variable;

} // namespace SkSL

namespace skslcode {

class Type;

class Var {
public:
    Var(const char* name);

    Var(const SkSL::Type& type, const char* name = "var");

    Expression operator=(Expression&& expr);

    const SkSL::Variable* var() const;

private:
    SkSL::String fName;
    const SkSL::Variable* fVar;
};

} // namespace skslcode

#endif
