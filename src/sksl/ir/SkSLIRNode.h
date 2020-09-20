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
    virtual ~IRNode();

    virtual String description() const = 0;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

    const Type& type() const {
        return *this->typeData();
    }

protected:
    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct NodeData {
        TypeTokenData fData;

        enum class Kind {
            kType,
            kTypeToken,
        } fKind;

        NodeData(const Type* data)
        : fKind(Kind::kType) {
            fData.fType = data;
        }
    };

    IRNode(int offset, int kind, const Type* data = nullptr);

    const Type* typeData() const {
        return fData.fData.fType;
    }

    int fKind;

private:
    NodeData fData;
};

}  // namespace SkSL

#endif
