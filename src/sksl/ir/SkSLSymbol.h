/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOL
#define SKSL_SYMBOL

#include "src/sksl/ir/SkSLIRNode.h"

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
        kField_Kind,
        kExternal_Kind
    };

    Symbol(int offset, Kind kind, StringFragment name)
    : INHERITED(offset)
    , fKind(kind)
    , fName(name) {}

    virtual ~Symbol() {}

    Kind fKind;
    StringFragment fName;

    typedef IRNode INHERITED;
};

} // namespace

#endif
