/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

std::unique_ptr<Statement> VarDeclaration::clone() const {
    return std::make_unique<VarDeclaration>(&this->var(),
                                            &this->baseType(),
                                            fArraySize,
                                            this->value() ? this->value()->clone() : nullptr);
}

String VarDeclaration::description() const {
    String result = this->var().modifiers().description() + this->baseType().description() + " " +
                    this->var().name();
    if (this->arraySize() > 0) {
        result.appendf("[%d]", this->arraySize());
    } else if (this->arraySize() == Type::kUnsizedArray) {
        result += "[]";
    }
    if (this->value()) {
        result += " = " + this->value()->description();
    }
    result += ";";
    return result;
}

std::unique_ptr<Statement> VarDeclaration::Convert(const Context& context,
                                                   Variable* var,
                                                   std::unique_ptr<Expression> value) {
    if (var->modifiers().fFlags & Modifiers::kConst_Flag) {
        if (!value) {
            context.errors().error(var->fOffset, "'const' variables must be initialized");
            return nullptr;
        }
        if (!Analysis::IsConstantExpression(*value)) {
            context.errors().error(value->fOffset,
                                   "'const' variable initializer must be a constant expression");
            return nullptr;
        }
    }
    if (value) {
        if (var->storage() == Variable::Storage::kGlobal &&
            !Analysis::IsConstantExpression(*value)) {
            context.errors().error(value->fOffset,
                                   "global variable initializer must be a constant expression");
            return nullptr;
        }
        if (var->type().isOpaque()) {
            context.errors().error(
                    value->fOffset,
                    "opaque type '" + var->type().name() + "' cannot use initializer expressions");
            return nullptr;
        }
        if (var->modifiers().fFlags & Modifiers::kIn_Flag) {
            context.errors().error(value->fOffset,
                                   "'in' variables cannot use initializer expressions");
            return nullptr;
        }
        if (var->modifiers().fFlags & Modifiers::kUniform_Flag) {
            context.errors().error(value->fOffset,
                                   "'uniform' variables cannot use initializer expressions");
            return nullptr;
        }
        value = var->type().coerceExpression(std::move(value), context);
        if (!value) {
            return nullptr;
        }
    }
    const Type* baseType = &var->type();
    int arraySize = 0;
    if (baseType->isArray()) {
        arraySize = baseType->columns();
        baseType = &baseType->componentType();
    }
    return VarDeclaration::Make(context, var, baseType, arraySize, std::move(value));
}

std::unique_ptr<Statement> VarDeclaration::Make(const Context& context,
                                                Variable* var,
                                                const Type* baseType,
                                                int arraySize,
                                                std::unique_ptr<Expression> value) {
    SkASSERT(!baseType->isArray());
    // 'const' variables must be initialized
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) || value);
    // 'const' variable initializer must be a constant expression
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) ||
             Analysis::IsConstantExpression(*value));
    // global variable initializer must be a constant expression
    SkASSERT(!(value && var->storage() == Variable::Storage::kGlobal &&
               !Analysis::IsConstantExpression(*value)));
    // opaque type cannot use initializer expressions
    SkASSERT(!(value && var->type().isOpaque()));
    // 'in' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kIn_Flag)));
    // 'uniform' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kUniform_Flag)));

    // Detect and report out-of-range initial-values for this variable.
    if (value) {
        var->type().checkForOutOfRangeLiteral(context, *value);
    }

    auto result = std::make_unique<VarDeclaration>(var, baseType, arraySize, std::move(value));
    var->setDeclaration(result.get());
    return std::move(result);
}

}  // namespace SkSL
