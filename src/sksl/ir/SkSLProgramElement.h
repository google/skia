/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMELEMENT
#define SKSL_PROGRAMELEMENT

#include "src/sksl/ir/SkSLIRNode.h"

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

    Kind fKind;

    virtual std::unique_ptr<ProgramElement> clone() const = 0;

    typedef IRNode INHERITED;
};

} // namespace

#endif
