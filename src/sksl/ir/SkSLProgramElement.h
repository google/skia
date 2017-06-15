/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMELEMENT
#define SKSL_PROGRAMELEMENT

#include "SkSLIRNode.h"

namespace SkSL {

/**
 * Represents a top-level element (e.g. function or global variable) in a program.
 */
struct ProgramElement : public IRNode {
    enum Kind {
        kVar_Kind,
        kFunction_Kind,
        kInterfaceBlock_Kind,
        kExtension_Kind,
        kModifiers_Kind,
        kSection_Kind
    };

    ProgramElement(Position position, Kind kind)
    : INHERITED(position)
    , fKind(kind) {}

    Kind fKind;

    typedef IRNode INHERITED;
};

} // namespace

#endif
