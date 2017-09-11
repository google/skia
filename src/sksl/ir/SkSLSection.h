/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SECTION
#define SKSL_SECTION

#include "SkSLProgramElement.h"

namespace SkSL {

/**
 * A section declaration (e.g. @body { body code here })..
 */
struct Section : public ProgramElement {
    Section(Position position, String name, String arg, String text)
    : INHERITED(position, kSection_Kind)
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

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
