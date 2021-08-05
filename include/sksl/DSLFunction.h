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
#include "include/sksl/DSLWrapper.h"

namespace SkSL {

class Block;
class FunctionDeclaration;
class Variable;

namespace dsl {

class DSLType;

class DSLFunction {
public:
    template<class... Parameters>
    DSLFunction(const DSLType& returnType, skstd::string_view name, Parameters&... parameters)
        : DSLFunction(DSLModifiers(), returnType, name, parameters...) {}

    template<class... Parameters>
    DSLFunction(const DSLModifiers& modifiers, const DSLType& returnType, skstd::string_view name,
                Parameters&... parameters) {
        SkTArray<DSLParameter*> parameterArray;
        parameterArray.reserve_back(sizeof...(parameters));

        // in C++17, we could just do:
        // (parameterArray.push_back(&parameters), ...);
        int unused[] = {0, (static_cast<void>(parameterArray.push_back(&parameters)), 0)...};
        static_cast<void>(unused);
        this->init(modifiers, returnType, name, std::move(parameterArray));
    }

    DSLFunction(const DSLType& returnType, skstd::string_view name,
                SkTArray<DSLParameter*> parameters) {
        this->init(DSLModifiers(), returnType, name, std::move(parameters));
    }

    DSLFunction(const DSLModifiers& modifiers, const DSLType& returnType, skstd::string_view name,
                SkTArray<DSLParameter*> parameters) {
        this->init(modifiers, returnType, name, std::move(parameters));
    }

    DSLFunction(const SkSL::FunctionDeclaration* decl)
        : fDecl(decl) {}

    virtual ~DSLFunction() = default;

    template<class... Stmt>
    void define(Stmt... stmts) {
        DSLBlock block = DSLBlock(DSLStatement(std::move(stmts))...);
        this->define(std::move(block));
    }

    void define(DSLBlock block);

    /**
     * Invokes the function with the given arguments.
     */
    template<class... Args>
    DSLExpression operator()(Args&&... args) {
        SkTArray<DSLWrapper<DSLExpression>> argArray;
        argArray.reserve_back(sizeof...(args));
        this->collectArgs(argArray, std::forward<Args>(args)...);
        return this->call(std::move(argArray));
    }

    /**
     * Invokes the function with the given arguments.
     */
    DSLExpression call(SkTArray<DSLWrapper<DSLExpression>> args);

private:
    void collectArgs(SkTArray<DSLWrapper<DSLExpression>>& args) {}

    template<class... RemainingArgs>
    void collectArgs(SkTArray<DSLWrapper<DSLExpression>>& args, DSLVar& var,
                     RemainingArgs&&... remaining) {
        args.push_back(DSLWrapper<DSLExpression>(var));
        collectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    template<class... RemainingArgs>
    void collectArgs(SkTArray<DSLWrapper<DSLExpression>>& args, DSLExpression expr,
                     RemainingArgs&&... remaining) {
        args.push_back(DSLWrapper<DSLExpression>(std::move(expr)));
        collectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    void init(DSLModifiers modifiers, const DSLType& returnType, skstd::string_view name,
              SkTArray<DSLParameter*> params);

    const SkSL::FunctionDeclaration* fDecl = nullptr;
};

} // namespace dsl

} // namespace SkSL

#endif
