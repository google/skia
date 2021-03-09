/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SECTION
#define SKSL_SECTION

#include "include/private/SkSLProgramElement.h"

namespace SkSL {

/**
 * A section declaration (e.g. @body { body code here })..
 */
class Section final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kSection;

    Section(int offset, String name, String arg, String text)
    : INHERITED(offset, kProgramElementKind)
    , fName(std::move(name))
    , fArgument(std::move(arg))
    , fText(std::move(text)) {}

    const String& name() const {
        return fName;
    }

    const String& argument() const {
        return fArgument;
    }

    const String& text() const {
        return fText;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Section(fOffset, this->name(), this->argument(),
                                                           this->text()));
    }

    String description() const override {
        String result = "@" + this->name();
        if (this->argument().size()) {
            result += "(" + this->argument() + ")";
        }
        result += " { " + this->text() + " }";
        return result;
    }

private:
    String fName;
    String fArgument;
    String fText;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
