/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/dsl/DSLExpression.h"

namespace SkSL {

class Variable;

namespace dsl {

class DSLType;

class DSLVar {
public:
    DSLVar(const char* name);

    DSLVar(DSLType type, const char* name = "var");

    DSLVar(DSLVar&&) = delete;

private:
    const SkSL::Variable* var() const;

    const SkSL::String& name() const {
        return fName;
    }

    // this object owns the var until it is added to a symboltable
    std::unique_ptr<SkSL::Variable> fOwnedVar;
    const SkSL::Variable* fVar = nullptr;
    SkSL::String fName;

    friend class DSLExpression;
};

} // namespace dsl

} // namespace SkSL


#endif
