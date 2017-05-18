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
#ifdef SKSL_STANDALONE
        const StandaloneShaderCaps* fCaps = &standaloneCaps;
#else
        const GrShaderCaps* fCaps = nullptr;
#endif
        // if false, sk_FragCoord is exactly the same as gl_FragCoord. If true, the y coordinate
        // must be flipped.
        bool fFlipY = false;
    };

    struct Inputs {
        // if true, this program requires the render target height uniform to be defined
        bool fRTHeight;

        // if true, this program must be recompiled if the flipY setting changes. If false, the
        // program will compile to the same code regardless of the flipY setting.
        bool fFlipY;

        void reset() {
            fRTHeight = false;
            fFlipY = false;
        }

        bool isEmpty() {
            return !fRTHeight && !fFlipY;
        }
    };

    enum Kind {
        kFragment_Kind,
        kVertex_Kind,
        kGeometry_Kind
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
    , fSymbols(symbols)
    , fElements(std::move(elements))
    , fInputs(inputs) {}

    Kind fKind;
    Settings fSettings;
    // FIXME handle different types; currently it assumes this is for floats
    Modifiers::Flag fDefaultPrecision;
    Context* fContext;
    // it's important to keep fElements defined after (and thus destroyed before) fSymbols,
    // because destroying elements can modify reference counts in symbols
    std::shared_ptr<SymbolTable> fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    Inputs fInputs;
};

} // namespace

#endif
