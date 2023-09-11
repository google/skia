/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFieldAccess.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLConstructorStruct.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLMethodReference.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"  // IWYU pragma: keep

#include <cstddef>

namespace SkSL {

std::unique_ptr<Expression> FieldAccess::Convert(const Context& context,
                                                 Position pos,
                                                 std::unique_ptr<Expression> base,
                                                 std::string_view field) {
    const Type& baseType = base->type();
    if (baseType.isEffectChild()) {
        // Turn the field name into a free function name, prefixed with '$':
        std::string methodName = "$" + std::string(field);
        const Symbol* result = context.fSymbolTable->find(methodName);
        if (result && result->is<FunctionDeclaration>()) {
            return std::make_unique<MethodReference>(context, pos, std::move(base),
                                                     &result->as<FunctionDeclaration>());
        }
        context.fErrors->error(pos, "type '" + baseType.displayName() + "' has no method named '" +
                                    std::string(field) + "'");
        return nullptr;
    }
    if (baseType.isStruct()) {
        SkSpan<const Field> fields = baseType.fields();
        for (size_t i = 0; i < fields.size(); i++) {
            if (fields[i].fName == field) {
                return FieldAccess::Make(context, pos, std::move(base), (int)i);
            }
        }
    }
    if (baseType.matches(*context.fTypes.fSkCaps)) {
        return Setting::Convert(context, pos, field);
    }

    context.fErrors->error(pos, "type '" + baseType.displayName() +
                                "' does not have a field named '" + std::string(field) + "'");
    return nullptr;
}

static std::unique_ptr<Expression> extract_field(Position pos,
                                                 const ConstructorStruct& ctor,
                                                 int fieldIndex) {
    // Confirm that the fields that are being removed are side-effect free.
    const ExpressionArray& args = ctor.arguments();
    int numFields = args.size();
    for (int index = 0; index < numFields; ++index) {
        if (fieldIndex == index) {
            continue;
        }
        if (Analysis::HasSideEffects(*args[index])) {
            return nullptr;
        }
    }

    // Return the desired field.
    return args[fieldIndex]->clone(pos);
}

std::unique_ptr<Expression> FieldAccess::Make(const Context& context,
                                              Position pos,
                                              std::unique_ptr<Expression> base,
                                              int fieldIndex,
                                              OwnerKind ownerKind) {
    SkASSERT(base->type().isStruct());
    SkASSERT(fieldIndex >= 0);
    SkASSERT(fieldIndex < (int)base->type().fields().size());

    // Replace `knownStruct.field` with the field's value if there are no side-effects involved.
    const Expression* expr = ConstantFolder::GetConstantValueForVariable(*base);
    if (expr->is<ConstructorStruct>()) {
        if (std::unique_ptr<Expression> field = extract_field(pos, expr->as<ConstructorStruct>(),
                                                              fieldIndex)) {
            return field;
        }
    }

    return std::make_unique<FieldAccess>(pos, std::move(base), fieldIndex, ownerKind);
}

size_t FieldAccess::initialSlot() const {
    SkSpan<const Field> fields = this->base()->type().fields();
    const int fieldIndex = this->fieldIndex();

    size_t slot = 0;
    for (int index = 0; index < fieldIndex; ++index) {
        slot += fields[index].fType->slotCount();
    }
    return slot;
}

std::string FieldAccess::description(OperatorPrecedence) const {
    std::string f = this->base()->description(OperatorPrecedence::kPostfix);
    if (!f.empty()) {
        f.push_back('.');
    }
    return f + std::string(this->base()->type().fields()[this->fieldIndex()].fName);
}

}  // namespace SkSL
