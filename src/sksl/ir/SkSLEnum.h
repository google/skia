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
    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, kEnum_Kind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols)) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Enum(fOffset, fTypeName, fSymbols));
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
            const Expression& initialValue = *((Variable*) s)->fInitialValue;
            SkASSERT(initialValue.fKind == Expression::kIntLiteral_Kind);
            result += separator + "    " + s->fName + " = " +
                      to_string(((IntLiteral&) initialValue).fValue);
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

#ifdef SK_DEBUG
    String description() const override {
        return this->code();
    }
#endif

    bool fBuiltin = false;
    const StringFragment fTypeName;
    const std::shared_ptr<SymbolTable> fSymbols;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
