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
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLDSLParser.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLIntrinsicMap.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/DSL_priv.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"
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
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_public.dehydrated.sksl"
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
        SkASSERT(!fCompiler->errorReporter().source());
        fCompiler->errorReporter().setSource(source);
    }

    ~AutoSource() {
        fCompiler->errorReporter().setSource(nullptr);
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

Compiler::Compiler(const ShaderCapsClass* caps)
        : fErrorReporter(this)
        , fContext(std::make_shared<Context>(fErrorReporter, *caps, fMangler))
        , fInliner(fContext.get()) {
    SkASSERT(caps);
    fRootModule.fSymbols = this->makeRootSymbolTable();
    fPrivateModule.fSymbols = this->makePrivateSymbolTable(fRootModule.fSymbols);
    fIRGenerator = std::make_unique<IRGenerator>(fContext.get());
}

Compiler::~Compiler() {}

#define TYPE(t) fContext->fTypes.f ## t .get()

std::shared_ptr<SymbolTable> Compiler::makeRootSymbolTable() {
    auto rootSymbolTable = std::make_shared<SymbolTable>(*fContext, /*builtin=*/true);

    const SkSL::Symbol* rootTypes[] = {
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

        TYPE(Vec),     TYPE(IVec),     TYPE(UVec),
        TYPE(HVec),    TYPE(SVec),     TYPE(USVec),
        TYPE(BVec),

        TYPE(ColorFilter),
        TYPE(Shader),
        TYPE(Blender),
    };

    for (const SkSL::Symbol* type : rootTypes) {
        rootSymbolTable->addWithoutOwnership(type);
    }

    return rootSymbolTable;
}

std::shared_ptr<SymbolTable> Compiler::makePrivateSymbolTable(std::shared_ptr<SymbolTable> parent) {
    auto privateSymbolTable = std::make_shared<SymbolTable>(parent, /*builtin=*/true);

    const SkSL::Symbol* privateTypes[] = {
        TYPE(Sampler1D), TYPE(Sampler2D), TYPE(Sampler3D),
        TYPE(SamplerExternalOES),
        TYPE(Sampler2DRect),

        TYPE(ISampler2D),
        TYPE(SubpassInput), TYPE(SubpassInputMS),

        TYPE(Sampler),
        TYPE(Texture2D),
    };

    for (const SkSL::Symbol* type : privateTypes) {
        privateSymbolTable->addWithoutOwnership(type);
    }

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    privateSymbolTable->add(std::make_unique<Variable>(/*line=*/-1,
                                                       fCoreModifiers.add(Modifiers{}),
                                                       "sk_Caps",
                                                       fContext->fTypes.fSkCaps.get(),
                                                       /*builtin=*/false,
                                                       Variable::Storage::kGlobal));

    return privateSymbolTable;
}

#undef TYPE

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

const ParsedModule& Compiler::loadPublicModule() {
    if (!fPublicModule.fSymbols) {
        fPublicModule = this->parseModule(ProgramKind::kGeneric, MODULE_DATA(public), fRootModule);
        add_glsl_type_aliases(fPublicModule.fSymbols.get(), fContext->fTypes);
    }
    return fPublicModule;
}

const ParsedModule& Compiler::loadRuntimeShaderModule() {
    if (!fRuntimeShaderModule.fSymbols) {
        fRuntimeShaderModule = this->parseModule(
                ProgramKind::kRuntimeShader, MODULE_DATA(rt_shader), this->loadPublicModule());
    }
    return fRuntimeShaderModule;
}

const ParsedModule& Compiler::moduleForProgramKind(ProgramKind kind) {
    switch (kind) {
        case ProgramKind::kVertex:             return this->loadVertexModule();        break;
        case ProgramKind::kFragment:           return this->loadFragmentModule();      break;
        case ProgramKind::kRuntimeColorFilter: return this->loadPublicModule();        break;
        case ProgramKind::kRuntimeShader:      return this->loadRuntimeShaderModule(); break;
        case ProgramKind::kRuntimeBlender:     return this->loadPublicModule();        break;
        case ProgramKind::kGeneric:            return this->loadPublicModule();        break;
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
        base = fPrivateModule.fSymbols;
    }
    SkASSERT(base);

    // Put the core-module modifier pool into the context.
    AutoModifiersPool autoPool(fContext, &fCoreModifiers);

    // Built-in modules always use default program settings.
    Program::Settings settings;
    settings.fReplaceSettings = !dehydrate;

#if defined(SKSL_STANDALONE)
    SkASSERT(this->errorCount() == 0);
    SkASSERT(data.fPath);
    std::ifstream in(data.fPath);
    String text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading %s\n", data.fPath);
        abort();
    }
    ParsedModule baseModule = {base, /*fIntrinsics=*/nullptr};
    LoadedModule result = DSLParser(this, settings, kind,
            std::move(text)).moduleInheritingFrom(std::move(baseModule));
    if (this->errorCount()) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
#else
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = kind;
    config.fSettings = settings;
    AutoProgramConfig autoConfig(fContext, &config);
    SkASSERT(data.fData && (data.fSize != 0));
    Rehydrator rehydrator(fContext.get(), base, data.fData, data.fSize);
    LoadedModule result = { kind, rehydrator.symbolTable(), rehydrator.elements() };
#endif

    return result;
}

ParsedModule Compiler::parseModule(ProgramKind kind, ModuleData data, const ParsedModule& base) {
    LoadedModule module = this->loadModule(kind, data, base.fSymbols, /*dehydrate=*/false);
    this->optimize(module);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's intrinsic map:
    if (module.fElements.empty()) {
        return ParsedModule{module.fSymbols, base.fIntrinsics};
    }

    auto intrinsics = std::make_shared<IntrinsicMap>(base.fIntrinsics.get());

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

std::unique_ptr<Program> Compiler::convertProgram(ProgramKind kind,
                                                  String text,
                                                  Program::Settings settings) {
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

    // Runtime effects always allow narrowing conversions.
    if (ProgramConfig::IsRuntimeEffect(kind)) {
        settings.fAllowNarrowingConversions = true;
    }

    this->resetErrors();
    fInliner.reset();

    settings.fDSLMangling = false;
    return DSLParser(this, settings, kind, std::move(text)).program();
}

bool Compiler::optimize(LoadedModule& module) {
    SkASSERT(!this->errorCount());

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = module.fKind;
    AutoProgramConfig autoConfig(fContext, &config);

    // Reset the Inliner.
    fInliner.reset();

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    while (this->errorCount() == 0) {
        // Perform inline-candidate analysis and inline any functions deemed suitable.
        if (!this->runInliner(module.fElements, module.fSymbols, usage.get())) {
            break;
        }
    }
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
        this->runInliner(program.fOwnedElements, program.fSymbols, usage);

        // Unreachable code can confuse some drivers, so it's worth removing. (skia:12012)
        Transform::EliminateUnreachableCode(program, usage);

        while (Transform::EliminateDeadFunctions(program, usage)) {
            // Removing dead functions may cause more functions to become unreferenced. Try again.
        }
        while (Transform::EliminateDeadLocalVariables(program, usage)) {
            // Removing dead variables may cause more variables to become unreferenced. Try again.
        }

        Transform::EliminateDeadGlobalVariables(program, usage);
    }

    return this->errorCount() == 0;
}

bool Compiler::runInliner(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                          std::shared_ptr<SymbolTable> symbols,
                          ProgramUsage* usage) {
    // The program's SymbolTable was taken out of the IRGenerator when the program was bundled, but
    // the inliner relies (indirectly) on having a valid SymbolTable in the IRGenerator.
    // In particular, inlining can turn a non-optimizable expression like `normalize(myVec)` into
    // `normalize(vec2(7))`, which is now optimizable. The optimizer can use DSL to simplify this
    // expression--e.g., in the case of normalize, using DSL's Length(). The DSL relies on
    // irGenerator.convertIdentifier() to look up `length`. convertIdentifier() needs a valid symbol
    // table to find the declaration of `length`. To allow this chain of events to succeed, we
    // re-insert the program's symbol table back into the IRGenerator temporarily.
    SkASSERT(!fIRGenerator->fSymbolTable);
    fIRGenerator->fSymbolTable = symbols;

    bool result = fInliner.analyze(elements, symbols, usage);

    fIRGenerator->fSymbolTable = nullptr;
    return result;
}

bool Compiler::finalize(Program& program) {
    // Do a pass looking for @if/@switch statements that didn't optimize away, or dangling
    // FunctionReference or TypeReference expressions. Report these as errors.
    Analysis::VerifyStaticTestsAndExpressions(program);

    // Verify that the program conforms to ES2 limitations.
    if (fContext->fConfig->strictES2Mode() && this->errorCount() == 0) {
        // Enforce Appendix A, Section 5 of the GLSL ES 1.00 spec -- Indexing. This logic assumes
        // that all loops meet the criteria of Section 4, and if they don't, could crash.
        for (const auto& pe : program.fOwnedElements) {
            Analysis::ValidateIndexingForES2(*pe, this->errorReporter());
        }
        // Verify that the program size is reasonable after unrolling and inlining. This also
        // issues errors for static recursion and overly-deep function-call chains.
        Analysis::CheckProgramUnrolledSize(program);
    }

    return this->errorCount() == 0;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toSPIRV");
    AutoSource as(this, program.fSource->c_str());
    ProgramSettings settings;
    settings.fDSLUseMemoryPool = false;
    dsl::Start(this, program.fConfig->fKind, settings);
    dsl::SetErrorReporter(&fErrorReporter);
    ThreadContext::IRGenerator().fSymbolTable = program.fSymbols;
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    SPIRVCodeGenerator cg(fContext.get(), &program, &buffer);
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
            this->errorReporter().error(-1, errors);
            this->errorReporter().reportPendingErrors(PositionInfo());
#else
            SkDEBUGFAILF("%s", errors.c_str());
#endif
        }
        out.write(data.c_str(), data.size());
    }
#else
    SPIRVCodeGenerator cg(fContext.get(), &program, &out);
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
    GLSLCodeGenerator cg(fContext.get(), &program, &out);
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
    MetalCodeGenerator cg(fContext.get(), &program, &out);
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

void Compiler::handleError(skstd::string_view msg, PositionInfo pos) {
    fErrorText += "error: " + (pos.line() >= 1 ? to_string(pos.line()) + ": " : "") + msg + "\n";
}

String Compiler::errorText(bool showCount) {
    if (showCount) {
        this->writeErrorCount();
    }
    String result = fErrorText;
    this->resetErrors();
    return result;
}

void Compiler::writeErrorCount() {
    int count = this->errorCount();
    if (count) {
        fErrorText += to_string(count) + " error";
        if (count > 1) {
            fErrorText += "s";
        }
        fErrorText += "\n";
    }
}

}  // namespace SkSL
