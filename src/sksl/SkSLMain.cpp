/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fstream>
#include "SkSLCompiler.h"
#include "SkSLFileOutputStream.h"

// Given the path to a file (e.g. src/gpu/effects/GrFooFragmentProcessor.fp) and the expected
// filename prefix and suffix (e.g. "Gr" and ".fp"), returns the "base name" of the
// file (in this case, 'FooFragmentProcessor'). If no match, returns the empty string.
static SkSL::String base_name(const char* fpPath, const char* prefix, const char* suffix) {
    SkSL::String result;
    const char* end = fpPath + strlen(fpPath);
    const char* fileName = end;
    // back up until we find a slash
    while (fileName != fpPath && '/' != *(fileName - 1) && '\\' != *(fileName - 1)) {
        --fileName;
    }
    if (!strncmp(fileName, prefix, strlen(prefix)) &&
        !strncmp(end - strlen(suffix), suffix, strlen(suffix))) {
        result.append(fileName + strlen(prefix), end - fileName - strlen(prefix) - strlen(suffix));
    }
    return result;
}

/**
 * Very simple standalone executable to facilitate testing.
 */
int main(int argc, const char** argv) {
    if (argc != 3) {
        printf("usage: skslc <input> <output>\n");
        exit(1);
    }
    SkSL::Program::Kind kind;
    SkSL::String input(argv[1]);
    if (input.endsWith(".vert")) {
        kind = SkSL::Program::kVertex_Kind;
    } else if (input.endsWith(".frag")) {
        kind = SkSL::Program::kFragment_Kind;
    } else if (input.endsWith(".geom")) {
        kind = SkSL::Program::kGeometry_Kind;
    } else if (input.endsWith(".fp")) {
        kind = SkSL::Program::kFragmentProcessor_Kind;
    } else {
        printf("input filename must end in '.vert', '.frag', '.geom', or '.fp'\n");
        exit(1);
    }

    std::ifstream in(argv[1]);
    std::string stdText((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    SkSL::String text(stdText.c_str());
    if (in.rdstate()) {
        printf("error reading '%s'\n", argv[1]);
        exit(2);
    }
    SkSL::Program::Settings settings;
    settings.fArgs.insert(std::make_pair("gpImplementsDistanceVector", 1));
    SkSL::String name(argv[2]);
    if (name.endsWith(".spirv")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toSPIRV(*program, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".glsl")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toGLSL(*program, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".h")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler(SkSL::Compiler::kPermitInvalidStaticTests_Flag);
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        settings.fReplaceSettings = false;
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toH(*program, base_name(argv[1], "Gr", ".fp"), out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".cpp")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler(SkSL::Compiler::kPermitInvalidStaticTests_Flag);
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        settings.fReplaceSettings = false;
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toCPP(*program, base_name(argv[1], "Gr", ".fp"), out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else {
        printf("expected output filename to end with '.spirv', '.glsl', '.cpp', or '.h'");
    }
}
