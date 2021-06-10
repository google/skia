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

    Section(int offset, skstd::string_view name, skstd::string_view arg, skstd::string_view text)
    : INHERITED(offset, kProgramElementKind)
    , fName(name)
    , fArgument(arg)
    , fText(text) {}

    skstd::string_view name() const {
        return fName;
    }

    skstd::string_view argument() const {
        return fArgument;
    }

    skstd::string_view text() const {
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
    skstd::string_view fName;
    skstd::string_view fArgument;
    skstd::string_view fText;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
