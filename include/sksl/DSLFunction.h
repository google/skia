/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "include/sksl/DSLBlock.h"
#include "include/sksl/DSLType.h"

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
        std::vector<DSLVar*> parameterArray;
        parameterArray.reserve(sizeof...(parameters));

        // in C++17, we could just do:
        // (parameterArray.push_back(&parameters), ...);
        int unused[] = {0, (static_cast<void>(parameterArray.push_back(&parameters)), 0)...};
        static_cast<void>(unused);
        this->init(returnType, name, std::move(parameterArray));
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
        int unused[] = {0, (static_cast<void>(argArray.push_back(args.release())), 0)...};
        static_cast<void>(unused);
        return this->call(std::move(argArray));
    }

private:
    void init(const DSLType& returnType, const char* name, std::vector<DSLVar*> params);

    DSLExpression call(ExpressionArray args);

    const SkSL::Type* fReturnType;
    const /* SkSL::FunctionDeclaration* */void* fDecl;
};

} // namespace dsl

} // namespace SkSL

#endif
