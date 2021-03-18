/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include <memory>
#include <unordered_set>

#include "src/core/SkScopeExit.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCPPCodeGenerator.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"
#include "src/sksl/SkSLHCodeGenerator.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLMetalCodeGenerator.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/SkSLSPIRVtoHLSL.h"
#include "src/sksl/ir/SkSLEnum.h"
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
#include "src/sksl/generated/sksl_fp.dehydrated.sksl"
#include "src/sksl/generated/sksl_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_geom.dehydrated.sksl"
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_public.dehydrated.sksl"
#include "src/sksl/generated/sksl_runtime.dehydrated.sksl"
#include "src/sksl/generated/sksl_vert.dehydrated.sksl"

#define MODULE_DATA(name) MakeModuleData(SKSL_INCLUDE_sksl_##name,\
                                         SKSL_INCLUDE_sksl_##name##_LENGTH)

#endif

namespace SkSL {

// Set these flags to `false` to disable optimization passes unilaterally.
// These flags allow tools like Viewer or Nanobench to override the compiler's ProgramSettings.
bool gSkSLOptimizer = true;
bool gSkSLInliner = true;
bool gSkSLDeadCodeElimination = true;

using RefKind = VariableReference::RefKind;

class AutoSource {
public:
    AutoSource(Compiler* compiler, const String* source)
            : fCompiler(compiler), fOldSource(fCompiler->fSource) {
        fCompiler->fSource = source;
    }

    ~AutoSource() { fCompiler->fSource = fOldSource; }

    Compiler* fCompiler;
    const String* fOldSource;
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

Compiler::Compiler(const ShaderCapsClass* caps)
        : fContext(std::make_shared<Context>(/*errors=*/*this, *caps))
        , fInliner(fContext.get())
        , fErrorCount(0) {
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

        TYPE(FragmentProcessor),
    };

    const SkSL::Symbol* privateTypes[] = {
        TYPE(  UInt), TYPE(  UInt2), TYPE(  UInt3), TYPE(  UInt4),
        TYPE( Short), TYPE( Short2), TYPE( Short3), TYPE( Short4),
        TYPE(UShort), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4),
        TYPE(  Byte), TYPE(  Byte2), TYPE(  Byte3), TYPE(  Byte4),
        TYPE( UByte), TYPE( UByte2), TYPE( UByte3), TYPE( UByte4),

        TYPE(GenUType), TYPE(UVec),
        TYPE(SVec), TYPE(USVec), TYPE(ByteVec), TYPE(UByteVec),

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
    fPrivateSymbolTable->add(
            std::make_unique<Variable>(/*offset=*/-1,
                                       fIRGenerator->fModifiers->addToPool(Modifiers()),
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

const ParsedModule& Compiler::loadFPModule() {
    if (!fFPModule.fSymbols) {
        fFPModule = this->parseModule(ProgramKind::kFragmentProcessor, MODULE_DATA(fp),
                                      this->loadGPUModule());
    }
    return fFPModule;
}

const ParsedModule& Compiler::loadPublicModule() {
    if (!fPublicModule.fSymbols) {
        fPublicModule = this->parseModule(ProgramKind::kGeneric, MODULE_DATA(public), fRootModule);
    }
    return fPublicModule;
}

const ParsedModule& Compiler::loadRuntimeEffectModule() {
    if (!fRuntimeEffectModule.fSymbols) {
        fRuntimeEffectModule = this->parseModule(ProgramKind::kRuntimeEffect, MODULE_DATA(runtime),
                                                 this->loadPublicModule());

        // Add some aliases to the runtime effect module so that it's friendlier, and more like GLSL
        fRuntimeEffectModule.fSymbols->addAlias("shader", fContext->fTypes.fFragmentProcessor.get());

        fRuntimeEffectModule.fSymbols->addAlias("vec2", fContext->fTypes.fFloat2.get());
        fRuntimeEffectModule.fSymbols->addAlias("vec3", fContext->fTypes.fFloat3.get());
        fRuntimeEffectModule.fSymbols->addAlias("vec4", fContext->fTypes.fFloat4.get());

        fRuntimeEffectModule.fSymbols->addAlias("bvec2", fContext->fTypes.fBool2.get());
        fRuntimeEffectModule.fSymbols->addAlias("bvec3", fContext->fTypes.fBool3.get());
        fRuntimeEffectModule.fSymbols->addAlias("bvec4", fContext->fTypes.fBool4.get());

        fRuntimeEffectModule.fSymbols->addAlias("mat2", fContext->fTypes.fFloat2x2.get());
        fRuntimeEffectModule.fSymbols->addAlias("mat3", fContext->fTypes.fFloat3x3.get());
        fRuntimeEffectModule.fSymbols->addAlias("mat4", fContext->fTypes.fFloat4x4.get());
    }
    return fRuntimeEffectModule;
}

const ParsedModule& Compiler::moduleForProgramKind(ProgramKind kind) {
    switch (kind) {
        case ProgramKind::kVertex:            return this->loadVertexModule();        break;
        case ProgramKind::kFragment:          return this->loadFragmentModule();      break;
        case ProgramKind::kGeometry:          return this->loadGeometryModule();      break;
        case ProgramKind::kFragmentProcessor: return this->loadFPModule();            break;
        case ProgramKind::kRuntimeEffect:     return this->loadRuntimeEffectModule(); break;
        case ProgramKind::kGeneric:           return this->loadPublicModule();        break;
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

    // Built-in modules always use default program settings.
    ProgramConfig config;
    config.fKind = kind;
    config.fSettings.fReplaceSettings = !dehydrate;
    AutoProgramConfig autoConfig(fContext, &config);

#if defined(SKSL_STANDALONE)
    SkASSERT(data.fPath);
    std::ifstream in(data.fPath);
    String text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading %s\n", data.fPath);
        abort();
    }
    const String* source = fRootSymbolTable->takeOwnershipOfString(std::move(text));
    AutoSource as(this, source);

    SkASSERT(fIRGenerator->fCanInline);
    fIRGenerator->fCanInline = false;

    ParsedModule baseModule = {base, /*fIntrinsics=*/nullptr};
    IRGenerator::IRBundle ir = fIRGenerator->convertProgram(baseModule, /*isBuiltinCode=*/true,
                                                            source->c_str(), source->length(),
                                                            /*externalFunctions=*/nullptr);
    SkASSERT(ir.fSharedElements.empty());
    LoadedModule module = { kind, std::move(ir.fSymbolTable), std::move(ir.fElements) };
    fIRGenerator->fCanInline = true;
    if (this->fErrorCount) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
    fModifiers.push_back(std::move(ir.fModifiers));
#else
    SkASSERT(data.fData && (data.fSize != 0));
    Rehydrator rehydrator(fContext.get(), fIRGenerator->fModifiers.get(), base, this,
                          data.fData, data.fSize);
    LoadedModule module = { kind, rehydrator.symbolTable(), rehydrator.elements() };
    fModifiers.push_back(fIRGenerator->releaseModifiers());
#endif

    return module;
}

ParsedModule Compiler::parseModule(ProgramKind kind, ModuleData data, const ParsedModule& base) {
    LoadedModule module = this->loadModule(kind, data, base.fSymbols, /*dehydrate=*/false);
    this->optimize(module);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's intrinsic map:
    if (module.fElements.empty()) {
        return {module.fSymbols, base.fIntrinsics};
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
            case ProgramElement::Kind::kEnum: {
                const Enum& e = element->as<Enum>();
                SkASSERT(e.isBuiltin());
                intrinsics->insertOrDie(e.typeName(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
                const Variable& var = global.declaration()->as<VarDeclaration>().var();
                SkASSERT(var.isBuiltin());
                intrinsics->insertOrDie(var.name(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kInterfaceBlock: {
                const Variable& var = element->as<InterfaceBlock>().variable();
                SkASSERT(var.isBuiltin());
                intrinsics->insertOrDie(var.name(), std::move(element));
                break;
            }
            default:
                printf("Unsupported element: %s\n", element->description().c_str());
                SkASSERT(false);
                break;
        }
    }

    return {module.fSymbols, std::move(intrinsics)};
}

std::unique_ptr<Program> Compiler::convertProgram(
        ProgramKind kind,
        String text,
        const Program::Settings& settings,
        const std::vector<std::unique_ptr<ExternalFunction>>* externalFunctions) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::convertProgram");

    SkASSERT(!externalFunctions || (kind == ProgramKind::kGeneric));

    // Loading and optimizing our base module might reset the inliner, so do that first,
    // *then* configure the inliner with the settings for this program.
    const ParsedModule& baseModule = this->moduleForProgramKind(kind);

    // Update our context to point to the program configuration for the duration of compilation.
    auto config = std::make_unique<ProgramConfig>(ProgramConfig{kind, settings});
    AutoProgramConfig autoConfig(fContext, config.get());

    // Honor our global optimization-disable flags.
    config->fSettings.fOptimize &= gSkSLOptimizer;
    config->fSettings.fInlineThreshold *= (int)gSkSLInliner;
    config->fSettings.fRemoveDeadVariables &= gSkSLDeadCodeElimination;
    config->fSettings.fRemoveDeadFunctions &= gSkSLDeadCodeElimination;

    // Disable optimization settings that depend on a parent setting which has been disabled.
    config->fSettings.fInlineThreshold *= (int)config->fSettings.fOptimize;
    config->fSettings.fRemoveDeadFunctions &= config->fSettings.fOptimize;
    config->fSettings.fRemoveDeadVariables &= config->fSettings.fOptimize;

    fErrorText = "";
    fErrorCount = 0;
    fInliner.reset(fIRGenerator->fModifiers.get());

    // Not using AutoSource, because caller is likely to call errorText() if we fail to compile
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();

    // Enable node pooling while converting and optimizing the program for a performance boost.
    // The Program will take ownership of the pool.
    std::unique_ptr<Pool> pool;
    if (fContext->fCaps.useNodePools()) {
        pool = Pool::Create();
        pool->attachToThread();
    }
    IRGenerator::IRBundle ir = fIRGenerator->convertProgram(baseModule, /*isBuiltinCode=*/false,
                                                            textPtr->c_str(), textPtr->size(),
                                                            externalFunctions);
    auto program = std::make_unique<Program>(std::move(textPtr),
                                             std::move(config),
                                             fContext,
                                             std::move(ir.fElements),
                                             std::move(ir.fSharedElements),
                                             std::move(ir.fModifiers),
                                             std::move(ir.fSymbolTable),
                                             std::move(pool),
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

    if (program->fPool) {
        program->fPool->detachFromThread();
    }
    return success ? std::move(program) : nullptr;
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
    fInliner.reset(fModifiers.back().get());

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
            if (fn.declaration().name() == "main" || usage->get(fn.declaration()) > 0) {
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

bool Compiler::optimize(Program& program) {
    // The optimizer only needs to run when it is enabled.
    if (!program.fConfig->fSettings.fOptimize) {
        return true;
    }

    SkASSERT(!fErrorCount);
    ProgramUsage* usage = program.fUsage.get();

    while (fErrorCount == 0) {
        bool madeChanges = fInliner.analyze(program.ownedElements(), program.fSymbols, usage);

        madeChanges |= this->removeDeadFunctions(program, usage);
        madeChanges |= this->removeDeadLocalVariables(program, usage);

        if (program.fConfig->fKind != ProgramKind::kFragmentProcessor) {
            madeChanges |= this->removeDeadGlobalVariables(program, usage);
        }

        if (!madeChanges) {
            break;
        }
    }

    if (fErrorCount == 0) {
        this->verifyStaticTests(program);
    }

    return fErrorCount == 0;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toSPIRV");
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    AutoSource as(this, program.fSource.get());
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
    AutoSource as(this, program.fSource.get());
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
#endif
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
    AutoSource as(this, program.fSource.get());
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

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS
bool Compiler::toCPP(Program& program, String name, OutputStream& out) {
    AutoSource as(this, program.fSource.get());
    CPPCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toH(Program& program, String name, OutputStream& out) {
    AutoSource as(this, program.fSource.get());
    HCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    return result;
}
#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

Position Compiler::position(int offset) {
    if (fSource && offset >= 0) {
        int line = 1;
        int column = 1;
        for (int i = 0; i < offset; i++) {
            if ((*fSource)[i] == '\n') {
                ++line;
                column = 1;
            }
            else {
                ++column;
            }
        }
        return Position(line, column);
    } else {
        return Position(-1, -1);
    }
}

void Compiler::error(int offset, String msg) {
    fErrorCount++;
    Position pos = this->position(offset);
    fErrorTextLength.push_back(fErrorText.length());
    fErrorText += "error: " + (pos.fLine >= 1 ? to_string(pos.fLine) + ": " : "") + msg + "\n";
}

void Compiler::setErrorCount(int c) {
    if (c < fErrorCount) {
        fErrorText.resize(fErrorTextLength[c]);
        fErrorTextLength.resize(c);
        fErrorCount = c;
    }
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
