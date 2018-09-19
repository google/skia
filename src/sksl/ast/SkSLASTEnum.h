/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTENUM
#define SKSL_ASTENUM

#include "src/sksl/ast/SkSLASTDeclaration.h"
namespace SkSL {

struct ASTEnum : public ASTDeclaration {
    ASTEnum(int offset, StringFragment typeName, std::vector<StringFragment> names,
            std::vector<std::unique_ptr<ASTExpression>> values)
    : INHERITED(offset, kEnum_Kind)
    , fTypeName(typeName)
    , fNames(std::move(names))
    , fValues(std::move(values)) {
        SkASSERT(fNames.size() == fValues.size());
    }

    String description() const override {
        String result = "enum class " + fTypeName + " {\n";
        String separator;
        for (StringFragment name : fNames) {
            result += separator + "    " + name;
            separator = ",\n";
        }
        result += "};";
        return result;
    }

    const StringFragment fTypeName;
    const std::vector<StringFragment> fNames;
    const std::vector<std::unique_ptr<ASTExpression>> fValues;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
