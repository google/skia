/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFunctionDeclaration.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

static bool check_modifiers(const Context& context, int offset, const Modifiers& modifiers) {
    IRGenerator::CheckModifiers(
            context,
            offset,
            modifiers,
            Modifiers::kHasSideEffects_Flag | Modifiers::kInline_Flag | Modifiers::kNoInline_Flag,
            /*permittedLayoutFlags=*/0);
    if ((modifiers.fFlags & Modifiers::kInline_Flag) &&
        (modifiers.fFlags & Modifiers::kNoInline_Flag)) {
        context.fErrors.error(offset, "functions cannot be both 'inline' and 'noinline'");
        return false;
    }
    return true;
}

static bool check_return_type(const Context& context, int offset, const Type& returnType,
                              bool isBuiltin) {
    ErrorReporter& errors = context.fErrors;
    if (returnType.isArray()) {
        errors.error(offset, "functions may not return type '" + returnType.displayName() + "'");
        return false;
    }
    if (context.fConfig->strictES2Mode() && returnType.isOrContainsArray()) {
        errors.error(offset, "functions may not return structs containing arrays");
        return false;
    }
    if (!isBuiltin && !returnType.isVoid() && returnType.componentType().isOpaque()) {
        errors.error(offset, "functions may not return opaque type '" + returnType.displayName() +
                             "'");
        return false;
    }
    return true;
}

static bool check_parameters(const Context& context, ModifiersPool& modifiersPool,
                             std::vector<std::unique_ptr<Variable>>& parameters, bool isMain,
                             bool isBuiltin) {
    auto typeIsValidForColor = [&](const Type& type) {
        return type == *context.fTypes.fHalf4 || type == *context.fTypes.fFloat4;
    };

    // Check modifiers on each function parameter.
    for (auto& param : parameters) {
        IRGenerator::CheckModifiers(context, param->fOffset, param->modifiers(),
                                    Modifiers::kConst_Flag | Modifiers::kIn_Flag |
                                    Modifiers::kOut_Flag, /*permittedLayoutFlags=*/0);
        const Type& type = param->type();
        // Only the (builtin) declarations of 'sample' are allowed to have shader/colorFilter or FP
        // parameters. You can pass other opaque types to functions safely; this restriction is
        // specific to "child" objects.
        if ((type.isEffectChild() || type.isFragmentProcessor()) && !isBuiltin) {
            context.fErrors.error(param->fOffset, "parameters of type '" + type.displayName() +
                                                  "' not allowed");
            return false;
        }

        Modifiers m = param->modifiers();
        ProgramKind kind = context.fConfig->fKind;
        if (isMain && (kind == ProgramKind::kRuntimeColorFilter ||
                       kind == ProgramKind::kRuntimeShader ||
                       kind == ProgramKind::kFragmentProcessor)) {
            // We verify that the signature is fully correct later. For now, if this is an .fp or
            // runtime effect of any flavor, a float2 param is supposed to be the coords, and
            // a half4/float parameter is supposed to be the input color:
            if (type == *context.fTypes.fFloat2) {
                m.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
            } else if(typeIsValidForColor(type)) {
                m.fLayout.fBuiltin = SK_INPUT_COLOR_BUILTIN;
            }
            if (m.fLayout.fBuiltin) {
                param->setModifiers(modifiersPool.add(m));
            }
        }
        if (isMain && (kind == ProgramKind::kFragment)) {
            // For testing purposes, we have .sksl inputs that are treated as both runtime effects
            // and fragment shaders. To make that work, fragment shaders are allowed to have a
            // coords parameter. We turn it into sk_FragCoord.
            if (type == *context.fTypes.fFloat2) {
                m.fLayout.fBuiltin = SK_FRAGCOORD_BUILTIN;
                param->setModifiers(modifiersPool.add(m));
            }
        }
    }
    return true;
}

static bool check_main_signature(const Context& context, int offset, const Type& returnType,
                                 std::vector<std::unique_ptr<Variable>>& parameters,
                                 bool isBuiltin) {
    ErrorReporter& errors = context.fErrors;
    ProgramKind kind = context.fConfig->fKind;

    auto typeIsValidForColor = [&](const Type& type) {
        return type == *context.fTypes.fHalf4 || type == *context.fTypes.fFloat4;
    };

    auto paramIsCoords = [&](int idx) {
        const Variable& p = *parameters[idx];
        return p.type() == *context.fTypes.fFloat2 &&
               p.modifiers().fFlags == 0 &&
               p.modifiers().fLayout.fBuiltin == (kind == ProgramKind::kFragment
                                                           ? SK_FRAGCOORD_BUILTIN
                                                           : SK_MAIN_COORDS_BUILTIN);
    };

    auto paramIsInputColor = [&](int idx) {
        return typeIsValidForColor(parameters[idx]->type()) &&
               parameters[idx]->modifiers().fFlags == 0 &&
               parameters[idx]->modifiers().fLayout.fBuiltin == SK_INPUT_COLOR_BUILTIN;
    };

    switch (kind) {
        case ProgramKind::kRuntimeColorFilter: {
            // (half4|float4) main(half4|float4)
            if (!typeIsValidForColor(returnType)) {
                errors.error(offset, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            bool validParams = (parameters.size() == 1 && paramIsInputColor(0));
            if (!validParams) {
                errors.error(offset, "'main' parameter must be 'vec4', 'float4', or 'half4'");
                return false;
            }
            break;
        }
        case ProgramKind::kRuntimeShader: {
            // (half4|float4) main(float2)  -or-  (half4|float4) main(float2, half4|float4)
            if (!typeIsValidForColor(returnType)) {
                errors.error(offset, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            bool validParams =
                    (parameters.size() == 1 && paramIsCoords(0)) ||
                    (parameters.size() == 2 && paramIsCoords(0) && paramIsInputColor(1));
            if (!validParams) {
                errors.error(offset, "'main' parameters must be (float2, (vec4|float4|half4)?)");
                return false;
            }
            break;
        }
        case ProgramKind::kFragmentProcessor: {
            if (returnType != *context.fTypes.fHalf4) {
                errors.error(offset, ".fp 'main' must return 'half4'");
                return false;
            }
            bool validParams = (parameters.size() == 0) ||
                               (parameters.size() == 1 && paramIsCoords(0));
            if (!validParams) {
                errors.error(offset, ".fp 'main' must be declared main() or main(float2)");
                return false;
            }
            break;
        }
        case ProgramKind::kGeneric:
            // No rules apply here
            break;
        case ProgramKind::kFragment: {
            bool validParams = (parameters.size() == 0) ||
                               (parameters.size() == 1 && paramIsCoords(0));
            if (!validParams) {
                errors.error(offset, "shader 'main' must be main() or main(float2)");
                return false;
            }
            break;
        }
        case ProgramKind::kVertex:
        case ProgramKind::kGeometry:
            if (parameters.size()) {
                errors.error(offset, "shader 'main' must have zero parameters");
                return false;
            }
            break;
    }
    return true;
}

/**
 * Checks for a previously existing declaration of this function, reporting errors if there is an
 * incompatible symbol. Returns true and sets outExistingDecl to point to the existing declaration
 * (or null if none) on success, returns false on error.
 */
static bool find_existing_declaration(const Context& context, SymbolTable& symbols, int offset,
                                      StringFragment name,
                                      std::vector<std::unique_ptr<Variable>>& parameters,
                                      const Type* returnType, bool isBuiltin,
                                      const FunctionDeclaration** outExistingDecl) {
    ErrorReporter& errors = context.fErrors;
    const Symbol* entry = symbols[name];
    *outExistingDecl = nullptr;
    if (entry) {
        std::vector<const FunctionDeclaration*> functions;
        switch (entry->kind()) {
            case Symbol::Kind::kUnresolvedFunction:
                functions = entry->as<UnresolvedFunction>().functions();
                break;
            case Symbol::Kind::kFunctionDeclaration:
                functions.push_back(&entry->as<FunctionDeclaration>());
                break;
            default:
                errors.error(offset, "symbol '" + name + "' was already defined");
                return false;
        }
        for (const FunctionDeclaration* other : functions) {
            SkASSERT(name == other->name());
            if (parameters.size() != other->parameters().size()) {
                continue;
            }
            bool match = true;
            for (size_t i = 0; i < parameters.size(); i++) {
                if (parameters[i]->type() != other->parameters()[i]->type()) {
                    match = false;
                    break;
                }
            }
            if (!match) {
                continue;
            }
            if (*returnType != other->returnType()) {
                std::vector<const Variable*> paramPtrs;
                paramPtrs.reserve(parameters.size());
                for (std::unique_ptr<Variable>& param : parameters) {
                    paramPtrs.push_back(param.get());
                }
                FunctionDeclaration invalidDecl(offset,
                                                &other->modifiers(),
                                                name,
                                                std::move(paramPtrs),
                                                returnType,
                                                isBuiltin);
                errors.error(offset,
                             "functions '" + invalidDecl.description() + "' and '" +
                             other->description() + "' differ only in return type");
                return false;
            }
            for (size_t i = 0; i < parameters.size(); i++) {
                if (parameters[i]->modifiers() != other->parameters()[i]->modifiers()) {
                    errors.error(offset,
                                 "modifiers on parameter " + to_string((uint64_t)i + 1) +
                                 " differ between declaration and definition");
                    return false;
                }
            }
            if (other->definition() && !other->isBuiltin()) {
                errors.error(offset, "duplicate definition of " + other->description());
                return false;
            }
            *outExistingDecl = other;
            break;
        }
    }
    return true;
}

const FunctionDeclaration* FunctionDeclaration::Convert(const Context& context,
        SymbolTable& symbols, ModifiersPool& modifiersPool, int offset, const Modifiers* modifiers,
        StringFragment name, std::vector<std::unique_ptr<Variable>> parameters,
        const Type* returnType, bool isBuiltin) {
    bool isMain = (name == "main");

    const FunctionDeclaration* decl = nullptr;
    if (!check_modifiers(context, offset, *modifiers) ||
        !check_return_type(context, offset, *returnType, isBuiltin) ||
        !check_parameters(context, modifiersPool, parameters, isMain, isBuiltin) ||
        (isMain && !check_main_signature(context, offset, *returnType, parameters, isBuiltin)) ||
        !find_existing_declaration(context, symbols, offset, name, parameters, returnType,
                                   isBuiltin, &decl)) {
        return nullptr;
    }
    std::vector<const Variable*> finalParameters;
    finalParameters.reserve(parameters.size());
    for (auto& param : parameters) {
        finalParameters.push_back(symbols.takeOwnershipOfSymbol(std::move(param)));
    }
    if (decl) {
        return decl;
    }
    auto result = std::make_unique<FunctionDeclaration>(offset, modifiers, name,
                                                        std::move(finalParameters), returnType,
                                                        isBuiltin);
    return symbols.add(std::move(result));
}

}  // namespace SkSL
