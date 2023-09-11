/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SK_OPTS_NS skslc_standalone
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "src/base/SkStringView.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "src/utils/SkShaderUtils.h"
#include "tools/skslc/ProcessWorklist.h"

#include "spirv-tools/libspirv.hpp"

#include <fstream>
#include <limits.h>
#include <optional>
#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

namespace SkOpts {
    size_t raster_pipeline_highp_stride = 1;
}

static std::unique_ptr<SkWStream> as_SkWStream(SkSL::OutputStream& s) {
    struct Adapter : public SkWStream {
    public:
        Adapter(SkSL::OutputStream& out) : fOut(out), fBytesWritten(0) {}

        bool write(const void* buffer, size_t size) override {
            fOut.write(buffer, size);
            fBytesWritten += size;
            return true;
        }
        void flush() override {}
        size_t bytesWritten() const override { return fBytesWritten; }

    private:
        SkSL::OutputStream& fOut;
        size_t fBytesWritten;
    };

    return std::make_unique<Adapter>(s);
}

static bool consume_suffix(std::string* str, const char suffix[]) {
    if (!skstd::ends_with(*str, suffix)) {
        return false;
    }
    str->resize(str->length() - strlen(suffix));
    return true;
}

class ShaderCapsTestFactory : public SkSL::ShaderCapsFactory {
public:
    static const SkSL::ShaderCaps* AddAndTrueToLoopCondition() {
        static const SkSL::ShaderCaps* sCaps = []{
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fAddAndTrueToLoopCondition = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* CannotUseFractForNegativeValues() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fCanUseFractForNegativeValues = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* CannotUseFragCoord() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fCanUseFragCoord = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* CannotUseMinAndAbsTogether() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fCanUseMinAndAbsTogether = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* CannotUseVoidInSequenceExpressions() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fCanUseVoidInSequenceExpressions = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* EmulateAbsIntFunction() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fEmulateAbsIntFunction = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* FramebufferFetchSupport() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fFBFetchSupport = true;
            caps->fFBFetchColorName = "gl_LastFragData[0]";
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* IncompleteShortIntPrecision() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 310es";
            caps->fUsesPrecisionModifiers = true;
            caps->fIncompleteShortIntPrecision = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* MustForceNegatedAtanParamToFloat() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fMustForceNegatedAtanParamToFloat = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* MustForceNegatedLdexpParamToMultiply() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fMustForceNegatedLdexpParamToMultiply = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* MustGuardDivisionEvenAfterExplicitZeroCheck() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* NoBuiltinDeterminantSupport() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fBuiltinDeterminantSupport = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* NoBuiltinFMASupport() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fBuiltinFMASupport = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* NoExternalTextureSupport() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fExternalTextureSupport = false;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* RemovePowWithConstantExponent() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fRemovePowWithConstantExponent = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* RewriteDoWhileLoops() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fRewriteDoWhileLoops = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* RewriteMatrixComparisons() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fRewriteMatrixComparisons = true;
            caps->fUsesPrecisionModifiers = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* RewriteMatrixVectorMultiply() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fRewriteMatrixVectorMultiply = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* RewriteSwitchStatements() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fRewriteSwitchStatements = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* SampleMaskSupport() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fShaderDerivativeSupport = true;
            caps->fSampleMaskSupport = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* ShaderDerivativeExtensionString() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fShaderDerivativeSupport = true;
            caps->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
            caps->fUsesPrecisionModifiers = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* UnfoldShortCircuitAsTernary() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fUnfoldShortCircuitAsTernary = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* UsesPrecisionModifiers() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fUsesPrecisionModifiers = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* Version110() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 110";
            caps->fGLSLGeneration = SkSL::GLSLGeneration::k110;
            return caps.release();
        }();
        return sCaps;
    }

    static const SkSL::ShaderCaps* Version450Core() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<SkSL::ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 450 core";
            return caps.release();
        }();
        return sCaps;
    }
};

// Given a string containing an SkSL program, searches for a #pragma settings comment, like so:
//    /*#pragma settings Default Sharpen*/
// The passed-in Settings object will be updated accordingly. Any number of options can be provided.
static bool detect_shader_settings(const std::string& text,
                                   SkSL::ProgramSettings* settings,
                                   const SkSL::ShaderCaps** caps,
                                   std::unique_ptr<SkSL::DebugTracePriv>* debugTrace) {
    using Factory = ShaderCapsTestFactory;

    // Find a matching comment and isolate the name portion.
    static constexpr char kPragmaSettings[] = "/*#pragma settings ";
    const char* settingsPtr = strstr(text.c_str(), kPragmaSettings);
    if (settingsPtr != nullptr) {
        // Subtract one here in order to preserve the leading space, which is necessary to allow
        // consumeSuffix to find the first item.
        settingsPtr += strlen(kPragmaSettings) - 1;

        const char* settingsEnd = strstr(settingsPtr, "*/");
        if (settingsEnd != nullptr) {
            std::string settingsText{settingsPtr, size_t(settingsEnd - settingsPtr)};

            // Apply settings as requested. Since they can come in any order, repeat until we've
            // consumed them all.
            for (;;) {
                const size_t startingLength = settingsText.length();

                if (consume_suffix(&settingsText, " AddAndTrueToLoopCondition")) {
                    *caps = Factory::AddAndTrueToLoopCondition();
                }
                if (consume_suffix(&settingsText, " CannotUseFractForNegativeValues")) {
                    *caps = Factory::CannotUseFractForNegativeValues();
                }
                if (consume_suffix(&settingsText, " CannotUseFragCoord")) {
                    *caps = Factory::CannotUseFragCoord();
                }
                if (consume_suffix(&settingsText, " CannotUseMinAndAbsTogether")) {
                    *caps = Factory::CannotUseMinAndAbsTogether();
                }
                if (consume_suffix(&settingsText, " CannotUseVoidInSequenceExpressions")) {
                    *caps = Factory::CannotUseVoidInSequenceExpressions();
                }
                if (consume_suffix(&settingsText, " Default")) {
                    *caps = Factory::Default();
                }
                if (consume_suffix(&settingsText, " EmulateAbsIntFunction")) {
                    *caps = Factory::EmulateAbsIntFunction();
                }
                if (consume_suffix(&settingsText, " FramebufferFetchSupport")) {
                    *caps = Factory::FramebufferFetchSupport();
                }
                if (consume_suffix(&settingsText, " IncompleteShortIntPrecision")) {
                    *caps = Factory::IncompleteShortIntPrecision();
                }
                if (consume_suffix(&settingsText, " MustGuardDivisionEvenAfterExplicitZeroCheck")) {
                    *caps = Factory::MustGuardDivisionEvenAfterExplicitZeroCheck();
                }
                if (consume_suffix(&settingsText, " MustForceNegatedAtanParamToFloat")) {
                    *caps = Factory::MustForceNegatedAtanParamToFloat();
                }
                if (consume_suffix(&settingsText, " MustForceNegatedLdexpParamToMultiply")) {
                    *caps = Factory::MustForceNegatedLdexpParamToMultiply();
                }
                if (consume_suffix(&settingsText, " NoBuiltinDeterminantSupport")) {
                    *caps = Factory::NoBuiltinDeterminantSupport();
                }
                if (consume_suffix(&settingsText, " NoBuiltinFMASupport")) {
                    *caps = Factory::NoBuiltinFMASupport();
                }
                if (consume_suffix(&settingsText, " NoExternalTextureSupport")) {
                    *caps = Factory::NoExternalTextureSupport();
                }
                if (consume_suffix(&settingsText, " RemovePowWithConstantExponent")) {
                    *caps = Factory::RemovePowWithConstantExponent();
                }
                if (consume_suffix(&settingsText, " RewriteDoWhileLoops")) {
                    *caps = Factory::RewriteDoWhileLoops();
                }
                if (consume_suffix(&settingsText, " RewriteSwitchStatements")) {
                    *caps = Factory::RewriteSwitchStatements();
                }
                if (consume_suffix(&settingsText, " RewriteMatrixVectorMultiply")) {
                    *caps = Factory::RewriteMatrixVectorMultiply();
                }
                if (consume_suffix(&settingsText, " RewriteMatrixComparisons")) {
                    *caps = Factory::RewriteMatrixComparisons();
                }
                if (consume_suffix(&settingsText, " ShaderDerivativeExtensionString")) {
                    *caps = Factory::ShaderDerivativeExtensionString();
                }
                if (consume_suffix(&settingsText, " UnfoldShortCircuitAsTernary")) {
                    *caps = Factory::UnfoldShortCircuitAsTernary();
                }
                if (consume_suffix(&settingsText, " UsesPrecisionModifiers")) {
                    *caps = Factory::UsesPrecisionModifiers();
                }
                if (consume_suffix(&settingsText, " Version110")) {
                    *caps = Factory::Version110();
                }
                if (consume_suffix(&settingsText, " Version450Core")) {
                    *caps = Factory::Version450Core();
                }
                if (consume_suffix(&settingsText, " AllowNarrowingConversions")) {
                    settings->fAllowNarrowingConversions = true;
                }
                if (consume_suffix(&settingsText, " ForceHighPrecision")) {
                    settings->fForceHighPrecision = true;
                }
                if (consume_suffix(&settingsText, " NoInline")) {
                    settings->fInlineThreshold = 0;
                }
                if (consume_suffix(&settingsText, " NoOptimize")) {
                    settings->fOptimize = false;
                    settings->fInlineThreshold = 0;
                }
                if (consume_suffix(&settingsText, " NoRTFlip")) {
                    settings->fForceNoRTFlip = true;
                }
                if (consume_suffix(&settingsText, " InlineThresholdMax")) {
                    settings->fInlineThreshold = INT_MAX;
                }
                if (consume_suffix(&settingsText, " Sharpen")) {
                    settings->fSharpenTextures = true;
                }
                if (consume_suffix(&settingsText, " DebugTrace")) {
                    settings->fOptimize = false;
                    *debugTrace = std::make_unique<SkSL::DebugTracePriv>();
                }

                if (settingsText.empty()) {
                    break;
                }
                if (settingsText.length() == startingLength) {
                    printf("Unrecognized #pragma settings: %s\n", settingsText.c_str());
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * Displays a usage banner; used when the command line arguments don't make sense.
 */
static void show_usage() {
    printf("usage: skslc <input> <output> <flags>\n"
           "       skslc <worklist>\n"
           "\n"
           "Allowed flags:\n"
           "--settings:   honor embedded /*#pragma settings*/ comments.\n"
           "--nosettings: ignore /*#pragma settings*/ comments\n");
}

static bool set_flag(std::optional<bool>* flag, const char* name, bool value) {
    if (flag->has_value()) {
        printf("%s flag was specified multiple times\n", name);
        return false;
    }
    *flag = value;
    return true;
}

/**
 * Handle a single input.
 */
static ResultCode process_command(SkSpan<std::string> args) {
    std::optional<bool> honorSettings;
    std::vector<std::string> paths;
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--settings") {
            if (!set_flag(&honorSettings, "settings", true)) {
                return ResultCode::kInputError;
            }
        } else if (arg == "--nosettings") {
            if (!set_flag(&honorSettings, "settings", false)) {
                return ResultCode::kInputError;
            }
        } else if (!skstd::starts_with(arg, "--")) {
            paths.push_back(arg);
        } else {
            show_usage();
            return ResultCode::kInputError;
        }
    }
    if (paths.size() != 2) {
        show_usage();
        return ResultCode::kInputError;
    }

    if (!honorSettings.has_value()) {
        honorSettings = true;
    }

    const std::string& inputPath = paths[0];
    const std::string& outputPath = paths[1];
    SkSL::ProgramKind kind;
    if (skstd::ends_with(inputPath, ".vert")) {
        kind = SkSL::ProgramKind::kVertex;
    } else if (skstd::ends_with(inputPath, ".frag") || skstd::ends_with(inputPath, ".sksl")) {
        kind = SkSL::ProgramKind::kFragment;
    } else if (skstd::ends_with(inputPath, ".compute")) {
        kind = SkSL::ProgramKind::kCompute;
    } else if (skstd::ends_with(inputPath, ".rtb")) {
        kind = SkSL::ProgramKind::kRuntimeBlender;
    } else if (skstd::ends_with(inputPath, ".rtcf")) {
        kind = SkSL::ProgramKind::kRuntimeColorFilter;
    } else if (skstd::ends_with(inputPath, ".rts")) {
        kind = SkSL::ProgramKind::kRuntimeShader;
    } else {
        printf("input filename must end in '.vert', '.frag', '.compute', '.rtb', '.rtcf', "
               "'.rts' or '.sksl'\n");
        return ResultCode::kInputError;
    }

    std::ifstream in(inputPath);
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (in.rdstate()) {
        printf("error reading '%s'\n", inputPath.c_str());
        return ResultCode::kInputError;
    }

    SkSL::ProgramSettings settings;
    const SkSL::ShaderCaps* caps = SkSL::ShaderCapsFactory::Standalone();
    std::unique_ptr<SkSL::DebugTracePriv> debugTrace;
    if (*honorSettings) {
        if (!detect_shader_settings(text, &settings, &caps, &debugTrace)) {
            return ResultCode::kInputError;
        }
    }

    // This tells the compiler where the rt-flip uniform will live should it be required. For
    // testing purposes we don't care where that is, but the compiler will report an error if we
    // leave them at their default invalid values, or if the offset overlaps another uniform.
    settings.fRTFlipOffset  = 16384;
    settings.fRTFlipSet     = 0;
    settings.fRTFlipBinding = 0;

    auto emitCompileError = [&](const char* errorText) {
        // Overwrite the compiler output, if any, with an error message.
        SkSL::FileOutputStream errorStream(outputPath.c_str());
        errorStream.writeText("### Compilation failed:\n\n");
        errorStream.writeText(errorText);
        errorStream.close();
        // Also emit the error directly to stdout.
        puts(errorText);
    };

    auto compileProgram = [&](const auto& writeFn) -> ResultCode {
        SkSL::FileOutputStream out(outputPath.c_str());
        SkSL::Compiler compiler(caps);
        if (!out.isValid()) {
            printf("error writing '%s'\n", outputPath.c_str());
            return ResultCode::kOutputError;
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !writeFn(compiler, *program, out)) {
            out.close();
            emitCompileError(compiler.errorText().c_str());
            return ResultCode::kCompileError;
        }
        if (!out.close()) {
            printf("error writing '%s'\n", outputPath.c_str());
            return ResultCode::kOutputError;
        }
        return ResultCode::kSuccess;
    };

    auto compileProgramAsRuntimeShader = [&](const auto& writeFn) -> ResultCode {
        if (kind == SkSL::ProgramKind::kVertex) {
            emitCompileError("Runtime shaders do not support vertex programs\n");
            return ResultCode::kCompileError;
        }
        if (kind == SkSL::ProgramKind::kFragment) {
            // Handle .sksl and .frag programs as runtime shaders.
            kind = SkSL::ProgramKind::kPrivateRuntimeShader;
        }
        return compileProgram(writeFn);
    };

    if (skstd::ends_with(outputPath, ".spirv")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    return compiler.toSPIRV(program, out);
                });
    } else if (skstd::ends_with(outputPath, ".asm.frag") ||
               skstd::ends_with(outputPath, ".asm.vert") ||
               skstd::ends_with(outputPath, ".asm.comp")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    // Compile program to SPIR-V assembly in a string-stream.
                    SkSL::StringStream assembly;
                    if (!compiler.toSPIRV(program, assembly)) {
                        return false;
                    }
                    // Convert the string-stream to a SPIR-V disassembly.
                    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
                    const std::string& spirv(assembly.str());
                    std::string disassembly;
                    uint32_t options = spvtools::SpirvTools::kDefaultDisassembleOption;
                    options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
                    if (!tools.Disassemble((const uint32_t*)spirv.data(),
                                           spirv.size() / 4,
                                           &disassembly,
                                           options)) {
                        return false;
                    }
                    // Finally, write the disassembly to our output stream.
                    out.write(disassembly.data(), disassembly.size());
                    return true;
                });
    } else if (skstd::ends_with(outputPath, ".glsl")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    return compiler.toGLSL(program, out);
                });
    } else if (skstd::ends_with(outputPath, ".metal")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    return compiler.toMetal(program, out);
                });
    } else if (skstd::ends_with(outputPath, ".hlsl")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    return compiler.toHLSL(program, out);
                });
    } else if (skstd::ends_with(outputPath, ".wgsl")) {
        return compileProgram(
                [](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    return compiler.toWGSL(program, out);
                });
    } else if (skstd::ends_with(outputPath, ".skrp")) {
        settings.fMaxVersionAllowed = SkSL::Version::k300;
        return compileProgramAsRuntimeShader(
                [&](SkSL::Compiler& compiler, SkSL::Program& program, SkSL::OutputStream& out) {
                    SkSL::DebugTracePriv skrpDebugTrace;
                    const SkSL::FunctionDeclaration* main = program.getFunction("main");
                    if (!main) {
                        compiler.errorReporter().error({}, "code has no entrypoint");
                        return false;
                    }
                    bool wantTraceOps = (debugTrace != nullptr);
                    std::unique_ptr<SkSL::RP::Program> rasterProg = SkSL::MakeRasterPipelineProgram(
                            program, *main->definition(), &skrpDebugTrace, wantTraceOps);
                    if (!rasterProg) {
                        compiler.errorReporter().error({}, "code is not supported");
                        return false;
                    }
                    rasterProg->dump(as_SkWStream(out).get(), /*writeInstructionCount=*/true);
                    return true;
                });
    } else if (skstd::ends_with(outputPath, ".stage")) {
        return compileProgram(
                [](SkSL::Compiler&, SkSL::Program& program, SkSL::OutputStream& out) {
                    class Callbacks : public SkSL::PipelineStage::Callbacks {
                    public:
                        std::string getMangledName(const char* name) override {
                            return std::string(name) + "_0";
                        }

                        std::string declareUniform(const SkSL::VarDeclaration* decl) override {
                            fOutput += decl->description();
                            return std::string(decl->var()->name());
                        }

                        void defineFunction(const char* decl,
                                            const char* body,
                                            bool /*isMain*/) override {
                            fOutput += std::string(decl) + '{' + body + '}';
                        }

                        void declareFunction(const char* decl) override {
                            fOutput += decl;
                        }

                        void defineStruct(const char* definition) override {
                            fOutput += definition;
                        }

                        void declareGlobal(const char* declaration) override {
                            fOutput += declaration;
                        }

                        std::string sampleShader(int index, std::string coords) override {
                            return "child_" + std::to_string(index) + ".eval(" + coords + ')';
                        }

                        std::string sampleColorFilter(int index, std::string color) override {
                            return "child_" + std::to_string(index) + ".eval(" + color + ')';
                        }

                        std::string sampleBlender(int index,
                                                  std::string src,
                                                  std::string dst) override {
                            return "child_" + std::to_string(index) +
                                   ".eval(" + src + ", " + dst + ')';
                        }

                        std::string toLinearSrgb(std::string color) override {
                            return "toLinearSrgb(" + color + ')';
                        }
                        std::string fromLinearSrgb(std::string color) override {
                            return "fromLinearSrgb(" + color + ')';
                        }

                        std::string fOutput;
                    };
                    // The .stage output looks almost like valid SkSL, but not quite.
                    // The PipelineStageGenerator bridges the gap between the SkSL in `program`,
                    // and the C++ FP builder API (see GrSkSLFP). In that API, children don't need
                    // to be declared (so they don't emit declarations here). Children are sampled
                    // by index, not name - so all children here are just "child_N".
                    // The input color and coords have names in the original SkSL (as parameters to
                    // main), but those are ignored here. References to those variables become
                    // "_coords" and "_inColor". At runtime, those variable names are irrelevant
                    // when the new SkSL is emitted inside the FP - references to those variables
                    // are replaced with strings from EmitArgs, and might be varyings or differently
                    // named parameters.
                    Callbacks callbacks;
                    SkSL::PipelineStage::ConvertProgram(program, "_coords", "_inColor",
                                                        "_canvasColor", &callbacks);
                    out.writeString(SkShaderUtils::PrettyPrint(callbacks.fOutput));
                    return true;
                });
    } else {
        printf("expected output path to end with one of: .glsl, .html, .metal, .hlsl, .wgsl, "
               ".spirv, .asm.vert, .asm.frag, .asm.comp, .skrp, .stage (got '%s')\n",
               outputPath.c_str());
        return ResultCode::kConfigurationError;
    }
    return ResultCode::kSuccess;
}

int main(int argc, const char** argv) {
    if (argc == 2) {
        // Worklists are the only two-argument case for skslc, and we don't intend to support
        // nested worklists, so we can process them here.
        return (int)ProcessWorklist(argv[1], process_command);
    } else {
        // Process non-worklist inputs.
        std::vector<std::string> args;
        for (int index=0; index<argc; ++index) {
            args.push_back(argv[index]);
        }

        return (int)process_command(args);
    }
}
