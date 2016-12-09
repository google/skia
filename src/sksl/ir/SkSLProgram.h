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

#include "SkSLContext.h"
#include "SkSLModifiers.h"
#include "SkSLProgramElement.h"
#include "SkSLSymbolTable.h"

// name of the render target height uniform
#define SKSL_RTHEIGHT_NAME "u_skRTHeight"

namespace SkSL {

/**
 * Represents a fully-digested program, ready for code generation.
 */
struct Program {
    struct Settings {
        const GrShaderCaps* fCaps = nullptr;
        bool fFlipY = false;
    };

    struct Inputs {
        // if true, this program requires the render target height uniform to be defined
        bool fRTHeight;

        void reset() {
            fRTHeight = false;
        }

        bool isEmpty() {
            return !fRTHeight;
        }
    };

    enum Kind {
        kFragment_Kind,
        kVertex_Kind
    };

    Program(Kind kind,
            Settings settings,
            Modifiers::Flag defaultPrecision,
            Context* context,
            std::vector<std::unique_ptr<ProgramElement>> elements,
            std::shared_ptr<SymbolTable> symbols,
            Inputs inputs)
    : fKind(kind)
    , fSettings(settings)
    , fDefaultPrecision(defaultPrecision)
    , fContext(context)
    , fElements(std::move(elements))
    , fSymbols(symbols)
    , fInputs(inputs) {}

    Kind fKind;
    Settings fSettings;
    // FIXME handle different types; currently it assumes this is for floats
    Modifiers::Flag fDefaultPrecision;
    Context* fContext;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    std::shared_ptr<SymbolTable> fSymbols;
    Inputs fInputs;
};

} // namespace

#endif
