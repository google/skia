/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SECTION
#define SKSL_SECTION

#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * A section declaration (e.g. @body { body code here })..
 */
struct Section : public ProgramElement {
    Section(int offset, String name, String arg, String text)
    : INHERITED(offset, kSection_Kind)
    , fName(std::move(name))
    , fArgument(std::move(arg))
    , fText(std::move(text)) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Section(fOffset, fName, fArgument, fText));
    }

#ifdef SK_DEBUG
    String description() const override {
        String result = "@" + fName;
        if (fArgument.size()) {
            result += "(" + fArgument + ")";
        }
        result += " { " + fText + " }";
        return result;
    }
#endif

    const String fName;
    const String fArgument;
    const String fText;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
