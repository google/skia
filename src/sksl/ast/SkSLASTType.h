/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTTYPE
#define SKSL_ASTTYPE

namespace SkSL {

/**
 * A type, such as 'int' or 'struct foo'.
 */
struct ASTType : public ASTPositionNode {
    enum Kind {
        kIdentifier_Kind,
        kStruct_Kind
    };

    ASTType(Position position, std::string name, Kind kind)
    : INHERITED(position)
    , fName(std::move(name))
    , fKind(kind) {}

    std::string description() const override {
        return fName;
    }

    const std::string fName;

    const Kind fKind;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
