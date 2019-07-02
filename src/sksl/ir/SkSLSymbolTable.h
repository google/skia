/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include <unordered_map>
#include <memory>
#include <vector>
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLSymbol.h"

namespace SkSL {

struct FunctionDeclaration;

/**
 * Maps identifiers to symbols. Functions, in particular, are mapped to either FunctionDeclaration
 * or UnresolvedFunction depending on whether they are overloaded or not.
 */
class SymbolTable {
public:
    SymbolTable(IRGenerator* irGenerator)
    : fIRGenerator(irGenerator) {}

    SymbolTable(std::shared_ptr<SymbolTable> parent, IRGenerator* irGenerator)
    : fParent(parent)
    , fIRGenerator(irGenerator) {}

    IRNode::ID operator[](StringFragment name);

    void add(StringFragment name, IRNode::ID symbol);

    void markAllFunctionsBuiltin();

    std::unordered_map<StringFragment, IRNode::ID>::iterator begin();

    std::unordered_map<StringFragment, IRNode::ID>::iterator end();

    const std::shared_ptr<SymbolTable> fParent;

    IRGenerator* fIRGenerator;

private:
    std::vector<IRNode::ID> getFunctions(IRNode::ID id);

    std::unordered_map<StringFragment, IRNode::ID> fSymbols;
};

} // namespace

#endif
