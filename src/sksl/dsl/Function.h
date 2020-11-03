/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class Block;

} // namespace SkSL

namespace skslcode {

class Type;

template<class... Stmts>
std::unique_ptr<SkSL::Statement> Block(Stmts... stmts);

template<class... ArgType>
class Function {
public:
    Function(const SkSL::Type& returnType, const char* name)
        : fName(DSLWriter::Instance().name(name))
        , fReturnType(returnType) {}

    template<class... Stmt>
    void define(Stmt... stmts) {
        printf("%s\n", Block(std::move(stmts)...)->description().c_str());
    }

protected:
    void addParameters(std::vector<const SkSL::Variable*>& parameters) {
    }

private:
    SkSL::String fName;
    const SkSL::Type& fReturnType;
};

template<class First, class... Rest>
class Function<First, Rest...> : Function<Rest...> {
public:
    Function(const SkSL::Type& returnType, const char* name, First first, Rest... rest)
        : Function<Rest...>(returnType, name, rest...)
        , fFirstName(DSLWriter::Instance().name(name))
        , fFirst(std::make_unique<SkSL::Variable>(
                                                 /*offset=*/-1,
                                                 DSLWriter::Instance().modifiers(SkSL::Modifiers()),
                                                 SkSL::StringFragment(fFirstName.c_str()),
                                                 &first,
                                                 /*builtin=*/false,
                                                 SkSL::Variable::Storage::kLocal)){}

protected:
    void addParameters(std::vector<const SkSL::Variable*>& parameters) {
        parameters.push_back(fFirst);
        Function<Rest...>::addParameters(parameters);
    }

private:
    SkSL::String fFirstName;
    const SkSL::Variable* fFirst;
};

template<class First, class... Rest>
Function(const Type&, const char*, First, Rest...) -> Function<First, Rest...>;

} // namespace skslcode

#endif
