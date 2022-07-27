/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SK_OPTS_NS sksl_precompile_standalone
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/private/SkStringView.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/opts/SkChecksum_opts.h"
#include "src/opts/SkVM_opts.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDehydrator.h"
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkShaderUtils.h"

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
    decltype(hash_fn) hash_fn = sksl_precompile_standalone::hash_fn;
    decltype(interpret_skvm) interpret_skvm;
}

enum class ResultCode {
    kSuccess = 0,
    kCompileError = 1,
    kInputError = 2,
    kOutputError = 3,
};

/**
 * Displays a usage banner; used when the command line arguments don't make sense.
 */
static void show_usage() {
    printf("usage: sksl-precompile <output> <input>\n");
}

/**
 * Handle a single input.
 */
ResultCode processCommand(const std::vector<std::string>& paths) {
    if (paths.size() < 2) {
        show_usage();
        return ResultCode::kInputError;
    }

    SkSL::ProgramSettings settings;
    auto standaloneCaps = SkSL::ShaderCapsFactory::Standalone();
    const SkSL::ShaderCaps* caps = standaloneCaps.get();

    // This tells the compiler where the rt-flip uniform will live should it be required. For
    // testing purposes we don't care where that is, but the compiler will report an error if we
    // leave them at their default invalid values, or if the offset overlaps another uniform.
    settings.fRTFlipOffset  = 16384;
    settings.fRTFlipSet     = 0;
    settings.fRTFlipBinding = 0;

    // Load in each input as a module, from right to left.
    // Each module inherits the symbols from its parent module.
    SkSL::Compiler compiler(caps);
    std::list<SkSL::LoadedModule> modules;
    std::shared_ptr<SkSL::SymbolTable> inheritedSymbols = nullptr;
    for (int inputIdx = paths.size() - 1; inputIdx >= 1; --inputIdx) {
        const std::string& modulePath = paths[inputIdx];
        std::ifstream in(modulePath);
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (in.rdstate()) {
            printf("error reading '%s'\n", modulePath.c_str());
            return ResultCode::kInputError;
        }

        modules.push_front(compiler.loadModule(SkSL::ProgramKind::kFragment,
                                               SkSL::Compiler::MakeModulePath(modulePath.c_str()),
                                               /*base=*/inheritedSymbols,
                                               /*dehydrate=*/inheritedSymbols == nullptr));
        inheritedSymbols = modules.front().fSymbols;
    }

    // Dehydrate the leftmost input file into a buffer.
    const std::string& inputPath = paths[1];
    SkSL::LoadedModule& module = modules.front();
    SkSL::Dehydrator dehydrator;
    dehydrator.write(*module.fSymbols);
    dehydrator.write(module.fElements);
    SkString baseName = SkOSPath::Basename(inputPath.c_str());
    if (int extension = baseName.findLastOf('.'); extension > 0) {
        baseName.resize(extension);
    }

    SkSL::StringStream buffer;
    dehydrator.finish(buffer);
    const std::string& data = buffer.str();

    // Emit the dehydrated data into our output file.
    const std::string& outputPath = paths[0];
    SkSL::FileOutputStream out(outputPath.c_str());
    if (!out.isValid()) {
        printf("error writing '%s'\n", outputPath.c_str());
        return ResultCode::kOutputError;
    }
    out.printf("static constexpr uint8_t SKSL_INCLUDE_%s[] = {", baseName.c_str());
    for (size_t i = 0; i < data.length(); ++i) {
        out.printf("%s%d,", dehydrator.prefixAtOffset(i), uint8_t(data[i]));
    }
    out.printf("};\n");
    out.printf("static constexpr size_t SKSL_INCLUDE_%s_LENGTH = sizeof(SKSL_INCLUDE_%s);\n",
               baseName.c_str(), baseName.c_str());
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
