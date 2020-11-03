/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {
    class Compiler;
    class Context;
    class IRGenerator;
    class SymbolTable;
    class Type;
} // namespace sksl

namespace skslcode {

class DSLWriter {
public:
    SkSL::Compiler& compiler();

    SkSL::IRGenerator& irGenerator();

    const SkSL::Context& context();

    const std::shared_ptr<SkSL::SymbolTable>& symbolTable();

    SkSL::ModifiersPool::Handle modifiers(SkSL::Modifiers modifiers);

    SkSL::String name(const char* name);

    static DSLWriter& Instance();

private:
    DSLWriter(SkSL::Compiler* compiler)
        : fCompiler(*compiler) {
        this->irGenerator().fSettings = &fSettings;
    }

    SkSL::Program::Settings fSettings;
    SkSL::Compiler& fCompiler;
    int fNameCount = 0;
};

} // namespace skslcode

#endif
