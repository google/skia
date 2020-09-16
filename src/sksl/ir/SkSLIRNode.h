/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <vector>

namespace SkSL {

struct Expression;
class Type;

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
class IRNode {
public:
    virtual ~IRNode();

    IRNode& operator=(const IRNode& other) {
        // Need to have a copy assignment operator because Type requires it, but can't use the
        // default version until we finish migrating away from std::unique_ptr children. For now,
        // just assert that there are no children (we could theoretically clone them, but we never
        // actually copy nodes containing children).
        SkASSERT(other.fExpressionChildren.empty());
        fKind = other.fKind;
        fOffset = other.fOffset;
        fData = other.fData;
        return *this;
    }

    virtual String description() const = 0;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

    const Type& type() const {
        switch (fData.fKind) {
            case NodeData::Kind::kType:
                return *this->typeData();
            case NodeData::Kind::kTypeToken:
                return *this->typeTokenData().fType;
            default:
                SkUNREACHABLE;
        }
    }

protected:
    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct NodeData {
        char fBytes[std::max(sizeof(Type*),
                             sizeof(TypeTokenData))];

        enum class Kind {
            kType,
            kTypeToken,
        } fKind;

        NodeData() = default;

        NodeData(const Type* data)
            : fKind(Kind::kType) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(TypeTokenData data)
            : fKind(Kind::kTypeToken) {
            memcpy(fBytes, &data, sizeof(data));
        }
    };

    IRNode(int offset, int kind, const Type* data = nullptr);

    IRNode(int offset, int kind, TypeTokenData data);

    IRNode(const IRNode& other);

    Expression& expressionChild(int index) const {
        return *fExpressionChildren[index];
    }

    std::unique_ptr<Expression>& expressionPointer(int index) {
        return fExpressionChildren[index];
    }

    const std::unique_ptr<Expression>& expressionPointer(int index) const {
        return fExpressionChildren[index];
    }

    Type* typeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kType);
        Type* result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    TypeTokenData typeTokenData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
        TypeTokenData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    int fKind;
    std::vector<std::unique_ptr<Expression>> fExpressionChildren;

private:
    NodeData fData;
};

}  // namespace SkSL

#endif
