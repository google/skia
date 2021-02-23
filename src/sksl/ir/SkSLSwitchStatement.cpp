/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchStatement.h"

#include <forward_list>

#include "include/private/SkTHash.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> SwitchStatement::clone() const {
    std::vector<std::unique_ptr<SwitchCase>> cloned;
    for (const std::unique_ptr<SwitchCase>& sc : this->cases()) {
        cloned.emplace_back(&sc->clone().release()->as<SwitchCase>());
    }
    return std::make_unique<SwitchStatement>(fOffset,
                                             this->isStatic(),
                                             this->value()->clone(),
                                             std::move(cloned),
                                             SymbolTable::WrapIfBuiltin(this->symbols()));
}

String SwitchStatement::description() const {
    String result;
    if (this->isStatic()) {
        result += "@";
    }
    result += String::printf("switch (%s) {\n", this->value()->description().c_str());
    for (const auto& c : this->cases()) {
        result += c->description();
    }
    result += "}";
    return result;
}

static std::forward_list<const SwitchCase*> find_duplicate_case_values(
        const std::vector<std::unique_ptr<SwitchCase>>& cases) {
    std::forward_list<const SwitchCase*> duplicateCases;
    SkTHashSet<SKSL_INT> intValues;
    bool foundDefault = false;

    for (const std::unique_ptr<SwitchCase>& sc : cases) {
        const std::unique_ptr<Expression>& valueExpr = sc->value();

        // A null case-value indicates the `default` switch-case.
        if (!valueExpr) {
            if (foundDefault) {
                duplicateCases.push_front(sc.get());
                continue;
            }
            foundDefault = true;
            continue;
        }

        // GetConstantInt already succeeded when the SwitchCase was first assembled, so it should
        // succeed this time too.
        SKSL_INT intValue = 0;
        SkAssertResult(ConstantFolder::GetConstantInt(*valueExpr, &intValue));
        if (intValues.contains(intValue)) {
            duplicateCases.push_front(sc.get());
            continue;
        }
        intValues.add(intValue);
    }

    return duplicateCases;
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 int offset,
                                                 bool isStatic,
                                                 std::unique_ptr<Expression> value,
                                                 ExpressionArray caseValues,
                                                 SkTArray<StatementArray> caseStatements,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(offset, "switch statements are not supported");
        return nullptr;
    }

    if (!value->type().isEnum()) {
        value = context.fTypes.fInt->coerceExpression(std::move(value), context);
        if (!value) {
            return nullptr;
        }
    }

    std::vector<std::unique_ptr<SwitchCase>> cases;
    for (int i = 0; i < caseValues.count(); ++i) {
        int caseOffset;
        std::unique_ptr<Expression> caseValue;
        if (caseValues[i]) {
            caseOffset = caseValues[i]->fOffset;

            // Case values must be the same type as the switch value--`int` or a particular enum.
            caseValue = value->type().coerceExpression(std::move(caseValues[i]), context);
            if (!caseValue) {
                return nullptr;
            }
            // Case values must be a literal integer or a `const int` variable reference.
            SKSL_INT intValue;
            if (!ConstantFolder::GetConstantInt(*caseValue, &intValue)) {
                context.fErrors.error(caseValue->fOffset, "case value must be a constant integer");
                return nullptr;
            }
        } else {
            // The null case-expression corresponds to `default:`.
            caseOffset = offset;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseOffset, std::move(caseValue),
                                                     std::move(caseStatements[i])));
    }

    // Detect duplicate `case` labels and report an error.
    // (Using forward_list here to optimize for the common case of no results.)
    std::forward_list<const SwitchCase*> duplicateCases = find_duplicate_case_values(cases);
    if (!duplicateCases.empty()) {
        duplicateCases.reverse();
        for (const SwitchCase* sc : duplicateCases) {
            if (sc->value() != nullptr) {
                context.fErrors.error(sc->fOffset,
                                      "duplicate case value '" + sc->value()->description() + "'");
            } else {
                context.fErrors.error(sc->fOffset, "duplicate default case");
            }
        }
        return nullptr;
    }

    return SwitchStatement::Make(context, offset, isStatic, std::move(value), std::move(cases),
                                 std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 int offset,
                                                 bool isStatic,
                                                 std::unique_ptr<Expression> value,
                                                 std::vector<std::unique_ptr<SwitchCase>> cases,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    // Confirm that every switch-case has been coerced to the proper type.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<SwitchCase>& sc) {
        return !sc->value() ||  // `default` case has a null value
               value->type() == sc->value()->type();
    }));

    // Confirm that every switch-case value is unique.
    SkASSERT(find_duplicate_case_values(cases).empty());

    // TODO(skia:11340): Optimize static switches.
    return std::make_unique<SwitchStatement>(offset, isStatic, std::move(value), std::move(cases),
                                             std::move(symbolTable));
}

}  // namespace SkSL
