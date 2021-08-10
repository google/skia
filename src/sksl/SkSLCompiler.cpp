/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include <memory>
#include <unordered_set>

#include "include/sksl/DSLCore.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLDSLParser.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/DSL_priv.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/utils/SkBitSet.h"

#include <fstream>

#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrShaderCaps.h"
#endif

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

#if defined(SKSL_STANDALONE)

// In standalone mode, we load the textual sksl source files. GN generates or copies these files
// to the skslc executable directory. The "data" in this mode is just the filename.
#define MODULE_DATA(name) MakeModulePath("sksl_" #name ".sksl")

#else

// At runtime, we load the dehydrated sksl data files. The data is a (pointer, size) pair.
#include "src/sksl/generated/sksl_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_geom.dehydrated.sksl"
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_public.dehydrated.sksl"
#include "src/sksl/generated/sksl_rt_blend.dehydrated.sksl"
#include "src/sksl/generated/sksl_rt_colorfilter.dehydrated.sksl"
#include "src/sksl/generated/sksl_rt_shader.dehydrated.sksl"
#include "src/sksl/generated/sksl_vert.dehydrated.sksl"

#define MODULE_DATA(name) MakeModuleData(SKSL_INCLUDE_sksl_##name,\
                                         SKSL_INCLUDE_sksl_##name##_LENGTH)

#endif

namespace SkSL {

// These flags allow tools like Viewer or Nanobench to override the compiler's ProgramSettings.
Compiler::OverrideFlag Compiler::sOptimizer = OverrideFlag::kDefault;
Compiler::OverrideFlag Compiler::sInliner = OverrideFlag::kDefault;

using RefKind = VariableReference::RefKind;

class AutoSource {
public:
    AutoSource(Compiler* compiler, const char* source)
            : fCompiler(compiler) {
        SkASSERT(!fCompiler->fSource);
        fCompiler->fSource = source;
    }

    ~AutoSource() {
        fCompiler->fSource = nullptr;
    }

    Compiler* fCompiler;
};

class AutoProgramConfig {
public:
    AutoProgramConfig(std::shared_ptr<Context>& context, ProgramConfig* config)
            : fContext(context.get()) {
        SkASSERT(!fContext->fConfig);
        fContext->fConfig = config;
    }

    ~AutoProgramConfig() {
        fContext->fConfig = nullptr;
    }

    Context* fContext;
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

Compiler::Compiler(const ShaderCapsClass* caps)
        : fContext(std::make_shared<Context>(/*errors=*/*this, *caps))
        , fInliner(fContext.get()) {
    SkASSERT(caps);
    fRootSymbolTable = std::make_shared<SymbolTable>(this, /*builtin=*/true);
    fPrivateSymbolTable = std::make_shared<SymbolTable>(fRootSymbolTable, /*builtin=*/true);
    fIRGenerator = std::make_unique<IRGenerator>(fContext.get());

#define TYPE(t) fContext->fTypes.f ## t .get()

    const SkSL::Symbol* rootTypes[] = {
        TYPE(Void),

        TYPE( Float), TYPE( Float2), TYPE( Float3), TYPE( Float4),
        TYPE(  Half), TYPE(  Half2), TYPE(  Half3), TYPE(  Half4),
        TYPE(   Int), TYPE(   Int2), TYPE(   Int3), TYPE(   Int4),
        TYPE(  Bool), TYPE(  Bool2), TYPE(  Bool3), TYPE(  Bool4),

        TYPE(Float2x2), TYPE(Float3x3), TYPE(Float4x4),
        TYPE( Half2x2), TYPE( Half3x3), TYPE(Half4x4),

        TYPE(SquareMat), TYPE(SquareHMat),

        TYPE(GenType), TYPE(GenHType), TYPE(GenIType), TYPE(GenBType),
        TYPE(Vec),     TYPE(HVec),     TYPE(IVec),     TYPE(BVec),

        TYPE(ColorFilter),
        TYPE(Shader),
        TYPE(Blender),
    };

    const SkSL::Symbol* privateTypes[] = {
        TYPE(  UInt), TYPE(  UInt2), TYPE(  UInt3), TYPE(  UInt4),
        TYPE( Short), TYPE( Short2), TYPE( Short3), TYPE( Short4),
        TYPE(UShort), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4),

        TYPE(GenUType), TYPE(UVec),
        TYPE(SVec), TYPE(USVec),

        TYPE(Float2x3), TYPE(Float2x4),
        TYPE(Float3x2), TYPE(Float3x4),
        TYPE(Float4x2), TYPE(Float4x3),

        TYPE(Half2x3),  TYPE(Half2x4),
        TYPE(Half3x2),  TYPE(Half3x4),
        TYPE(Half4x2),  TYPE(Half4x3),

        TYPE(Mat), TYPE(HMat),

        TYPE(Sampler1D), TYPE(Sampler2D), TYPE(Sampler3D),
        TYPE(SamplerExternalOES),
        TYPE(Sampler2DRect),

        TYPE(ISampler2D),
        TYPE(SubpassInput), TYPE(SubpassInputMS),

        TYPE(Sampler),
        TYPE(Texture2D),
    };

    for (const SkSL::Symbol* type : rootTypes) {
        fRootSymbolTable->addWithoutOwnership(type);
    }
    for (const SkSL::Symbol* type : privateTypes) {
        fPrivateSymbolTable->addWithoutOwnership(type);
    }

#undef TYPE

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    fPrivateSymbolTable->add(std::make_unique<Variable>(/*offset=*/-1,
                                                        fCoreModifiers.add(Modifiers{}),
                                                        "sk_Caps",
                                                        fContext->fTypes.fSkCaps.get(),
                                                        /*builtin=*/false,
                                                        Variable::Storage::kGlobal));

    fRootModule = {fRootSymbolTable, /*fIntrinsics=*/nullptr};
    fPrivateModule = {fPrivateSymbolTable, /*fIntrinsics=*/nullptr};
}

Compiler::~Compiler() {}

const ParsedModule& Compiler::loadGPUModule() {
    if (!fGPUModule.fSymbols) {
        fGPUModule = this->parseModule(ProgramKind::kFragment, MODULE_DATA(gpu), fPrivateModule);
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

const ParsedModule& Compiler::loadGeometryModule() {
    if (!fGeometryModule.fSymbols) {
        fGeometryModule = this->parseModule(ProgramKind::kGeometry, MODULE_DATA(geom),
                                            this->loadGPUModule());
    }
    return fGeometryModule;
}

const ParsedModule& Compiler::loadPublicModule() {
    if (!fPublicModule.fSymbols) {
        fPublicModule = this->parseModule(ProgramKind::kGeneric, MODULE_DATA(public), fRootModule);
    }
    return fPublicModule;
}

static void add_glsl_type_aliases(SkSL::SymbolTable* symbols, const SkSL::BuiltinTypes& types) {
    // Add some aliases to the runtime effect modules so that it's friendlier, and more like GLSL
    symbols->addAlias("vec2", types.fFloat2.get());
    symbols->addAlias("vec3", types.fFloat3.get());
    symbols->addAlias("vec4", types.fFloat4.get());

    symbols->addAlias("ivec2", types.fInt2.get());
    symbols->addAlias("ivec3", types.fInt3.get());
    symbols->addAlias("ivec4", types.fInt4.get());

    symbols->addAlias("bvec2", types.fBool2.get());
    symbols->addAlias("bvec3", types.fBool3.get());
    symbols->addAlias("bvec4", types.fBool4.get());

    symbols->addAlias("mat2", types.fFloat2x2.get());
    symbols->addAlias("mat3", types.fFloat3x3.get());
    symbols->addAlias("mat4", types.fFloat4x4.get());
}

const ParsedModule& Compiler::loadRuntimeColorFilterModule() {
    if (!fRuntimeColorFilterModule.fSymbols) {
        fRuntimeColorFilterModule = this->parseModule(ProgramKind::kRuntimeColorFilter,
                                                      MODULE_DATA(rt_colorfilter),
                                                      this->loadPublicModule());
        add_glsl_type_aliases(fRuntimeColorFilterModule.fSymbols.get(), fContext->fTypes);
    }
    return fRuntimeColorFilterModule;
}

const ParsedModule& Compiler::loadRuntimeShaderModule() {
    if (!fRuntimeShaderModule.fSymbols) {
        fRuntimeShaderModule = this->parseModule(
                ProgramKind::kRuntimeShader, MODULE_DATA(rt_shader), this->loadPublicModule());
        add_glsl_type_aliases(fRuntimeShaderModule.fSymbols.get(), fContext->fTypes);
    }
    return fRuntimeShaderModule;
}

const ParsedModule& Compiler::loadRuntimeBlenderModule() {
    if (!fRuntimeBlenderModule.fSymbols) {
        fRuntimeBlenderModule = this->parseModule(
                ProgramKind::kRuntimeBlender, MODULE_DATA(rt_blend), this->loadPublicModule());
        add_glsl_type_aliases(fRuntimeBlenderModule.fSymbols.get(), fContext->fTypes);
    }
    return fRuntimeBlenderModule;
}

const ParsedModule& Compiler::moduleForProgramKind(ProgramKind kind) {
    switch (kind) {
        case ProgramKind::kVertex:             return this->loadVertexModule();             break;
        case ProgramKind::kFragment:           return this->loadFragmentModule();           break;
        case ProgramKind::kGeometry:           return this->loadGeometryModule();           break;
        case ProgramKind::kRuntimeColorFilter: return this->loadRuntimeColorFilterModule(); break;
        case ProgramKind::kRuntimeShader:      return this->loadRuntimeShaderModule();      break;
        case ProgramKind::kRuntimeBlender:     return this->loadRuntimeBlenderModule();     break;
        case ProgramKind::kGeneric:            return this->loadPublicModule();             break;
    }
    SkUNREACHABLE;
}

LoadedModule Compiler::loadModule(ProgramKind kind,
                                  ModuleData data,
                                  std::shared_ptr<SymbolTable> base,
                                  bool dehydrate) {
    if (dehydrate) {
        // NOTE: This is a workaround. When dehydrating includes, skslc doesn't know which module
        // it's preparing, nor what the correct base module is. We can't use 'Root', because many
        // GPU intrinsics reference private types, like samplers or textures. Today, 'Private' does
        // contain the union of all known types, so this is safe. If we ever have types that only
        // exist in 'Public' (for example), this logic needs to be smarter (by choosing the correct
        // base for the module we're compiling).
        base = fPrivateSymbolTable;
    }
    SkASSERT(base);

    // Put the core-module modifier pool into the context.
    AutoModifiersPool autoPool(fContext, &fCoreModifiers);

    // Built-in modules always use default program settings.
    Program::Settings settings;
    settings.fReplaceSettings = !dehydrate;

#if defined(SKSL_STANDALONE)
    SkASSERT(data.fPath);
    std::ifstream in(data.fPath);
    String text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading %s\n", data.fPath);
        abort();
    }
    const String* source = fRootSymbolTable->takeOwnershipOfString(std::move(text));

    ParsedModule baseModule = {base, /*fIntrinsics=*/nullptr};
    std::vector<std::unique_ptr<ProgramElement>> elements;
    std::vector<const ProgramElement*> sharedElements;
    dsl::StartModule(this, kind, settings, baseModule);
    AutoSource as(this, source->c_str());
    IRGenerator::IRBundle ir = fIRGenerator->convertProgram(baseModule, /*isBuiltinCode=*/true,
                                                            *source);
    SkASSERT(ir.fSharedElements.empty());
    LoadedModule module = { kind, std::move(ir.fSymbolTable), std::move(ir.fElements) };
    dsl::End();
    if (this->fErrorCount) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
#else
    ProgramConfig config;
    config.fKind = kind;
    config.fSettings = settings;
    AutoProgramConfig autoConfig(fContext, &config);
    SkASSERT(data.fData && (data.fSize != 0));
    Rehydrator rehydrator(fContext.get(), base, data.fData, data.fSize);
    LoadedModule module = { kind, rehydrator.symbolTable(), rehydrator.elements() };
#endif

    return module;
}

ParsedModule Compiler::parseModule(ProgramKind kind, ModuleData data, const ParsedModule& base) {
    LoadedModule module = this->loadModule(kind, data, base.fSymbols, /*dehydrate=*/false);
    this->optimize(module);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's intrinsic map:
    if (module.fElements.empty()) {
        return ParsedModule{module.fSymbols, base.fIntrinsics};
    }

    auto intrinsics = std::make_shared<IRIntrinsicMap>(base.fIntrinsics.get());

    // Now, transfer all of the program elements to an intrinsic map. This maps certain types of
    // global objects to the declaring ProgramElement.
    for (std::unique_ptr<ProgramElement>& element : module.fElements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDefinition& f = element->as<FunctionDefinition>();
                SkASSERT(f.declaration().isBuiltin());
                intrinsics->insertOrDie(f.declaration().description(), std::move(element));
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
                intrinsics->insertOrDie(String(var.name()), std::move(element));
                break;
            }
            case ProgramElement::Kind::kInterfaceBlock: {
                const Variable& var = element->as<InterfaceBlock>().variable();
                SkASSERT(var.isBuiltin());
                intrinsics->insertOrDie(String(var.name()), std::move(element));
                break;
            }
            default:
                printf("Unsupported element: %s\n", element->description().c_str());
                SkASSERT(false);
                break;
        }
    }

    return ParsedModule{module.fSymbols, std::move(intrinsics)};
}

std::unique_ptr<Program> Compiler::convertProgram(
        ProgramKind kind,
        String text,
        Program::Settings settings) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::convertProgram");

    SkASSERT(!settings.fExternalFunctions || (kind == ProgramKind::kGeneric));

#if !SKSL_DSL_PARSER
    // Loading and optimizing our base module might reset the inliner, so do that first,
    // *then* configure the inliner with the settings for this program.
    const ParsedModule& baseModule = this->moduleForProgramKind(kind);
#endif

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

    // Runtime effects always allow narrowing conversions.
    if (ProgramConfig::IsRuntimeEffect(kind)) {
        settings.fAllowNarrowingConversions = true;
    }

    fErrorText = "";
    fErrorCount = 0;
    fInliner.reset();

#if SKSL_DSL_PARSER
    settings.fDSLMangling = false;
    return DSLParser(this, settings, kind, text).program();
#else
    auto textPtr = std::make_unique<String>(std::move(text));
    AutoSource as(this, textPtr->c_str());

    dsl::Start(this, kind, settings);
    dsl::SetErrorHandler(this);
    IRGenerator::IRBundle ir = fIRGenerator->convertProgram(baseModule, /*isBuiltinCode=*/false,
                                                            *textPtr);
    // Ideally, we would just use dsl::ReleaseProgram and not have to do any manual mucking about
    // with the memory pool, but we've got some impedance mismatches to solve first
    Pool* memoryPool = dsl::DSLWriter::MemoryPool().get();
    auto program = std::make_unique<Program>(std::move(textPtr),
                                             std::move(dsl::DSLWriter::GetProgramConfig()),
                                             fContext,
                                             std::move(ir.fElements),
                                             std::move(ir.fSharedElements),
                                             std::move(dsl::DSLWriter::GetModifiersPool()),
                                             std::move(ir.fSymbolTable),
                                             std::move(dsl::DSLWriter::MemoryPool()),
                                             ir.fInputs);
    bool success = false;
    if (fErrorCount) {
        // Do not return programs that failed to compile.
    } else if (!this->optimize(*program)) {
        // Do not return programs that failed to optimize.
    } else {
        // We have a successful program!
        success = true;
    }
    dsl::End();
    if (memoryPool) {
        memoryPool->detachFromThread();
    }
    return success ? std::move(program) : nullptr;
#endif // SKSL_DSL_PARSER
}

void Compiler::verifyStaticTests(const Program& program) {
    class StaticTestVerifier : public ProgramVisitor {
    public:
        StaticTestVerifier(ErrorReporter* r) : fReporter(r) {}

        using ProgramVisitor::visitProgramElement;

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kIf:
                    if (stmt.as<IfStatement>().isStatic()) {
                        fReporter->error(stmt.fOffset, "static if has non-static test");
                    }
                    break;

                case Statement::Kind::kSwitch:
                    if (stmt.as<SwitchStatement>().isStatic()) {
                        fReporter->error(stmt.fOffset, "static switch has non-static test");
                    }
                    break;

                default:
                    break;
            }
            return INHERITED::visitStatement(stmt);
        }

        bool visitExpression(const Expression&) override {
            // We aren't looking for anything inside an Expression, so skip them entirely.
            return false;
        }

    private:
        using INHERITED = ProgramVisitor;
        ErrorReporter* fReporter;
    };

    // If invalid static tests are permitted, we don't need to check anything.
    if (fContext->fConfig->fSettings.fPermitInvalidStaticTests) {
        return;
    }

    // Check all of the program's owned elements. (Built-in elements are assumed to be valid.)
    StaticTestVerifier visitor{this};
    for (const std::unique_ptr<ProgramElement>& element : program.ownedElements()) {
        if (element->is<FunctionDefinition>()) {
            visitor.visitProgramElement(*element);
        }
    }
}

bool Compiler::optimize(LoadedModule& module) {
    SkASSERT(!fErrorCount);

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fKind = module.fKind;
    AutoProgramConfig autoConfig(fContext, &config);

    // Reset the Inliner.
    fInliner.reset();

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    while (fErrorCount == 0) {
        // Perform inline-candidate analysis and inline any functions deemed suitable.
        if (!fInliner.analyze(module.fElements, module.fSymbols, usage.get())) {
            break;
        }
    }
    return fErrorCount == 0;
}

bool Compiler::removeDeadFunctions(Program& program, ProgramUsage* usage) {
    bool madeChanges = false;

    if (program.fConfig->fSettings.fRemoveDeadFunctions) {
        auto isDeadFunction = [&](const ProgramElement* element) {
            if (!element->is<FunctionDefinition>()) {
                return false;
            }
            const FunctionDefinition& fn = element->as<FunctionDefinition>();
            if (fn.declaration().isMain() || usage->get(fn.declaration()) > 0) {
                return false;
            }
            usage->remove(*element);
            madeChanges = true;
            return true;
        };

        program.fElements.erase(std::remove_if(program.fElements.begin(),
                                               program.fElements.end(),
                                               [&](const std::unique_ptr<ProgramElement>& element) {
                                                   return isDeadFunction(element.get());
                                               }),
                                program.fElements.end());
        program.fSharedElements.erase(std::remove_if(program.fSharedElements.begin(),
                                                     program.fSharedElements.end(),
                                                     isDeadFunction),
                                      program.fSharedElements.end());
    }
    return madeChanges;
}

bool Compiler::removeDeadGlobalVariables(Program& program, ProgramUsage* usage) {
    bool madeChanges = false;

    if (program.fConfig->fSettings.fRemoveDeadVariables) {
        auto isDeadVariable = [&](const ProgramElement* element) {
            if (!element->is<GlobalVarDeclaration>()) {
                return false;
            }
            const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
            const VarDeclaration& varDecl = global.declaration()->as<VarDeclaration>();
            if (!usage->isDead(varDecl.var())) {
                return false;
            }
            madeChanges = true;
            return true;
        };

        program.fElements.erase(std::remove_if(program.fElements.begin(),
                                               program.fElements.end(),
                                               [&](const std::unique_ptr<ProgramElement>& element) {
                                                   return isDeadVariable(element.get());
                                               }),
                                program.fElements.end());
        program.fSharedElements.erase(std::remove_if(program.fSharedElements.begin(),
                                                     program.fSharedElements.end(),
                                                     isDeadVariable),
                                      program.fSharedElements.end());
    }
    return madeChanges;
}

bool Compiler::removeDeadLocalVariables(Program& program, ProgramUsage* usage) {
    class DeadLocalVariableEliminator : public ProgramWriter {
    public:
        DeadLocalVariableEliminator(const Context& context, ProgramUsage* usage)
                : fContext(context)
                , fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (stmt->is<VarDeclaration>()) {
                VarDeclaration& varDecl = stmt->as<VarDeclaration>();
                const Variable* var = &varDecl.var();
                ProgramUsage::VariableCounts* counts = fUsage->fVariableCounts.find(var);
                SkASSERT(counts);
                SkASSERT(counts->fDeclared);
                if (CanEliminate(var, *counts)) {
                    if (var->initialValue()) {
                        // The variable has an initial-value expression, which might have side
                        // effects. ExpressionStatement::Make will preserve side effects, but
                        // replaces pure expressions with Nop.
                        fUsage->remove(stmt.get());
                        stmt = ExpressionStatement::Make(fContext, std::move(varDecl.value()));
                        fUsage->add(stmt.get());
                    } else {
                        // The variable has no initial-value and can be cleanly eliminated.
                        fUsage->remove(stmt.get());
                        stmt = std::make_unique<Nop>();
                    }
                    fMadeChanges = true;
                }
                return false;
            }
            return INHERITED::visitStatementPtr(stmt);
        }

        static bool CanEliminate(const Variable* var, const ProgramUsage::VariableCounts& counts) {
            if (!counts.fDeclared || counts.fRead || var->storage() != VariableStorage::kLocal) {
                return false;
            }
            if (var->initialValue()) {
                SkASSERT(counts.fWrite >= 1);
                return counts.fWrite == 1;
            } else {
                return counts.fWrite == 0;
            }
        }

        bool fMadeChanges = false;
        const Context& fContext;
        ProgramUsage* fUsage;

        using INHERITED = ProgramWriter;
    };

    DeadLocalVariableEliminator visitor{*fContext, usage};

    if (program.fConfig->fSettings.fRemoveDeadVariables) {
        for (auto& [var, counts] : usage->fVariableCounts) {
            if (DeadLocalVariableEliminator::CanEliminate(var, counts)) {
                // This program contains at least one dead local variable.
                // Scan the program for any dead local variables and eliminate them all.
                for (std::unique_ptr<ProgramElement>& pe : program.ownedElements()) {
                    if (pe->is<FunctionDefinition>()) {
                        visitor.visitProgramElement(*pe);
                    }
                }
                break;
            }
        }
    }

    return visitor.fMadeChanges;
}

void Compiler::removeUnreachableCode(Program& program, ProgramUsage* usage) {
    class UnreachableCodeEliminator : public ProgramWriter {
    public:
        UnreachableCodeEliminator(const Context& context, ProgramUsage* usage)
                : fContext(context)
                , fUsage(usage) {
            fFoundFunctionExit.push(false);
            fFoundLoopExit.push(false);
        }

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (fFoundFunctionExit.top() || fFoundLoopExit.top()) {
                // If we already found an exit in this section, anything beyond it is dead code.
                if (!stmt->is<Nop>()) {
                    // Eliminate the dead statement by substituting a Nop.
                    fUsage->remove(stmt.get());
                    stmt = std::make_unique<Nop>();
                }
                return false;
            }

            switch (stmt->kind()) {
                case Statement::Kind::kReturn:
                case Statement::Kind::kDiscard:
                    // We found a function exit on this path.
                    fFoundFunctionExit.top() = true;
                    break;

                case Statement::Kind::kBreak:
                case Statement::Kind::kContinue:
                    // We found a loop exit on this path. Note that we skip over switch statements
                    // completely when eliminating code, so any `break` statement would be breaking
                    // out of a loop, not out of a switch.
                    fFoundLoopExit.top() = true;
                    break;

                case Statement::Kind::kExpression:
                case Statement::Kind::kInlineMarker:
                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't affect control flow.
                    break;

                case Statement::Kind::kBlock:
                    // Blocks are on the straight-line path and don't affect control flow.
                    return INHERITED::visitStatementPtr(stmt);

                case Statement::Kind::kDo: {
                    // Function-exits are allowed to propagate outside of a do-loop, because it
                    // always executes its body at least once.
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    return result;
                }
                case Statement::Kind::kFor: {
                    // Function-exits are not allowed to propagate out, because a for-loop or while-
                    // loop could potentially run zero times.
                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    fFoundFunctionExit.pop();
                    return result;
                }
                case Statement::Kind::kIf: {
                    // This statement is conditional and encloses two inner sections of code.
                    // If both sides contain a function-exit or loop-exit, that exit is allowed to
                    // propagate out.
                    IfStatement& ifStmt = stmt->as<IfStatement>();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = (ifStmt.ifTrue() && this->visitStatementPtr(ifStmt.ifTrue()));
                    bool foundFunctionExitOnTrue = fFoundFunctionExit.top();
                    bool foundLoopExitOnTrue = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    result |= (ifStmt.ifFalse() && this->visitStatementPtr(ifStmt.ifFalse()));
                    bool foundFunctionExitOnFalse = fFoundFunctionExit.top();
                    bool foundLoopExitOnFalse = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.top() |= foundFunctionExitOnTrue && foundFunctionExitOnFalse;
                    fFoundLoopExit.top() |= foundLoopExitOnTrue && foundLoopExitOnFalse;
                    return result;
                }
                case Statement::Kind::kSwitch:
                case Statement::Kind::kSwitchCase:
                    // We skip past switch statements entirely when scanning for dead code. Their
                    // control flow is quite complex and we already do a good job of flattening out
                    // switches on constant values.
                    break;
            }

            return false;
        }

        const Context& fContext;
        ProgramUsage* fUsage;
        std::stack<bool> fFoundFunctionExit;
        std::stack<bool> fFoundLoopExit;

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : program.ownedElements()) {
        if (pe->is<FunctionDefinition>()) {
            UnreachableCodeEliminator visitor{*fContext, usage};
            visitor.visitProgramElement(*pe);
        }
    }
}

bool Compiler::optimize(Program& program) {
    // The optimizer only needs to run when it is enabled.
    if (!program.fConfig->fSettings.fOptimize) {
        return true;
    }

    SkASSERT(!fErrorCount);
    ProgramUsage* usage = program.fUsage.get();

    if (fErrorCount == 0) {
        // Run the inliner only once; it is expensive! Multiple passes can occasionally shake out
        // more wins, but it's diminishing returns.
        fInliner.analyze(program.ownedElements(), program.fSymbols, usage);

        while (this->removeDeadFunctions(program, usage)) {
            // Removing dead functions may cause more functions to become unreferenced. Try again.
        }
        while (this->removeDeadLocalVariables(program, usage)) {
            // Removing dead variables may cause more variables to become unreferenced. Try again.
        }
        // Unreachable code can confuse some drivers, so it's worth removing. (skia:12012)
        this->removeUnreachableCode(program, usage);

        this->removeDeadGlobalVariables(program, usage);
    }

    if (fErrorCount == 0) {
        this->verifyStaticTests(program);
    }

    return fErrorCount == 0;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toSPIRV");
    AutoSource as(this, program.fSource->c_str());
    ProgramSettings settings;
    settings.fDSLUseMemoryPool = false;
    dsl::Start(this, program.fConfig->fKind, settings);
    dsl::DSLWriter::IRGenerator().fSymbolTable = program.fSymbols;
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &buffer);
    bool result = cg.generateCode();
    if (result && program.fConfig->fSettings.fValidateSPIRV) {
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
        const String& data = buffer.str();
        SkASSERT(0 == data.size() % 4);
        String errors;
        auto dumpmsg = [&errors](spv_message_level_t, const char*, const spv_position_t&,
                                 const char* m) {
            errors.appendf("SPIR-V validation error: %s\n", m);
        };
        tools.SetMessageConsumer(dumpmsg);

        // Verify that the SPIR-V we produced is valid. At runtime, we will abort() with a message
        // explaining the error. In standalone mode (skslc), we will send the message, plus the
        // entire disassembled SPIR-V (for easier context & debugging) as *our* error message.
        result = tools.Validate((const uint32_t*) data.c_str(), data.size() / 4);

        if (!result) {
#if defined(SKSL_STANDALONE)
            // Convert the string-stream to a SPIR-V disassembly.
            std::string disassembly;
            if (tools.Disassemble((const uint32_t*)data.data(), data.size() / 4, &disassembly)) {
                errors.append(disassembly);
            }
            this->error(-1, errors);
#else
            SkDEBUGFAILF("%s", errors.c_str());
#endif
        }
        out.write(data.c_str(), data.size());
    }
#else
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
#endif
    dsl::End();
    return result;
}

bool Compiler::toSPIRV(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toSPIRV(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toGLSL(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toGLSL");
    AutoSource as(this, program.fSource->c_str());
    GLSLCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toGLSL(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toGLSL(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toHLSL(Program& program, String* out) {
    String spirv;
    if (!this->toSPIRV(program, &spirv)) {
        return false;
    }

    return SPIRVtoHLSL(spirv, out);
}

bool Compiler::toMetal(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toMetal");
    AutoSource as(this, program.fSource->c_str());
    MetalCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toMetal(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toMetal(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

#endif // defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

void Compiler::handleError(const char* msg, dsl::PositionInfo pos) {
    if (strstr(msg, POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    fErrorCount++;
    fErrorText += "error: " + (pos.line() >= 1 ? to_string(pos.line()) + ": " : "") + msg + "\n";
}

String Compiler::errorText(bool showCount) {
    if (showCount) {
        this->writeErrorCount();
    }
    fErrorCount = 0;
    String result = fErrorText;
    fErrorText = "";
    return result;
}

void Compiler::writeErrorCount() {
    if (fErrorCount) {
        fErrorText += to_string(fErrorCount) + " error";
        if (fErrorCount > 1) {
            fErrorText += "s";
        }
        fErrorText += "\n";
    }
}

}  // namespace SkSL
