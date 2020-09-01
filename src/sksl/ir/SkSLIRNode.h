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

    enum class Index {
        kBinaryLeft = 0,
        kBinaryRight = 1,
    };

    struct BinaryData {
        const Type* fType;
        Token::Kind fOperator;
    };

    struct NodeData {
        char fBytes[sizeof(BinaryData)];

#ifdef SK_DEBUG
        enum class Kind {
            kToken,
        } fKind;
#endif
        NodeData() = default;

        NodeData(BinaryData data)
#ifdef SK_DEBUG
            : fKind(Kind::kToken)
#endif
        {
            memcpy(fBytes, &data, sizeof(data));
        }
    };

    IRNode& operator=(const IRNode& other) {
        fOffset = other.fOffset;
        SkASSERT(other.fChildren.empty());
        return *this;
    }

    virtual ~IRNode() {}

    IRNode& child(Index index) const {
        return *fChildren[(int) index];
    }

    Expression& expressionChild(Index index) const {
        return reinterpret_cast<Expression&>(*fChildren[(int) index]);
    }

    std::unique_ptr<Expression>& expressionPointer(Index index) {
        return *reinterpret_cast<std::unique_ptr<Expression>*>(&fChildren[(int) index]);
    }

    const std::unique_ptr<Expression>& expressionPointer(Index index) const {
        return *reinterpret_cast<const std::unique_ptr<Expression>*>(&fChildren[(int) index]);
    }

    BinaryData getBinaryData() const {
#ifdef SK_DEBUG
        SkASSERT(fData.fKind == NodeData::Kind::kToken);
#endif
        BinaryData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    virtual String description() const = 0;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

protected:
    IRNode(int offset, int kind)
    : fOffset(offset)
    , fKind(kind) {}

    IRNode(int offset, int kind, BinaryData data, std::vector<std::unique_ptr<IRNode>> children)
    : fOffset(offset)
    , fKind(kind)
    , fChildren(std::move(children))
    , fData(data) {}

    IRNode(const IRNode& other)
        : fOffset(other.fOffset)
        , fData(other.fData) {
        SkASSERT(other.fChildren.empty());
    }

    int fKind;

    std::vector<std::unique_ptr<IRNode>> fChildren;

    NodeData fData;
};

}  // namespace SkSL

#endif
