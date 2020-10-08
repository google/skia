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
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <vector>

namespace SkSL {

class Expression;
class ExternalValue;
class FunctionDeclaration;
struct FunctionDefinition;
class Statement;
class Symbol;
class SymbolTable;
class Type;
class Variable;
class VariableReference;

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

    struct FieldData {
        StringFragment fName;
        const Type* fType;
        const Variable* fOwner;
        int fFieldIndex;
    };

    struct FloatLiteralData {
        const Type* fType;
        float fValue;
    };

    struct ForStatementData {
        std::shared_ptr<SymbolTable> fSymbolTable;
    };

    struct FunctionCallData {
        const Type* fType;
        const FunctionDeclaration* fFunction;
    };

    struct FunctionDeclarationData {
        StringFragment fName;
        mutable const FunctionDefinition* fDefinition;
        ModifiersPool::Handle fModifiersHandle;
        // FIXME after killing fExpressionChildren / fStatementChildren in favor of just fChildren,
        // the parameters should move into that vector
        std::vector<Variable*> fParameters;
        const Type* fReturnType;
        mutable std::atomic<int> fCallCount;
        bool fBuiltin;

        FunctionDeclarationData& operator=(const FunctionDeclarationData& other) {
            fName = other.fName;
            fDefinition = other.fDefinition;
            fModifiersHandle = other.fModifiersHandle;
            fParameters = other.fParameters;
            fReturnType = other.fReturnType;
            fCallCount = other.fCallCount.load();
            fBuiltin = other.fBuiltin;
            return *this;
        }
    };

    struct IfStatementData {
        bool fIsStatic;
    };

    struct IntLiteralData {
        const Type* fType;
        int64_t fValue;
    };

    struct SettingData {
        String fName;
        const Type* fType;
    };

    struct SymbolData {
        StringFragment fName;
        const Type* fType;
    };

    struct SymbolAliasData {
        StringFragment fName;
        Symbol* fOrigSymbol;
    };

    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct VariableData {
        StringFragment fName;
        const Type* fType;
        const Expression* fInitialValue = nullptr;
        ModifiersPool::Handle fModifiersHandle;
        // Tracks how many sites read from the variable. If this is zero for a non-out variable (or
        // becomes zero during optimization), the variable is dead and may be eliminated.
        mutable int16_t fReadCount;
        // Tracks how many sites write to the variable. If this is zero, the variable is dead and
        // may be eliminated.
        mutable int16_t fWriteCount;
        /*Variable::Storage*/int8_t fStorage;
        bool fBuiltin;
    };

    struct VariableReferenceData {
        const Variable* fVariable;
        /*VariableReference::RefKind*/int8_t fRefKind;
    };

    struct NodeData {
        enum class Kind {
            kBlock,
            kBoolLiteral,
            kEnum,
            kExternalValue,
            kField,
            kFloatLiteral,
            kForStatement,
            kFunctionCall,
            kFunctionDeclaration,
            kIfStatement,
            kIntLiteral,
            kSetting,
            kString,
            kSymbol,
            kSymbolAlias,
            kType,
            kTypeToken,
            kVariable,
            kVariableReference,
        } fKind = Kind::kType;
        // it doesn't really matter what kind we default to, as long as it's a POD type

        union Contents {
            BlockData fBlock;
            BoolLiteralData fBoolLiteral;
            EnumData fEnum;
            ExternalValueData fExternalValue;
            FieldData fField;
            FloatLiteralData fFloatLiteral;
            ForStatementData fForStatement;
            FunctionCallData fFunctionCall;
            FunctionDeclarationData fFunctionDeclaration;
            IfStatementData fIfStatement;
            IntLiteralData fIntLiteral;
            SettingData fSetting;
            String fString;
            SymbolData fSymbol;
            SymbolAliasData fSymbolAlias;
            const Type* fType;
            TypeTokenData fTypeToken;
            VariableData fVariable;
            VariableReferenceData fVariableReference;

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

        NodeData(const FieldData& data)
            : fKind(Kind::kField) {
            *(new(&fContents) FieldData) = data;
        }

        NodeData(const FloatLiteralData& data)
            : fKind(Kind::kFloatLiteral) {
            *(new(&fContents) FloatLiteralData) = data;
        }

        NodeData(const ForStatementData& data)
            : fKind(Kind::kForStatement) {
            *(new(&fContents) ForStatementData) = data;
        }

        NodeData(const FunctionCallData& data)
            : fKind(Kind::kFunctionCall) {
            *(new(&fContents) FunctionCallData) = data;
        }

        NodeData(const FunctionDeclarationData& data)
            : fKind(Kind::kFunctionDeclaration) {
            *(new(&fContents) FunctionDeclarationData) = data;
        }

        NodeData(IfStatementData data)
            : fKind(Kind::kIfStatement) {
            *(new(&fContents) IfStatementData) = data;
        }

        NodeData(IntLiteralData data)
            : fKind(Kind::kIntLiteral) {
            *(new(&fContents) IntLiteralData) = data;
        }

        NodeData(const SettingData& data)
            : fKind(Kind::kSetting) {
            *(new(&fContents) SettingData) = data;
        }

        NodeData(const String& data)
            : fKind(Kind::kString) {
            *(new(&fContents) String) = data;
        }

        NodeData(const SymbolData& data)
            : fKind(Kind::kSymbol) {
            *(new(&fContents) SymbolData) = data;
        }

        NodeData(const SymbolAliasData& data)
            : fKind(Kind::kSymbolAlias) {
            *(new(&fContents) SymbolAliasData) = data;
        }

        NodeData(const Type* data)
            : fKind(Kind::kType) {
            *(new(&fContents) const Type*) = data;
        }

        NodeData(const TypeTokenData& data)
            : fKind(Kind::kTypeToken) {
            *(new(&fContents) TypeTokenData) = data;
        }

        NodeData(const VariableData& data)
            : fKind(Kind::kVariable) {
            *(new(&fContents) VariableData) = data;
        }

        NodeData(const VariableReferenceData& data)
            : fKind(Kind::kVariableReference) {
            *(new(&fContents) VariableReferenceData) = data;
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
                case Kind::kField:
                    *(new(&fContents) FieldData) = other.fContents.fField;
                    break;
                case Kind::kFloatLiteral:
                    *(new(&fContents) FloatLiteralData) = other.fContents.fFloatLiteral;
                    break;
                case Kind::kForStatement:
                    *(new(&fContents) ForStatementData) = other.fContents.fForStatement;
                    break;
                case Kind::kFunctionCall:
                    *(new(&fContents) FunctionCallData) = other.fContents.fFunctionCall;
                    break;
                case Kind::kFunctionDeclaration:
                    *(new(&fContents) FunctionDeclarationData) =
                                                               other.fContents.fFunctionDeclaration;
                    break;
                case Kind::kIfStatement:
                    *(new(&fContents) IfStatementData) = other.fContents.fIfStatement;
                    break;
                case Kind::kIntLiteral:
                    *(new(&fContents) IntLiteralData) = other.fContents.fIntLiteral;
                    break;
                case Kind::kSetting:
                    *(new(&fContents) SettingData) = other.fContents.fSetting;
                    break;
                case Kind::kString:
                    *(new(&fContents) String) = other.fContents.fString;
                    break;
                case Kind::kSymbol:
                    *(new(&fContents) SymbolData) = other.fContents.fSymbol;
                    break;
                case Kind::kSymbolAlias:
                    *(new(&fContents) SymbolAliasData) = other.fContents.fSymbolAlias;
                    break;
                case Kind::kType:
                    *(new(&fContents) const Type*) = other.fContents.fType;
                    break;
                case Kind::kTypeToken:
                    *(new(&fContents) TypeTokenData) = other.fContents.fTypeToken;
                    break;
                case Kind::kVariable:
                    *(new(&fContents) VariableData) = other.fContents.fVariable;
                    break;
                case Kind::kVariableReference:
                    *(new(&fContents) VariableReferenceData) = other.fContents.fVariableReference;
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
                case Kind::kField:
                    fContents.fField.~FieldData();
                    break;
                case Kind::kFloatLiteral:
                    fContents.fFloatLiteral.~FloatLiteralData();
                    break;
                case Kind::kForStatement:
                    fContents.fForStatement.~ForStatementData();
                    break;
                case Kind::kFunctionCall:
                    fContents.fFunctionCall.~FunctionCallData();
                    break;
                case Kind::kIfStatement:
                    fContents.fIfStatement.~IfStatementData();
                    break;
                case Kind::kFunctionDeclaration:
                    fContents.fFunctionDeclaration.~FunctionDeclarationData();
                    break;
                case Kind::kIntLiteral:
                    fContents.fIntLiteral.~IntLiteralData();
                    break;
                case Kind::kSetting:
                    fContents.fSetting.~SettingData();
                    break;
                case Kind::kString:
                    fContents.fString.~String();
                    break;
                case Kind::kSymbol:
                    fContents.fSymbol.~SymbolData();
                    break;
                case Kind::kSymbolAlias:
                    fContents.fSymbolAlias.~SymbolAliasData();
                    break;
                case Kind::kType:
                    break;
                case Kind::kTypeToken:
                    fContents.fTypeToken.~TypeTokenData();
                    break;
                case Kind::kVariable:
                    fContents.fVariable.~VariableData();
                    break;
                case Kind::kVariableReference:
                    fContents.fVariableReference.~VariableReferenceData();
                    break;
            }
        }
    };

    IRNode(int offset, int kind, const BlockData& data,
           std::vector<std::unique_ptr<Statement>> stmts);

    IRNode(int offset, int kind, const BoolLiteralData& data);

    IRNode(int offset, int kind, const EnumData& data);

    IRNode(int offset, int kind, const ExternalValueData& data);

    IRNode(int offset, int kind, const FieldData& data);

    IRNode(int offset, int kind, const FloatLiteralData& data);

    IRNode(int offset, int kind, const ForStatementData& data);

    IRNode(int offset, int kind, const FunctionCallData& data);

    IRNode(int offset, int kind, const IfStatementData& data);

    IRNode(int offset, int kind, const FunctionDeclarationData& data);

    IRNode(int offset, int kind, const IntLiteralData& data);

    IRNode(int offset, int kind, const SettingData& data);

    IRNode(int offset, int kind, const String& data);

    IRNode(int offset, int kind, const SymbolData& data);

    IRNode(int offset, int kind, const SymbolAliasData& data);

    IRNode(int offset, int kind, const Type* data = nullptr);

    IRNode(int offset, int kind, const TypeTokenData& data);

    IRNode(int offset, int kind, const VariableData& data);

    IRNode(int offset, int kind, const VariableReferenceData& data);

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

    const FieldData& fieldData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kField);
        return fData.fContents.fField;
    }

    const FloatLiteralData& floatLiteralData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFloatLiteral);
        return fData.fContents.fFloatLiteral;
    }

    const ForStatementData& forStatementData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kForStatement);
        return fData.fContents.fForStatement;
    }

    const FunctionCallData& functionCallData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionCall);
        return fData.fContents.fFunctionCall;
    }

    FunctionDeclarationData& functionDeclarationData() {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionDeclaration);
        return fData.fContents.fFunctionDeclaration;
    }

    const IfStatementData& ifStatementData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kIfStatement);
        return fData.fContents.fIfStatement;
    }

    const FunctionDeclarationData& functionDeclarationData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionDeclaration);
        return fData.fContents.fFunctionDeclaration;
    }

    const IntLiteralData& intLiteralData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kIntLiteral);
        return fData.fContents.fIntLiteral;
    }

    const SettingData& settingData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSetting);
        return fData.fContents.fSetting;
    }

    const String& stringData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kString);
        return fData.fContents.fString;
    }

    SymbolData& symbolData() {
        SkASSERT(fData.fKind == NodeData::Kind::kSymbol);
        return fData.fContents.fSymbol;
    }

    const SymbolData& symbolData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSymbol);
        return fData.fContents.fSymbol;
    }

    const SymbolAliasData& symbolAliasData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSymbolAlias);
        return fData.fContents.fSymbolAlias;
    }

    const Type* typeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kType);
        return fData.fContents.fType;
    }

    const TypeTokenData& typeTokenData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
        return fData.fContents.fTypeToken;
    }

    VariableData& variableData() {
        SkASSERT(fData.fKind == NodeData::Kind::kVariable);
        return fData.fContents.fVariable;
    }

    const VariableData& variableData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVariable);
        return fData.fContents.fVariable;
    }

    VariableReferenceData& variableReferenceData() {
        SkASSERT(fData.fKind == NodeData::Kind::kVariableReference);
        return fData.fContents.fVariableReference;
    }

    const VariableReferenceData& variableReferenceData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVariableReference);
        return fData.fContents.fVariableReference;
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
