/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTPARAMETER
#define SKSL_ASTPARAMETER

#include "SkSLASTPositionNode.h"
#include "SkSLASTType.h"
#include "../ir/SkSLModifiers.h"

namespace SkSL {

/**
 * A declaration of a parameter, as part of a function declaration.
 */
struct ASTParameter : public ASTPositionNode {
    // 'sizes' is a list of the array sizes appearing on a parameter, in source order.
    // e.g. int x[3][1] would have sizes [3, 1].
    ASTParameter(Position position, Modifiers modifiers, std::unique_ptr<ASTType> type,
                 String name, std::vector<int> sizes)
    : INHERITED(position)
    , fModifiers(modifiers)
    , fType(std::move(type))
    , fName(std::move(name))
    , fSizes(std::move(sizes)) {}

    String description() const override {
        String result = fModifiers.description() + fType->description() + " " + fName;
        for (int size : fSizes) {
            result += "[" + to_string(size) + "]";
        }
        return result;
    }

    const Modifiers fModifiers;
    const std::unique_ptr<ASTType> fType;
    const String fName;
    const std::vector<int> fSizes;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
