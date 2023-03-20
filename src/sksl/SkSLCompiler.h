/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_COMPILER
#define SKSL_COMPILER

#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLContext.h"  // IWYU pragma: keep

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#define SK_FRAGCOLOR_BUILTIN           10001
#define SK_LASTFRAGCOLOR_BUILTIN       10008
#define SK_MAIN_COORDS_BUILTIN         10009
#define SK_INPUT_COLOR_BUILTIN         10010
#define SK_DEST_COLOR_BUILTIN          10011
#define SK_SECONDARYFRAGCOLOR_BUILTIN  10012
#define SK_FRAGCOORD_BUILTIN              15
#define SK_CLOCKWISE_BUILTIN              17

#define SK_VERTEXID_BUILTIN               42
#define SK_INSTANCEID_BUILTIN             43
#define SK_POSITION_BUILTIN                0
#define SK_POINTSIZE_BUILTIN               1

#define SK_NUMWORKGROUPS_BUILTIN          24
#define SK_WORKGROUPID_BUILTIN            26
#define SK_LOCALINVOCATIONID_BUILTIN      27
#define SK_GLOBALINVOCATIONID_BUILTIN     28
#define SK_LOCALINVOCATIONINDEX_BUILTIN   29

namespace SkSL {

namespace dsl {
    class DSLCore;
}

class Expression;
class Inliner;
class ModifiersPool;
class OutputStream;
class ProgramUsage;
class SymbolTable;
enum class ProgramKind : int8_t;
struct Program;
struct ProgramSettings;
struct ShaderCaps;

struct Module {
    const Module*                                fParent = nullptr;
    std::shared_ptr<SymbolTable>                 fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
};

/**
 * Main compiler entry point. The compiler parses the SkSL text directly into a tree of IRNodes,
 * while performing basic optimizations such as constant-folding and dead-code elimination. Then the
 * Program is passed into a CodeGenerator to produce compiled output.
 *
 * See the README for information about SkSL.
 */
class SK_API Compiler {
public:
    inline static constexpr const char FRAGCOLOR_NAME[] = "sk_FragColor";
    inline static constexpr const char RTADJUST_NAME[]  = "sk_RTAdjust";
    inline static constexpr const char POSITION_NAME[]  = "sk_Position";
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
     * Uniform values used by the compiler to implement origin-neutral dFdy, sk_Clockwise, and
     * sk_FragCoord.
     */
    static std::array<float, 2> GetRTFlipVector(int rtHeight, bool flipY) {
        std::array<float, 2> result;
        result[0] = flipY ? rtHeight : 0.f;
        result[1] = flipY ?     -1.f : 1.f;
        return result;
    }

    Compiler(const ShaderCaps* caps);

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

    std::unique_ptr<Program> convertProgram(ProgramKind kind,
                                            std::string text,
                                            ProgramSettings settings);

    std::unique_ptr<Expression> convertIdentifier(Position pos, std::string_view name);

    bool toSPIRV(Program& program, OutputStream& out);

    bool toSPIRV(Program& program, std::string* out);

    bool toGLSL(Program& program, OutputStream& out);

    bool toGLSL(Program& program, std::string* out);

    bool toHLSL(Program& program, OutputStream& out);

    bool toHLSL(Program& program, std::string* out);

    bool toMetal(Program& program, OutputStream& out);

    bool toMetal(Program& program, std::string* out);

    bool toWGSL(Program& program, OutputStream& out);

    void handleError(std::string_view msg, Position pos);

    std::string errorText(bool showCount = true);

    ErrorReporter& errorReporter() { return *fContext->fErrors; }

    int errorCount() const { return fContext->fErrors->errorCount(); }

    void writeErrorCount();

    void resetErrors() {
        fErrorText.clear();
        this->errorReporter().resetErrorCount();
    }

    Context& context() const {
        return *fContext;
    }

    std::shared_ptr<SymbolTable>& symbolTable() {
        return fSymbolTable;
    }

    std::unique_ptr<Module> compileModule(ProgramKind kind,
                                          const char* moduleName,
                                          std::string moduleSource,
                                          const Module* parent,
                                          ModifiersPool& modifiersPool,
                                          bool shouldInline);

    /** Optimize a module at minification time, before writing it out. */
    bool optimizeModuleBeforeMinifying(ProgramKind kind, Module& module);

    const Module* moduleForProgramKind(ProgramKind kind);

private:
    class CompilerErrorReporter : public ErrorReporter {
    public:
        CompilerErrorReporter(Compiler* compiler)
            : fCompiler(*compiler) {}

        void handleError(std::string_view msg, Position pos) override {
            fCompiler.handleError(msg, pos);
        }

    private:
        Compiler& fCompiler;
    };

    /** Updates ProgramSettings to eliminate contradictions and to honor the ProgramKind. */
    static void FinalizeSettings(ProgramSettings* settings, ProgramKind kind);

    /** Optimize every function in the program. */
    bool optimize(Program& program);

    /** Performs final checks to confirm that a fully-assembled/optimized is valid. */
    bool finalize(Program& program);

    /** Optimize a module at Skia runtime, after loading it. */
    bool optimizeModuleAfterLoading(ProgramKind kind, Module& module);

    /** Flattens out function calls when it is safe to do so. */
    bool runInliner(Inliner* inliner,
                    const std::vector<std::unique_ptr<ProgramElement>>& elements,
                    std::shared_ptr<SymbolTable> symbols,
                    ProgramUsage* usage);

    CompilerErrorReporter fErrorReporter;
    std::shared_ptr<Context> fContext;
    const ShaderCaps* fCaps;

    // This is the current symbol table of the code we are processing, and therefore changes during
    // compilation
    std::shared_ptr<SymbolTable> fSymbolTable;

    std::string fErrorText;

    static OverrideFlag sOptimizer;
    static OverrideFlag sInliner;

    friend class ThreadContext;
    friend class dsl::DSLCore;
};

}  // namespace SkSL

#endif
