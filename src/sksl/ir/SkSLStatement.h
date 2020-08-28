/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STATEMENT
#define SKSL_STATEMENT

#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Abstract supertype of all statements.
 */
struct Statement : public IRNode {
    Statement(int offset, Kind kind)
    : INHERITED(offset, kind) {}

    virtual bool isEmpty() const {
        return false;
    }

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
