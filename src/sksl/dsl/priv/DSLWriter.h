/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWriter
#define SKSL_DSLWriter

#include "src/sksl/SkSLModifiersPool.h"

#include <unordered_map>

namespace SkSL {

class Context;
class IRGenerator;
class SymbolTable;
class Type;
class Variable;

} // namespace SkSL

namespace skslcode {

template<class T> class Var;

class Value;

class DSLWriter {
public:
    DSLWriter(SkSL::IRGenerator* irGenerator);

    DSLWriter(const DSLWriter&) = delete;

    ~DSLWriter();

    const SkSL::Context& context();

    const std::shared_ptr<SkSL::SymbolTable> symbolTable();

    SkSL::ModifiersPool::Handle modifiers(SkSL::Modifiers modifiers);

    int declare(std::unique_ptr<SkSL::Variable> var);

    const SkSL::Variable* var(int index) {
        return fVariables[index];
    }

private:
    SkSL::IRGenerator& fIRGenerator;
    std::vector<const SkSL::Variable*> fVariables;
};

DSLWriter& writer();

} // namespace skslcode

#endif
