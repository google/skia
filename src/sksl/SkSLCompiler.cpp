/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLType.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLInliner.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLStringStream.h"
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
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stdio.h>
#include <utility>

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#endif

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

namespace SkSL {

class ModifiersPool;
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

class AutoShaderCaps {
public:
    AutoShaderCaps(std::shared_ptr<Context>& context, const ShaderCaps* caps)
            : fContext(context.get())
            , fOldCaps(fContext->fCaps) {
        fContext->fCaps = caps;
    }

    ~AutoShaderCaps() {
        fContext->fCaps = fOldCaps;
    }

    Context* fContext;
    const ShaderCaps* fOldCaps;
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

Compiler::Compiler(const ShaderCaps* caps) : fErrorReporter(this), fCaps(caps) {
    SkASSERT(caps);

    auto moduleLoader = ModuleLoader::Get();
    fContext = std::make_shared<Context>(moduleLoader.builtinTypes(), /*caps=*/nullptr,
                                         fErrorReporter);
}

Compiler::~Compiler() {}

const ParsedModule& Compiler::moduleForProgramKind(ProgramKind kind) {
    auto m = ModuleLoader::Get();
    switch (kind) {
        case ProgramKind::kVertex:               return m.loadVertexModule(this);           break;
        case ProgramKind::kFragment:             return m.loadFragmentModule(this);         break;
        case ProgramKind::kCompute:              return m.loadComputeModule(this);          break;
        case ProgramKind::kGraphiteVertex:       return m.loadGraphiteVertexModule(this);   break;
        case ProgramKind::kGraphiteFragment:     return m.loadGraphiteFragmentModule(this); break;
        case ProgramKind::kRuntimeColorFilter:   return m.loadPublicModule(this);           break;
        case ProgramKind::kRuntimeShader:        return m.loadPublicModule(this);           break;
        case ProgramKind::kRuntimeBlender:       return m.loadPublicModule(this);           break;
        case ProgramKind::kPrivateRuntimeShader: return m.loadPrivateRTShaderModule(this);  break;
        case ProgramKind::kMeshVertex:           return m.loadPublicModule(this);           break;
        case ProgramKind::kMeshFragment:         return m.loadPublicModule(this);           break;
        case ProgramKind::kGeneric:              return m.loadPublicModule(this);           break;
    }
    SkUNREACHABLE;
}

LoadedModule Compiler::loadModule(ProgramKind kind,
                                  ModuleData data,
                                  ModifiersPool& modifiersPool,
                                  std::shared_ptr<SymbolTable> base) {
    SkASSERT(base);
    // Modules are shared and cannot rely on shader caps.
    AutoShaderCaps autoCaps(fContext, nullptr);
    AutoModifiersPool autoPool(fContext, &modifiersPool);

#if REHYDRATE
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = kind;
    AutoProgramConfig autoConfig(fContext, &config);
    SkASSERT(!data.fData.empty());
    Rehydrator rehydrator(*this, data.fData, std::move(base));
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
    // Built-in modules always use default program settings.
    ProgramSettings settings;
    LoadedModule module = Parser(this, settings, kind, std::move(text))
                                  .moduleInheritingFrom(baseModule);
    if (this->errorCount()) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
    this->optimizeModuleForDehydration(module, baseModule);
#endif

    return module;
}

ParsedModule Compiler::parseModule(ProgramKind kind,
                                   ModuleData data,
                                   const ParsedModule& base,
                                   ModifiersPool& modifiersPool) {
    LoadedModule module = this->loadModule(kind, data, modifiersPool, base.fSymbols);
    this->optimizeRehydratedModule(module, base, modifiersPool);

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

    // Put the ShaderCaps into the context while compiling a program.
    AutoShaderCaps autoCaps(fContext, fCaps);

    this->resetErrors();

    return Parser(this, settings, kind, std::move(text)).program();
}

std::unique_ptr<Expression> Compiler::convertIdentifier(Position pos, std::string_view name) {
    const Symbol* result = (*fSymbolTable)[name];
    if (!result) {
        this->errorReporter().error(pos, "unknown identifier '" + std::string(name) + "'");
        return nullptr;
    }
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration: {
            return std::make_unique<FunctionReference>(*fContext, pos,
                                                       &result->as<FunctionDeclaration>());
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

bool Compiler::optimizeRehydratedModule(LoadedModule& module,
                                        const ParsedModule& base,
                                        ModifiersPool& modifiersPool) {
#ifdef SK_ENABLE_OPTIMIZE_SIZE
    // We don't have an inliner, so there's nothing to do.
    return true;
#else
    SkASSERT(!this->errorCount());

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = module.fKind;
    AutoProgramConfig autoConfig(fContext, &config);
    AutoModifiersPool autoPool(fContext, &modifiersPool);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module, base);

    // Perform inline-candidate analysis and inline any functions deemed suitable.
    Inliner inliner(fContext.get());
    while (this->errorCount() == 0) {
        if (!this->runInliner(&inliner, module.fElements, module.fSymbols, usage.get())) {
            break;
        }
    }
    return this->errorCount() == 0;
#endif
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

    AutoShaderCaps autoCaps(fContext, fCaps);

    SkASSERT(!this->errorCount());
    if (this->errorCount() == 0) {
#ifndef SK_ENABLE_OPTIMIZE_SIZE
        // Run the inliner only once; it is expensive! Multiple passes can occasionally shake out
        // more wins, but it's diminishing returns.
        ProgramUsage* usage = program.fUsage.get();
        Inliner inliner(fContext.get());
        this->runInliner(&inliner, program.fOwnedElements, program.fSymbols, usage);
#endif

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

bool Compiler::runInliner(Inliner* inliner,
                          const std::vector<std::unique_ptr<ProgramElement>>& elements,
                          std::shared_ptr<SymbolTable> symbols,
                          ProgramUsage* usage) {
#ifdef SK_ENABLE_OPTIMIZE_SIZE
    return true;
#else
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

    bool result = inliner->analyze(elements, symbols, usage);

    fSymbolTable = nullptr;
    return result;
#endif
}

bool Compiler::finalize(Program& program) {
    AutoShaderCaps autoCaps(fContext, fCaps);

    // Copy all referenced built-in functions into the Program.
    Transform::FindAndDeclareBuiltinFunctions(program);

    // Variables defined in the pre-includes need their declaring elements added to the program.
    Transform::FindAndDeclareBuiltinVariables(program);

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
    AutoShaderCaps autoCaps(fContext, fCaps);
    ProgramSettings settings;
    settings.fUseMemoryPool = false;
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
    AutoShaderCaps autoCaps(fContext, fCaps);
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
    AutoShaderCaps autoCaps(fContext, fCaps);
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
