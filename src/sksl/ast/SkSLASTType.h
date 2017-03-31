/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTTYPE
#define SKSL_ASTTYPE

#include "SkSLASTPositionNode.h"

namespace SkSL {

/**
 * A type, such as 'int' or 'struct foo'.
 */
struct ASTType : public ASTPositionNode {
    enum Kind {
        kIdentifier_Kind,
        kStruct_Kind
    };

    ASTType(Position position, String name, Kind kind, std::vector<int> sizes)
    : INHERITED(position)
    , fName(std::move(name))
    , fKind(kind)
    , fSizes(std::move(sizes)) {}

    String description() const override {
        return fName;
    }

    const String fName;

    const Kind fKind;

    // array sizes, -1 meaning unspecified
    const std::vector<int> fSizes;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
