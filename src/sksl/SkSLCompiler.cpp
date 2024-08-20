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
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"  // IWYU pragma: keep
#include "src/sksl/ir/SkSLSymbolTable.h"     // IWYU pragma: keep
#include "src/sksl/transform/SkSLTransform.h"

#include <cstdint>
#include <memory>
#include <utility>

#if defined(SKSL_STANDALONE)
#include <fstream>
#endif

namespace SkSL {

// These flags allow tools like Viewer or Nanobench to override the compiler's ProgramSettings.
Compiler::OverrideFlag Compiler::sOptimizer = OverrideFlag::kDefault;
Compiler::OverrideFlag Compiler::sInliner = OverrideFlag::kDefault;

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

Compiler::Compiler() : fErrorReporter(this) {
    auto moduleLoader = ModuleLoader::Get();
    fContext = std::make_shared<Context>(moduleLoader.builtinTypes(), fErrorReporter);
}

Compiler::~Compiler() {}

const Module* Compiler::moduleForProgramKind(ProgramKind kind) {
    auto m = ModuleLoader::Get();
    switch (kind) {
        case ProgramKind::kFragment:              return m.loadFragmentModule(this);
        case ProgramKind::kVertex:                return m.loadVertexModule(this);
        case ProgramKind::kCompute:               return m.loadComputeModule(this);
        case ProgramKind::kGraphiteFragment:      return m.loadGraphiteFragmentModule(this);
        case ProgramKind::kGraphiteVertex:        return m.loadGraphiteVertexModule(this);
        case ProgramKind::kGraphiteFragmentES2:   return m.loadGraphiteFragmentES2Module(this);
        case ProgramKind::kGraphiteVertexES2:     return m.loadGraphiteVertexES2Module(this);
        case ProgramKind::kPrivateRuntimeBlender:
        case ProgramKind::kPrivateRuntimeColorFilter:
        case ProgramKind::kPrivateRuntimeShader:  return m.loadPrivateRTShaderModule(this);
        case ProgramKind::kRuntimeColorFilter:
        case ProgramKind::kRuntimeShader:
        case ProgramKind::kRuntimeBlender:
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

void Compiler::initializeContext(const SkSL::Module* module,
                                 ProgramKind kind,
                                 ProgramSettings settings,
                                 std::string_view source,
                                 ModuleType moduleType) {
    SkASSERT(!fPool);
    SkASSERT(!fConfig);
    SkASSERT(!fContext->fSymbolTable);
    SkASSERT(!fContext->fConfig);
    SkASSERT(!fContext->fModule);

    // Start the ErrorReporter with a clean slate.
    this->resetErrors();

    fConfig = std::make_unique<ProgramConfig>();
    fConfig->fModuleType = moduleType;
    fConfig->fSettings = settings;
    fConfig->fKind = kind;

    // Make sure the passed-in settings are valid.
    FinalizeSettings(&fConfig->fSettings, kind);

    if (settings.fUseMemoryPool) {
        fPool = Pool::Create();
        fPool->attachToThread();
    }

    fContext->fConfig = fConfig.get();
    fContext->fModule = module;
    fContext->fErrors->setSource(source);

    // Set up a clean symbol table atop the parent module's symbols.
    fGlobalSymbols = std::make_unique<SymbolTable>(module->fSymbols.get(),
                                                   moduleType != ModuleType::program);
    fGlobalSymbols->markModuleBoundary();
    fContext->fSymbolTable = fGlobalSymbols.get();
}

void Compiler::cleanupContext() {
    // Clear out the fields we initialized above.
    fContext->fConfig = nullptr;
    fContext->fModule = nullptr;
    fContext->fErrors->setSource(std::string_view());
    fContext->fSymbolTable = nullptr;

    fConfig = nullptr;
    fGlobalSymbols = nullptr;

    if (fPool) {
        fPool->detachFromThread();
        fPool = nullptr;
    }
}

std::unique_ptr<Module> Compiler::compileModule(ProgramKind kind,
                                                ModuleType moduleType,
                                                std::string moduleSource,
                                                const Module* parentModule,
                                                bool shouldInline) {
    SkASSERT(parentModule);
    SkASSERT(this->errorCount() == 0);

    // Wrap the program source in a pointer so it is guaranteed to be stable across moves.
    auto sourcePtr = std::make_unique<std::string>(std::move(moduleSource));

    // Compile the module from source, using default program settings (but no memory pooling).
    ProgramSettings settings;
    settings.fUseMemoryPool = false;
    this->initializeContext(parentModule, kind, settings, *sourcePtr, moduleType);

    std::unique_ptr<Module> module = SkSL::Parser(this, settings, kind, std::move(sourcePtr))
                                             .moduleInheritingFrom(parentModule);

    this->cleanupContext();

    if (this->errorCount() != 0) {
        SkDebugf("Unexpected errors compiling %s:\n\n%s\n",
                 ModuleTypeToString(moduleType),
                 this->errorText().c_str());
        return nullptr;
    }
    if (shouldInline) {
        this->optimizeModuleAfterLoading(kind, *module);
    }
    return module;
}

std::unique_ptr<Program> Compiler::convertProgram(ProgramKind kind,
                                                  std::string programSource,
                                                  const ProgramSettings& settings) {
    TRACE_EVENT0("skia.shaders", "SkSL::Compiler::convertProgram");

    // Wrap the program source in a pointer so it is guaranteed to be stable across moves.
    auto sourcePtr = std::make_unique<std::string>(std::move(programSource));

    // Load the module used by this ProgramKind.
    const SkSL::Module* module = this->moduleForProgramKind(kind);

    this->initializeContext(module, kind, settings, *sourcePtr, ModuleType::program);

    std::unique_ptr<Program> program = SkSL::Parser(this, settings, kind, std::move(sourcePtr))
                                               .programInheritingFrom(module);

    this->cleanupContext();
    return program;
}

std::unique_ptr<SkSL::Program> Compiler::releaseProgram(
        std::unique_ptr<std::string> source,
        std::vector<std::unique_ptr<SkSL::ProgramElement>> programElements) {
    Pool* pool = fPool.get();
    auto result = std::make_unique<SkSL::Program>(std::move(source),
                                                  std::move(fConfig),
                                                  fContext,
                                                  std::move(programElements),
                                                  std::move(fGlobalSymbols),
                                                  std::move(fPool));
    fContext->fSymbolTable = nullptr;

    bool success = this->finalize(*result) &&
                   this->optimize(*result);
    if (pool) {
        pool->detachFromThread();
    }
    return success ? std::move(result) : nullptr;
}

bool Compiler::optimizeModuleBeforeMinifying(ProgramKind kind, Module& module, bool shrinkSymbols) {
    SkASSERT(this->errorCount() == 0);

    auto m = SkSL::ModuleLoader::Get();

    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fModuleType = module.fModuleType;
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

    // We can eliminate `{}` around single-statement blocks.
    SkSL::Transform::EliminateUnnecessaryBraces(this->context(), module);

    // We can convert `float4(myFloat)` with `myFloat.xxxx` to save a few characters.
    SkSL::Transform::ReplaceSplatCastsWithSwizzles(this->context(), module);

    // Make sure that program usage is still correct after the optimization pass is complete.
    SkASSERT(*usage == *Analysis::GetUsage(module));

    return this->errorCount() == 0;
}

bool Compiler::optimizeModuleAfterLoading(ProgramKind kind, Module& module) {
    SkASSERT(this->errorCount() == 0);

#ifndef SK_ENABLE_OPTIMIZE_SIZE
    // Create a temporary program configuration with default settings.
    ProgramConfig config;
    config.fModuleType = module.fModuleType;
    config.fKind = kind;
    AutoProgramConfig autoConfig(this->context(), &config);

    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    // Perform inline-candidate analysis and inline any functions deemed suitable.
    Inliner inliner(fContext.get());
    while (this->errorCount() == 0) {
        if (!this->runInliner(&inliner, module.fElements, module.fSymbols.get(), usage.get())) {
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

    SkASSERT(!this->errorCount());
    if (this->errorCount() == 0) {
#ifndef SK_ENABLE_OPTIMIZE_SIZE
        // Run the inliner only once; it is expensive! Multiple passes can occasionally shake out
        // more wins, but it's diminishing returns.
        Inliner inliner(fContext.get());
        this->runInliner(&inliner, program.fOwnedElements, program.fSymbols.get(),
                         program.fUsage.get());
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

        // Make sure that variables are still declared in the correct symbol tables.
        SkDEBUGCODE(Analysis::CheckSymbolTableCorrectness(program));
    }

    return this->errorCount() == 0;
}

void Compiler::runInliner(Program& program) {
#ifndef SK_ENABLE_OPTIMIZE_SIZE
    AutoProgramConfig autoConfig(this->context(), program.fConfig.get());
    Inliner inliner(fContext.get());
    this->runInliner(&inliner, program.fOwnedElements, program.fSymbols.get(),
                     program.fUsage.get());
#endif
}

bool Compiler::runInliner(Inliner* inliner,
                          const std::vector<std::unique_ptr<ProgramElement>>& elements,
                          SymbolTable* symbols,
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
    // Copy all referenced built-in functions into the Program.
    Transform::FindAndDeclareBuiltinFunctions(program);

    // Variables defined in modules need their declaring elements added to the program.
    Transform::FindAndDeclareBuiltinVariables(program);

    // Structs from module code need to be added to the program's shared elements.
    Transform::FindAndDeclareBuiltinStructs(program);

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
        Analysis::CheckProgramStructure(program);

        // Make sure that variables are declared in the symbol tables that immediately enclose them.
        SkDEBUGCODE(Analysis::CheckSymbolTableCorrectness(program));
    }

    // Make sure that program usage is still correct after finalization is complete.
    SkASSERT(*program.usage() == *Analysis::GetUsage(program));

    return this->errorCount() == 0;
}

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
        fErrorText += std::to_string(count) +
                      ((count == 1) ? " error\n" : " errors\n");
    }
}

}  // namespace SkSL
