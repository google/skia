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

    ASTType(int offset, StringFragment name, Kind kind, std::vector<int> sizes, bool nullable)
    : INHERITED(offset)
    , fName(name)
    , fKind(kind)
    , fSizes(std::move(sizes))
    , fNullable(nullable) {}

    String description() const override {
        return fName;
    }

    const StringFragment fName;

    const Kind fKind;

    // array sizes, -1 meaning unspecified
    const std::vector<int> fSizes;

    bool fNullable;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
