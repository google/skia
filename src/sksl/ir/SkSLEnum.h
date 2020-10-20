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

class Symbol;

class Enum : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kEnum;

    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols,
         bool isSharedWithCpp, bool isBuiltin = true)
    : INHERITED(offset, EnumData{typeName, std::move(symbols), isSharedWithCpp, isBuiltin}) {}

    StringFragment typeName() const {
        return this->enumData().fTypeName;
    }

    std::shared_ptr<SymbolTable> symbols() const {
        return this->enumData().fSymbols;
    }

    bool isBuiltin() const {
        return this->enumData().fIsBuiltin;
    }

    bool isSharedWithCpp() const {
        return this->enumData().fIsSharedWithCpp;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<Enum>(fOffset, this->typeName(),
                                      SymbolTable::WrapIfBuiltin(this->symbols()),
                                      this->isSharedWithCpp(), /*isBuiltin=*/false);
    }

    String code() const {
        String result = "enum class " + this->typeName() + " {\n";
        String separator;
        std::vector<const Symbol*> sortedSymbols;
        sortedSymbols.reserve(symbols()->count());
        this->symbols()->foreach([&](StringFragment, const Symbol* symbol) {
            sortedSymbols.push_back(symbol);
        });
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](const Symbol* a, const Symbol* b) { return EnumValue(a) < EnumValue(b); });
        for (const Symbol* s : sortedSymbols) {
            result += separator + "    " + s->name() + " = " + to_string(EnumValue(s));
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

    String description() const override {
        return this->code();
    }

private:
    static int EnumValue(const Symbol* symbol) {
        return symbol->as<Variable>().initialValue()->as<IntLiteral>().value();
    }

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
