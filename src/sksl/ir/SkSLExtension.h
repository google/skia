/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTENSION
#define SKSL_EXTENSION

#include "include/private/SkSLProgramElement.h"

namespace SkSL {

/**
 * An extension declaration.
 */
class Extension final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kExtension;

    Extension(int offset, skstd::string_view name)
        : INHERITED(offset, kProgramElementKind)
        , fName(name) {}

    skstd::string_view name() const {
        return fName;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Extension(fOffset, this->name()));
    }

    String description() const override {
        return "#extension " + this->name() + " : enable";
    }

private:
    skstd::string_view fName;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
