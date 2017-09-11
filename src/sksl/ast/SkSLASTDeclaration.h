/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTDECLARATION
#define SKSL_ASTDECLARATION

#include "SkSLASTPositionNode.h"

namespace SkSL {

/**
 * Abstract supertype of declarations such as variables and functions.
 */
struct ASTDeclaration : public ASTPositionNode {
    enum Kind {
        kVar_Kind,
        kFunction_Kind,
        kInterfaceBlock_Kind,
        kExtension_Kind,
        kPrecision_Kind,
        kModifiers_Kind,
        kSection_Kind
    };

    ASTDeclaration(Position position, Kind kind)
    : INHERITED(position)
    , fKind(kind) {}

    Kind fKind;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
