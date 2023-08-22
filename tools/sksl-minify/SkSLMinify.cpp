/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SK_OPTS_NS sksl_minify_standalone
#include "include/core/SkStream.h"
#include "src/base/SkStringView.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/transform/SkSLTransform.h"
#include "src/utils/SkOSPath.h"
#include "tools/SkGetExecutablePath.h"
#include "tools/skslc/ProcessWorklist.h"

#include <cctype>
#include <forward_list>
#include <fstream>
#include <limits.h>
#include <optional>
#include <stdarg.h>
#include <stdio.h>

static bool gUnoptimized = false;
static bool gStringify = false;
static SkSL::ProgramKind gProgramKind = SkSL::ProgramKind::kFragment;

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

namespace SkOpts {
    size_t raster_pipeline_highp_stride = 1;
}

static std::string base_name(const std::string& path) {
    size_t slashPos = path.find_last_of("/\\");
    return path.substr(slashPos == std::string::npos ? 0 : slashPos + 1);
}

static std::string remove_extension(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    return path.substr(0, dotPos);
}

/**
 * Displays a usage banner; used when the command line arguments don't make sense.
 */
static void show_usage() {
    printf("usage: sksl-minify <output> <input> [--frag|--vert|--compute|--shader|"
           "--colorfilter|--blender] [dependencies...]\n");
}

static std::string_view stringize(const SkSL::Token& token, std::string_view text) {
    return text.substr(token.fOffset, token.fLength);
}

static bool maybe_identifier(char c) {
    return std::isalnum(c) || c == '$' || c == '_';
}

static std::forward_list<std::unique_ptr<const SkSL::Module>> compile_module_list(
        SkSpan<const std::string> paths, SkSL::ProgramKind kind) {
    std::forward_list<std::unique_ptr<const SkSL::Module>> modules;

    // If we are compiling a Runtime Effect...
    if (SkSL::ProgramConfig::IsRuntimeEffect(kind)) {
        // ... the parent modules still need to be compiled as Fragment programs.
        // If no modules are explicitly specified, we automatically include the built-in modules for
        // runtime effects (sksl_shared, sksl_public) so that casual users don't need to always
        // remember to specify these modules.
        if (paths.size() == 1) {
            const std::string minifyDir = SkOSPath::Dirname(SkGetExecutablePath().c_str()).c_str();
            std::string defaultRuntimeShaderPaths[] = {
                    minifyDir + SkOSPath::SEPARATOR + "sksl_public.sksl",
                    minifyDir + SkOSPath::SEPARATOR + "sksl_shared.sksl",
            };
            modules = compile_module_list(defaultRuntimeShaderPaths, SkSL::ProgramKind::kFragment);
        } else {
            // The parent modules were listed on the command line; we need to compile them as
            // fragment programs. The final module keeps the Runtime Shader program-kind.
            modules = compile_module_list(paths.subspan(1), SkSL::ProgramKind::kFragment);
            paths = paths.first(1);
        }
        // Set up the public type aliases so that Runtime Shader code with GLSL types works as-is.
        SkSL::ModuleLoader::Get().addPublicTypeAliases(modules.front().get());
    }

    // Load in each input as a module, from right to left.
    // Each module inherits the symbols from its parent module.
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    for (auto modulePath = paths.rbegin(); modulePath != paths.rend(); ++modulePath) {
        std::ifstream in(*modulePath);
        std::string moduleSource{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};
        if (in.rdstate()) {
            printf("error reading '%s'\n", modulePath->c_str());
            return {};
        }

        const SkSL::Module* parent = modules.empty() ? SkSL::ModuleLoader::Get().rootModule()
                                                     : modules.front().get();
        std::unique_ptr<SkSL::Module> m = compiler.compileModule(kind,
                                                                 modulePath->c_str(),
                                                                 std::move(moduleSource),
                                                                 parent,
                                                                 /*shouldInline=*/false);
        if (!m) {
            return {};
        }
        // We need to optimize every module in the chain. We rename private functions at global
        // scope, and we need to make sure there are no name collisions between nested modules.
        // (i.e., if module A claims names `$a` and `$b` at global scope, module B will need to
        // start at `$c`. The most straightforward way to handle this is to actually perform the
        // renames.)
        compiler.optimizeModuleBeforeMinifying(kind, *m, /*shrinkSymbols=*/!gUnoptimized);
        modules.push_front(std::move(m));
    }
    // Return all of the modules to transfer their ownership to the caller.
    return modules;
}

static bool generate_minified_text(std::string_view inputPath,
                                   std::string_view text,
                                   SkSL::FileOutputStream& out) {
    using TokenKind = SkSL::Token::Kind;

    SkSL::Lexer lexer;
    lexer.start(text);

    SkSL::Token token;
    std::string_view lastTokenText = " ";
    int lineWidth = 1;
    for (;;) {
        token = lexer.next();
        if (token.fKind == TokenKind::TK_END_OF_FILE) {
            break;
        }
        if (token.fKind == TokenKind::TK_LINE_COMMENT ||
            token.fKind == TokenKind::TK_BLOCK_COMMENT ||
            token.fKind == TokenKind::TK_WHITESPACE) {
            continue;
        }
        std::string_view thisTokenText = stringize(token, text);
        if (token.fKind == TokenKind::TK_INVALID) {
            printf("%.*s: unable to parse '%.*s' at offset %d\n",
                   (int)inputPath.size(), inputPath.data(),
                   (int)thisTokenText.size(), thisTokenText.data(),
                   token.fOffset);
            return false;
        }
        if (thisTokenText.empty()) {
            continue;
        }
        if (token.fKind == TokenKind::TK_FLOAT_LITERAL) {
            // We can reduce `3.0` to `3.` safely.
            if (skstd::contains(thisTokenText, '.')) {
                while (thisTokenText.back() == '0' && thisTokenText.size() >= 3) {
                    thisTokenText.remove_suffix(1);
                }
            }
            // We can reduce `0.5` to `.5` safely.
            if (skstd::starts_with(thisTokenText, "0.") && thisTokenText.size() >= 3) {
                thisTokenText.remove_prefix(1);
            }
        }
        SkASSERT(!lastTokenText.empty());
        if (gStringify && lineWidth > 75) {
            // We're getting full-ish; wrap to a new line.
            out.writeText("\"\n\"");
            lineWidth = 1;
        }
        if (maybe_identifier(lastTokenText.back()) && maybe_identifier(thisTokenText.front())) {
            // We are about to put two alphanumeric characters side-by-side; add whitespace between
            // the tokens.
            out.writeText(" ");
            lineWidth++;
        }
        out.write(thisTokenText.data(), thisTokenText.size());
        lineWidth += thisTokenText.size();
        lastTokenText = thisTokenText;
    }

    return true;
}

static bool find_boolean_flag(SkSpan<std::string>* args, std::string_view flagName) {
    size_t startingCount = args->size();
    auto iter = std::remove_if(args->begin(), args->end(),
                               [&](const std::string& a) { return a == flagName; });
    *args = args->subspan(0, std::distance(args->begin(), iter));
    return args->size() < startingCount;
}

static bool has_overlapping_flags(SkSpan<const bool> flags) {
    // Returns true if more than one boolean is set.
    return std::count(flags.begin(), flags.end(), true) > 1;
}

static ResultCode process_command(SkSpan<std::string> args) {
    // Ignore the process name.
    SkASSERT(!args.empty());
    args = args.subspan(1);

    // Process command line flags.
    gUnoptimized = find_boolean_flag(&args, "--unoptimized");
    gStringify = find_boolean_flag(&args, "--stringify");
    bool isFrag = find_boolean_flag(&args, "--frag");
    bool isVert = find_boolean_flag(&args, "--vert");
    bool isCompute = find_boolean_flag(&args, "--compute");
    bool isShader = find_boolean_flag(&args, "--shader");
    bool isColorFilter = find_boolean_flag(&args, "--colorfilter");
    bool isBlender = find_boolean_flag(&args, "--blender");
    if (has_overlapping_flags({isFrag, isVert, isCompute, isShader, isColorFilter, isBlender})) {
        show_usage();
        return ResultCode::kInputError;
    }
    if (isFrag) {
        gProgramKind = SkSL::ProgramKind::kFragment;
    } else if (isVert) {
        gProgramKind = SkSL::ProgramKind::kVertex;
    } else if (isCompute) {
        gProgramKind = SkSL::ProgramKind::kCompute;
    } else if (isColorFilter) {
        gProgramKind = SkSL::ProgramKind::kRuntimeColorFilter;
    } else if (isBlender) {
        gProgramKind = SkSL::ProgramKind::kRuntimeBlender;
    } else {
        // Default case, if no option is specified.
        gProgramKind = SkSL::ProgramKind::kRuntimeShader;
    }

    // We expect, at a minimum, an output path and one or more input paths.
    if (args.size() < 2) {
        show_usage();
        return ResultCode::kInputError;
    }
    const std::string& outputPath = args[0];
    SkSpan inputPaths = args.subspan(1);

    // Compile the original SkSL from the input path.
    std::forward_list<std::unique_ptr<const SkSL::Module>> modules =
            compile_module_list(inputPaths, gProgramKind);
    if (modules.empty()) {
        return ResultCode::kInputError;
    }
    const SkSL::Module* module = modules.front().get();

    // Emit the minified SkSL into our output path.
    SkSL::FileOutputStream out(outputPath.c_str());
    if (!out.isValid()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    std::string baseName = remove_extension(base_name(inputPaths.front()));
    if (gStringify) {
        out.printf("static constexpr char SKSL_MINIFIED_%s[] =\n\"", baseName.c_str());
    }

    // Generate the program text by getting the program's description.
    std::string text;
    for (const std::unique_ptr<SkSL::ProgramElement>& element : module->fElements) {
        text += element->description();
    }

    // Eliminate whitespace and perform other basic simplifications via a lexer pass.
    if (!generate_minified_text(inputPaths.front(), text, out)) {
        return ResultCode::kInputError;
    }

    if (gStringify) {
        out.writeText("\";");
    }
    out.writeText("\n");

    if (!out.close()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    return ResultCode::kSuccess;
}

int main(int argc, const char** argv) {
    if (argc == 2) {
        // Worklists are the only two-argument case for sksl-minify, and we don't intend to support
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
