/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "include/sksl/DSLBlock.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/DSLVar.h"

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
    DSLExpression operator()(Args&&... args) {
        SkASSERT(fArgs.empty());
        this->collectArgs(std::forward<Args>(args)...);
        return this->call(std::move(fArgs));
    }

private:
    void collectArgs() {}

    template<class... RemainingArgs>
    void collectArgs(DSLVar& var, RemainingArgs&&... remaining) {
        fArgs.push_back(var);
        collectArgs(std::forward<RemainingArgs>(remaining)...);
    }

    template<class... RemainingArgs>
    void collectArgs(DSLExpression expr, RemainingArgs&&... remaining) {
        fArgs.push_back(std::move(expr));
        collectArgs(std::forward<RemainingArgs>(remaining)...);
    }

    void init(const DSLType& returnType, const char* name, std::vector<DSLVar*> params);

    DSLExpression call(SkTArray<DSLExpression> args);

    mutable SkTArray<DSLExpression> fArgs;

    const SkSL::Type* fReturnType;
    const /* SkSL::FunctionDeclaration* */void* fDecl;
};

} // namespace dsl

} // namespace SkSL

#endif
