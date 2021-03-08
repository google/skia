/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMELEMENT
#define SKSL_PROGRAMELEMENT

#include "include/private/SkSLIRNode.h"

#include <memory>

namespace SkSL {

/**
 * Represents a top-level element (e.g. function or global variable) in a program.
 */
class ProgramElement : public IRNode {
public:
    enum class Kind {
        kEnum = 0,
        kExtension,
        kFunction,
        kFunctionPrototype,
        kGlobalVar,
        kInterfaceBlock,
        kModifiers,
        kSection,
        kStructDefinition,

        kFirst = kEnum,
        kLast = kStructDefinition
    };

    ProgramElement(int offset, Kind kind)
        : INHERITED(offset, (int) kind) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind) fKind;
    }

    /**
     *  Use is<T> to check the type of a program element.
     *  e.g. replace `el.kind() == ProgramElement::Kind::kEnum` with `el.is<Enum>()`.
     */
    template <typename T>
    bool is() const {
        return this->kind() == T::kProgramElementKind;
    }

    /**
     *  Use as<T> to downcast program elements. e.g. replace `(Enum&) el` with `el.as<Enum>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
    }

    virtual std::unique_ptr<ProgramElement> clone() const = 0;

private:
    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
