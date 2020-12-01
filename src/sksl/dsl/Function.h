/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "src/sksl/dsl/Block.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class Block;

namespace dsl {

class Type;

template<class... ArgType>
class Function {
public:
    Function(const SkSL::Type& returnType, const char* name)
        : fName(DSLWriter::Instance().name(name))
        , fReturnType(returnType) {
        this->declare();
    }

    virtual ~Function() = default;

    template<class... Stmt>
    void define(Stmt... stmts) {
        this->define(std::move(Block(std::move(stmts)...).release()->as<SkSL::Block>().children()));
    }

    void define(Block block) {
        DSLWriter::Instance().programElements().emplace_back(
                                              new SkSL::FunctionDefinition(/*offset=*/-1,
                                                                           fDecl,
                                                                           /*builtin=*/false,
                                                                           block.release()));
    }

protected:
    void addParameters(std::vector<const SkSL::Variable*>& parameters) {
    }

    virtual void declare() {
        SkSL::SymbolTable& symbols = *DSLWriter::Instance().symbolTable();
        fDecl = symbols.add(std::make_unique<SkSL::FunctionDeclaration>(
                                                 /*offset=*/-1,
                                                 DSLWriter::Instance().modifiers(SkSL::Modifiers()),
                                                 SkSL::StringFragment(fName.c_str()),
                                                 std::vector<const SkSL::Variable*>{}, &fReturnType,
                                                 /*builtin=*/false));
    }

    SkSL::String fName;
    const SkSL::Type& fReturnType;
    const SkSL::FunctionDeclaration* fDecl;
};

template<class First, class... Rest>
class Function<First, Rest...> : public Function<Rest...> {
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

    void declare() override {
        std::vector<const SkSL::Variable*> parameters;
        this->addParameters(parameters);
        SkSL::SymbolTable& symbols = *DSLWriter::Instance().symbolTable();
        this->fDecl = symbols.add(std::make_unique<SkSL::FunctionDeclaration>(
                                                 /*offset=*/-1,
                                                 DSLWriter::Instance().modifiers(SkSL::Modifiers()),
                                                 StringFragment(this->fName.c_str()),
                                                 std::move(parameters), &this->fReturnType,
                                                 /*builtin=*/false));
    }

private:
    SkSL::String fFirstName;
    const SkSL::Variable* fFirst;
};

template<class First, class... Rest>
Function(const Type&, const char*, First, Rest...) -> Function<First, Rest...>;

} // namespace dsl

} // namespace SkSL

#endif
