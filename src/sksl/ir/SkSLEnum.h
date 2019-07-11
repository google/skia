/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ENUM
#define SKSL_ENUM

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <algorithm>
#include <vector>

namespace SkSL {

struct Symbol;

struct Enum : public ProgramElement {
    Enum(IRGenerator* irGenerator, int offset, StringFragment typeName,
         std::shared_ptr<SymbolTable> symbols)
    : INHERITED(irGenerator, offset, kEnum_Kind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new Enum(fIRGenerator, fOffset, fTypeName, fSymbols));
    }

    String description() const override {
        String result = "enum class " + fTypeName + " {\n";
        String separator;
        std::vector<IRNode::ID> sortedSymbols;
        for (const auto& pair : *fSymbols) {
            sortedSymbols.push_back(pair.second);
        }
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](IRNode::ID a, IRNode::ID b) { return ((Symbol&) a.node()).fName <
                                                          ((Symbol&) b.node()).fName; });
        for (const auto& s : sortedSymbols) {
            Variable& v = (Variable&) s.node();
            result += separator + "    " + v.fName + " = " + v.fInitialValue.node().description();
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

    bool fBuiltin = false;
    const StringFragment fTypeName;
    const std::shared_ptr<SymbolTable> fSymbols;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
