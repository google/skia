/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "stdio.h"
#include <fstream>
#include "SkSLCompiler.h"
#include "GrContextOptions.h"

bool endsWith(const std::string& s, const std::string& ending) {
    if (s.length() >= ending.length()) {
        return (0 == s.compare(s.length() - ending.length(), ending.length(), ending));
    }
    return false;
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
    size_t len = strlen(argv[1]);
    if (len > 5 && !strcmp(argv[1] + strlen(argv[1]) - 5, ".vert")) {
        kind = SkSL::Program::kVertex_Kind;
    } else if (len > 5 && !strcmp(argv[1] + strlen(argv[1]) - 5, ".frag")) {
        kind = SkSL::Program::kFragment_Kind;
    } else {
        printf("input filename must end in '.vert' or '.frag'\n");
        exit(1);
    }

    std::ifstream in(argv[1]);
    std::string text((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
    if (in.rdstate()) {
        printf("error reading '%s'\n", argv[1]);
        exit(2);
    }
    std::string name(argv[2]);
    if (endsWith(name, ".spirv")) {
        std::ofstream out(argv[2], std::ofstream::binary);
        SkSL::Compiler compiler;
        if (!compiler.toSPIRV(kind, text, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (out.rdstate()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (endsWith(name, ".glsl")) {
        std::ofstream out(argv[2], std::ofstream::binary);
        SkSL::Compiler compiler;
        if (!compiler.toGLSL(kind, text, *SkSL::GLSLCapsFactory::Default(), out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (out.rdstate()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else {
        printf("expected output filename to end with '.spirv' or '.glsl'");
    }
}
