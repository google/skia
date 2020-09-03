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

#include <memory>
#include <vector>
#include <unordered_map>

namespace SkSL {

struct Expression;
class IRGenerator;
class Type;
struct Variable;

using DefinitionMap = std::unordered_map<const Variable*, std::unique_ptr<Expression>*>;

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
struct IRNode {
    enum class Property {
        kSideEffects,
        kContainsRTAdjust
    };

    IRNode& operator=(const IRNode& other) {
        fOffset = other.fOffset;
        SkASSERT(other.fExpressionChildren.empty());
        return *this;
    }

    virtual ~IRNode();

    virtual String description() const = 0;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

protected:
    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct NodeData {
        char fBytes[sizeof(TypeTokenData)];

#ifdef SK_DEBUG
        enum class Kind {
            kType,
            kTypeToken,
        } fKind;
#endif
        NodeData() = default;

        NodeData(const Type* data)
#ifdef SK_DEBUG
            : fKind(Kind::kType)
#endif
        {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(TypeTokenData data)
#ifdef SK_DEBUG
            : fKind(Kind::kTypeToken)
#endif
        {
            memcpy(fBytes, &data, sizeof(data));
        }
    };

    IRNode(int offset, int kind);

    IRNode(int offset, int kind, const Type* data);

    IRNode(int offset, int kind, TypeTokenData data,
          std::vector<std::unique_ptr<Expression>> children);

    IRNode(const IRNode& other);

    Expression& expressionChild(int index) const {
        return *fExpressionChildren[(int) index];
    }

    std::unique_ptr<Expression>& expressionPointer(int index) {
        return fExpressionChildren[(int) index];
    }

    Type* typeData() const {
#ifdef SK_DEBUG
        SkASSERT(fData.fKind == NodeData::Kind::kType);
#endif
        Type* result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    TypeTokenData typeTokenData() const {
#ifdef SK_DEBUG
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
#endif
        TypeTokenData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    int fKind;

private:
    std::vector<std::unique_ptr<Expression>> fExpressionChildren;

    NodeData fData;
};

}  // namespace SkSL

#endif
