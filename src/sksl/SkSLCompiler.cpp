/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLType.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDSLParser.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExternalFunction.h"
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stdio.h>
#include <utility>

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

#ifdef SK_ENABLE_WGSL_VALIDATION
#include "tint/tint.h"
#endif

#ifdef SKSL_STANDALONE
#define REHYDRATE 0
#include <fstream>
#else
#define REHYDRATE 1
#endif

#if REHYDRATE

// At runtime, we load the dehydrated sksl data files. The data is a (pointer, size) pair.
#include "src/sksl/generated/sksl_shared.dehydrated.sksl"
#include "src/sksl/generated/sksl_compute.dehydrated.sksl"
#include "src/sksl/generated/sksl_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_public.dehydrated.sksl"
#include "src/sksl/generated/sksl_rt_shader.dehydrated.sksl"
#include "src/sksl/generated/sksl_vert.dehydrated.sksl"
#if defined(SK_GRAPHITE_ENABLED)
#include "src/sksl/generated/sksl_graphite_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_graphite_vert.dehydrated.sksl"
#endif

#define MODULE_DATA(name) MakeModuleData(SKSL_INCLUDE_sksl_##name,\
                                         SKSL_INCLUDE_sksl_##name##_LENGTH)

#else

// In standalone mode, we load the textual sksl source files. GN generates or copies these files
// to the skslc executable directory. The "data" in this mode is just the filename.
#define MODULE_DATA(name) MakeModulePath("sksl_" #name ".sksl")

#endif

namespace SkSL {

class ProgramUsage;

// These flags allow tools like Viewer or Nanobench to override the compiler's ProgramSettings.
Compiler::OverrideFlag Compiler::sOptimizer = OverrideFlag::kDefault;
Compiler::OverrideFlag Compiler::sInliner = OverrideFlag::kDefault;

using RefKind = VariableReference::RefKind;

class AutoSource {
public:
    AutoSource(Compiler* compiler, std::string_view source)
            : fCompiler(compiler) {
        SkASSERT(!fCompiler->errorReporter().source().data());
        fCompiler->errorReporter().setSource(source);
    }

    ~AutoSource() {
        fCompiler->errorReporter().setSource(std::string_view());
    }

    Compiler* fCompiler;
};

class AutoProgramConfig {
public:
    AutoProgramConfig(std::shared_ptr<Context>& context, ProgramConfig* config)
            : fContext(context.get())
            , fOldConfig(fContext->fConfig) {
        fContext->fConfig = config;
    }

    ~AutoProgramConfig() {
        fContext->fConfig = fOldConfig;
    }

    Context* fContext;
    ProgramConfig* fOldConfig;
};

class AutoModifiersPool {
public:
    AutoModifiersPool(std::shared_ptr<Context>& context, ModifiersPool* modifiersPool)
            : fContext(context.get()) {
        SkASSERT(!fContext->fModifiersPool);
        fContext->fModifiersPool = modifiersPool;
    }

    ~AutoModifiersPool() {
        fContext->fModifiersPool = nullptr;
    }

    Context* fContext;
};

Compiler::Compiler(const ShaderCaps* caps)
        : fErrorReporter(this)
        , fContext(std::make_shared<Context>(fErrorReporter, *caps, fMangler))
        , fInliner(fContext.get()) {
    SkASSERT(caps);
    fRootModule.fSymbols = this->makeRootSymbolTable();
}

Compiler::~Compiler() {}

#define TYPE(t) &BuiltinTypes::f ## t

using BuiltinTypePtr = const std::unique_ptr<Type> BuiltinTypes::*;

inline static constexpr BuiltinTypePtr kRootTypes[] = {
    TYPE(Void),

    TYPE( Float), TYPE( Float2), TYPE( Float3), TYPE( Float4),
    TYPE(  Half), TYPE(  Half2), TYPE(  Half3), TYPE(  Half4),
    TYPE(   Int), TYPE(   Int2), TYPE(   Int3), TYPE(   Int4),
    TYPE(  UInt), TYPE(  UInt2), TYPE(  UInt3), TYPE(  UInt4),
    TYPE( Short), TYPE( Short2), TYPE( Short3), TYPE( Short4),
    TYPE(UShort), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4),
    TYPE(  Bool), TYPE(  Bool2), TYPE(  Bool3), TYPE(  Bool4),

    TYPE(Float2x2), TYPE(Float2x3), TYPE(Float2x4),
    TYPE(Float3x2), TYPE(Float3x3), TYPE(Float3x4),
    TYPE(Float4x2), TYPE(Float4x3), TYPE(Float4x4),

    TYPE(Half2x2),  TYPE(Half2x3),  TYPE(Half2x4),
    TYPE(Half3x2),  TYPE(Half3x3),  TYPE(Half3x4),
    TYPE(Half4x2),  TYPE(Half4x3),  TYPE(Half4x4),

    TYPE(SquareMat), TYPE(SquareHMat),
    TYPE(Mat),       TYPE(HMat),

    // TODO(skia:12349): generic short/ushort
    TYPE(GenType),   TYPE(GenIType), TYPE(GenUType),
    TYPE(GenHType),   /* (GenSType)      (GenUSType) */
    TYPE(GenBType),
    TYPE(IntLiteral),
    TYPE(FloatLiteral),

    TYPE(Vec),     TYPE(IVec),     TYPE(UVec),
    TYPE(HVec),    TYPE(SVec),     TYPE(USVec),
    TYPE(BVec),

    TYPE(ColorFilter),
    TYPE(Shader),
    TYPE(Blender),
};

inline static constexpr BuiltinTypePtr kPrivateTypes[] = {
    TYPE(Sampler1D), TYPE(Sampler2D), TYPE(Sampler3D),
    TYPE(SamplerExternalOES),
    TYPE(Sampler2DRect),

    TYPE(ISampler2D),
    TYPE(SubpassInput), TYPE(SubpassInputMS),

    TYPE(Sampler),
    TYPE(Texture2D),
};

#undef TYPE

std::shared_ptr<SymbolTable> Compiler::makeRootSymbolTable() {
    auto rootSymbolTable = std::make_shared<SymbolTable>(*fContext, /*builtin=*/true);

    for (BuiltinTypePtr rootType : kRootTypes) {
        rootSymbolTable->addWithoutOwnership((fContext->fTypes.*rootType).get());
    }

    for (BuiltinTypePtr privateType : kPrivateTypes) {
        rootSymbolTable->addWithoutOwnership((fContext->fTypes.*privateType).get());
    }

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    rootSymbolTable->add(std::make_unique<Variable>(/*pos=*/Position(),
                                                    /*modifiersPosition=*/Position(),
                                                    fCoreModifiers.add(Modifiers{}),
                                                    "sk_Caps",
                                                    fContext->fTypes.fSkCaps.get(),
                                                    /*builtin=*/false,
                                                    Variable::Storage::kGlobal));
    return rootSymbolTable;
}

const ParsedModule& Compiler::loadSharedModule() {
    if (!fSharedModule.fSymbols) {
        fSharedModule = this->parseModule(ProgramKind::kFragment, MODULE_DATA(shared),
                                          fRootModule);
    }
    return fSharedModule;
}

const ParsedModule& Compiler::loadGPUModule() {
    if (!fGPUModule.fSymbols) {
        fGPUModule = this->parseModule(ProgramKind::kFragment, MODULE_DATA(gpu),
                                       this->loadSharedModule());
    }
    return fGPUModule;
}

const ParsedModule& Compiler::loadFragmentModule() {
    if (!fFragmentModule.fSymbols) {
        fFragmentModule = this->parseModule(ProgramKind::kFragment, MODULE_DATA(frag),
                                            this->loadGPUModule());
    }
    return fFragmentModule;
}

const ParsedModule& Compiler::loadVertexModule() {
    if (!fVertexModule.fSymbols) {
        fVertexModule = this->parseModule(ProgramKind::kVertex, MODULE_DATA(vert),
                                          this->loadGPUModule());
    }
    return fVertexModule;
}

const ParsedModule& Compiler::loadComputeModule() {
    if (!fComputeModule.fSymbols) {
        fComputeModule = this->parseModule(ProgramKind::kCompute, MODULE_DATA(compute),
                                           this->loadGPUModule());
    }
    return fComputeModule;
}

const ParsedModule& Compiler::loadGraphiteFragmentModule() {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fGraphiteFragmentModule.fSymbols) {
        fGraphiteFragmentModule = this->parseModule(ProgramKind::kGraphiteFragment,
                                                    MODULE_DATA(graphite_frag),
                                                    this->loadFragmentModule());
    }
    return fGraphiteFragmentModule;
#else
    return this->loadFragmentModule();
#endif
}

const ParsedModule& Compiler::loadGraphiteVertexModule() {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fGraphiteVertexModule.fSymbols) {
        fGraphiteVertexModule = this->parseModule(ProgramKind::kGraphiteVertex,
                                                  MODULE_DATA(graphite_vert),
                                                  this->loadVertexModule());
    }
    return fGraphiteVertexModule;
#else
    return this->loadVertexModule();
#endif
}

static void add_public_type_aliases(SkSL::SymbolTable* symbols, const SkSL::BuiltinTypes& types) {
    // Add some aliases to the runtime effect modules so that it's friendlier, and more like GLSL.
    symbols->addWithoutOwnership(types.fVec2.get());
    symbols->addWithoutOwnership(types.fVec3.get());
    symbols->addWithoutOwnership(types.fVec4.get());

    symbols->addWithoutOwnership(types.fIVec2.get());
    symbols->addWithoutOwnership(types.fIVec3.get());
    symbols->addWithoutOwnership(types.fIVec4.get());

    symbols->addWithoutOwnership(types.fBVec2.get());
    symbols->addWithoutOwnership(types.fBVec3.get());
    symbols->addWithoutOwnership(types.fBVec4.get());

    symbols->addWithoutOwnership(types.fMat2.get());
    symbols->addWithoutOwnership(types.fMat3.get());
    symbols->addWithoutOwnership(types.fMat4.get());

    symbols->addWithoutOwnership(types.fMat2x2.get());
    symbols->addWithoutOwnership(types.fMat2x3.get());
    symbols->addWithoutOwnership(types.fMat2x4.get());
    symbols->addWithoutOwnership(types.fMat3x2.get());
    symbols->addWithoutOwnership(types.fMat3x3.get());
    symbols->addWithoutOwnership(types.fMat3x4.get());
    symbols->addWithoutOwnership(types.fMat4x2.get());
    symbols->addWithoutOwnership(types.fMat4x3.get());
    symbols->addWithoutOwnership(types.fMat4x4.get());

    // Hide all the private symbols by aliasing them all to "invalid". This will prevent code from
    // using built-in names like `sampler2D` as variable names.
    for (BuiltinTypePtr privateType : kPrivateTypes) {
        symbols->add(Type::MakeAliasType((types.*privateType)->name(), *types.fInvalid));
    }
    symbols->add(Type::MakeAliasType("sk_Caps", *types.fInvalid));
}

std::shared_ptr<SymbolTable> Compiler::makeRootSymbolTableWithPublicTypes() {
    auto result = this->makeRootSymbolTable();
    add_public_type_aliases(result.get(), fContext->fTypes);
    return result;
}

const ParsedModule& Compiler::loadPublicModule() {
    if (!fPublicModule.fSymbols) {
        fPublicModule = this->parseModule(ProgramKind::kGeneric, MODULE_DATA(public),
                                          this->loadSharedModule());
        add_public_type_aliases(fPublicModule.fSymbols.get(), fContext->fTypes);
    }
    return fPublicModule;
}

const ParsedModule& Compiler::loadPrivateRTShaderModule() {
    if (!fRuntimeShaderModule.fSymbols) {
        fRuntimeShaderModule = this->parseModule(
                ProgramKind::kRuntimeShader, MODULE_DATA(rt_shader), this->loadPublicModule());
    }
    return fRuntimeShaderModule;
}

const ParsedModule& Compiler::moduleForProgramKind(ProgramKind kind) {
    switch (kind) {
        case ProgramKind::kVertex:               return this->loadVertexModule();           break;
        case ProgramKind::kFragment:             return this->loadFragmentModule();         break;
        case ProgramKind::kCompute:              return this->loadComputeModule();          break;
        case ProgramKind::kGraphiteVertex:       return this->loadGraphiteVertexModule();   break;
        case ProgramKind::kGraphiteFragment:     return this->loadGraphiteFragmentModule(); break;
        case ProgramKind::kRuntimeColorFilter:   return this->loadPublicModule();           break;
        case ProgramKind::kRuntimeShader:        return this->loadPublicModule();           break;
        case ProgramKind::kRuntimeBlender:       return this->loadPublicModule();           break;
        case ProgramKind::kPrivateRuntimeShader: return this->loadPrivateRTShaderModule();  break;
        case ProgramKind::kMeshVertex:           return this->loadPublicModule();           break;
        case ProgramKind::kMeshFragment:         return this->loadPublicModule();           break;
        case ProgramKind::kGeneric:              return this->loadPublicModule();           break;
    }
    SkUNREACHABLE;
}

LoadedModule Compiler::loadModule(ProgramKind kind,
                                  ModuleData data,
                                  std::shared_ptr<SymbolTable> base,
                                  bool dehydrate) {
    if (dehydrate) {
        // sksl-precompile passes `true` when dehydrating the lowest-level module to indicate that
        // we should use the root module. Child modules that depend on the earlier module will pass
        // `false` and pass the lower-level module's symbol table in `base`.
        SkASSERT(base == nullptr);
        base = fRootModule.fSymbols;
    }
    SkASSERT(base);

    // Put the core-module modifier pool into the context.
    AutoModifiersPool autoPool(fContext, &fCoreModifiers);

    // Built-in modules always use default program settings.
    ProgramSettings settings;
    settings.fReplaceSettings = !dehydrate;

#if REHYDRATE
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = kind;
    config.fSettings = settings;
    AutoProgramConfig autoConfig(fContext, &config);
    SkASSERT(data.fData && (data.fSize != 0));
    Rehydrator rehydrator(*this, data.fData, data.fSize, std::move(base));
    LoadedModule module = {kind, rehydrator.symbolTable(), rehydrator.elements()};
#else
    SkASSERT(this->errorCount() == 0);
    SkASSERT(data.fPath);
    std::ifstream in(data.fPath);
    std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading %s\n", data.fPath);
        abort();
    }
    ParsedModule baseModule = {std::move(base), /*fElements=*/nullptr};
    LoadedModule module = DSLParser(this, settings, kind, std::move(text))
                                  .moduleInheritingFrom(baseModule);
    if (this->errorCount()) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
    this->optimizeModuleForDehydration(module, baseModule);
#endif

    return module;
}

ParsedModule Compiler::parseModule(ProgramKind kind, ModuleData data, const ParsedModule& base) {
    LoadedModule module = this->loadModule(kind, data, base.fSymbols, /*dehydrate=*/false);
    this->optimizeRehydratedModule(module, base);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's element map:
    if (module.fElements.empty()) {
        return ParsedModule{module.fSymbols, base.fElements};
    }

    auto elements = std::make_shared<BuiltinMap>(base.fElements.get());

    // Now, transfer all of the program elements to a builtin element map. This maps certain types
    // of global objects to the declaring ProgramElement.
    for (std::unique_ptr<ProgramElement>& element : module.fElements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDefinition& f = element->as<FunctionDefinition>();
                SkASSERT(f.declaration().isBuiltin());
                elements->insertOrDie(f.declaration().description(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kFunctionPrototype: {
                // These are already in the symbol table.
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
                const Variable& var = global.declaration()->as<VarDeclaration>().var();
                SkASSERT(var.isBuiltin());
                elements->insertOrDie(std::string(var.name()), std::move(element));
                break;
            }
            case ProgramElement::Kind::kInterfaceBlock: {
                const Variable& var = element->as<InterfaceBlock>().variable();
                SkASSERT(var.isBuiltin());
                elements->insertOrDie(std::string(var.name()), std::move(element));
                break;
            }
            default:
                printf("Unsupported element: %s\n", element->description().c_str());
                SkASSERT(false);
                break;
        }
    }

    return ParsedModule{module.fSymbols, std::move(elements)};
}

std::unique_ptr<Program> Compiler::convertProgram(ProgramKind kind,
                                                  std::string text,
                                                  ProgramSettings settings) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::convertProgram");

    SkASSERT(!settings.fExternalFunctions || (kind == ProgramKind::kGeneric));

    // Honor our optimization-override flags.
    switch (sOptimizer) {
        case OverrideFlag::kDefault:
            break;
        case OverrideFlag::kOff:
            settings.fOptimize = false;
            break;
        case OverrideFlag::kOn:
            settings.fOptimize = true;
            break;
    }

    switch (sInliner) {
        case OverrideFlag::kDefault:
            break;
        case OverrideFlag::kOff:
            settings.fInlineThreshold = 0;
            break;
        case OverrideFlag::kOn:
            if (settings.fInlineThreshold == 0) {
                settings.fInlineThreshold = kDefaultInlineThreshold;
            }
            break;
    }

    // Disable optimization settings that depend on a parent setting which has been disabled.
    settings.fInlineThreshold *= (int)settings.fOptimize;
    settings.fRemoveDeadFunctions &= settings.fOptimize;
    settings.fRemoveDeadVariables &= settings.fOptimize;

    // For "generic" interpreter programs, leave all functions intact. (The API supports calling
    // any function, not just 'main').
    if (kind == ProgramKind::kGeneric) {
        settings.fRemoveDeadFunctions = false;
    }

    // Runtime effects always allow narrowing conversions.
    if (ProgramConfig::IsRuntimeEffect(kind)) {
        settings.fAllowNarrowingConversions = true;
    }

    this->resetErrors();
    fInliner.reset();

    return DSLParser(this, settings, kind, std::move(text)).program();
}

void Compiler::updateInputsForBuiltinVariable(const Variable& var) {
    switch (var.modifiers().fLayout.fBuiltin) {
        case SK_FRAGCOORD_BUILTIN:
            if (fContext->fCaps.fCanUseFragCoord) {
                ThreadContext::Inputs().fUseFlipRTUniform =
                        !fContext->fConfig->fSettings.fForceNoRTFlip;
            }
            break;
        case SK_CLOCKWISE_BUILTIN:
            ThreadContext::Inputs().fUseFlipRTUniform =
                    !fContext->fConfig->fSettings.fForceNoRTFlip;
            break;
    }
}

std::unique_ptr<Expression> Compiler::convertIdentifier(Position pos, std::string_view name) {
    const Symbol* result = (*fSymbolTable)[name];
    if (!result) {
        this->errorReporter().error(pos, "unknown identifier '" + std::string(name) + "'");
        return nullptr;
    }
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration: {
            std::vector<const FunctionDeclaration*> f = {
                &result->as<FunctionDeclaration>()
            };
            return std::make_unique<FunctionReference>(*fContext, pos, f);
        }
        case Symbol::Kind::kUnresolvedFunction: {
            const UnresolvedFunction* f = &result->as<UnresolvedFunction>();
            return std::make_unique<FunctionReference>(*fContext, pos, f->functions());
        }
        case Symbol::Kind::kVariable: {
            const Variable* var = &result->as<Variable>();
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return VariableReference::Make(pos, var, VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const Field* field = &result->as<Field>();
            auto base = VariableReference::Make(pos, &field->owner(),
                                                VariableReference::RefKind::kRead);
            return FieldAccess::Make(*fContext, pos, std::move(base), field->fieldIndex(),
                                     FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType: {
            // go through DSLType so we report errors on private types
            dsl::DSLModifiers modifiers;
            dsl::DSLType dslType(result->name(), &modifiers, pos);
            return TypeReference::Convert(*fContext, pos, &dslType.skslType());
        }
        case Symbol::Kind::kExternal: {
            const ExternalFunction* r = &result->as<ExternalFunction>();
            return std::make_unique<ExternalFunctionReference>(pos, r);
        }
        default:
            SK_ABORT("unsupported symbol type %d\n", (int) result->kind());
    }
}

bool Compiler::optimizeRehydratedModule(LoadedModule& module, const ParsedModule& base) {
    SkASSERT(!this->errorCount());

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = module.fKind;
    AutoProgramConfig autoConfig(fContext, &config);
    AutoModifiersPool autoPool(fContext, &fCoreModifiers);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module, base);

    // Perform inline-candidate analysis and inline any functions deemed suitable.
    fInliner.reset();
    while (this->errorCount() == 0) {
        if (!this->runInliner(module.fElements, module.fSymbols, usage.get())) {
            break;
        }
    }
    return this->errorCount() == 0;
}

bool Compiler::optimizeModuleForDehydration(LoadedModule& module, const ParsedModule& base) {
    SkASSERT(!this->errorCount());

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = module.fKind;
    AutoProgramConfig autoConfig(fContext, &config);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module, base);

    // Remove any unreachable code.
    Transform::EliminateUnreachableCode(module, usage.get());

    while (Transform::EliminateDeadLocalVariables(*fContext, module, usage.get())) {
        // Removing dead variables may cause more variables to become unreferenced. Try again.
    }

    // Save space by eliminating empty statements from the code.
    Transform::EliminateEmptyStatements(module);

    // Note that we intentionally don't attempt to eliminate unreferenced global variables or
    // functions here, since those can be referenced by the finished program even if they're
    // unreferenced now. We also don't run the inliner to avoid growing the program; that is done in
    // `optimizeRehydratedModule` above.

    return this->errorCount() == 0;
}

bool Compiler::optimize(Program& program) {
    // The optimizer only needs to run when it is enabled.
    if (!program.fConfig->fSettings.fOptimize) {
        return true;
    }

    SkASSERT(!this->errorCount());
    ProgramUsage* usage = program.fUsage.get();

    if (this->errorCount() == 0) {
        // Run the inliner only once; it is expensive! Multiple passes can occasionally shake out
        // more wins, but it's diminishing returns.
        fInliner.reset();
        this->runInliner(program.fOwnedElements, program.fSymbols, usage);

        // Unreachable code can confuse some drivers, so it's worth removing. (skia:12012)
        Transform::EliminateUnreachableCode(program);

        while (Transform::EliminateDeadFunctions(program)) {
            // Removing dead functions may cause more functions to become unreferenced. Try again.
        }
        while (Transform::EliminateDeadLocalVariables(program)) {
            // Removing dead variables may cause more variables to become unreferenced. Try again.
        }

        Transform::EliminateDeadGlobalVariables(program);
    }

    return this->errorCount() == 0;
}

bool Compiler::runInliner(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                          std::shared_ptr<SymbolTable> symbols,
                          ProgramUsage* usage) {
    // The program's SymbolTable was taken out of fSymbolTable when the program was bundled, but
    // the inliner relies (indirectly) on having a valid SymbolTable.
    // In particular, inlining can turn a non-optimizable expression like `normalize(myVec)` into
    // `normalize(vec2(7))`, which is now optimizable. The optimizer can use DSL to simplify this
    // expression--e.g., in the case of normalize, using DSL's Length(). The DSL relies on
    // convertIdentifier() to look up `length`. convertIdentifier() needs a valid symbol table to
    // find the declaration of `length`. To allow this chain of events to succeed, we re-insert the
    // program's symbol table temporarily.
    SkASSERT(!fSymbolTable);
    fSymbolTable = symbols;

    bool result = fInliner.analyze(elements, symbols, usage);

    fSymbolTable = nullptr;
    return result;
}

bool Compiler::finalize(Program& program) {
    // Do one last correctness-check pass. This looks for @if/@switch statements that didn't
    // optimize away, or dangling FunctionReference or TypeReference expressions, and reports them
    // as errors.
    Analysis::DoFinalizationChecks(program);

    if (fContext->fConfig->strictES2Mode() && this->errorCount() == 0) {
        // Enforce Appendix A, Section 5 of the GLSL ES 1.00 spec -- Indexing. This logic assumes
        // that all loops meet the criteria of Section 4, and if they don't, could crash.
        for (const auto& pe : program.fOwnedElements) {
            Analysis::ValidateIndexingForES2(*pe, this->errorReporter());
        }
    }
    if (this->errorCount() == 0) {
        bool enforceSizeLimit = ProgramConfig::IsRuntimeEffect(program.fConfig->fKind);
        Analysis::CheckProgramStructure(program, enforceSizeLimit);
    }

    return this->errorCount() == 0;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)

#if defined(SK_ENABLE_SPIRV_VALIDATION)
static bool validate_spirv(ErrorReporter& reporter, std::string_view program) {
    SkASSERT(0 == program.size() % 4);
    const uint32_t* programData = reinterpret_cast<const uint32_t*>(program.data());
    size_t programSize = program.size() / 4;

    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
    std::string errors;
    auto msgFn = [&errors](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
        String::appendf(&errors, "SPIR-V validation error: %s\n", m);
    };
    tools.SetMessageConsumer(msgFn);

    // Verify that the SPIR-V we produced is valid. At runtime, we will abort() with a message
    // explaining the error. In standalone mode (skslc), we will send the message, plus the
    // entire disassembled SPIR-V (for easier context & debugging) as *our* error message.
    bool result = tools.Validate(programData, programSize);
    if (!result) {
#if defined(SKSL_STANDALONE)
        // Convert the string-stream to a SPIR-V disassembly.
        std::string disassembly;
        if (tools.Disassemble(programData, programSize, &disassembly)) {
            errors.append(disassembly);
        }
        reporter.error(Position(), errors);
#else
        SkDEBUGFAILF("%s", errors.c_str());
#endif
    }
    return result;
}
#endif

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toSPIRV");
    AutoSource as(this, *program.fSource);
    ProgramSettings settings;
    settings.fDSLUseMemoryPool = false;
    dsl::Start(this, program.fConfig->fKind, settings);
    dsl::SetErrorReporter(&fErrorReporter);
    fSymbolTable = program.fSymbols;
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    SPIRVCodeGenerator cg(fContext.get(), &program, &buffer);
    bool result = cg.generateCode();

    if (result && program.fConfig->fSettings.fValidateSPIRV) {
        std::string_view binary = buffer.str();
        result = validate_spirv(this->errorReporter(), binary);
        out.write(binary.data(), binary.size());
    }
#else
    SPIRVCodeGenerator cg(fContext.get(), &program, &out);
    bool result = cg.generateCode();
#endif
    dsl::End();
    return result;
}

bool Compiler::toSPIRV(Program& program, std::string* out) {
    StringStream buffer;
    bool result = this->toSPIRV(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toGLSL(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toGLSL");
    AutoSource as(this, *program.fSource);
    GLSLCodeGenerator cg(fContext.get(), &program, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toGLSL(Program& program, std::string* out) {
    StringStream buffer;
    bool result = this->toGLSL(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toHLSL(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toHLSL");
    std::string hlsl;
    if (!this->toHLSL(program, &hlsl)) {
        return false;
    }
    out.writeString(hlsl);
    return true;
}

bool Compiler::toHLSL(Program& program, std::string* out) {
    std::string spirv;
    if (!this->toSPIRV(program, &spirv)) {
        return false;
    }

    if (!SPIRVtoHLSL(spirv, out)) {
        fErrorText += "HLSL cross-compilation not enabled";
        return false;
    }

    return true;
}

bool Compiler::toMetal(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toMetal");
    AutoSource as(this, *program.fSource);
    MetalCodeGenerator cg(fContext.get(), &program, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toMetal(Program& program, std::string* out) {
    StringStream buffer;
    bool result = this->toMetal(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

#if defined(SK_ENABLE_WGSL_VALIDATION)
static bool validate_wgsl(ErrorReporter& reporter, const std::string& wgsl) {
    tint::Source::File srcFile("", wgsl);
    tint::Program program(tint::reader::wgsl::Parse(&srcFile));
    if (program.Diagnostics().count() > 0) {
        tint::diag::Formatter diagFormatter;
        std::string diagOutput = diagFormatter.format(program.Diagnostics());
#if defined(SKSL_STANDALONE)
        reporter.error(Position(), diagOutput);
#else
        SkDEBUGFAILF("%s", diagOutput.c_str());
#endif
        return false;
    }
    return true;
}
#endif  // defined(SK_ENABLE_WGSL_VALIDATION)

bool Compiler::toWGSL(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toWGSL");
    AutoSource as(this, *program.fSource);
#ifdef SK_ENABLE_WGSL_VALIDATION
    StringStream wgsl;
    WGSLCodeGenerator cg(fContext.get(), &program, &wgsl);
    bool result = cg.generateCode();
    if (result) {
        std::string wgslString = wgsl.str();
        result = validate_wgsl(this->errorReporter(), wgslString);
        out.writeString(wgslString);
    }
#else
    WGSLCodeGenerator cg(fContext.get(), &program, &out);
    bool result = cg.generateCode();
#endif
    return result;
}

#endif // defined(SKSL_STANDALONE) || SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)

void Compiler::handleError(std::string_view msg, Position pos) {
    fErrorText += "error: ";
    bool printLocation = false;
    std::string_view src = this->errorReporter().source();
    int line = -1;
    if (pos.valid()) {
        line = pos.line(src);
        printLocation = pos.startOffset() < (int)src.length();
        fErrorText += std::to_string(line) + ": ";
    }
    fErrorText += std::string(msg) + "\n";
    if (printLocation) {
        // Find the beginning of the line
        int lineStart = pos.startOffset();
        while (lineStart > 0) {
            if (src[lineStart - 1] == '\n') {
                break;
            }
            --lineStart;
        }

        // echo the line
        for (int i = lineStart; i < (int)src.length(); i++) {
            switch (src[i]) {
                case '\t': fErrorText += "    "; break;
                case '\0': fErrorText += " ";    break;
                case '\n': i = src.length();     break;
                default:   fErrorText += src[i]; break;
            }
        }
        fErrorText += '\n';

        // print the carets underneath it, pointing to the range in question
        for (int i = lineStart; i < (int)src.length(); i++) {
            if (i >= pos.endOffset()) {
                break;
            }
            switch (src[i]) {
                case '\t':
                   fErrorText += (i >= pos.startOffset()) ? "^^^^" : "    ";
                   break;
                case '\n':
                    SkASSERT(i >= pos.startOffset());
                    // use an ellipsis if the error continues past the end of the line
                    fErrorText += (pos.endOffset() > i + 1) ? "..." : "^";
                    i = src.length();
                    break;
                default:
                    fErrorText += (i >= pos.startOffset()) ? '^' : ' ';
                    break;
            }
        }
        fErrorText += '\n';
    }
}

std::string Compiler::errorText(bool showCount) {
    if (showCount) {
        this->writeErrorCount();
    }
    std::string result = fErrorText;
    this->resetErrors();
    return result;
}

void Compiler::writeErrorCount() {
    int count = this->errorCount();
    if (count) {
        fErrorText += std::to_string(count) + " error";
        if (count > 1) {
            fErrorText += "s";
        }
        fErrorText += "\n";
    }
}

}  // namespace SkSL
