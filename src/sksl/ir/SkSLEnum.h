/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ENUM
#define SKSL_ENUM

#include "src/sksl/ir/SkSLProgramElement.h"

#include <algorithm>

namespace SkSL {

struct Enum : public ProgramElement {
    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, kEnum_Kind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols)) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Enum(fOffset, fTypeName, fSymbols));
    }

    String description() const override {
        String result = "enum class " + fTypeName + " {\n";
        String separator;
        std::vector<const Symbol*> sortedSymbols;
        for (const auto& pair : *fSymbols) {
            sortedSymbols.push_back(pair.second);
        }
        std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                  [](const Symbol* a, const Symbol* b) { return a->fName < b->fName; });
        for (const auto& s : sortedSymbols) {
            result += separator + "    " + s->fName + " = " +
                      ((Variable*) s)->fInitialValue->description();
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
