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
class FunctionDeclaration;
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
        SkTArray<DSLExpression> argArray;
        argArray.reserve_back(sizeof...(args));
        this->collectArgs(argArray, std::forward<Args>(args)...);
        return this->call(std::move(argArray));
    }

private:
    void collectArgs(SkTArray<DSLExpression>& args) {}

    template<class... RemainingArgs>
    void collectArgs(SkTArray<DSLExpression>& args, DSLVar& var, RemainingArgs&&... remaining) {
        args.push_back(var);
        collectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    template<class... RemainingArgs>
    void collectArgs(SkTArray<DSLExpression>& args, DSLExpression expr,
                     RemainingArgs&&... remaining) {
        args.push_back(std::move(expr));
        collectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    void init(const DSLType& returnType, const char* name, std::vector<DSLVar*> params);

    DSLExpression call(SkTArray<DSLExpression> args);

    const SkSL::Type* fReturnType;
    const SkSL::FunctionDeclaration* fDecl;
};

} // namespace dsl

} // namespace SkSL

#endif
