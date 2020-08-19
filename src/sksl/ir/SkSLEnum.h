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
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <vector>

namespace SkSL {

struct Symbol;

struct Enum : public ProgramElement {
    static constexpr Kind kProgramElementKind = kEnum_Kind;

    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols,
         bool isBuiltin = true)
    : INHERITED(offset, kProgramElementKind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols))
    , fBuiltin(isBuiltin) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Enum(fOffset, fTypeName, fSymbols, fBuiltin));
    }

    String code() const {
        String result = "enum class " + fTypeName + " {\n";
        String separator;
        std::vector<const Symbol*> sortedSymbols;
        for (const auto& pair : *fSymbols) {
            sortedSymbols.push_back(pair.second);
        }
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](const Symbol* a, const Symbol* b) { return a->fName < b->fName; });
        for (const auto& s : sortedSymbols) {
            const Expression& initialValue = *s->as<Variable>().fInitialValue;
            result += separator + "    " + s->fName + " = " +
                      to_string(initialValue.as<IntLiteral>().fValue);
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

    String description() const override {
        return this->code();
    }

    const StringFragment fTypeName;
    const std::shared_ptr<SymbolTable> fSymbols;
    bool fBuiltin;

    typedef ProgramElement INHERITED;
};

}  // namespace SkSL

#endif
