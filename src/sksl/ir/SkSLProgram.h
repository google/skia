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

#include "SkSLBoolLiteral.h"
#include "SkSLExpression.h"
#include "SkSLIntLiteral.h"
#include "SkSLModifiers.h"
#include "SkSLProgramElement.h"
#include "SkSLSymbolTable.h"

// name of the render target height uniform
#define SKSL_RTHEIGHT_NAME "u_skRTHeight"

namespace SkSL {

class Context;

/**
 * Represents a fully-digested program, ready for code generation.
 */
struct Program {
    struct Settings {
        struct Value {
            Value(bool b)
            : fKind(kBool_Kind)
            , fValue(b) {}

            Value(int i)
            : fKind(kInt_Kind)
            , fValue(i) {}

            std::unique_ptr<Expression> literal(const Context& context, int offset) const {
                switch (fKind) {
                    case Program::Settings::Value::kBool_Kind:
                        return std::unique_ptr<Expression>(new BoolLiteral(context,
                                                                           offset,
                                                                           fValue));
                    case Program::Settings::Value::kInt_Kind:
                        return std::unique_ptr<Expression>(new IntLiteral(context,
                                                                          offset,
                                                                          fValue));
                    default:
                        ASSERT(false);
                        return nullptr;
                }
            }

            enum {
                kBool_Kind,
                kInt_Kind,
            } fKind;

            int fValue;
        };

#ifdef SKSL_STANDALONE
        const StandaloneShaderCaps* fCaps = &standaloneCaps;
#else
        const GrShaderCaps* fCaps = nullptr;
#endif
        // if false, sk_FragCoord is exactly the same as gl_FragCoord. If true, the y coordinate
        // must be flipped.
        bool fFlipY = false;
        // If true the destination fragment color is read sk_FragColor. It must be declared inout.
        bool fFragColorIsInOut = false;
        // if true, Setting objects (e.g. sk_Caps.fbFetchSupport) should be replaced with their
        // constant equivalents during compilation
        bool fReplaceSettings = true;
        // if true, all halfs are forced to be floats
        bool fForceHighPrecision = false;
        std::unordered_map<String, Value> fArgs;
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
        kGeometry_Kind,
        kFragmentProcessor_Kind
    };

    Program(Kind kind,
            std::unique_ptr<String> source,
            Settings settings,
            Context* context,
            std::vector<std::unique_ptr<ProgramElement>> elements,
            std::shared_ptr<SymbolTable> symbols,
            Inputs inputs)
    : fKind(kind)
    , fSource(std::move(source))
    , fSettings(settings)
    , fContext(context)
    , fSymbols(symbols)
    , fElements(std::move(elements))
    , fInputs(inputs) {}

    Kind fKind;
    std::unique_ptr<String> fSource;
    Settings fSettings;
    Context* fContext;
    // it's important to keep fElements defined after (and thus destroyed before) fSymbols,
    // because destroying elements can modify reference counts in symbols
    std::shared_ptr<SymbolTable> fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    Inputs fInputs;
};

} // namespace

#endif
