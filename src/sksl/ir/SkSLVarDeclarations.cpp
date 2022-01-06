/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVarDeclarations.h"

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"

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
    }
    if (this->value()) {
        result += " = " + this->value()->description();
    }
    result += ";";
    return result;
}

void VarDeclaration::ErrorCheck(const Context& context,
                                int line,
                                const Modifiers& modifiers,
                                const Type* baseType,
                                Variable::Storage storage) {
    if (baseType->matches(*context.fTypes.fInvalid)) {
        context.fErrors->error(line, "invalid type");
        return;
    }
    if (baseType->isVoid()) {
        context.fErrors->error(line, "variables of type 'void' are not allowed");
        return;
    }
    if (context.fConfig->strictES2Mode() && baseType->isArray()) {
        context.fErrors->error(line, "array size must appear after variable name");
    }

    if (baseType->componentType().isOpaque() && storage != Variable::Storage::kGlobal) {
        context.fErrors->error(line,
                "variables of type '" + baseType->displayName() + "' must be global");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) && baseType->isMatrix()) {
        context.fErrors->error(line, "'in' variables may not have matrix type");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) && (modifiers.fFlags & Modifiers::kUniform_Flag)) {
        context.fErrors->error(line, "'in uniform' variables not permitted");
    }
    if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        if (modifiers.fFlags & Modifiers::kIn_Flag) {
            context.fErrors->error(line, "'in' variables not permitted in runtime effects");
        }
    }
    if (baseType->isEffectChild() && !(modifiers.fFlags & Modifiers::kUniform_Flag)) {
        context.fErrors->error(line,
                "variables of type '" + baseType->displayName() + "' must be uniform");
    }
    if (modifiers.fFlags & SkSL::Modifiers::kUniform_Flag &&
        (context.fConfig->fKind == ProgramKind::kCustomMeshVertex ||
         context.fConfig->fKind == ProgramKind::kCustomMeshFragment)) {
        context.fErrors->error(line, "uniforms are not permitted in custom mesh shaders");
    }
    if (modifiers.fLayout.fFlags & Layout::kColor_Flag) {
        if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            context.fErrors->error(line, "'layout(color)' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            context.fErrors->error(line,
                                   "'layout(color)' is only permitted on 'uniform' variables");
        }
        auto validColorXformType = [](const Type& t) {
            return t.isVector() && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType) && !(baseType->isArray() &&
                                                 validColorXformType(baseType->componentType()))) {
            context.fErrors->error(line,
                                   "'layout(color)' is not permitted on variables of type '" +
                                           baseType->displayName() + "'");
        }
    }
    int permitted = Modifiers::kConst_Flag | Modifiers::kHighp_Flag | Modifiers::kMediump_Flag |
                    Modifiers::kLowp_Flag;
    if (storage == Variable::Storage::kGlobal) {
        permitted |= Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag |
                     Modifiers::kFlat_Flag | Modifiers::kNoPerspective_Flag;
    }
    // TODO(skbug.com/11301): Migrate above checks into building a mask of permitted layout flags

    int permittedLayoutFlags = ~0;
    // We don't allow 'binding' or 'set' on normal uniform variables, only on textures, samplers,
    // and interface blocks (holding uniform variables). They're also only allowed at global scope,
    // not on interface block fields (or locals/parameters).
    bool permitBindingAndSet = baseType->typeKind() == Type::TypeKind::kSampler ||
                               baseType->typeKind() == Type::TypeKind::kSeparateSampler ||
                               baseType->typeKind() == Type::TypeKind::kTexture ||
                               baseType->isInterfaceBlock();
    if (storage != Variable::Storage::kGlobal ||
        ((modifiers.fFlags & Modifiers::kUniform_Flag) && !permitBindingAndSet)) {
        permittedLayoutFlags &= ~Layout::kBinding_Flag;
        permittedLayoutFlags &= ~Layout::kSet_Flag;
    }
    modifiers.checkPermitted(context, line, permitted, permittedLayoutFlags);
}

bool VarDeclaration::ErrorCheckAndCoerce(const Context& context, const Variable& var,
        std::unique_ptr<Expression>& value) {
    const Type* baseType = &var.type();
    if (baseType->isArray()) {
        baseType = &baseType->componentType();
    }
    ErrorCheck(context, var.fLine, var.modifiers(), baseType, var.storage());
    if (value) {
        if (var.type().isOpaque()) {
            context.fErrors->error(value->fLine,
                    "opaque type '" + var.type().name() + "' cannot use initializer expressions");
            return false;
        }
        if (var.modifiers().fFlags & Modifiers::kIn_Flag) {
            context.fErrors->error(value->fLine,
                    "'in' variables cannot use initializer expressions");
            return false;
        }
        if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
            context.fErrors->error(value->fLine,
                    "'uniform' variables cannot use initializer expressions");
            return false;
        }
        if (var.storage() == Variable::Storage::kInterfaceBlock) {
            context.fErrors->error(value->fLine,
                    "initializers are not permitted on interface block fields");
            return false;
        }
        value = var.type().coerceExpression(std::move(value), context);
        if (!value) {
            return false;
        }
    }
    if (var.modifiers().fFlags & Modifiers::kConst_Flag) {
        if (!value) {
            context.fErrors->error(var.fLine, "'const' variables must be initialized");
            return false;
        }
        if (!Analysis::IsConstantExpression(*value)) {
            context.fErrors->error(value->fLine,
                    "'const' variable initializer must be a constant expression");
            return false;
        }
    }
    if (var.storage() == Variable::Storage::kInterfaceBlock) {
        if (var.type().isOpaque()) {
            context.fErrors->error(var.fLine, "opaque type '" + var.type().name() +
                    "' is not permitted in an interface block");
            return false;
        }
    }
    if (var.storage() == Variable::Storage::kGlobal) {
        if (value && !Analysis::IsConstantExpression(*value)) {
            context.fErrors->error(value->fLine,
                    "global variable initializer must be a constant expression");
            return false;
        }
    }
    return true;
}

std::unique_ptr<Statement> VarDeclaration::Convert(const Context& context,
        std::unique_ptr<Variable> var, std::unique_ptr<Expression> value, bool addToSymbolTable) {
    if (!ErrorCheckAndCoerce(context, *var, value)) {
        return nullptr;
    }
    const Type* baseType = &var->type();
    int arraySize = 0;
    if (baseType->isArray()) {
        arraySize = baseType->columns();
        baseType = &baseType->componentType();
    }
    std::unique_ptr<Statement> varDecl = VarDeclaration::Make(context, var.get(), baseType,
            arraySize, std::move(value));
    if (!varDecl) {
        return nullptr;
    }

    // Detect the declaration of magical variables.
    if ((var->storage() == Variable::Storage::kGlobal) && var->name() == Compiler::FRAGCOLOR_NAME) {
        // Silently ignore duplicate definitions of `sk_FragColor`.
        const Symbol* symbol = (*ThreadContext::SymbolTable())[var->name()];
        if (symbol) {
            return nullptr;
        }
    } else if ((var->storage() == Variable::Storage::kGlobal ||
                var->storage() == Variable::Storage::kInterfaceBlock) &&
               var->name() == Compiler::RTADJUST_NAME) {
        // `sk_RTAdjust` is special, and makes the IR generator emit position-fixup expressions.
        if (ThreadContext::RTAdjustState().fVar || ThreadContext::RTAdjustState().fInterfaceBlock) {
            context.fErrors->error(var->fLine, "duplicate definition of 'sk_RTAdjust'");
            return nullptr;
        }
        if (!var->type().matches(*context.fTypes.fFloat4)) {
            context.fErrors->error(var->fLine, "sk_RTAdjust must have type 'float4'");
            return nullptr;
        }
        ThreadContext::RTAdjustState().fVar = var.get();
    }

    if (addToSymbolTable) {
        ThreadContext::SymbolTable()->add(std::move(var));
    } else {
        ThreadContext::SymbolTable()->takeOwnershipOfSymbol(std::move(var));
    }
    return varDecl;
}

std::unique_ptr<Statement> VarDeclaration::Make(const Context& context, Variable* var,
        const Type* baseType, int arraySize, std::unique_ptr<Expression> value) {
    SkASSERT(!baseType->isArray());
    // function parameters cannot have variable declarations
    SkASSERT(var->storage() != Variable::Storage::kParameter);
    // 'const' variables must be initialized
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) || value);
    // 'const' variable initializer must be a constant expression
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) ||
             Analysis::IsConstantExpression(*value));
    // global variable initializer must be a constant expression
    SkASSERT(!(value && var->storage() == Variable::Storage::kGlobal &&
               !Analysis::IsConstantExpression(*value)));
    // opaque type not permitted on an interface block
    SkASSERT(!(var->storage() == Variable::Storage::kInterfaceBlock && var->type().isOpaque()));
    // initializers are not permitted on interface block fields
    SkASSERT(!(var->storage() == Variable::Storage::kInterfaceBlock && value));
    // opaque type cannot use initializer expressions
    SkASSERT(!(value && var->type().isOpaque()));
    // 'in' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kIn_Flag)));
    // 'uniform' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kUniform_Flag)));

    auto result = std::make_unique<VarDeclaration>(var, baseType, arraySize, std::move(value));
    var->setDeclaration(result.get());
    return std::move(result);
}

}  // namespace SkSL
