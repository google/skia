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
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"

#include <cctype>
#include <fstream>
#include <limits.h>
#include <list>
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
    printf("usage: sksl-minify <output> <input>\n");
}

static std::string_view stringize(const SkSL::Token& token, std::string_view text) {
    return text.substr(token.fOffset, token.fLength);
}

static bool maybe_identifier(char c) {
    return std::isalnum(c) || c == '$' || c == '_';
}

/**
 * Handle a single input.
 */
ResultCode processCommand(const std::vector<std::string>& paths) {
    using TokenKind = SkSL::Token::Kind;

    if (paths.size() != 2) {
        show_usage();
        return ResultCode::kInputError;
    }

    // Read the original SkSL from the input path.
    const std::string& inputPath = paths[1];
    std::ifstream in(inputPath);
    std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading '%s'\n", inputPath.c_str());
        return ResultCode::kInputError;
    }

    // Emit the minified SkSL into our output path.
    const std::string& outputPath = paths[0];
    SkSL::FileOutputStream out(outputPath.c_str());
    if (!out.isValid()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    std::string baseName = remove_extension(base_name(inputPath));
    out.printf("static constexpr char SKSL_MINIFIED_%s[] =\n\"", baseName.c_str());

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
            printf("%s: unable to parse '%.*s' at offset %d\n",
                   inputPath.c_str(),
                   (int)thisTokenText.size(), thisTokenText.data(),
                   token.fOffset);
            return ResultCode::kInputError;
        }
        if (thisTokenText.empty()) {
            continue;
        }
        if (token.fKind == TokenKind::TK_FLOAT_LITERAL) {
            // If we have a floating point number that doesn't use exponential notation...
            if (!skstd::contains(thisTokenText, "e") && !skstd::contains(thisTokenText, "E")) {
                // ... and has a decimal point...
                if (skstd::contains(thisTokenText, ".")) {
                    // ... strip any trailing zeros to save space.
                    while (thisTokenText.back() == '0' && thisTokenText.size() > 2) {
                        thisTokenText.remove_suffix(1);
                    }
                }
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

    out.writeText("\";\n");

    if (!out.close()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }

    return ResultCode::kSuccess;
}

int main(int argc, const char** argv) {
    std::vector<std::string> args;
    for (int index=1; index<argc; ++index) {
        args.push_back(argv[index]);
    }

    return (int)processCommand(args);
}
