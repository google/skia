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

static IntrinsicKind identify_intrinsic(const String& functionName) {
    #define SKSL_INTRINSIC(name) {#name, k_##name##_IntrinsicKind},
    static const auto* kAllIntrinsics = new std::unordered_map<String, IntrinsicKind>{
        SKSL_INTRINSIC_LIST
    };
    #undef SKSL_INTRINSIC

    auto iter = kAllIntrinsics->find(functionName);
    if (iter != kAllIntrinsics->end()) {
        return iter->second;
    }

    return kNotIntrinsic;
}

static bool check_modifiers(const Context& context, int offset, const Modifiers& modifiers) {
    IRGenerator::CheckModifiers(
            context,
            offset,
            modifiers,
            Modifiers::kHasSideEffects_Flag | Modifiers::kInline_Flag | Modifiers::kNoInline_Flag,
            /*permittedLayoutFlags=*/0);
    if ((modifiers.fFlags & Modifiers::kInline_Flag) &&
        (modifiers.fFlags & Modifiers::kNoInline_Flag)) {
        context.errors().error(offset, "functions cannot be both 'inline' and 'noinline'");
        return false;
    }
    return true;
}

static bool check_return_type(const Context& context, int offset, const Type& returnType,
                              bool isBuiltin) {
    ErrorReporter& errors = context.errors();
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

static bool check_parameters(const Context& context,
                             std::vector<std::unique_ptr<Variable>>& parameters, bool isMain,
                             bool isBuiltin) {
    auto typeIsValidForColor = [&](const Type& type) {
        return type == *context.fTypes.fHalf4 || type == *context.fTypes.fFloat4;
    };

    // The first color parameter passed to main() is the input color; the second is the dest color.
    static constexpr int kBuiltinColorIDs[] = {SK_INPUT_COLOR_BUILTIN, SK_DEST_COLOR_BUILTIN};
    unsigned int builtinColorIndex = 0;

    // Check modifiers on each function parameter.
    for (auto& param : parameters) {
        IRGenerator::CheckModifiers(context, param->fOffset, param->modifiers(),
                                    Modifiers::kConst_Flag | Modifiers::kIn_Flag |
                                    Modifiers::kOut_Flag, /*permittedLayoutFlags=*/0);
        const Type& type = param->type();
        // Only the (builtin) declarations of 'sample' are allowed to have shader/colorFilter or FP
        // parameters. You can pass other opaque types to functions safely; this restriction is
        // specific to "child" objects.
        if (type.isEffectChild() && !isBuiltin) {
            context.errors().error(param->fOffset, "parameters of type '" + type.displayName() +
                                                   "' not allowed");
            return false;
        }

        Modifiers m = param->modifiers();
        if (isMain) {
            if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
                // We verify that the signature is fully correct later. For now, if this is a
                // runtime effect of any flavor, a float2 param is supposed to be the coords, and a
                // half4/float parameter is supposed to be the input or destination color:
                if (type == *context.fTypes.fFloat2) {
                    m.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
                } else if (typeIsValidForColor(type) &&
                           builtinColorIndex < SK_ARRAY_COUNT(kBuiltinColorIDs)) {
                    m.fLayout.fBuiltin = kBuiltinColorIDs[builtinColorIndex++];
                }
                if (m.fLayout.fBuiltin) {
                    param->setModifiers(context.fModifiersPool->add(m));
                }
            } else if (context.fConfig->fKind == ProgramKind::kFragment) {
                // For testing purposes, we have .sksl inputs that are treated as both runtime
                // effects and fragment shaders. To make that work, fragment shaders are allowed to
                // have a coords parameter. We turn it into sk_FragCoord.
                if (type == *context.fTypes.fFloat2) {
                    m.fLayout.fBuiltin = SK_FRAGCOORD_BUILTIN;
                    param->setModifiers(context.fModifiersPool->add(m));
                }
            }
        }
    }
    return true;
}

static bool check_main_signature(const Context& context, int offset, const Type& returnType,
                                 std::vector<std::unique_ptr<Variable>>& parameters,
                                 bool isBuiltin) {
    ErrorReporter& errors = context.errors();
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

    auto paramIsBuiltinColor = [&](int idx, int builtinID) {
        const Variable& p = *parameters[idx];
        return typeIsValidForColor(p.type()) &&
               p.modifiers().fFlags == 0 &&
               p.modifiers().fLayout.fBuiltin == builtinID;
    };

    auto paramIsInputColor = [&](int n) { return paramIsBuiltinColor(n, SK_INPUT_COLOR_BUILTIN); };
    auto paramIsDestColor  = [&](int n) { return paramIsBuiltinColor(n, SK_DEST_COLOR_BUILTIN); };

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
        case ProgramKind::kRuntimeBlender: {
            // (half4|float4) main(half4|float4, half4|float4)
            if (!typeIsValidForColor(returnType)) {
                errors.error(offset, "'main' must return: 'vec4', 'float4', or 'half4'");
                return false;
            }
            if (!(parameters.size() == 2 &&
                  paramIsInputColor(0) &&
                  paramIsDestColor(1))) {
                errors.error(offset, "'main' parameters must be (vec4|float4|half4, "
                                                                "vec4|float4|half4)");
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
                                      skstd::string_view name,
                                      std::vector<std::unique_ptr<Variable>>& parameters,
                                      const Type* returnType, bool isBuiltin,
                                      const FunctionDeclaration** outExistingDecl) {
    ErrorReporter& errors = context.errors();
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

FunctionDeclaration::FunctionDeclaration(int offset,
                                         const Modifiers* modifiers,
                                         skstd::string_view name,
                                         std::vector<const Variable*> parameters,
                                         const Type* returnType,
                                         bool builtin)
        : INHERITED(offset, kSymbolKind, name, /*type=*/nullptr)
        , fDefinition(nullptr)
        , fModifiers(modifiers)
        , fParameters(std::move(parameters))
        , fReturnType(returnType)
        , fBuiltin(builtin)
        , fIsMain(name == "main")
        , fIntrinsicKind(builtin ? identify_intrinsic(String(name)) : kNotIntrinsic) {}

const FunctionDeclaration* FunctionDeclaration::Convert(const Context& context,
        SymbolTable& symbols, int offset, const Modifiers* modifiers,
        skstd::string_view name, std::vector<std::unique_ptr<Variable>> parameters,
        const Type* returnType, bool isBuiltin) {
    bool isMain = (name == "main");

    const FunctionDeclaration* decl = nullptr;
    if (!check_modifiers(context, offset, *modifiers) ||
        !check_return_type(context, offset, *returnType, isBuiltin) ||
        !check_parameters(context, parameters, isMain, isBuiltin) ||
        (isMain && !check_main_signature(context, offset, *returnType, parameters, isBuiltin)) ||
        !find_existing_declaration(context, symbols, offset, name, parameters, returnType,
                                   isBuiltin, &decl)) {
        return nullptr;
    }
    std::vector<const Variable*> finalParameters;
    finalParameters.reserve(parameters.size());
    for (std::unique_ptr<Variable>& param : parameters) {
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

String FunctionDeclaration::mangledName() const {
    if ((this->isBuiltin() && !this->definition()) || this->isMain()) {
        // Builtins without a definition (like `sin` or `sqrt`) must use their real names.
        return String(this->name());
    }
    // GLSL forbids two underscores in a row; add an extra character if necessary to avoid this.
    const char* splitter = this->name().ends_with("_") ? "x_" : "_";
    // Rename function to `funcname_returntypeparamtypes`.
    String result = this->name() + splitter + this->returnType().abbreviatedName();
    for (const Variable* p : this->parameters()) {
        result += p->type().abbreviatedName();
    }
    return result;
}

String FunctionDeclaration::description() const {
    String result = this->returnType().displayName() + " " + this->name() + "(";
    String separator;
    for (const Variable* p : this->parameters()) {
        result += separator;
        separator = ", ";
        result += p->type().displayName();
        result += " ";
        result += p->name();
    }
    result += ")";
    return result;
}

bool FunctionDeclaration::matches(const FunctionDeclaration& f) const {
    if (this->name() != f.name()) {
        return false;
    }
    const std::vector<const Variable*>& parameters = this->parameters();
    const std::vector<const Variable*>& otherParameters = f.parameters();
    if (parameters.size() != otherParameters.size()) {
        return false;
    }
    for (size_t i = 0; i < parameters.size(); i++) {
        if (parameters[i]->type() != otherParameters[i]->type()) {
            return false;
        }
    }
    return true;
}

bool FunctionDeclaration::determineFinalTypes(const ExpressionArray& arguments,
                                              ParamTypes* outParameterTypes,
                                              const Type** outReturnType) const {
    const std::vector<const Variable*>& parameters = this->parameters();
    SkASSERT(arguments.size() == parameters.size());

    outParameterTypes->reserve_back(arguments.size());
    int genericIndex = -1;
    for (size_t i = 0; i < arguments.size(); i++) {
        // Non-generic parameters are final as-is.
        const Type& parameterType = parameters[i]->type();
        if (parameterType.typeKind() != Type::TypeKind::kGeneric) {
            outParameterTypes->push_back(&parameterType);
            continue;
        }
        // We use the first generic parameter we find to lock in the generic index;
        // e.g. if we find `float3` here, all `$genType`s will be assumed to be `float3`.
        const std::vector<const Type*>& types = parameterType.coercibleTypes();
        if (genericIndex == -1) {
            for (size_t j = 0; j < types.size(); j++) {
                if (arguments[i]->type().canCoerceTo(*types[j], /*allowNarrowing=*/true)) {
                    genericIndex = j;
                    break;
                }
            }
            if (genericIndex == -1) {
                // The passed-in type wasn't a match for ANY of the generic possibilities.
                // This function isn't a match at all.
                return false;
            }
        }
        outParameterTypes->push_back(types[genericIndex]);
    }
    // Apply the generic index to our return type.
    const Type& returnType = this->returnType();
    if (returnType.typeKind() == Type::TypeKind::kGeneric) {
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
