/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMELEMENT
#define SKSL_PROGRAMELEMENT

#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>

namespace SkSL {

/**
 * Represents a top-level element (e.g. function or global variable) in a program.
 */
struct ProgramElement : public IRNode {
    enum class Kind {
        kEnum = 0,
        kExtension,
        kFunction,
        kInterfaceBlock,
        kModifiers,
        kSection,
        kVar,

        kFirstProgramElement = kEnum,
        kLastProgramElement = kVar
    };

    ProgramElement(int offset, Kind kind)
    : INHERITED(offset, (int) kind) {}

    Kind kind() const {
        return (Kind) fKind;
    }

    /**
     *  Use as<T> to downcast program elements. e.g. replace `(Enum&) el` with `el.as<Enum>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->kind() == T::kProgramElementKind);
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->kind() == T::kProgramElementKind);
        return static_cast<T&>(*this);
    }

    virtual std::unique_ptr<ProgramElement> clone() const = 0;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
