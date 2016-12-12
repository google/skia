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

#include "SkSLModifiers.h"
#include "SkSLProgramElement.h"
#include "SkSLSymbolTable.h"

namespace SkSL {

/**
 * Represents a fully-digested program, ready for code generation.
 */
struct Program {
    enum Kind {
        kFragment_Kind,
        kVertex_Kind
    };

    Program(Kind kind, 
            Modifiers::Flag defaultPrecision,
            std::vector<std::unique_ptr<ProgramElement>> elements, 
            std::shared_ptr<SymbolTable> symbols)
    : fKind(kind) 
    , fDefaultPrecision(defaultPrecision)
    , fElements(std::move(elements))
    , fSymbols(symbols) {}

    Kind fKind;
    // FIXME handle different types; currently it assumes this is for floats
    Modifiers::Flag fDefaultPrecision;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    std::shared_ptr<SymbolTable> fSymbols;
};

} // namespace

#endif
