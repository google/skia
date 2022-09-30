/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SK_OPTS_NS sksl_minify_standalone
#include "include/core/SkStream.h"
#include "include/private/SkStringView.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/opts/SkChecksum_opts.h"
#include "src/opts/SkVM_opts.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <cctype>
#include <fstream>
#include <limits.h>
#include <list>
#include <optional>
#include <stdarg.h>
#include <stdio.h>

static bool gUnoptimized = false;

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

namespace SkOpts {
    decltype(hash_fn) hash_fn = sksl_minify_standalone::hash_fn;
    decltype(interpret_skvm) interpret_skvm;
}

enum class ResultCode {
    kSuccess = 0,
    kCompileError = 1,
    kInputError = 2,
    kOutputError = 3,
};

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
    printf("usage: sksl-minify <output> <input> [dependencies...]\n");
}

static std::string_view stringize(const SkSL::Token& token, std::string_view text) {
    return text.substr(token.fOffset, token.fLength);
}

static bool maybe_identifier(char c) {
    return std::isalnum(c) || c == '$' || c == '_';
}

static std::optional<SkSL::LoadedModule> compile_module_list(SkSpan<const std::string> paths) {
    // Load in each input as a module, from right to left.
    // Each module inherits the symbols from its parent module.
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    SkSL::LoadedModule loadedModule;
    std::list<SkSL::ParsedModule> modules = {{SkSL::ModuleLoader::Get().rootModule().fSymbols,
                                              /*fElements=*/nullptr}};
    for (auto modulePath = paths.rbegin(); modulePath != paths.rend(); ++modulePath) {
        std::ifstream in(*modulePath);
        std::string moduleSource{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};
        if (in.rdstate()) {
            printf("error reading '%s'\n", modulePath->c_str());
            return std::nullopt;
        }

        // If we have a loaded module, parse it and put it on the list.
        if (loadedModule.fSymbols) {
            modules.push_front(loadedModule.parse(modules.front()));
        }

        // TODO(skia:13778): We don't know the module's ProgramKind here, so we always pass
        // kFragment. For minification purposes, the ProgramKind doesn't really make a difference
        // as long as it doesn't limit what we can do.
        loadedModule = compiler.compileModule(SkSL::ProgramKind::kFragment,
                                              modulePath->c_str(),
                                              std::move(moduleSource),
                                              modules.front(),
                                              SkSL::ModuleLoader::Get().coreModifiers(),
                                              /*shouldInline=*/false);
    }
    if (!gUnoptimized) {
        // Run an optimization pass on the target module before returning it.
        compiler.optimizeModuleBeforeMinifying(SkSL::ProgramKind::kFragment,
                                               loadedModule,
                                               modules.front());
    }
    return std::move(loadedModule);
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
        if (lineWidth > 75) {
            // We're getting full-ish; wrap to a new line
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

ResultCode processCommand(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        show_usage();
        return ResultCode::kInputError;
    }

    // Compile the original SkSL from the input path.
    SkSpan inputPaths(args);
    inputPaths = inputPaths.subspan(1);
    std::optional<SkSL::LoadedModule> module = compile_module_list(inputPaths);
    if (!module.has_value()) {
        return ResultCode::kInputError;
    }

    // Emit the minified SkSL into our output path.
    const std::string& outputPath = args[0];
    SkSL::FileOutputStream out(outputPath.c_str());
    if (!out.isValid()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    std::string baseName = remove_extension(base_name(inputPaths.front()));
    out.printf("static constexpr char SKSL_MINIFIED_%s[] =\n\"", baseName.c_str());

    // Generate the program text by getting the program's description.
    std::string text;
    for (const std::unique_ptr<SkSL::ProgramElement>& element : module->fElements) {
        text += element->description();
    }

    // Eliminate whitespace and perform other basic simplifications via a lexer pass.
    if (!generate_minified_text(inputPaths.front(), text, out)) {
        return ResultCode::kInputError;
    }

    out.writeText("\";\n");

    if (!out.close()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    return ResultCode::kSuccess;
}

bool find_boolean_flag(std::vector<std::string>& args, std::string_view flagName) {
    size_t startingCount = args.size();
    args.erase(std::remove_if(args.begin(), args.end(),
                              [&](const std::string& a) { return a == flagName; }),
               args.end());
    return args.size() < startingCount;
}

int main(int argc, const char** argv) {
    std::vector<std::string> args;
    for (int index=1; index<argc; ++index) {
        args.push_back(argv[index]);
    }

    gUnoptimized = find_boolean_flag(args, "--unoptimized");

    return (int)processCommand(args);
}
