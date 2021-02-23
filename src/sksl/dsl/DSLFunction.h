/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/dsl/DSLBlock.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"

namespace SkSL {

class Block;
class Variable;

namespace dsl {

class DSLType;

class DSLFunction {
public:
    template<class... Parameters>
    DSLFunction(const DSLType& returnType, const char* name, Parameters&... parameters)
        : fReturnType(&returnType.skslType()) {
        std::vector<const Variable*> parameterArray;
        parameterArray.reserve(sizeof...(parameters));
        (parameterArray.push_back(&DSLWriter::Var(parameters)), ...);
        SkSL::SymbolTable& symbols = *DSLWriter::SymbolTable();
        fDecl = symbols.add(std::make_unique<SkSL::FunctionDeclaration>(
                                                 /*offset=*/-1,
                                                 DSLWriter::Modifiers(SkSL::Modifiers()),
                                                 DSLWriter::Name(name),
                                                 std::move(parameterArray), fReturnType,
                                                 /*builtin=*/false));
    }

    virtual ~DSLFunction() = default;

    template<class... Stmt>
    void define(Stmt... stmts) {
        DSLBlock block = DSLBlock(DSLStatement(std::move(stmts))...);
        this->define(std::move(block));
    }

    void define(DSLBlock block);

    template<class... Args>
    DSLExpression operator()(Args... args) {
        ExpressionArray argArray;
        argArray.reserve_back(sizeof...(args));
        int unused[] = {0, (DSLWriter::Ignore(argArray.push_back(args.release())), 0)...};
        static_cast<void>(unused);
        return DSLWriter::IRGenerator().call(/*offset=*/-1, *fDecl, std::move(argArray));
    }

protected:
    const SkSL::Type* fReturnType;
    const SkSL::FunctionDeclaration* fDecl;
};

} // namespace dsl

} // namespace SkSL

#endif
