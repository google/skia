/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTENSION
#define SKSL_EXTENSION

#include "SkSLProgramElement.h"

namespace SkSL {

/**
 * An extension declaration.
 */
struct Extension : public ProgramElement {
    Extension(Position position, String name)
    : INHERITED(position, kExtension_Kind)
    , fName(std::move(name)) {}

    String description() const override {
        return "#extension " + fName + " : enable";
    }

    const String fName;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
