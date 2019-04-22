/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTFIELDSUFFIX
#define SKSL_ASTFIELDSUFFIX

#include "src/sksl/ast/SkSLASTSuffix.h"

namespace SkSL {

/**
 * A dotted identifier of the form ".foo". We refer to these as "fields" at parse time even if it is
 * actually vector swizzle (which looks the same to the parser).
 */
struct ASTFieldSuffix : public ASTSuffix {
    ASTFieldSuffix(int offset, StringFragment field)
    : INHERITED(offset, ASTSuffix::kField_Kind)
    , fField(field) {}

    String description() const override {
        return "." + fField;
    }

    StringFragment fField;

    typedef ASTSuffix INHERITED;
};

} // namespace

#endif
