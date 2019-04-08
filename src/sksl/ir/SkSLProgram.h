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
#include "SkSLFloatLiteral.h"
#include "SkSLIntLiteral.h"
#include "SkSLModifiers.h"
#include "SkSLProgramElement.h"
#include "SkSLSymbolTable.h"

// name of the render target width uniform
#define SKSL_RTWIDTH_NAME "u_skRTWidth"

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

            Value(unsigned int i)
            : fKind(kInt_Kind)
            , fValue(i) {}

            Value(float f)
            : fKind(kFloat_Kind)
            , fValue(f) {}

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
                    case Program::Settings::Value::kFloat_Kind:
                        return std::unique_ptr<Expression>(new FloatLiteral(context,
                                                                          offset,
                                                                          fValue));
                    default:
                        SkASSERT(false);
                        return nullptr;
                }
            }

            enum {
                kBool_Kind,
                kInt_Kind,
                kFloat_Kind,
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
        // if true, add -0.5 bias to LOD of all texture lookups
        bool fSharpenTextures = false;
        std::unordered_map<String, Value> fArgs;
    };

    struct Inputs {
        // if true, this program requires the render target width uniform to be defined
        bool fRTWidth;

        // if true, this program requires the render target height uniform to be defined
        bool fRTHeight;

        // if true, this program must be recompiled if the flipY setting changes. If false, the
        // program will compile to the same code regardless of the flipY setting.
        bool fFlipY;

        void reset() {
            fRTWidth = false;
            fRTHeight = false;
            fFlipY = false;
        }

        bool isEmpty() {
            return !fRTWidth && !fRTHeight && !fFlipY;
        }
    };

    class iterator {
    public:
        ProgramElement& operator*() {
            if (fIter1 != fEnd1) {
                return **fIter1;
            }
            return **fIter2;
        }

        iterator& operator++() {
            if (fIter1 != fEnd1) {
                ++fIter1;
                return *this;
            }
            ++fIter2;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return fIter1 == other.fIter1 && fIter2 == other.fIter2;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        using inner = std::vector<std::unique_ptr<ProgramElement>>::iterator;

        iterator(inner begin1, inner end1, inner begin2, inner end2)
        : fIter1(begin1)
        , fEnd1(end1)
        , fIter2(begin2)
        , fEnd2(end2) {}

        inner fIter1;
        inner fEnd1;
        inner fIter2;
        inner fEnd2;

        friend struct Program;
    };

    class const_iterator {
    public:
        const ProgramElement& operator*() {
            if (fIter1 != fEnd1) {
                return **fIter1;
            }
            return **fIter2;
        }

        const_iterator& operator++() {
            if (fIter1 != fEnd1) {
                ++fIter1;
                return *this;
            }
            ++fIter2;
            return *this;
        }

        bool operator==(const const_iterator& other) const {
            return fIter1 == other.fIter1 && fIter2 == other.fIter2;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        using inner = std::vector<std::unique_ptr<ProgramElement>>::const_iterator;

        const_iterator(inner begin1, inner end1, inner begin2, inner end2)
        : fIter1(begin1)
        , fEnd1(end1)
        , fIter2(begin2)
        , fEnd2(end2) {}

        inner fIter1;
        inner fEnd1;
        inner fIter2;
        inner fEnd2;

        friend struct Program;
    };

    enum Kind {
        kFragment_Kind,
        kVertex_Kind,
        kGeometry_Kind,
        kFragmentProcessor_Kind,
        kPipelineStage_Kind,
        kMixer_Kind
    };

    Program(Kind kind,
            std::unique_ptr<String> source,
            Settings settings,
            std::shared_ptr<Context> context,
            std::vector<std::unique_ptr<ProgramElement>>* inheritedElements,
            std::vector<std::unique_ptr<ProgramElement>> elements,
            std::shared_ptr<SymbolTable> symbols,
            Inputs inputs)
    : fKind(kind)
    , fSource(std::move(source))
    , fSettings(settings)
    , fContext(context)
    , fSymbols(symbols)
    , fInputs(inputs)
    , fInheritedElements(inheritedElements)
    , fElements(std::move(elements)) {}

    iterator begin() {
        if (fInheritedElements) {
            return iterator(fInheritedElements->begin(), fInheritedElements->end(),
                            fElements.begin(), fElements.end());
        }
        return iterator(fElements.begin(), fElements.end(), fElements.end(), fElements.end());
    }

    iterator end() {
        if (fInheritedElements) {
            return iterator(fInheritedElements->end(), fInheritedElements->end(),
                            fElements.end(), fElements.end());
        }
        return iterator(fElements.end(), fElements.end(), fElements.end(), fElements.end());
    }

    const_iterator begin() const {
        if (fInheritedElements) {
            return const_iterator(fInheritedElements->begin(), fInheritedElements->end(),
                                  fElements.begin(), fElements.end());
        }
        return const_iterator(fElements.begin(), fElements.end(), fElements.end(), fElements.end());
    }

    const_iterator end() const {
        if (fInheritedElements) {
            return const_iterator(fInheritedElements->end(), fInheritedElements->end(),
                                  fElements.end(), fElements.end());
        }
        return const_iterator(fElements.end(), fElements.end(), fElements.end(), fElements.end());
    }

    Kind fKind;
    std::unique_ptr<String> fSource;
    Settings fSettings;
    std::shared_ptr<Context> fContext;
    // it's important to keep fElements defined after (and thus destroyed before) fSymbols,
    // because destroying elements can modify reference counts in symbols
    std::shared_ptr<SymbolTable> fSymbols;
    Inputs fInputs;
    bool fIsOptimized = false;

private:
    std::vector<std::unique_ptr<ProgramElement>>* fInheritedElements;
    std::vector<std::unique_ptr<ProgramElement>> fElements;

    friend class Compiler;
};

} // namespace

#endif
