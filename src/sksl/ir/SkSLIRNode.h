/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "include/private/SkTArray.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <atomic>
#include <unordered_set>
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
enum class FieldAccessOwnerKind : int8_t;
enum class VariableRefKind : int8_t;
enum class VariableStorage : int8_t;

using ExpressionArray = SkSTArray<2, std::unique_ptr<Expression>>;
using StatementArray = SkSTArray<2, std::unique_ptr<Statement>>;

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

    struct FieldAccessData {
        const Type* fType;
        int fFieldIndex;
        FieldAccessOwnerKind fOwnerKind;
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
        std::vector<const Variable*> fParameters;
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

    struct FunctionDefinitionData {
        const FunctionDeclaration* fDeclaration;
        // We track intrinsic functions we reference so that we can ensure that all of them end up
        // copied into the final output.
        std::unordered_set<const FunctionDeclaration*> fReferencedIntrinsics;
        // This pointer may be null, and even when non-null is not guaranteed to remain valid for
        // the entire lifespan of this object. The parse tree's lifespan is normally controlled by
        // IRGenerator, so the IRGenerator being destroyed or being used to compile another file
        // will invalidate this pointer.
        const ASTNode* fSource;
    };

    struct FunctionReferenceData {
        const Type* fType;
        std::vector<const FunctionDeclaration*> fFunctions;
     };

    struct IfStatementData {
        bool fIsStatic;
    };

    struct InterfaceBlockData {
        const Variable* fVariable;
        String fTypeName;
        String fInstanceName;
        std::shared_ptr<SymbolTable> fTypeOwner;
    };

    struct IntLiteralData {
        const Type* fType;
        int64_t fValue;
    };

    struct InlineMarkerData {
        const FunctionDeclaration* fFunction;
    };

    struct ModifiersDeclarationData {
        ModifiersPool::Handle fModifiersHandle;
    };

    struct SectionData {
        String fName;
        String fArgument;
        String fText;
    };

    struct SettingData {
        String fName;
        const Type* fType;
    };

    struct SwizzleData {
        const Type* fType;
        std::vector<int> fComponents;
    };

    struct SymbolData {
        StringFragment fName;
        const Type* fType;
    };

    struct SymbolAliasData {
        StringFragment fName;
        const Symbol* fOrigSymbol;
    };

    struct TypeReferenceData {
        const Type* fType;
        const Type* fValue;
     };

    struct TypeTokenData {
        const Type* fType;
        Token::Kind fToken;
    };

    struct UnresolvedFunctionData {
        // FIXME move this into the child vector after killing fExpressionChildren /
        // fStatementChildren
        std::vector<const FunctionDeclaration*> fFunctions;
    };

    struct VarDeclarationData {
        const Type* fBaseType;
        const Variable* fVar;
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
        VariableStorage fStorage;
        bool fBuiltin;
    };

    struct VariableReferenceData {
        const Variable* fVariable;
        VariableRefKind fRefKind;
    };

    struct NodeData {
        enum class Kind {
            kBlock,
            kBoolLiteral,
            kEnum,
            kExternalValue,
            kField,
            kFieldAccess,
            kFloatLiteral,
            kForStatement,
            kFunctionCall,
            kFunctionDeclaration,
            kFunctionDefinition,
            kFunctionReference,
            kIfStatement,
            kInlineMarker,
            kInterfaceBlock,
            kIntLiteral,
            kModifiersDeclaration,
            kSection,
            kSetting,
            kString,
            kSwizzle,
            kSymbol,
            kSymbolAlias,
            kType,
            kTypeReference,
            kTypeToken,
            kUnresolvedFunction,
            kVarDeclaration,
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
            FieldAccessData fFieldAccess;
            FloatLiteralData fFloatLiteral;
            ForStatementData fForStatement;
            FunctionCallData fFunctionCall;
            FunctionDeclarationData fFunctionDeclaration;
            FunctionDefinitionData fFunctionDefinition;
            FunctionReferenceData fFunctionReference;
            IfStatementData fIfStatement;
            InlineMarkerData fInlineMarker;
            InterfaceBlockData fInterfaceBlock;
            IntLiteralData fIntLiteral;
            ModifiersDeclarationData fModifiersDeclaration;
            SectionData fSection;
            SettingData fSetting;
            String fString;
            SwizzleData fSwizzle;
            SymbolData fSymbol;
            SymbolAliasData fSymbolAlias;
            const Type* fType;
            TypeReferenceData fTypeReference;
            TypeTokenData fTypeToken;
            UnresolvedFunctionData fUnresolvedFunction;
            VarDeclarationData fVarDeclaration;
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

        NodeData(const FieldAccessData& data)
            : fKind(Kind::kFieldAccess) {
            *(new(&fContents) FieldAccessData) = data;
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

        NodeData(const FunctionDefinitionData& data)
            : fKind(Kind::kFunctionDefinition) {
            *(new(&fContents) FunctionDefinitionData) = data;
        }

        NodeData(const FunctionReferenceData& data)
            : fKind(Kind::kFunctionReference) {
            *(new(&fContents) FunctionReferenceData) = data;
        }

        NodeData(IfStatementData data)
            : fKind(Kind::kIfStatement) {
            *(new(&fContents) IfStatementData) = data;
        }

        NodeData(InlineMarkerData data)
            : fKind(Kind::kInlineMarker) {
            *(new(&fContents) InlineMarkerData) = data;
        }

        NodeData(InterfaceBlockData data)
            : fKind(Kind::kInterfaceBlock) {
            *(new(&fContents) InterfaceBlockData) = data;
        }

        NodeData(IntLiteralData data)
            : fKind(Kind::kIntLiteral) {
            *(new(&fContents) IntLiteralData) = data;
        }

        NodeData(ModifiersDeclarationData data)
            : fKind(Kind::kModifiersDeclaration) {
            *(new(&fContents) ModifiersDeclarationData) = data;
        }

        NodeData(const SectionData& data)
            : fKind(Kind::kSection) {
            *(new(&fContents) SectionData) = data;
        }

        NodeData(const SettingData& data)
            : fKind(Kind::kSetting) {
            *(new(&fContents) SettingData) = data;
        }

        NodeData(const String& data)
            : fKind(Kind::kString) {
            *(new(&fContents) String) = data;
        }

        NodeData(const SwizzleData& data)
            : fKind(Kind::kSwizzle) {
            *(new(&fContents) SwizzleData) = data;
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

        NodeData(const TypeReferenceData& data)
            : fKind(Kind::kTypeReference) {
            *(new(&fContents) TypeReferenceData) = data;
        }

        NodeData(const TypeTokenData& data)
            : fKind(Kind::kTypeToken) {
            *(new(&fContents) TypeTokenData) = data;
        }

        NodeData(const UnresolvedFunctionData& data)
            : fKind(Kind::kUnresolvedFunction) {
            *(new(&fContents) UnresolvedFunctionData) = data;
        }

        NodeData(const VarDeclarationData& data)
            : fKind(Kind::kVarDeclaration) {
            *(new(&fContents) VarDeclarationData) = data;
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
                case Kind::kFieldAccess:
                    *(new(&fContents) FieldAccessData) = other.fContents.fFieldAccess;
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
                case Kind::kFunctionDefinition:
                    *(new(&fContents) FunctionDefinitionData) = other.fContents.fFunctionDefinition;
                    break;
                case Kind::kFunctionReference:
                    *(new(&fContents) FunctionReferenceData) = other.fContents.fFunctionReference;
                    break;
                case Kind::kIfStatement:
                    *(new(&fContents) IfStatementData) = other.fContents.fIfStatement;
                    break;
                case Kind::kInlineMarker:
                    *(new(&fContents) InlineMarkerData) = other.fContents.fInlineMarker;
                    break;
                case Kind::kInterfaceBlock:
                    *(new(&fContents) InterfaceBlockData) = other.fContents.fInterfaceBlock;
                    break;
                case Kind::kIntLiteral:
                    *(new(&fContents) IntLiteralData) = other.fContents.fIntLiteral;
                    break;
                case Kind::kModifiersDeclaration:
                    *(new(&fContents) ModifiersDeclarationData) =
                                                              other.fContents.fModifiersDeclaration;
                    break;
                case Kind::kSection:
                    *(new(&fContents) SectionData) = other.fContents.fSection;
                    break;
                case Kind::kSetting:
                    *(new(&fContents) SettingData) = other.fContents.fSetting;
                    break;
                case Kind::kString:
                    *(new(&fContents) String) = other.fContents.fString;
                    break;
                case Kind::kSwizzle:
                    *(new(&fContents) SwizzleData) = other.fContents.fSwizzle;
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
                case Kind::kTypeReference:
                    *(new(&fContents) TypeReferenceData) = other.fContents.fTypeReference;
                    break;
                case Kind::kTypeToken:
                    *(new(&fContents) TypeTokenData) = other.fContents.fTypeToken;
                    break;
                case Kind::kUnresolvedFunction:
                    *(new(&fContents) UnresolvedFunctionData) = other.fContents.fUnresolvedFunction;
                    break;
                case Kind::kVarDeclaration:
                    *(new(&fContents) VarDeclarationData) = other.fContents.fVarDeclaration;
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
                case Kind::kFieldAccess:
                    fContents.fFieldAccess.~FieldAccessData();
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
                case Kind::kFunctionDeclaration:
                    fContents.fFunctionDeclaration.~FunctionDeclarationData();
                    break;
                case Kind::kFunctionDefinition:
                    fContents.fFunctionDefinition.~FunctionDefinitionData();
                    break;
                case Kind::kFunctionReference:
                    fContents.fFunctionReference.~FunctionReferenceData();
                    break;
                case Kind::kIfStatement:
                    fContents.fIfStatement.~IfStatementData();
                    break;
                case Kind::kInlineMarker:
                    fContents.fInlineMarker.~InlineMarkerData();
                    break;
                case Kind::kInterfaceBlock:
                    fContents.fInterfaceBlock.~InterfaceBlockData();
                    break;
                case Kind::kIntLiteral:
                    fContents.fIntLiteral.~IntLiteralData();
                    break;
                case Kind::kModifiersDeclaration:
                    fContents.fModifiersDeclaration.~ModifiersDeclarationData();
                    break;
                case Kind::kSection:
                    fContents.fSection.~SectionData();
                    break;
                case Kind::kSetting:
                    fContents.fSetting.~SettingData();
                    break;
                case Kind::kString:
                    fContents.fString.~String();
                    break;
                case Kind::kSwizzle:
                    fContents.fSwizzle.~SwizzleData();
                    break;
                case Kind::kSymbol:
                    fContents.fSymbol.~SymbolData();
                    break;
                case Kind::kSymbolAlias:
                    fContents.fSymbolAlias.~SymbolAliasData();
                    break;
                case Kind::kType:
                    break;
                case Kind::kTypeReference:
                    fContents.fTypeReference.~TypeReferenceData();
                    break;
                case Kind::kTypeToken:
                    fContents.fTypeToken.~TypeTokenData();
                    break;
                case Kind::kUnresolvedFunction:
                    fContents.fUnresolvedFunction.~UnresolvedFunctionData();
                    break;
                case Kind::kVarDeclaration:
                    fContents.fVarDeclaration.~VarDeclarationData();
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

    IRNode(int offset, int kind, const BlockData& data, StatementArray stmts);

    IRNode(int offset, int kind, const BoolLiteralData& data);

    IRNode(int offset, int kind, const EnumData& data);

    IRNode(int offset, int kind, const ExternalValueData& data);

    IRNode(int offset, int kind, const FieldData& data);

    IRNode(int offset, int kind, const FieldAccessData& data);

    IRNode(int offset, int kind, const FloatLiteralData& data);

    IRNode(int offset, int kind, const ForStatementData& data);

    IRNode(int offset, int kind, const FunctionCallData& data);

    IRNode(int offset, int kind, const FunctionDeclarationData& data);

    IRNode(int offset, int kind, const FunctionDefinitionData& data);

    IRNode(int offset, int kind, const FunctionReferenceData& data);

    IRNode(int offset, int kind, const IfStatementData& data);

    IRNode(int offset, int kind, const InlineMarkerData& data);

    IRNode(int offset, int kind, const InterfaceBlockData& data);

    IRNode(int offset, int kind, const IntLiteralData& data);

    IRNode(int offset, int kind, const ModifiersDeclarationData& data);

    IRNode(int offset, int kind, const SectionData& data);

    IRNode(int offset, int kind, const SettingData& data);

    IRNode(int offset, int kind, const String& data);

    IRNode(int offset, int kind, const SwizzleData& data);

    IRNode(int offset, int kind, const SymbolData& data);

    IRNode(int offset, int kind, const SymbolAliasData& data);

    IRNode(int offset, int kind, const Type* data = nullptr);

    IRNode(int offset, int kind, const TypeReferenceData& data);

    IRNode(int offset, int kind, const TypeTokenData& data);

    IRNode(int offset, int kind, const UnresolvedFunctionData& data);

    IRNode(int offset, int kind, const VarDeclarationData& data);

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

    const FieldAccessData& fieldAccessData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFieldAccess);
        return fData.fContents.fFieldAccess;
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

    const FunctionDeclarationData& functionDeclarationData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionDeclaration);
        return fData.fContents.fFunctionDeclaration;
    }

    FunctionDefinitionData& functionDefinitionData() {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionDefinition);
        return fData.fContents.fFunctionDefinition;
    }

    const FunctionDefinitionData& functionDefinitionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionDefinition);
        return fData.fContents.fFunctionDefinition;
    }

    const FunctionReferenceData& functionReferenceData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionReference);
        return fData.fContents.fFunctionReference;
    }

    const IfStatementData& ifStatementData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kIfStatement);
        return fData.fContents.fIfStatement;
    }

    const InlineMarkerData& inlineMarkerData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInlineMarker);
        return fData.fContents.fInlineMarker;
    }

    InterfaceBlockData& interfaceBlockData() {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlock);
        return fData.fContents.fInterfaceBlock;
    }

    const InterfaceBlockData& interfaceBlockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlock);
        return fData.fContents.fInterfaceBlock;
    }

    const IntLiteralData& intLiteralData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kIntLiteral);
        return fData.fContents.fIntLiteral;
    }

    const ModifiersDeclarationData& modifiersDeclarationData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kModifiersDeclaration);
        return fData.fContents.fModifiersDeclaration;
    }

    const SectionData& sectionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSection);
        return fData.fContents.fSection;
    }

    const SettingData& settingData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSetting);
        return fData.fContents.fSetting;
    }

    const String& stringData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kString);
        return fData.fContents.fString;
    }

    SwizzleData& swizzleData() {
        SkASSERT(fData.fKind == NodeData::Kind::kSwizzle);
        return fData.fContents.fSwizzle;
    }

    const SwizzleData& swizzleData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSwizzle);
        return fData.fContents.fSwizzle;
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

    const TypeReferenceData& typeReferenceData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeReference);
        return fData.fContents.fTypeReference;
    }

    const TypeTokenData& typeTokenData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeToken);
        return fData.fContents.fTypeToken;
    }

    const UnresolvedFunctionData& unresolvedFunctionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kUnresolvedFunction);
        return fData.fContents.fUnresolvedFunction;
    }

    VarDeclarationData& varDeclarationData() {
        SkASSERT(fData.fKind == NodeData::Kind::kVarDeclaration);
        return fData.fContents.fVarDeclaration;
    }

    const VarDeclarationData& varDeclarationData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVarDeclaration);
        return fData.fContents.fVarDeclaration;
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
    ExpressionArray fExpressionChildren;
    // it's important to keep the statement array defined after (and thus destroyed before) fData,
    // because destroying statements can modify reference counts in a SymbolTable contained in fData
    StatementArray fStatementChildren;
};

}  // namespace SkSL

#endif
