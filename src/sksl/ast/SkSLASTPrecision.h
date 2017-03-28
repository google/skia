/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTPRECISION
#define SKSL_ASTPRECISION

#include "SkSLASTDeclaration.h"
#include "../ir/SkSLModifiers.h"

namespace SkSL {

/**
 * Represents a precision declaration (e.g. 'precision mediump float;').
 */
struct ASTPrecision : public ASTDeclaration {
    // FIXME handle the type
    ASTPrecision(Position position, Modifiers::Flag precision)
    : INHERITED(position, kPrecision_Kind)
    , fPrecision(precision) {}

    String description() const {
        switch (fPrecision) {
            case Modifiers::kLowp_Flag: return String("precision lowp float;");
            case Modifiers::kMediump_Flag: return String("precision mediump float;");
            case Modifiers::kHighp_Flag: return String("precision highp float;");
            default:
                ASSERT(false);
                return String("<error>");
        }
        ASSERT(false);
        return String("<error>");
    }

    const Modifiers::Flag fPrecision;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
