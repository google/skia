/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSECTION
#define SKSL_ASTSECTION

#include "src/sksl/ast/SkSLASTDeclaration.h"

namespace SkSL {

/**
 * A section declaration (e.g. @body { body code here })..
 */
struct ASTSection : public ASTDeclaration {
    ASTSection(int offset, String name, String arg, String text)
    : INHERITED(offset, kSection_Kind)
    , fName(std::move(name))
    , fArgument(std::move(arg))
    , fText(std::move(text)) {}

    String description() const override {
        String result = "@" + fName;
        if (fArgument.size()) {
            result += "(" + fArgument + ")";
        }
        result += " { " + fText + " }";
        return result;
    }

    const String fName;
    const String fArgument;
    const String fText;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
