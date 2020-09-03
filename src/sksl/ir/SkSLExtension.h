/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTENSION
#define SKSL_EXTENSION

#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * An extension declaration.
 */
struct Extension : public ProgramElement {
    static constexpr Kind kProgramElementKind = kExtension_Kind;

    Extension(int offset, String name)
    : INHERITED(offset, kProgramElementKind)
    , fName(std::move(name)) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new Extension(fOffset, fName));
    }

    String description() const override {
        return "#extension " + fName + " : enable";
    }

    const String fName;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
