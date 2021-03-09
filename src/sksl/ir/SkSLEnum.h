/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ENUM
#define SKSL_ENUM

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLExpression.h"
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
        struct Enumerant { StringFragment name; SKSL_INT value; };
        std::vector<Enumerant> sortedSymbols;
        sortedSymbols.reserve(symbols()->count());
        this->foreach([&](StringFragment name, SKSL_INT value){
            sortedSymbols.push_back({name, value});
        });
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](const Enumerant& a, const Enumerant& b) { return a.value < b.value; });
        for (const auto& entry : sortedSymbols) {
            result += separator + "    " + entry.name + " = " + to_string(entry.value);
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

    String description() const override {
        return this->code();
    }

    template <typename Fn> void foreach(Fn&& fn) const {
        this->symbols()->foreach(
                [&fn](StringFragment name, const Symbol* symbol) { fn(name, EnumValue(symbol)); });
    }

private:
    static SKSL_INT EnumValue(const Symbol* symbol) {
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
