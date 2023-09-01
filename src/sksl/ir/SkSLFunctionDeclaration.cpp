/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFunctionDeclaration.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstddef>
#include <utility>

using namespace skia_private;

namespace SkSL {

static bool check_modifiers(const Context& context, Position pos, ModifierFlags modifierFlags) {
    const ModifierFlags permitted = ModifierFlag::kInline |
                                    ModifierFlag::kNoInline |
                                    (context.fConfig->fIsBuiltinCode ? ModifierFlag::kES3 |
                                                                       ModifierFlag::kPure |
                                                                       ModifierFlag::kExport
                                                                     : ModifierFlag::kNone);
    modifierFlags.checkPermittedFlags(context, pos, permitted);
    if (modifierFlags.isInline() && modifierFlags.isNoInline()) {
        context.fErrors->error(pos, "functions cannot be both 'inline' and 'noinline'");
        return false;
    }
    return true;
}

static bool check_return_type(const Context& context, Position pos, const Type& returnType) {
    ErrorReporter& errors = *context.fErrors;
    if (returnType.isArray()) {
        errors.error(pos, "functions may not return type '" + returnType.displayName() + "'");
        return false;
    }
    if (context.fConfig->strictES2Mode() && returnType.isOrContainsArray()) {
        errors.error(pos, "functions may not return structs containing arrays");
        return false;
    }
    if (!context.fConfig->fIsBuiltinCode && returnType.componentType().isOpaque()) {
        errors.error(pos, "functions may not return opaque type '" + returnType.displayName() +
                "'");
        return false;
    }
    return true;
}

static bool check_parameters(const Context& context,
                             TArray<std::unique_ptr<Variable>>& parameters,
                             ModifierFlags modifierFlags,
                             IntrinsicKind intrinsicKind) {
    // Check modifiers on each function parameter.
    for (auto& param : parameters) {
        const Type& type = param->type();
        ModifierFlags permittedFlags = ModifierFlag::kConst | ModifierFlag::kIn;
        LayoutFlags permittedLayoutFlags = LayoutFlag::kNone;
        if (!type.isOpaque()) {
            permittedFlags |= ModifierFlag::kOut;
        }
        if (type.typeKind() == Type::TypeKind::kTexture) {
            // We allow `readonly` `writeonly` and `layout(pixel-format)` on storage textures.
            permittedFlags |= ModifierFlag::kReadOnly | ModifierFlag::kWriteOnly;
            permittedLayoutFlags |= LayoutFlag::kAllPixelFormats;

            // Intrinsics are allowed to accept any pixel format, but user code must explicitly
            // specify a pixel format like `layout(rgba32f)`.
            if (intrinsicKind == kNotIntrinsic &&
                !(param->layout().fFlags & LayoutFlag::kAllPixelFormats)) {
                context.fErrors->error(param->fPosition, "storage texture parameters must specify "
                                                         "a pixel format layout-qualifier");
                return false;
            }
        }
        param->modifierFlags().checkPermittedFlags(context, param->modifiersPosition(),
                                                   permittedFlags);
        param->layout().checkPermittedLayout(context, param->modifiersPosition(),
                                             permittedLayoutFlags);
        // Only the (builtin) declarations of 'sample' are allowed to have shader/colorFilter or FP
        // parameters. You can pass other opaque types to functions safely; this restriction is
        // specific to "child" objects.
        if (type.isEffectChild() && !context.fConfig->fIsBuiltinCode) {
            context.fErrors->error(param->fPosition, "parameters of type '" + type.displayName() +
                                                     "' not allowed");
            return false;
        }

        // Pure functions should not change any state, and should be safe to eliminate if their
        // result is not used; this is incompatible with out-parameters, so we forbid it here.
        // (We don't exhaustively guard against pure functions changing global state in other ways,
        // though, since they aren't allowed in user code.)
        if (modifierFlags.isPure() && (param->modifierFlags() & ModifierFlag::kOut)) {
            context.fErrors->error(param->modifiersPosition(),
                                   "pure functions cannot have out parameters");
            return false;
        }
    }
    return true;
}

static bool type_is_valid_for_color(const Type& type) {
    return type.isVector() && type.columns() == 4 && type.componentType().isFloat();
}

static bool type_is_valid_for_coords(const Type& type) {
    return type.isVector() && type.highPrecision() && type.columns() == 2 &&
           type.componentType().isFloat();
}

static bool check_main_signature(const Context& context, Position pos, const Type& returnType,
                                 TArray<std::unique_ptr<Variable>>& parameters) {
    ErrorReporter& errors = *context.fErrors;
    ProgramKind kind = context.fConfig->fKind;

    auto typeIsValidForAttributes = [](const Type& type) {
        return type.isStruct() && type.name() == "Attributes";
    };

    auto typeIsValidForVaryings = [](const Type& type) {
        return type.isStruct() && type.name() == "Varyings";
    };

    auto paramIsCoords = [&](int idx) {
        const Variable& p = *parameters[idx];
        return type_is_valid_for_coords(p.type()) && p.modifierFlags() == ModifierFlag::kNone;
    };

    auto paramIsColor = [&](int idx) {
        const Variable& p = *parameters[idx];
        return type_is_valid_for_color(p.type()) && p.modifierFlags() == ModifierFlag::kNone;
    };

    auto paramIsConstInAttributes = [&](int idx) {
        const Variable& p = *parameters[idx];
        return typeIsValidForAttributes(p.type()) && p.modifierFlags() == ModifierFlag::kConst;
    };

    auto paramIsConstInVaryings = [&](int idx) {
        const Variable& p = *parameters[idx];
        return typeIsValidForVaryings(p.type()) && p.modifierFlags() == ModifierFlag::kConst;
    };

    auto paramIsOutColor = [&](int idx) {
        const Variable& p = *parameters[idx];
        return type_is_valid_for_color(p.type()) && p.modifierFlags() == ModifierFlag::kOut;
    };

    switch (kind) {
        case ProgramKind::kRuntimeColorFilter:
        case ProgramKind::kPrivateRuntimeColorFilter: {
            // (half4|float4) main(half4|float4)
            if (!type_is_valid_for_color(returnType)) {
                errors.error(pos, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            bool validParams = (parameters.size() == 1 && paramIsColor(0));
            if (!validParams) {
                errors.error(pos, "'main' parameter must be 'vec4', 'float4', or 'half4'");
                return false;
            }
            break;
        }
        case ProgramKind::kRuntimeShader:
        case ProgramKind::kPrivateRuntimeShader: {
            // (half4|float4) main(float2)
            if (!type_is_valid_for_color(returnType)) {
                errors.error(pos, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            if (!(parameters.size() == 1 && paramIsCoords(0))) {
                errors.error(pos, "'main' parameter must be 'float2' or 'vec2'");
                return false;
            }
            break;
        }
        case ProgramKind::kRuntimeBlender:
        case ProgramKind::kPrivateRuntimeBlender: {
            // (half4|float4) main(half4|float4, half4|float4)
            if (!type_is_valid_for_color(returnType)) {
                errors.error(pos, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            if (!(parameters.size() == 2 && paramIsColor(0) && paramIsColor(1))) {
                errors.error(pos, "'main' parameters must be (vec4|float4|half4, "
                                  "vec4|float4|half4)");
                return false;
            }
            break;
        }
        case ProgramKind::kMeshVertex: {
            // Varyings main(const Attributes)
            if (!typeIsValidForVaryings(returnType)) {
                errors.error(pos, "'main' must return 'Varyings'.");
                return false;
            }
            if (!(parameters.size() == 1 && paramIsConstInAttributes(0))) {
                errors.error(pos, "'main' parameter must be 'const Attributes'.");
                return false;
            }
            break;
        }
        case ProgramKind::kMeshFragment: {
            // float2 main(const Varyings) -or- float2 main(const Varyings, out half4|float4)
            if (!type_is_valid_for_coords(returnType)) {
                errors.error(pos, "'main' must return: 'vec2' or 'float2'");
                return false;
            }
            if (!((parameters.size() == 1 && paramIsConstInVaryings(0)) ||
                  (parameters.size() == 2 && paramIsConstInVaryings(0) && paramIsOutColor(1)))) {
                errors.error(pos,
                             "'main' parameters must be (const Varyings, (out (half4|float4))?)");
                return false;
            }
            break;
        }
        case ProgramKind::kFragment:
        case ProgramKind::kGraphiteFragment: {
            bool validParams = (parameters.size() == 0) ||
                               (parameters.size() == 1 && paramIsCoords(0));
            if (!validParams) {
                errors.error(pos, "shader 'main' must be main() or main(float2)");
                return false;
            }
            break;
        }
        case ProgramKind::kVertex:
        case ProgramKind::kGraphiteVertex:
        case ProgramKind::kCompute:
            if (!returnType.matches(*context.fTypes.fVoid)) {
                errors.error(pos, "'main' must return 'void'");
                return false;
            }
            if (parameters.size()) {
                errors.error(pos, "shader 'main' must have zero parameters");
                return false;
            }
            break;
    }
    return true;
}

/**
 * Given a concrete type (`float3`) and a generic type (`$genType`), returns the index of the
 * concrete type within the generic type's typelist. Returns -1 if there is no match.
 */
static int find_generic_index(const Type& concreteType,
                              const Type& genericType,
                              bool allowNarrowing) {
    SkSpan<const Type* const> genericTypes = genericType.coercibleTypes();
    for (size_t index = 0; index < genericTypes.size(); ++index) {
        if (concreteType.canCoerceTo(*genericTypes[index], allowNarrowing)) {
            return index;
        }
    }
    return -1;
}

/** Returns true if the types match, or if `concreteType` can be found in `maybeGenericType`. */
static bool type_generically_matches(const Type& concreteType, const Type& maybeGenericType) {
    return maybeGenericType.isGeneric()
                ? find_generic_index(concreteType, maybeGenericType, /*allowNarrowing=*/false) != -1
                : concreteType.matches(maybeGenericType);
}

/**
 * Checks a parameter list (params) against the parameters of a function that was declared earlier
 * (otherParams). Returns true if they match, even if the parameters in `otherParams` contain
 * generic types.
 */
static bool parameters_match(SkSpan<const std::unique_ptr<Variable>> params,
                             SkSpan<Variable* const> otherParams) {
    // If the param lists are different lengths, they're definitely not a match.
    if (params.size() != otherParams.size()) {
        return false;
    }

    // Figure out a consistent generic index (or bail if we find a contradiction).
    int genericIndex = -1;
    for (size_t i = 0; i < params.size(); ++i) {
        const Type* paramType = &params[i]->type();
        const Type* otherParamType = &otherParams[i]->type();

        if (otherParamType->isGeneric()) {
            int genericIndexForThisParam = find_generic_index(*paramType, *otherParamType,
                                                              /*allowNarrowing=*/false);
            if (genericIndexForThisParam == -1) {
                // The type wasn't a match for this generic at all; these params can't be a match.
                return false;
            }
            if (genericIndex != -1 && genericIndex != genericIndexForThisParam) {
                // The generic index mismatches from what we determined on a previous parameter.
                return false;
            }
            genericIndex = genericIndexForThisParam;
        }
    }

    // Now that we've determined a generic index (if we needed one), do a parameter check.
    for (size_t i = 0; i < params.size(); i++) {
        const Type* paramType = &params[i]->type();
        const Type* otherParamType = &otherParams[i]->type();

        // Make generic types concrete.
        if (otherParamType->isGeneric()) {
            SkASSERT(genericIndex != -1);
            SkASSERT(genericIndex < (int)otherParamType->coercibleTypes().size());
            otherParamType = otherParamType->coercibleTypes()[genericIndex];
        }
        // Detect type mismatches.
        if (!paramType->matches(*otherParamType)) {
            return false;
        }
    }
    return true;
}

/**
 * Checks for a previously existing declaration of this function, reporting errors if there is an
 * incompatible symbol. Returns true and sets outExistingDecl to point to the existing declaration
 * (or null if none) on success, returns false on error.
 */
static bool find_existing_declaration(const Context& context,
                                      Position pos,
                                      ModifierFlags modifierFlags,
                                      IntrinsicKind intrinsicKind,
                                      std::string_view name,
                                      TArray<std::unique_ptr<Variable>>& parameters,
                                      Position returnTypePos,
                                      const Type* returnType,
                                      FunctionDeclaration** outExistingDecl) {
    auto invalidDeclDescription = [&]() -> std::string {
        TArray<Variable*> paramPtrs;
        paramPtrs.reserve_exact(parameters.size());
        for (std::unique_ptr<Variable>& param : parameters) {
            paramPtrs.push_back(param.get());
        }
        return FunctionDeclaration(context,
                                   pos,
                                   modifierFlags,
                                   name,
                                   std::move(paramPtrs),
                                   returnType,
                                   intrinsicKind)
                .description();
    };

    ErrorReporter& errors = *context.fErrors;
    Symbol* entry = context.fSymbolTable->findMutable(name);
    *outExistingDecl = nullptr;
    if (entry) {
        if (!entry->is<FunctionDeclaration>()) {
            errors.error(pos, "symbol '" + std::string(name) + "' was already defined");
            return false;
        }
        for (FunctionDeclaration* other = &entry->as<FunctionDeclaration>(); other;
             other = other->mutableNextOverload()) {
            SkASSERT(name == other->name());
            if (!parameters_match(parameters, other->parameters())) {
                continue;
            }
            if (!type_generically_matches(*returnType, other->returnType())) {
                errors.error(returnTypePos, "functions '" + invalidDeclDescription() + "' and '" +
                                            other->description() + "' differ only in return type");
                return false;
            }
            for (int i = 0; i < parameters.size(); i++) {
                if (parameters[i]->modifierFlags() != other->parameters()[i]->modifierFlags() ||
                    parameters[i]->layout() != other->parameters()[i]->layout()) {
                    errors.error(parameters[i]->fPosition,
                                 "modifiers on parameter " + std::to_string(i + 1) +
                                 " differ between declaration and definition");
                    return false;
                }
            }
            if (other->definition() || other->isIntrinsic() ||
                modifierFlags != other->modifierFlags()) {
                errors.error(pos, "duplicate definition of '" + invalidDeclDescription() + "'");
                return false;
            }
            *outExistingDecl = other;
            break;
        }
        if (!*outExistingDecl && entry->as<FunctionDeclaration>().isMain()) {
            errors.error(pos, "duplicate definition of 'main'");
            return false;
        }
    }
    return true;
}

FunctionDeclaration::FunctionDeclaration(const Context& context,
                                         Position pos,
                                         ModifierFlags modifierFlags,
                                         std::string_view name,
                                         TArray<Variable*> parameters,
                                         const Type* returnType,
                                         IntrinsicKind intrinsicKind)
        : INHERITED(pos, kIRNodeKind, name, /*type=*/nullptr)
        , fDefinition(nullptr)
        , fParameters(std::move(parameters))
        , fReturnType(returnType)
        , fModifierFlags(modifierFlags)
        , fIntrinsicKind(intrinsicKind)
        , fBuiltin(context.fConfig->fIsBuiltinCode)
        , fIsMain(name == "main") {
    int builtinColorIndex = 0;
    for (const Variable* param : fParameters) {
        // None of the parameters are allowed to be be null.
        SkASSERT(param);

        // Keep track of arguments to main for runtime effects.
        if (fIsMain) {
            if (ProgramConfig::IsRuntimeShader(context.fConfig->fKind) ||
                ProgramConfig::IsFragment(context.fConfig->fKind)) {
                // If this is a runtime shader, a float2 param is supposed to be the coords.
                // For testing purposes, we have .sksl inputs that are treated as both runtime
                // effects and fragment shaders. To make that work, fragment shaders are allowed to
                // have a coords parameter as well.
                if (type_is_valid_for_coords(param->type())) {
                    fHasMainCoordsParameter = true;
                }
            } else if (ProgramConfig::IsRuntimeColorFilter(context.fConfig->fKind) ||
                       ProgramConfig::IsRuntimeBlender(context.fConfig->fKind)) {
                // If this is a runtime color filter or blender, the params are an input color,
                // followed by a destination color for blenders.
                if (type_is_valid_for_color(param->type())) {
                    switch (builtinColorIndex++) {
                        case 0:  fHasMainInputColorParameter = true; break;
                        case 1:  fHasMainDestColorParameter = true;  break;
                        default: /* unknown color parameter */       break;
                    }
                }
            }
        }
    }
}

FunctionDeclaration* FunctionDeclaration::Convert(const Context& context,
                                                  Position pos,
                                                  const Modifiers& modifiers,
                                                  std::string_view name,
                                                  TArray<std::unique_ptr<Variable>> parameters,
                                                  Position returnTypePos,
                                                  const Type* returnType) {
    // No layout flag is permissible on a function.
    modifiers.fLayout.checkPermittedLayout(context, pos,
                                           /*permittedLayoutFlags=*/LayoutFlag::kNone);

    // If requested, apply the `noinline` modifier to every function. This allows us to test Runtime
    // Effects without any inlining, even when the code is later added to a paint.
    ModifierFlags modifierFlags = modifiers.fFlags;
    if (context.fConfig->fSettings.fForceNoInline) {
        modifierFlags &= ~ModifierFlag::kInline;
        modifierFlags |= ModifierFlag::kNoInline;
    }

    bool isMain = (name == "main");
    IntrinsicKind intrinsicKind = context.fConfig->fIsBuiltinCode ? FindIntrinsicKind(name)
                                                                  : kNotIntrinsic;
    FunctionDeclaration* decl = nullptr;
    if (!check_modifiers(context, modifiers.fPosition, modifierFlags) ||
        !check_return_type(context, returnTypePos, *returnType) ||
        !check_parameters(context, parameters, modifierFlags, intrinsicKind) ||
        (isMain && !check_main_signature(context, pos, *returnType, parameters)) ||
        !find_existing_declaration(context, pos, modifierFlags, intrinsicKind, name, parameters,
                                   returnTypePos, returnType, &decl)) {
        return nullptr;
    }
    TArray<Variable*> finalParameters;
    finalParameters.reserve_exact(parameters.size());
    for (std::unique_ptr<Variable>& param : parameters) {
        finalParameters.push_back(context.fSymbolTable->takeOwnershipOfSymbol(std::move(param)));
    }
    if (decl) {
        return decl;
    }
    return context.fSymbolTable->add(
            std::make_unique<FunctionDeclaration>(context,
                                                  pos,
                                                  modifierFlags,
                                                  name,
                                                  std::move(finalParameters),
                                                  returnType,
                                                  intrinsicKind));
}

void FunctionDeclaration::addParametersToSymbolTable(const Context& context) {
    for (Variable* param : fParameters) {
        context.fSymbolTable->addWithoutOwnership(param);
    }
}

std::string FunctionDeclaration::mangledName() const {
    if ((this->isBuiltin() && !this->definition()) || this->isMain()) {
        // Builtins without a definition (like `sin` or `sqrt`) must use their real names.
        return std::string(this->name());
    }
    // Built-in functions can have a $ prefix, which will fail to compile in GLSL. Remove the
    // $ and add a unique mangling specifier, so user code can't conflict with the name.
    std::string_view name = this->name();
    const char* builtinMarker = "";
    if (skstd::starts_with(name, '$')) {
        name.remove_prefix(1);
        builtinMarker = "Q";  // a unique, otherwise-unused mangle character
    }
    // Rename function to `funcname_returntypeparamtypes`.
    std::string result = std::string(name) + "_" + builtinMarker +
                         this->returnType().abbreviatedName();
    for (const Variable* p : this->parameters()) {
        result += p->type().abbreviatedName();
    }
    return result;
}

std::string FunctionDeclaration::description() const {
    std::string result = (fModifierFlags ? fModifierFlags.description() + " " : std::string()) +
                         this->returnType().displayName() + " " + std::string(this->name()) + "(";
    auto separator = SkSL::String::Separator();
    for (const Variable* p : this->parameters()) {
        result += separator();
        result += p->description();
    }
    result += ")";
    return result;
}

bool FunctionDeclaration::matches(const FunctionDeclaration& f) const {
    if (this->name() != f.name()) {
        return false;
    }
    SkSpan<Variable* const> parameters = this->parameters();
    SkSpan<Variable* const> otherParameters = f.parameters();
    if (parameters.size() != otherParameters.size()) {
        return false;
    }
    for (size_t i = 0; i < parameters.size(); i++) {
        if (!parameters[i]->type().matches(otherParameters[i]->type())) {
            return false;
        }
    }
    return true;
}

bool FunctionDeclaration::determineFinalTypes(const ExpressionArray& arguments,
                                              ParamTypes* outParameterTypes,
                                              const Type** outReturnType) const {
    SkSpan<Variable* const> parameters = this->parameters();
    SkASSERT(SkToSizeT(arguments.size()) == parameters.size());

    outParameterTypes->reserve_exact(arguments.size());
    int genericIndex = -1;
    for (int i = 0; i < arguments.size(); i++) {
        // Non-generic parameters are final as-is.
        const Type& parameterType = parameters[i]->type();
        if (!parameterType.isGeneric()) {
            outParameterTypes->push_back(&parameterType);
            continue;
        }
        // We use the first generic parameter we find to lock in the generic index;
        // e.g. if we find `float3` here, all `$genType`s will be assumed to be `float3`.
        if (genericIndex == -1) {
            genericIndex = find_generic_index(arguments[i]->type(), parameterType,
                                              /*allowNarrowing=*/true);
            if (genericIndex == -1) {
                // The passed-in type wasn't a match for ANY of the generic possibilities.
                // This function isn't a match at all.
                return false;
            }
        }
        outParameterTypes->push_back(parameterType.coercibleTypes()[genericIndex]);
    }
    // Apply the generic index to our return type.
    const Type& returnType = this->returnType();
    if (returnType.isGeneric()) {
        if (genericIndex == -1) {
            // We don't support functions with a generic return type and no other generics.
            return false;
        }
        *outReturnType = returnType.coercibleTypes()[genericIndex];
    } else {
        *outReturnType = &returnType;
    }
    return true;
}

}  // namespace SkSL
