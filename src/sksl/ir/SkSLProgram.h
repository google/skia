/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_PROGRAM
#define SKSL_PROGRAM

#include <vector>
#include <memory>

#include "SkSLProgramElement.h"

namespace SkSL {

/**
 * Represents a fully-digested program, ready for code generation.
 */
struct Program {
    enum Kind {
        kFragment_Kind,
        kVertex_Kind
    };

    Program(Kind kind, std::vector<std::unique_ptr<ProgramElement>> elements)
    : fKind(kind) 
    , fElements(std::move(elements)) {}

    Kind fKind;

    std::vector<std::unique_ptr<ProgramElement>> fElements;
};

} // namespace

#endif
