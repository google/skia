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
    using Kind = ProgramElementKind;

    ProgramElement(Position pos, Kind kind)
        : INHERITED(pos, (int) kind) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind) fKind;
    }

    virtual std::unique_ptr<ProgramElement> clone() const = 0;

private:
    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
