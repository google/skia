/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOL
#define SKSL_SYMBOL

#include "SkSLIRNode.h"

namespace SkSL {

/**
 * Represents a symboltable entry.
 */
struct Symbol : public IRNode {
    enum Kind {
        kFunctionDeclaration_Kind,
        kUnresolvedFunction_Kind,
        kType_Kind,
        kVariable_Kind,
        kField_Kind
    };

    Symbol(Position position, Kind kind, String name)
    : INHERITED(position)
    , fKind(kind)
    , fName(std::move(name)) {}

    const Kind fKind;
    const String fName;

    typedef IRNode INHERITED;
};

} // namespace

#endif
