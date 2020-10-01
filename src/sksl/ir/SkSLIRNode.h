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
class ExternalValue;
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
            case NodeData::Kind::kBoolLiteral:
                return *this->boolLiteralData().fType;
            case NodeData::Kind::kExternalValue:
                return *this->externalValueData().fType;
            case NodeData::Kind::kIntLiteral:
                return *this->intLiteralData().fType;
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

    struct BoolLiteralData {
        const Type* fType;
        bool fValue;
    };

    struct EnumData {
        StringFragment fTypeName;
        std::shared_ptr<SymbolTable> fSymbols;
        bool fIsBuiltin;
    };

    struct ExternalValueData {
        const Type* fType;
        const ExternalValue* fValue;
    };

    struct IntLiteralData {
        const Type* fType;
        int64_t fValue;
    };

    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct NodeData {
        enum class Kind {
            kBlock,
            kBoolLiteral,
            kEnum,
            kExternalValue,
            kIntLiteral,
            kString,
            kType,
            kTypeToken,
        } fKind = Kind::kType;
        // it doesn't really matter what kind we default to, as long as it's a POD type

        union Contents {
            BlockData fBlock;
            BoolLiteralData fBoolLiteral;
            EnumData fEnum;
            ExternalValueData fExternalValue;
            IntLiteralData fIntLiteral;
            String fString;
            const Type* fType;
            TypeTokenData fTypeToken;

            Contents() {}

            ~Contents() {}
        } fContents;

        NodeData(const BlockData& data)
            : fKind(Kind::kBlock) {
            *(new(&fContents) BlockData) = data;
        }

        NodeData(const BoolLiteralData& data)
            : fKind(Kind::kBoolLiteral) {
            *(new(&fContents) BoolLiteralData) = data;
        }

        NodeData(const EnumData& data)
            : fKind(Kind::kEnum) {
            *(new(&fContents) EnumData) = data;
        }

        NodeData(const ExternalValueData& data)
            : fKind(Kind::kExternalValue) {
            *(new(&fContents) ExternalValueData) = data;
        }

        NodeData(IntLiteralData data)
            : fKind(Kind::kIntLiteral) {
            *(new(&fContents) IntLiteralData) = data;
        }

        NodeData(const String& data)
            : fKind(Kind::kString) {
            *(new(&fContents) String) = data;
        }

        NodeData(const Type* data)
            : fKind(Kind::kType) {
            *(new(&fContents) const Type*) = data;
        }

        NodeData(const TypeTokenData& data)
            : fKind(Kind::kTypeToken) {
            *(new(&fContents) TypeTokenData) = data;
        }

        NodeData(const NodeData& other) {
            *this = other;
        }

        NodeData& operator=(const NodeData& other) {
            this->cleanup();
            fKind = other.fKind;
            switch (fKind) {
                case Kind::kBlock:
                    *(new(&fContents) BlockData) = other.fContents.fBlock;
                    break;
                case Kind::kBoolLiteral:
                    *(new(&fContents) BoolLiteralData) = other.fContents.fBoolLiteral;
                    break;
                case Kind::kEnum:
                    *(new(&fContents) EnumData) = other.fContents.fEnum;
                    break;
                case Kind::kExternalValue:
                    *(new(&fContents) ExternalValueData) = other.fContents.fExternalValue;
                    break;
                case Kind::kIntLiteral:
                    *(new(&fContents) IntLiteralData) = other.fContents.fIntLiteral;
                    break;
                case Kind::kString:
                    *(new(&fContents) String) = other.fContents.fString;
                    break;
                case Kind::kType:
                    *(new(&fContents) const Type*) = other.fContents.fType;
                    break;
                case Kind::kTypeToken:
                    *(new(&fContents) TypeTokenData) = other.fContents.fTypeToken;
                    break;
            }
            return *this;
        }

        ~NodeData() {
            this->cleanup();
        }

    private:
        void cleanup() {
            switch (fKind) {
                case Kind::kBlock:
                    fContents.fBlock.~BlockData();
                    break;
                case Kind::kBoolLiteral:
                    fContents.fBoolLiteral.~BoolLiteralData();
                    break;
                case Kind::kEnum:
                    fContents.fEnum.~EnumData();
                    break;
                case Kind::kExternalValue:
                    fContents.fExternalValue.~ExternalValueData();
                    break;
                case Kind::kIntLiteral:
                    fContents.fIntLiteral.~IntLiteralData();
                    break;
                case Kind::kString:
                    fContents.fString.~String();
                    break;
                case Kind::kType:
                    break;
                case Kind::kTypeToken:
                    fContents.fTypeToken.~TypeTokenData();
                    break;
            }
        }
    };

    IRNode(int offset, int kind, const BlockData& data,
           std::vector<std::unique_ptr<Statement>> stmts);

    IRNode(int offset, int kind, const BoolLiteralData& data);

    IRNode(int offset, int kind, const EnumData& data);

    IRNode(int offset, int kind, const ExternalValueData& data);

    IRNode(int offset, int kind, const IntLiteralData& data);

    IRNode(int offset, int kind, const String& data);

    IRNode(int offset, int kind, const Type* data = nullptr);

    IRNode(int offset, int kind, const TypeTokenData& data);

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
        return fData.fContents.fBlock;
    }

    const BlockData& blockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBlock);
        return fData.fContents.fBlock;
    }

    const BoolLiteralData& boolLiteralData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBoolLiteral);
        return fData.fContents.fBoolLiteral;
    }

    const EnumData& enumData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kEnum);
        return fData.fContents.fEnum;
    }

    const ExternalValueData& externalValueData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kExternalValue);
        return fData.fContents.fExternalValue;
    }

    const IntLiteralData& intLiteralData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kIntLiteral);
        return fData.fContents.fIntLiteral;
    }

    const String& stringData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kString);
        return fData.fContents.fString;
    }

    const Type* typeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kType);
        return fData.fContents.fType;
    }

    const TypeTokenData& typeTokenData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
        return fData.fContents.fTypeToken;
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
