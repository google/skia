/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAM
#define SKSL_PROGRAM

#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// name of the uniform used to handle features that are sensitive to whether Y is flipped.
#define SKSL_RTFLIP_NAME "u_skRTFlip"

namespace SkSL {

class Context;
class FunctionDeclaration;
class Pool;
class ProgramElement;
class ProgramUsage;
class SymbolTable;
struct ProgramConfig;

/** Represents a list the Uniforms contained within a Program. */
struct UniformInfo {
    struct Uniform {
        std::string fName;
        SkSL::Type::NumberKind fKind;
        int fColumns;
        int fRows;
        int fSlot;
    };
    std::vector<Uniform> fUniforms;
    int fUniformSlotCount = 0;
};

/** A program's inputs and outputs. */
struct ProgramInterface {
    enum RTFlip : uint8_t {
        kRTFlip_None       = 0b0000'0000,
        kRTFlip_FragCoord  = 0b0000'0001,
        kRTFlip_Clockwise  = 0b0000'0010,
        kRTFlip_Derivative = 0b0000'0100,
    };
    uint8_t fRTFlipUniform = kRTFlip_None;
    bool fUseLastFragColor = false;
    bool fOutputSecondaryColor = false;

    bool operator==(const ProgramInterface& that) const {
        return fRTFlipUniform == that.fRTFlipUniform &&
               fUseLastFragColor == that.fUseLastFragColor &&
               fOutputSecondaryColor == that.fOutputSecondaryColor;
    }
    bool operator!=(const ProgramInterface& that) const { return !(*this == that); }
};

/** Represents a fully-digested program, ready for code generation. */
struct Program {
    Program(std::unique_ptr<std::string> source,
            std::unique_ptr<ProgramConfig> config,
            std::shared_ptr<Context> context,
            std::vector<std::unique_ptr<ProgramElement>> elements,
            std::unique_ptr<SymbolTable> symbols,
            std::unique_ptr<Pool> pool);

    ~Program();

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
            return iterator(fProgram.fOwnedElements.begin(), fProgram.fOwnedElements.end(),
                            fProgram.fSharedElements.begin(), fProgram.fSharedElements.end());
        }

        iterator end() const {
            return iterator(fProgram.fOwnedElements.end(), fProgram.fOwnedElements.end(),
                            fProgram.fSharedElements.end(), fProgram.fSharedElements.end());
        }

    private:
        friend struct Program;

        ElementsCollection(const Program& program) : fProgram(program) {}
        const Program& fProgram;
    };

    /**
     * Iterates over *all* elements in this Program, both owned and shared (builtin). The iterator's
     * value type is `const ProgramElement*`, so it's clear that you *must not* modify anything (as
     * you might be mutating shared data).
     */
    ElementsCollection elements() const { return ElementsCollection(*this); }

    /**
     * Returns a function declaration with the given name; null is returned if the function doesn't
     * exist or has no definition. If the function might have overloads, you can use nextOverload()
     * to search for the function with the expected parameter list.
     */
    const FunctionDeclaration* getFunction(const char* functionName) const;

    std::string description() const;
    const ProgramUsage* usage() const { return fUsage.get(); }

    std::unique_ptr<std::string> fSource;
    std::unique_ptr<ProgramConfig> fConfig;
    std::shared_ptr<Context> fContext;
    std::unique_ptr<ProgramUsage> fUsage;
    // it's important to keep fOwnedElements defined after (and thus destroyed before) fSymbols,
    // because an IR element might access the symbol table during its destruction
    std::unique_ptr<SymbolTable> fSymbols;
    std::unique_ptr<Pool> fPool;
    // Contains *only* elements owned exclusively by this program.
    std::vector<std::unique_ptr<ProgramElement>> fOwnedElements;
    // Contains *only* elements owned by a built-in module that are included in this program.
    // Use elements() to iterate over the combined set of owned + shared elements.
    std::vector<const ProgramElement*> fSharedElements;
    ProgramInterface fInterface;

    using Interface = ProgramInterface;
};

}  // namespace SkSL

#endif
