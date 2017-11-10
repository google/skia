/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ENUM
#define SKSL_ENUM

#include "SkSLProgramElement.h"
namespace SkSL {

struct Enum : public ProgramElement {
    Enum(int offset, StringFragment typeName, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, kEnum_Kind)
    , fTypeName(typeName)
    , fSymbols(std::move(symbols)) {}

    String description() const override {
        String result = "enum class " + fTypeName + " {\n";
        String separator;
        for (auto iter = fSymbols->begin(); iter != fSymbols->end(); ++iter) {
            result += separator + "    " + iter->first + " = " +
                      ((Variable&) *iter->second).fInitialValue->description();
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
