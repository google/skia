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

class Enum : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kEnum;

    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols,
         bool isBuiltin = true)
    : INHERITED(offset, EnumData{typeName, std::move(symbols), isBuiltin}) {}

    StringFragment typeName() const {
        return this->enumData().fTypeName;
    }

    std::shared_ptr<SymbolTable> symbols() const {
        return this->enumData().fSymbols;
    }

    bool isBuiltin() const {
        return this->enumData().fIsBuiltin;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Enum(fOffset, this->typeName(), this->symbols(),
                                                        this->isBuiltin()));
    }

    String code() const {
        String result = "enum class " + this->typeName() + " {\n";
        String separator;
        std::vector<const Symbol*> sortedSymbols;
        for (const auto& pair : *this->symbols()) {
            sortedSymbols.push_back(pair.second);
        }
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](const Symbol* a, const Symbol* b) { return a->fName < b->fName; });
        for (const auto& s : sortedSymbols) {
            const Expression& initialValue = *s->as<Variable>().fInitialValue;
            result += separator + "    " + s->fName + " = " +
                      to_string(initialValue.as<IntLiteral>().value());
            separator = ",\n";
        }
        result += "\n};";
        return result;
    }

    String description() const override {
        return this->code();
    }

private:
    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
