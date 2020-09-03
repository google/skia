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
    enum Kind {
        kEnum_Kind,
        kExtension_Kind,
        kFunction_Kind,
        kInterfaceBlock_Kind,
        kModifiers_Kind,
        kSection_Kind,
        kVar_Kind
    };

    ProgramElement(int offset, Kind kind)
    : INHERITED(offset)
    , fKind(kind) {}

    /**
     *  Use is<T> to check the type of a program element.
     *  e.g. replace `el.fKind == ProgramElement::kEnum_Kind` with `el.is<Enum>()`.
     */
    template <typename T>
    bool is() const {
        return this->fKind == T::kProgramElementKind;
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

    Kind fKind;

    virtual std::unique_ptr<ProgramElement> clone() const = 0;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
