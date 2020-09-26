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
struct Statement;
class SymbolTable;
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
    struct BlockData {
        std::shared_ptr<SymbolTable> fSymbolTable;
        // if isScope is false, this is just a group of statements rather than an actual
        // language-level block. This allows us to pass around multiple statements as if they were a
        // single unit, with no semantic impact.
        bool fIsScope;
    };

    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct NodeData {
        char fBytes[std::max({sizeof(BlockData),
                              sizeof(Type*),
                              sizeof(TypeTokenData)})];

        enum class Kind {
            kBlock,
            kType,
            kTypeToken,
        } fKind;

        NodeData() = default;

        NodeData(BlockData data)
            : fKind(Kind::kBlock) {
            new(reinterpret_cast<BlockData*>(fBytes)) BlockData{std::move(data.fSymbolTable),
                                                                data.fIsScope};
        }

        NodeData(const Type* data)
            : fKind(Kind::kType) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(TypeTokenData data)
            : fKind(Kind::kTypeToken) {
            memcpy(fBytes, &data, sizeof(data));
        }

        ~NodeData() {
            if (fKind == Kind::kBlock) {
                reinterpret_cast<BlockData*>(fBytes)->~BlockData();
            }
        }
    };

    IRNode(int offset, int kind, BlockData data, std::vector<std::unique_ptr<Statement>> stmts);

    IRNode(int offset, int kind, const Type* data = nullptr);

    IRNode(int offset, int kind, TypeTokenData data);

    IRNode(const IRNode& other);

    Expression& expressionChild(int index) const {
        SkASSERT(index >= 0 && index < (int) fExpressionChildren.size());
        return *fExpressionChildren[index];
    }

    std::unique_ptr<Expression>& expressionPointer(int index) {
        SkASSERT(index >= 0 && index < (int) fExpressionChildren.size());
        return fExpressionChildren[index];
    }

    const std::unique_ptr<Expression>& expressionPointer(int index) const {
        SkASSERT(index >= 0 && index < (int) fExpressionChildren.size());
        return fExpressionChildren[index];
    }

    int expressionChildCount() const {
        return fExpressionChildren.size();
    }


    Statement& statementChild(int index) const {
        SkASSERT(index >= 0 && index < (int) fStatementChildren.size());
        return *fStatementChildren[index];
    }

    std::unique_ptr<Statement>& statementPointer(int index) {
        SkASSERT(index >= 0 && index < (int) fStatementChildren.size());
        return fStatementChildren[index];
    }

    const std::unique_ptr<Statement>& statementPointer(int index) const {
        SkASSERT(index >= 0 && index < (int) fStatementChildren.size());
        return fStatementChildren[index];
    }

    int statementChildCount() const {
        return fStatementChildren.size();
    }

    BlockData& blockData() {
        SkASSERT(fData.fKind == NodeData::Kind::kBlock);
        return *reinterpret_cast<BlockData*>(fData.fBytes);
    }

    const BlockData& blockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBlock);
        return *reinterpret_cast<const BlockData*>(fData.fBytes);
    }

    const Type* typeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kType);
        return *reinterpret_cast<const Type* const*>(fData.fBytes);
    }

    const TypeTokenData& typeTokenData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
        return *reinterpret_cast<const TypeTokenData*>(fData.fBytes);
    }

    int fKind;

    NodeData fData;

    // Needing two separate vectors is a temporary issue. Ideally, we'd just be able to use a single
    // vector of nodes, but there are various spots where we take pointers to std::unique_ptr<>,
    // and it isn't safe to pun std::unique_ptr<IRNode> to std::unique_ptr<Statement / Expression>.
    // And we can't update the call sites to expect std::unique_ptr<IRNode> while there are still
    // old-style nodes around.
    // When the transition is finished, we'll be able to drop the unique_ptrs and just handle
    // <IRNode> directly.
    std::vector<std::unique_ptr<Expression>> fExpressionChildren;
    // it's important to keep fStatements defined after (and thus destroyed before) fData,
    // because destroying statements can modify reference counts in a SymbolTable contained in fData
    std::vector<std::unique_ptr<Statement>> fStatementChildren;

};

}  // namespace SkSL

#endif
