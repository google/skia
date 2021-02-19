/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchStatement.h"

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

    SkTHashSet<SKSL_INT> intValues;
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
            // TODO(johnstiles): once BinaryExpression::Make exists and is able to perform constant
            // folding, this call should be unnecessary; by the time we reach this point, any
            // simplification that we intend to do should have already been done.
            SKSL_INT v = 0;
            if (!ConstantFolder::GetConstantInt(*caseValue, &v)) {
                context.fErrors.error(caseValue->fOffset, "case value must be a constant integer");
                return nullptr;
            }
            if (intValues.contains(v)) {
                context.fErrors.error(caseValue->fOffset, "duplicate case value");
            }
            intValues.add(v);
        } else {
            // The null case-expression corresponds to `default:`.
            caseOffset = offset;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseOffset, std::move(caseValue),
                                                     std::move(caseStatements[i])));
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
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<SwitchCase>& c) {
        return !c->value() ||  // for `default`
               value->type() == c->value()->type();
    }));

    // TODO(johnstiles): "every switch-case value is distinct" is one of our invariants. Assert that
    // this is true.

    // TODO(skia:11340): Optimize static switches.

    return std::make_unique<SwitchStatement>(offset, isStatic, std::move(value), std::move(cases),
                                             std::move(symbolTable));
}

}  // namespace SkSL
