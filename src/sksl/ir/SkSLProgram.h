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

#include "include/private/SkSLDefines.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

// name of the render target width uniform
#define SKSL_RTWIDTH_NAME "u_skRTWidth"

// name of the render target height uniform
#define SKSL_RTHEIGHT_NAME "u_skRTHeight"

namespace SkSL {

class Context;
class Pool;

/**
 * Side-car class holding mutable information about a Program's IR
 */
class ProgramUsage {
public:
    struct VariableCounts {
        int fDeclared = 0;
        int fRead = 0;
        int fWrite = 0;
    };
    VariableCounts get(const Variable&) const;
    bool isDead(const Variable&) const;

    int get(const FunctionDeclaration&) const;

    void replace(const Expression* oldExpr, const Expression* newExpr);
    void add(const Statement* stmt);
    void remove(const Expression* expr);
    void remove(const Statement* stmt);
    void remove(const ProgramElement& element);

    SkTHashMap<const Variable*, VariableCounts> fVariableCounts;
    SkTHashMap<const FunctionDeclaration*, int> fCallCounts;
};

/**
 * Represents a fully-digested program, ready for code generation.
 */
struct Program {
    using Settings = ProgramSettings;

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

    Program(std::unique_ptr<String> source,
            std::unique_ptr<ProgramConfig> config,
            std::shared_ptr<Context> context,
            std::vector<std::unique_ptr<ProgramElement>> elements,
            std::vector<const ProgramElement*> sharedElements,
            std::unique_ptr<ModifiersPool> modifiers,
            std::shared_ptr<SymbolTable> symbols,
            std::unique_ptr<Pool> pool,
            Inputs inputs)
    : fSource(std::move(source))
    , fConfig(std::move(config))
    , fContext(context)
    , fSymbols(symbols)
    , fPool(std::move(pool))
    , fInputs(inputs)
    , fElements(std::move(elements))
    , fSharedElements(std::move(sharedElements))
    , fModifiers(std::move(modifiers)) {
        fUsage = Analysis::GetUsage(*this);
    }

    ~Program() {
        // Some or all of the program elements are in the pool. To free them safely, we must attach
        // the pool before destroying any program elements. (Otherwise, we may accidentally call
        // delete on a pooled node.)
        if (fPool) {
            fPool->attachToThread();
        }
        fElements.clear();
        fContext.reset();
        fSymbols.reset();
        fModifiers.reset();
        if (fPool) {
            fPool->detachFromThread();
        }
    }

    class ElementsCollection {
    public:
        class iterator {
        public:
            const ProgramElement* operator*() {
                if (fShared != fSharedEnd) {
                    return *fShared;
                } else {
                    return fOwned->get();
                }
            }

            iterator& operator++() {
                if (fShared != fSharedEnd) {
                    ++fShared;
                } else {
                    ++fOwned;
                }
                return *this;
            }

            bool operator==(const iterator& other) const {
                return fOwned == other.fOwned && fShared == other.fShared;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }

        private:
            using Owned  = std::vector<std::unique_ptr<ProgramElement>>::const_iterator;
            using Shared = std::vector<const ProgramElement*>::const_iterator;
            friend class ElementsCollection;

            iterator(Owned owned, Owned ownedEnd, Shared shared, Shared sharedEnd)
                    : fOwned(owned), fOwnedEnd(ownedEnd), fShared(shared), fSharedEnd(sharedEnd) {}

            Owned  fOwned;
            Owned  fOwnedEnd;
            Shared fShared;
            Shared fSharedEnd;
        };

        iterator begin() const {
            return iterator(fProgram.fElements.begin(), fProgram.fElements.end(),
                            fProgram.fSharedElements.begin(), fProgram.fSharedElements.end());
        }

        iterator end() const {
            return iterator(fProgram.fElements.end(), fProgram.fElements.end(),
                            fProgram.fSharedElements.end(), fProgram.fSharedElements.end());
        }

    private:
        friend struct Program;

        ElementsCollection(const Program& program) : fProgram(program) {}
        const Program& fProgram;
    };

    // Can be used to iterate over *all* elements in this Program, both owned and shared (builtin).
    // The iterator's value type is 'const ProgramElement*', so it's clear that you *must not*
    // modify anything (as you might be mutating shared data).
    ElementsCollection elements() const { return ElementsCollection(*this); }

    // Can be used to iterate over *just* the elements owned by the Program, not shared builtins.
    // The iterator's value type is 'std::unique_ptr<ProgramElement>', and mutation is allowed.
    std::vector<std::unique_ptr<ProgramElement>>& ownedElements() { return fElements; }
    const std::vector<std::unique_ptr<ProgramElement>>& ownedElements() const { return fElements; }

    std::unique_ptr<String> fSource;
    std::unique_ptr<ProgramConfig> fConfig;
    std::shared_ptr<Context> fContext;
    // it's important to keep fElements defined after (and thus destroyed before) fSymbols,
    // because destroying elements can modify reference counts in symbols
    std::shared_ptr<SymbolTable> fSymbols;
    std::unique_ptr<Pool> fPool;
    Inputs fInputs;

private:
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    std::vector<const ProgramElement*>           fSharedElements;
    std::unique_ptr<ModifiersPool> fModifiers;
    std::unique_ptr<ProgramUsage> fUsage;

    friend class Compiler;
    friend class Inliner;             // fUsage
    friend class SPIRVCodeGenerator;  // fModifiers
};

}  // namespace SkSL

#endif
