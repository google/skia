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
#include "src/sksl/SkSLContext.h"  // IWYU pragma: keep
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

constexpr int SK_FRAGCOLOR_BUILTIN =           10001;
constexpr int SK_LASTFRAGCOLOR_BUILTIN =       10008;
constexpr int SK_SECONDARYFRAGCOLOR_BUILTIN =  10012;
constexpr int SK_FRAGCOORD_BUILTIN =              15;
constexpr int SK_CLOCKWISE_BUILTIN =              17;
constexpr int SK_SAMPLEMASKIN_BUILTIN =           20;
constexpr int SK_SAMPLEMASK_BUILTIN =          10020;

constexpr int SK_VERTEXID_BUILTIN =               42;
constexpr int SK_INSTANCEID_BUILTIN =             43;
constexpr int SK_POSITION_BUILTIN =                0;
constexpr int SK_POINTSIZE_BUILTIN =               1;

constexpr int SK_NUMWORKGROUPS_BUILTIN =          24;
constexpr int SK_WORKGROUPID_BUILTIN =            26;
constexpr int SK_LOCALINVOCATIONID_BUILTIN =      27;
constexpr int SK_GLOBALINVOCATIONID_BUILTIN =     28;
constexpr int SK_LOCALINVOCATIONINDEX_BUILTIN =   29;

namespace SkSL {

class Inliner;
struct Module;
enum class ModuleType : int8_t;
class Pool;
struct ProgramConfig;
class ProgramUsage;
enum class ProgramKind : int8_t;
struct Program;
class ProgramElement;
struct ProgramSettings;
class SymbolTable;

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

    Compiler();
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
                                            std::string programSource,
                                            const ProgramSettings& settings);

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

    SymbolTable* globalSymbols() {
        return fGlobalSymbols.get();
    }

    SymbolTable* symbolTable() {
        return fContext->fSymbolTable;
    }

    std::unique_ptr<Module> compileModule(ProgramKind kind,
                                          ModuleType moduleType,
                                          std::string moduleSource,
                                          const Module* parentModule,
                                          bool shouldInline);

    /** Optimize a module at minification time, before writing it out. */
    bool optimizeModuleBeforeMinifying(ProgramKind kind, Module& module, bool shrinkSymbols);

    const Module* moduleForProgramKind(ProgramKind kind);

    /** Run the inliner on a program which was compiled earlier (with inlining turned off). */
    void runInliner(Program& program);

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

    /** Prepares the Context for compilation of a program or module. */
    void initializeContext(const SkSL::Module* module,
                           ProgramKind kind,
                           ProgramSettings settings,
                           std::string_view source,
                           ModuleType moduleType);

    /** Cleans up the Context post-compilation. */
    void cleanupContext();

    /**
     * Returns all global elements (functions and global variables) as a self-contained Program.
     * The optional source string is retained as the program's source.
     */
    std::unique_ptr<SkSL::Program> releaseProgram(
            std::unique_ptr<std::string> source,
            std::vector<std::unique_ptr<SkSL::ProgramElement>> programElements);

    /** Optimize every function in the program. */
    bool optimize(Program& program);

    /** Performs final checks to confirm that a fully-assembled/optimized is valid. */
    bool finalize(Program& program);

    /** Optimize a module at Skia runtime, after loading it. */
    bool optimizeModuleAfterLoading(ProgramKind kind, Module& module);

    /** Flattens out function calls when it is safe to do so. */
    bool runInliner(Inliner* inliner,
                    const std::vector<std::unique_ptr<ProgramElement>>& elements,
                    SymbolTable* symbols,
                    ProgramUsage* usage);

    CompilerErrorReporter fErrorReporter;
    std::shared_ptr<Context> fContext;
    std::unique_ptr<SymbolTable> fGlobalSymbols;
    std::unique_ptr<ProgramConfig> fConfig;
    std::unique_ptr<Pool> fPool;

    std::string fErrorText;

    static OverrideFlag sOptimizer;
    static OverrideFlag sInliner;

    friend class Parser;
    friend class ThreadContext;
};

}  // namespace SkSL

#endif
