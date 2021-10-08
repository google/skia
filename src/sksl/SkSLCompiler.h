/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_COMPILER
#define SKSL_COMPILER

#include <set>
#include <unordered_set>
#include <vector>
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLInliner.h"
#include "src/sksl/SkSLParsedModule.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrShaderVar.h"
#endif

#define SK_FRAGCOLOR_BUILTIN           10001
#define SK_LASTFRAGCOLOR_BUILTIN       10008
#define SK_MAIN_COORDS_BUILTIN         10009
#define SK_INPUT_COLOR_BUILTIN         10010
#define SK_DEST_COLOR_BUILTIN          10011
#define SK_FRAGCOORD_BUILTIN              15
#define SK_CLOCKWISE_BUILTIN              17
#define SK_VERTEXID_BUILTIN               42
#define SK_INSTANCEID_BUILTIN             43
#define SK_POSITION_BUILTIN                0

class SkBitSet;
class SkSLCompileBench;

namespace SkSL {

namespace dsl {
    class DSLCore;
    class DSLWriter;
}

class ExternalFunction;
class FunctionDeclaration;
class IRGenerator;
class ProgramUsage;

struct LoadedModule {
    ProgramKind                                  fKind;
    std::shared_ptr<SymbolTable>                 fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
};

/**
 * Main compiler entry point. This is a traditional compiler design which first parses the .sksl
 * file into an abstract syntax tree (a tree of ASTNodes), then performs semantic analysis to
 * produce a Program (a tree of IRNodes), then feeds the Program into a CodeGenerator to produce
 * compiled output.
 *
 * See the README for information about SkSL.
 */
class SK_API Compiler {
public:
    inline static constexpr const char FRAGCOLOR_NAME[] = "sk_FragColor";
    inline static constexpr const char RTADJUST_NAME[]  = "sk_RTAdjust";
    inline static constexpr const char PERVERTEX_NAME[] = "sk_PerVertex";
    inline static constexpr const char POISON_TAG[]     = "<POISON>";

    /**
     * Gets a float4 that adjusts the position from Skia device coords to normalized device coords,
     * used to populate sk_RTAdjust.  Assuming the transformed position, pos, is a homogeneous
     * float4, the vec, v, is applied as such:
     * float4((pos.xy * v.xz) + sk_Position.ww * v.yw, 0, pos.w);
     */
    static std::array<float, 4> GetRTAdjustVector(SkISize rtDims, bool flipY) {
        std::array<float, 4> result;
        result[0] = 2.f/rtDims.width();
        result[2] = 2.f/rtDims.height();
        result[1] = -1.f;
        result[3] = -1.f;
        if (flipY) {
            result[2] = -result[2];
            result[3] = -result[3];
        }
        return result;
    }

    /**
     * Uniform values  by the compiler to implement origin-neutral dFdy, sk_Clockwise, and
     * sk_FragCoord.
     */
    static std::array<float, 2> GetRTFlipVector(int rtHeight, bool flipY) {
        std::array<float, 2> result;
        result[0] = flipY ? rtHeight : 0.f;
        result[1] = flipY ?     -1.f : 1.f;
        return result;
    }

    struct OptimizationContext {
        // nodes we have already reported errors for and should not error on again
        std::unordered_set<const IRNode*> fSilences;
        // true if we have updated the CFG during this pass
        bool fUpdated = false;
        // true if we need to completely regenerate the CFG
        bool fNeedsRescan = false;
        // Metadata about function and variable usage within the program
        ProgramUsage* fUsage = nullptr;
        // Nodes which we can't throw away until the end of optimization
        StatementArray fOwnedStatements;
    };

    Compiler(const ShaderCapsClass* caps);

    ~Compiler();

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;

    /**
     * Allows optimization settings to be unilaterally overridden. This is meant to allow tools like
     * Viewer or Nanobench to override the compiler's ProgramSettings and ShaderCaps for debugging.
     */
    enum class OverrideFlag {
        kDefault,
        kOff,
        kOn,
    };
    static void EnableOptimizer(OverrideFlag flag) { sOptimizer = flag; }
    static void EnableInliner(OverrideFlag flag) { sInliner = flag; }

    /**
     * If fExternalFunctions is supplied in the settings, those values are registered in the symbol
     * table of the Program, but ownership is *not* transferred. It is up to the caller to keep them
     * alive.
     */
    std::unique_ptr<Program> convertProgram(
            ProgramKind kind,
            String text,
            Program::Settings settings);

    bool toSPIRV(Program& program, OutputStream& out);

    bool toSPIRV(Program& program, String* out);

    bool toGLSL(Program& program, OutputStream& out);

    bool toGLSL(Program& program, String* out);

    bool toHLSL(Program& program, String* out);

    bool toMetal(Program& program, OutputStream& out);

    bool toMetal(Program& program, String* out);

    void handleError(skstd::string_view msg, PositionInfo pos);

    String errorText(bool showCount = true);

    ErrorReporter& errorReporter() { return *fContext->fErrors; }

    int errorCount() const { return fContext->fErrors->errorCount(); }

    void writeErrorCount();

    void resetErrors() {
        fErrorText.clear();
        this->errorReporter().resetErrorCount();
    }

    Context& context() {
        return *fContext;
    }

    // When  SKSL_STANDALONE, fPath is used. (fData, fSize) will be (nullptr, 0)
    // When !SKSL_STANDALONE, fData and fSize are used. fPath will be nullptr.
    struct ModuleData {
        const char*    fPath;

        const uint8_t* fData;
        size_t         fSize;
    };

    static ModuleData MakeModulePath(const char* path) {
        return ModuleData{path, /*fData=*/nullptr, /*fSize=*/0};
    }
    static ModuleData MakeModuleData(const uint8_t* data, size_t size) {
        return ModuleData{/*fPath=*/nullptr, data, size};
    }

    LoadedModule loadModule(ProgramKind kind, ModuleData data, std::shared_ptr<SymbolTable> base,
                            bool dehydrate);
    ParsedModule parseModule(ProgramKind kind, ModuleData data, const ParsedModule& base);

    IRGenerator& irGenerator() {
        return *fIRGenerator;
    }

    const ParsedModule& moduleForProgramKind(ProgramKind kind);

private:
    class CompilerErrorReporter : public ErrorReporter {
    public:
        CompilerErrorReporter(Compiler* compiler)
            : fCompiler(*compiler) {}

        void handleError(skstd::string_view msg, PositionInfo pos) override {
            fCompiler.handleError(msg, pos);
        }

    private:
        Compiler& fCompiler;
    };

    const ParsedModule& loadGPUModule();
    const ParsedModule& loadFragmentModule();
    const ParsedModule& loadVertexModule();
    const ParsedModule& loadPublicModule();
    const ParsedModule& loadRuntimeShaderModule();

    std::shared_ptr<SymbolTable> makeRootSymbolTable();
    std::shared_ptr<SymbolTable> makePrivateSymbolTable(std::shared_ptr<SymbolTable> parent);

    /** Optimize every function in the program. */
    bool optimize(Program& program);

    /** Performs final checks to confirm that a fully-assembled/optimized is valid. */
    bool finalize(Program& program);

    /** Optimize the module. */
    bool optimize(LoadedModule& module);

    /** Flattens out function calls when it is safe to do so. */
    bool runInliner(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                    std::shared_ptr<SymbolTable> symbols,
                    ProgramUsage* usage);

    CompilerErrorReporter fErrorReporter;
    std::shared_ptr<Context> fContext;

    ParsedModule fRootModule;                // Core types

    ParsedModule fPrivateModule;             // [Root] + Internal types
    ParsedModule fGPUModule;                 // [Private] + GPU intrinsics, helper functions
    ParsedModule fVertexModule;              // [GPU] + Vertex stage decls
    ParsedModule fFragmentModule;            // [GPU] + Fragment stage decls

    ParsedModule fPublicModule;              // [Root] + Public features
    ParsedModule fRuntimeShaderModule;       // [Public] + Runtime shader decls

    // holds ModifiersPools belonging to the core includes for lifetime purposes
    ModifiersPool fCoreModifiers;

    Mangler fMangler;
    Inliner fInliner;
    std::unique_ptr<IRGenerator> fIRGenerator;

    String fErrorText;

    static OverrideFlag sOptimizer;
    static OverrideFlag sInliner;

    friend class AutoSource;
    friend class ::SkSLCompileBench;
    friend class DSLParser;
    friend class ThreadContext;
    friend class dsl::DSLCore;
};

}  // namespace SkSL

#endif
