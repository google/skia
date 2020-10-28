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

class Enum final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kEnum;

    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols,
         bool isSharedWithCpp, bool isBuiltin = true)
    : INHERITED(offset, kProgramElementKind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols))
    , fIsSharedWithCpp(isSharedWithCpp)
    , fIsBuiltin(isBuiltin) {}

    StringFragment typeName() const {
        return fTypeName;
    }

    std::shared_ptr<SymbolTable> symbols() const {
        return fSymbols;
    }

    bool isBuiltin() const {
        return fIsBuiltin;
    }

    bool isSharedWithCpp() const {
        return fIsSharedWithCpp;
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

    StringFragment fTypeName;
    std::shared_ptr<SymbolTable> fSymbols;
    bool fIsSharedWithCpp;
    bool fIsBuiltin;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
