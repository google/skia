/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "src/sksl/SkSLString.h"
#include "src/sksl/dsl/DSLBlock.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"

namespace SkSL {

class Block;
class Variable;

namespace dsl {

class DSLType;

template<class... Arge>
class DSLFunction {
public:
    DSLFunction(const SkSL::Type& returnType, const char* name)
        : fName(DSLWriter::Instance().name(name))
        , fReturnType(returnType) {
        this->declare();
    }

    virtual ~DSLFunction() = default;

    template<class... Stmt>
    void define(Stmt... stmts) {
        DSLBlock block = DSLBlock(std::move(stmts)...);
        this->define(std::move(block.release()->as<SkSL::Block>().children()));
    }

    void define(DSLBlock block) {
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
class DSLFunction<First, Rest...> : public DSLFunction<Rest...> {
public:
    DSLFunction(const SkSL::Type& returnType, const char* name, First first, Rest... rest)
        : DSLFunction<Rest...>(returnType, name, rest...)
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
        DSLFunction<Rest...>::addParameters(parameters);
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
DSLFunction(const Type&, const char*, First, Rest...) -> DSLFunction<First, Rest...>;

} // namespace dsl

} // namespace SkSL

#endif
