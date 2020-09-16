/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLString.h"

namespace SkSL {

class Type;

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
class IRNode {
public:
    IRNode(int offset, int kind, const Type* type = nullptr)
    : fOffset(offset)
    , fKind(kind)
    , fType(type) {}

    virtual ~IRNode() {}

    virtual String description() const = 0;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

    const Type& type() const {
        SkASSERT(fType);
        return *fType;
    }

protected:
    int fKind;

private:
    const Type* fType;
};

}  // namespace SkSL

#endif
