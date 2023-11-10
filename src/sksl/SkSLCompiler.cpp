/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "include/private/base/SkDebug.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLInliner.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"  // IWYU pragma: keep
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(SKSL_STANDALONE)
#include <fstream>
#endif

#if defined(SKSL_STANDALONE) || defined(SK_GANESH) || defined(SK_GRAPHITE)
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
#include "src/tint/lang/wgsl/reader/options.h"
#include "src/tint/lang/wgsl/extension.h"
#endif

namespace SkSL {

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
    AutoProgramConfig(Context& context, ProgramConfig* config)
            : fContext(context)
            , fOldConfig(context.fConfig) {
        fContext.fConfig = config;
    }

    ~AutoProgramConfig() {
        fContext.fConfig = fOldConfig;
    }

    Context& fContext;
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

Compiler::Compiler(const ShaderCaps* caps) : fErrorReporter(this), fCaps(caps) {
    SkASSERT(caps);

    auto moduleLoader = ModuleLoader::Get();
    fContext = std::make_shared<Context>(moduleLoader.builtinTypes(), /*caps=*/nullptr,
                                         fErrorReporter);
}

Compiler::~Compiler() {}

const Module* Compiler::moduleForProgramKind(ProgramKind kind) {
    auto m = ModuleLoader::Get();
    switch (kind) {
        case ProgramKind::kVertex:                return m.loadVertexModule(this);
        case ProgramKind::kFragment:              return m.loadFragmentModule(this);
        case ProgramKind::kCompute:               return m.loadComputeModule(this);
        case ProgramKind::kGraphiteVertex:        return m.loadGraphiteVertexModule(this);
        case ProgramKind::kGraphiteFragment:      return m.loadGraphiteFragmentModule(this);
        case ProgramKind::kPrivateRuntimeShader:  return m.loadPrivateRTShaderModule(this);
        case ProgramKind::kRuntimeColorFilter:
        case ProgramKind::kRuntimeShader:
        case ProgramKind::kRuntimeBlender:
        case ProgramKind::kPrivateRuntimeColorFilter:
        case ProgramKind::kPrivateRuntimeBlender:
        case ProgramKind::kMeshVertex:
        case ProgramKind::kMeshFragment:          return m.loadPublicModule(this);
    }
    SkUNREACHABLE;
}

void Compiler::FinalizeSettings(ProgramSettings* settings, ProgramKind kind) {
    // Honor our optimization-override flags.
    switch (sOptimizer) {
        case OverrideFlag::kDefault:
            break;
        case OverrideFlag::kOff:
            settings->fOptimize = false;
            break;
        case OverrideFlag::kOn:
            settings->fOptimize = true;
            break;
    }

    switch (sInliner) {
        case OverrideFlag::kDefault:
            break;
        case OverrideFlag::kOff:
            settings->fInlineThreshold = 0;
            break;
        case OverrideFlag::kOn:
            if (settings->fInlineThreshold == 0) {
                settings->fInlineThreshold = kDefaultInlineThreshold;
            }
            break;
    }

    // Disable optimization settings that depend on a parent setting which has been disabled.
    settings->fInlineThreshold *= (int)settings->fOptimize;
    settings->fRemoveDeadFunctions &= settings->fOptimize;
    settings->fRemoveDeadVariables &= settings->fOptimize;

    // Runtime effects always allow narrowing conversions.
    if (ProgramConfig::IsRuntimeEffect(kind)) {
        settings->fAllowNarrowingConversions = true;
    }
}

std::unique_ptr<Module> Compiler::compileModule(ProgramKind kind,
                                                const char* moduleName,
                                                std::string moduleSource,
                                                const Module* parent,
                                                bool shouldInline) {
    SkASSERT(parent);
    SkASSERT(!moduleSource.empty());
    SkASSERT(this->errorCount() == 0);

    // Modules are shared and cannot rely on shader caps.
    AutoShaderCaps autoCaps(fContext, nullptr);

    // Compile the module from source, using default program settings.
    ProgramSettings settings;
    FinalizeSettings(&settings, kind);
    SkSL::Parser parser{this, settings, kind, std::move(moduleSource)};
    std::unique_ptr<Module> module = parser.moduleInheritingFrom(parent);
    if (this->errorCount() != 0) {
        SkDebugf("Unexpected errors compiling %s:\n\n%s\n", moduleName, this->errorText().c_str());
        return nullptr;
    }
    if (shouldInline) {
        this->optimizeModuleAfterLoading(kind, *module);
    }
    return module;
}

std::unique_ptr<Program> Compiler::convertProgram(ProgramKind kind,
                                                  std::string text,
                                                  ProgramSettings settings) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::convertProgram");

    // Make sure the passed-in settings are valid.
    FinalizeSettings(&settings, kind);

    // Put the ShaderCaps into the context while compiling a program.
    AutoShaderCaps autoCaps(fContext, fCaps);

    this->resetErrors();

    return Parser(this, settings, kind, std::move(text)).program();
}

std::unique_ptr<SkSL::Program> Compiler::releaseProgram(std::unique_ptr<std::string> source) {
    ThreadContext& instance = ThreadContext::Instance();
    Pool* pool = instance.fPool.get();
    auto result = std::make_unique<SkSL::Program>(std::move(source),
                                                  std::move(instance.fConfig),
                                                  fContext,
                                                  std::move(instance.fProgramElements),
                                                  std::move(instance.fSharedElements),
                                                  std::move(fContext->fSymbolTable),
                                                  std::move(instance.fPool),
                                                  instance.fInterface);
    bool success = this->finalize(*result) &&
                   this->optimize(*result);
    if (pool) {
        pool->detachFromThread();
    }
    SkASSERT(instance.fProgramElements.empty());
    SkASSERT(!fContext->fSymbolTable);
    return success ? std::move(result) : nullptr;
}

std::unique_ptr<Expression> Compiler::convertIdentifier(Position pos, std::string_view name) {
    const Symbol* result = this->symbolTable()->find(name);
    if (!result) {
        this->errorReporter().error(pos, "unknown identifier '" + std::string(name) + "'");
        return nullptr;
    }
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration:
            return std::make_unique<FunctionReference>(*fContext, pos,
                                                       &result->as<FunctionDeclaration>());

        case Symbol::Kind::kVariable: {
            const Variable* var = &result->as<Variable>();
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return VariableReference::Make(pos, var, VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const FieldSymbol* field = &result->as<FieldSymbol>();
            auto base = VariableReference::Make(pos, &field->owner(),
                                                VariableReference::RefKind::kRead);
            return FieldAccess::Make(*fContext, pos, std::move(base), field->fieldIndex(),
                                     FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType:
            return TypeReference::Convert(*fContext, pos, &result->as<Type>());

        default:
            SkDEBUGFAILF("unsupported symbol type %d\n", (int)result->kind());
            return nullptr;
    }
}

bool Compiler::optimizeModuleBeforeMinifying(ProgramKind kind, Module& module, bool shrinkSymbols) {
    SkASSERT(this->errorCount() == 0);

    auto m = SkSL::ModuleLoader::Get();

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = kind;
    AutoProgramConfig autoConfig(this->context(), &config);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    if (shrinkSymbols) {
        // Assign shorter names to symbols as long as it won't change the external meaning of the
        // code.
        Transform::RenamePrivateSymbols(this->context(), module, usage.get(), kind);

        // Replace constant variables with their literal values to save space.
        Transform::ReplaceConstVarsWithLiterals(module, usage.get());
    }

    // Remove any unreachable code.
    Transform::EliminateUnreachableCode(module, usage.get());

    // We can only remove dead functions from runtime shaders, since runtime-effect helper functions
    // are isolated from other parts of the program. In a module, an unreferenced function is
    // intended to be called by the code that includes the module.
    if (kind == ProgramKind::kRuntimeShader) {
        while (Transform::EliminateDeadFunctions(this->context(), module, usage.get())) {
            // Removing dead functions may cause more functions to become unreferenced. Try again.
        }
    }

    while (Transform::EliminateDeadLocalVariables(this->context(), module, usage.get())) {
        // Removing dead variables may cause more variables to become unreferenced. Try again.
    }

    // Runtime shaders are isolated from other parts of the program via name mangling, so we can
    // eliminate public globals if they aren't referenced. Otherwise, we only eliminate private
    // globals (prefixed with `$`) to avoid changing the meaning of the module code.
    bool onlyPrivateGlobals = !ProgramConfig::IsRuntimeEffect(kind);
    while (Transform::EliminateDeadGlobalVariables(this->context(), module, usage.get(),
                                                   onlyPrivateGlobals)) {
        // Repeat until no changes occur.
    }

    // We eliminate empty statements to avoid runs of `;;;;;;` caused by the previous passes.
    SkSL::Transform::EliminateEmptyStatements(module);

    // Make sure that program usage is still correct after the optimization pass is complete.
    SkASSERT(*usage == *Analysis::GetUsage(module));

    return this->errorCount() == 0;
}

bool Compiler::optimizeModuleAfterLoading(ProgramKind kind, Module& module) {
    SkASSERT(this->errorCount() == 0);

#ifndef SK_ENABLE_OPTIMIZE_SIZE
    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fIsBuiltinCode = true;
    config.fKind = kind;
    AutoProgramConfig autoConfig(this->context(), &config);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    // Perform inline-candidate analysis and inline any functions deemed suitable.
    Inliner inliner(fContext.get());
    while (this->errorCount() == 0) {
        if (!this->runInliner(&inliner, module.fElements, module.fSymbols, usage.get())) {
            break;
        }
    }
    // Make sure that program usage is still correct after the optimization pass is complete.
    SkASSERT(*usage == *Analysis::GetUsage(module));
#endif

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
        Inliner inliner(fContext.get());
        this->runInliner(&inliner, program.fOwnedElements, program.fSymbols, program.fUsage.get());
#endif

        // Unreachable code can confuse some drivers, so it's worth removing. (skia:12012)
        Transform::EliminateUnreachableCode(program);

        while (Transform::EliminateDeadFunctions(program)) {
            // Removing dead functions may cause more functions to become unreferenced. Try again.
        }
        while (Transform::EliminateDeadLocalVariables(program)) {
            // Removing dead variables may cause more variables to become unreferenced. Try again.
        }
        while (Transform::EliminateDeadGlobalVariables(program)) {
            // Repeat until no changes occur.
        }
        // Make sure that program usage is still correct after the optimization pass is complete.
        SkASSERT(*program.usage() == *Analysis::GetUsage(program));
    }

    return this->errorCount() == 0;
}

void Compiler::runInliner(Program& program) {
#ifndef SK_ENABLE_OPTIMIZE_SIZE
    AutoProgramConfig autoConfig(this->context(), program.fConfig.get());
    AutoShaderCaps autoCaps(fContext, fCaps);
    Inliner inliner(fContext.get());
    this->runInliner(&inliner, program.fOwnedElements, program.fSymbols, program.fUsage.get());
#endif
}

bool Compiler::runInliner(Inliner* inliner,
                          const std::vector<std::unique_ptr<ProgramElement>>& elements,
                          std::shared_ptr<SymbolTable> symbols,
                          ProgramUsage* usage) {
#ifdef SK_ENABLE_OPTIMIZE_SIZE
    return true;
#else
    // The program's SymbolTable was taken out of the context when the program was bundled, but
    // the inliner creates IR objects which may expect the context to hold a valid SymbolTable.
    SkASSERT(!fContext->fSymbolTable);
    fContext->fSymbolTable = symbols;

    bool result = inliner->analyze(elements, symbols, usage);

    fContext->fSymbolTable = nullptr;
    return result;
#endif
}

bool Compiler::finalize(Program& program) {
    AutoShaderCaps autoCaps(fContext, fCaps);

    // Copy all referenced built-in functions into the Program.
    Transform::FindAndDeclareBuiltinFunctions(program);

    // Variables defined in the pre-includes need their declaring elements added to the program.
    Transform::FindAndDeclareBuiltinVariables(program);

    // Do one last correctness-check pass. This looks for dangling FunctionReference/TypeReference
    // expressions, and reports them as errors.
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

    // Make sure that program usage is still correct after finalization is complete.
    SkASSERT(*program.usage() == *Analysis::GetUsage(program));

    return this->errorCount() == 0;
}

#if defined(SKSL_STANDALONE) || defined(SK_GANESH) || defined(SK_GRAPHITE)

#if defined(SK_ENABLE_SPIRV_VALIDATION)
static bool validate_spirv(ErrorReporter& reporter, std::string_view program) {
    SkASSERT(0 == program.size() % 4);
    const uint32_t* programData = reinterpret_cast<const uint32_t*>(program.data());
    size_t programSize = program.size() / 4;

    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
    std::string errors;
    auto msgFn = [&errors](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
        errors += "SPIR-V validation error: ";
        errors += m;
        errors += '\n';
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
        uint32_t options = spvtools::SpirvTools::kDefaultDisassembleOption;
        options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
        if (tools.Disassemble(programData, programSize, &disassembly, options)) {
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
    ThreadContext::Start(this, program.fConfig->fKind, settings);
    ThreadContext::SetErrorReporter(&fErrorReporter);
    fContext->fSymbolTable = program.fSymbols;
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
    ThreadContext::End();
    return result;
}

bool Compiler::toSPIRV(Program& program, std::string* out) {
    StringStream buffer;
    if (!this->toSPIRV(program, buffer)) {
        return false;
    }
    *out = buffer.str();
    return true;
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
    if (!this->toGLSL(program, buffer)) {
        return false;
    }
    *out = buffer.str();
    return true;
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
    if (!this->toMetal(program, buffer)) {
        return false;
    }
    *out = buffer.str();
    return true;
}

#if defined(SK_ENABLE_WGSL_VALIDATION)
static bool validate_wgsl(ErrorReporter& reporter, const std::string& wgsl, std::string* warnings) {
    // Enable the WGSL optional features that Skia might rely on.
    tint::wgsl::reader::Options options;
    for (auto extension : {tint::wgsl::Extension::kChromiumExperimentalPixelLocal,
                           tint::wgsl::Extension::kChromiumInternalDualSourceBlending}) {
        options.allowed_features.extensions.insert(extension);
    }

    // Verify that the WGSL we produced is valid.
    tint::Source::File srcFile("", wgsl);
    tint::Program program(tint::wgsl::reader::Parse(&srcFile, options));

    if (program.Diagnostics().contains_errors()) {
        // The program isn't valid WGSL. In debug, report the error via SkDEBUGFAIL. We also append
        // the generated program for ease of debugging.
        tint::diag::Formatter diagFormatter;
        std::string diagOutput = diagFormatter.format(program.Diagnostics());
        diagOutput += "\n";
        diagOutput += wgsl;
#if defined(SKSL_STANDALONE)
        reporter.error(Position(), diagOutput);
#else
        SkDEBUGFAILF("%s", diagOutput.c_str());
#endif
        return false;
    }

    if (!program.Diagnostics().empty()) {
        // The program contains warnings. Report them as-is.
        tint::diag::Formatter diagFormatter;
        *warnings = diagFormatter.format(program.Diagnostics());
    }
    return true;
}
#endif  // defined(SK_ENABLE_WGSL_VALIDATION)

bool Compiler::toWGSL(Program& program, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::toWGSL");
    AutoSource as(this, *program.fSource);
    AutoShaderCaps autoCaps(fContext, fCaps);
#ifdef SK_ENABLE_WGSL_VALIDATION
    StringStream wgsl;
    WGSLCodeGenerator cg(fContext.get(), &program, &wgsl);
    bool result = cg.generateCode();
    if (result) {
        std::string wgslString = wgsl.str();
        std::string warnings;
        result = validate_wgsl(this->errorReporter(), wgslString, &warnings);
        if (!warnings.empty()) {
            out.writeText("/*\n\n");
            out.writeString(warnings);
            out.writeText("*/\n\n");
        }
        out.writeString(wgslString);
    }
#else
    WGSLCodeGenerator cg(fContext.get(), &program, &out);
    bool result = cg.generateCode();
#endif
    return result;
}

bool Compiler::toWGSL(Program& program, std::string* out) {
    StringStream buffer;
    if (!this->toWGSL(program, buffer)) {
        return false;
    }
    *out = buffer.str();
    return true;
}

#endif // defined(SKSL_STANDALONE) || defined(SK_GANESH) || defined(SK_GRAPHITE)

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
        const int kMaxSurroundingChars = 100;

        // Find the beginning of the line.
        int lineStart = pos.startOffset();
        while (lineStart > 0) {
            if (src[lineStart - 1] == '\n') {
                break;
            }
            --lineStart;
        }

        // We don't want to show more than 100 characters surrounding the error, so push the line
        // start forward and add a leading ellipsis if there would be more than this.
        std::string lineText;
        std::string caretText;
        if ((pos.startOffset() - lineStart) > kMaxSurroundingChars) {
            lineStart = pos.startOffset() - kMaxSurroundingChars;
            lineText = "...";
            caretText = "   ";
        }

        // Echo the line. Again, we don't want to show more than 100 characters after the end of the
        // error, so truncate with a trailing ellipsis if needed.
        const char* lineSuffix = "...\n";
        int lineStop = pos.endOffset() + kMaxSurroundingChars;
        if (lineStop >= (int)src.length()) {
            lineStop = src.length() - 1;
            lineSuffix = "\n";  // no ellipsis if we reach end-of-file
        }
        for (int i = lineStart; i < lineStop; ++i) {
            char c = src[i];
            if (c == '\n') {
                lineSuffix = "\n";  // no ellipsis if we reach end-of-line
                break;
            }
            switch (c) {
                case '\t': lineText += "    "; break;
                case '\0': lineText += " ";    break;
                default:   lineText += src[i]; break;
            }
        }
        fErrorText += lineText + lineSuffix;

        // print the carets underneath it, pointing to the range in question
        for (int i = lineStart; i < (int)src.length(); i++) {
            if (i >= pos.endOffset()) {
                break;
            }
            switch (src[i]) {
                case '\t':
                   caretText += (i >= pos.startOffset()) ? "^^^^" : "    ";
                   break;
                case '\n':
                    SkASSERT(i >= pos.startOffset());
                    // use an ellipsis if the error continues past the end of the line
                    caretText += (pos.endOffset() > i + 1) ? "..." : "^";
                    i = src.length();
                    break;
                default:
                    caretText += (i >= pos.startOffset()) ? '^' : ' ';
                    break;
            }
        }
        fErrorText += caretText + '\n';
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
