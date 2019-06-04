/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTNODE
#define SKSL_ASTNODE

#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLModifiers.h"

#include <vector>

namespace SkSL {

/**
 * Represents a node in the abstract syntax tree (AST). The AST is based directly on the parse tree;
 * it is a parsed-but-not-yet-analyzed version of the program.
 */
struct ASTNode {
    class ID {
    public:
        static ID Invalid() {
            return ID();
        }

        bool operator==(const ID& other) {
            return fValue == other.fValue;
        }

        bool operator!=(const ID& other) {
            return fValue != other.fValue;
        }

        operator bool() const { return fValue >= 0; }

    private:
        ID()
            : fValue(-1) {}

        ID(int value)
            : fValue(value) {}

        int fValue;

        friend struct ASTFile;
        friend struct ASTNode;
        friend class Parser;
    };

    enum class Kind {
        // data: operator(Token), children: left, right
        kBinary,
        // children: statements
        kBlock,
        // data: value(bool)
        kBool,
        kBreak,
        // children: target, arg1, arg2...
        kCall,
        kContinue,
        kDiscard,
        // children: statement, test
        kDo,
        // data: name(StringFragment), children: enumCases
        kEnum,
        // data: name(StringFragment), children: value?
        kEnumCase,
        // data: name(StringFragment)
        kExtension,
        // data: field(StringFragment), children: base
        kField,
        // children: declarations
        kFile,
        // data: value(float)
        kFloat,
        // children: init, test, next, statement
        kFor,
        // data: FunctionData, children: returnType, parameters, statement?
        kFunction,
        // data: name(StringFragment)
        kIdentifier,
        // children: base, index?
        kIndex,
        // data: isStatic(bool), children: test, ifTrue, ifFalse?
        kIf,
        // value(data): int
        kInt,
        // data: InterfaceBlockData, children: declaration1, declaration2, ..., size1, size2, ...
        kInterfaceBlock,
        // data: Modifiers
        kModifiers,
        kNull,
        // data: VarData, children: type, arraySize1, arraySize2, ..., value?
        kParameter,
        // data: operator(Token), children: operand
        kPostfix,
        // data: operator(Token), children: operand
        kPrefix,
        // children: value
        kReturn,
        // ...
        kSection,
        // children: value, statement 1, statement 2...
        kSwitchCase,
        // children: value, case 1, case 2...
        kSwitch,
        // children: test, ifTrue, ifFalse
        kTernary,
        // data: TypeData, children: sizes
        kType,
        // data: VarData, children: arraySize1, arraySize2, ..., value?
        kVarDeclaration,
        // children: modifiers, type, varDeclaration1, varDeclaration2, ...
        kVarDeclarations,
        // children: test, statement
        kWhile,
    };

    class iterator {
    public:
        iterator operator++() {
            SkASSERT(fID);
            fID = (**this).fNext;
            return *this;
        }

        iterator operator++(int) {
            SkASSERT(fID);
            iterator old = *this;
            fID = (**this).fNext;
            return old;
        }

        iterator operator+=(int count) {
            SkASSERT(count >= 0);
            for (; count > 0; --count) {
                ++(*this);
            }
            return *this;
        }

        iterator operator+(int count) {
            iterator result(*this);
            return result += count;
        }

        bool operator==(const iterator& other) {
            return fID == other.fID;
        }

        bool operator!=(const iterator& other) {
            return fID != other.fID;
        }

        ASTNode& operator*();

        ASTNode* operator->();

    private:
        iterator(std::vector<ASTNode>* nodes, ID id)
            : fNodes(nodes)
            , fID(id) {}

        std::vector<ASTNode>* fNodes;

        ID fID;

        friend struct ASTNode;
    };

    struct TypeData {
        TypeData(StringFragment name, bool isStructDeclaration, bool isNullable)
            : fName(name)
            , fIsStructDeclaration(isStructDeclaration)
            , fIsNullable(isNullable) {}

        StringFragment fName;
        bool fIsStructDeclaration;
        bool fIsNullable;
    };

    struct ParameterData {
        ParameterData(Modifiers modifiers, StringFragment name, size_t sizeCount)
            : fModifiers(modifiers)
            , fName(name)
            , fSizeCount(sizeCount) {}

        Modifiers fModifiers;
        StringFragment fName;
        size_t fSizeCount;
    };

    struct VarData {
        VarData(StringFragment name, size_t sizeCount)
            : fName(name)
            , fSizeCount(sizeCount) {}

        StringFragment fName;
        size_t fSizeCount;
    };

    struct FunctionData {
        FunctionData(Modifiers modifiers, StringFragment name, size_t parameterCount)
            : fModifiers(modifiers)
            , fName(name)
            , fParameterCount(parameterCount) {}

        Modifiers fModifiers;
        StringFragment fName;
        size_t fParameterCount;
    };

    struct InterfaceBlockData {
        InterfaceBlockData(Modifiers modifiers, StringFragment typeName, size_t declarationCount,
                           StringFragment instanceName, size_t sizeCount)
            : fModifiers(modifiers)
            , fTypeName(typeName)
            , fDeclarationCount(declarationCount)
            , fInstanceName(instanceName)
            , fSizeCount(sizeCount) {}

        Modifiers fModifiers;
        StringFragment fTypeName;
        size_t fDeclarationCount;
        StringFragment fInstanceName;
        size_t fSizeCount;
    };

    struct SectionData {
        SectionData(StringFragment name, StringFragment argument, StringFragment text)
            : fName(name)
            , fArgument(argument)
            , fText(text) {}

        StringFragment fName;
        StringFragment fArgument;
        StringFragment fText;
    };

    struct NodeData {
        char fBytes[std::max(sizeof(Token), std::max(sizeof(StringFragment),
                    std::max(sizeof(bool), std::max(sizeof(SKSL_INT),
                    std::max(sizeof(SKSL_FLOAT), std::max(sizeof(Modifiers),
                    std::max(sizeof(TypeData), std::max(sizeof(FunctionData),
                    std::max(sizeof(ParameterData), std::max(sizeof(VarData),
                    std::max(sizeof(InterfaceBlockData), sizeof(SectionData))))))))))))];

        enum class Kind {
            kToken,
            kStringFragment,
            kBool,
            kInt,
            kFloat,
            kModifiers,
            kTypeData,
            kFunctionData,
            kParameterData,
            kVarData,
            kInterfaceBlockData,
            kSectionData
        } fKind;

        NodeData() = default;

        NodeData(Token data)
        : fKind(Kind::kToken) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(StringFragment data)
        : fKind(Kind::kStringFragment) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(bool data)
        : fKind(Kind::kBool) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SKSL_INT data)
        : fKind(Kind::kInt) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SKSL_FLOAT data)
        : fKind(Kind::kFloat) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(Modifiers data)
        : fKind(Kind::kModifiers) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(TypeData data)
        : fKind(Kind::kTypeData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(FunctionData data)
        : fKind(Kind::kFunctionData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(VarData data)
        : fKind(Kind::kVarData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(ParameterData data)
        : fKind(Kind::kParameterData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(InterfaceBlockData data)
        : fKind(Kind::kInterfaceBlockData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SectionData data)
        : fKind(Kind::kSectionData) {
            memcpy(fBytes, &data, sizeof(data));
        }
    };

    ASTNode()
    : fOffset(-1)
    , fKind(Kind::kNull) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind)
    : fNodes(nodes)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, Token t)
    : fNodes(nodes)
    , fData(t)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, StringFragment s)
    : fNodes(nodes)
    , fData(s)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, const char* s)
    : fNodes(nodes)
    , fData(StringFragment(s))
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, bool b)
    : fNodes(nodes)
    , fData(b)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, SKSL_INT i)
    : fNodes(nodes)
    , fData(i)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, SKSL_FLOAT f)
    : fNodes(nodes)
    , fData(f)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, Modifiers m)
    : fNodes(nodes)
    , fData(m)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, FunctionData f)
    : fNodes(nodes)
    , fData(f)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, TypeData td)
    : fNodes(nodes)
    , fData(td)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, VarData v)
    : fNodes(nodes)
    , fData(v)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, ParameterData p)
    : fNodes(nodes)
    , fData(p)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, InterfaceBlockData ib)
    : fNodes(nodes)
    , fData(ib)
    , fOffset(offset)
    , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, SectionData s)
    : fNodes(nodes)
    , fData(s)
    , fOffset(offset)
    , fKind(kind) {}

    operator bool() const {
        return fKind != Kind::kNull;
    }

    Token getToken() const {
        SkASSERT(fData.fKind == NodeData::Kind::kToken);
        return *(Token*) fData.fBytes;
    }

    bool getBool() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBool);
        return *(bool*) fData.fBytes;
    }

    SKSL_INT getInt() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInt);
        return *(SKSL_INT*) fData.fBytes;
    }

    SKSL_FLOAT getFloat() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFloat);
        return *(SKSL_FLOAT*) fData.fBytes;
    }

    StringFragment getString() const {
        SkASSERT(fData.fKind == NodeData::Kind::kStringFragment);
        return *(StringFragment*) fData.fBytes;
    }

    Modifiers& getModifiers() const {
        SkASSERT(fData.fKind == NodeData::Kind::kModifiers);
        return *(Modifiers*) fData.fBytes;
    }

    TypeData& getTypeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeData);
        return *(TypeData*) fData.fBytes;
    }

    ParameterData& getParameterData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kParameterData);
        return *(ParameterData*) fData.fBytes;
    }

    VarData& getVarData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVarData);
        return *(VarData*) fData.fBytes;
    }

    FunctionData& getFunctionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionData);
        return *(FunctionData*) fData.fBytes;
    }

    InterfaceBlockData& getInterfaceBlockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlockData);
        return *(InterfaceBlockData*) fData.fBytes;
    }

    SectionData& getSectionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSectionData);
        return *(SectionData*) fData.fBytes;
    }

    void addChild(ID id) {
        SkASSERT(!(*fNodes)[id.fValue].fNext);
        if (fLastChild) {
            SkASSERT(!(*fNodes)[fLastChild.fValue].fNext);
            (*fNodes)[fLastChild.fValue].fNext = id;
        } else {
            fFirstChild = id;
        }
        fLastChild = id;
        SkASSERT(!(*fNodes)[fLastChild.fValue].fNext);
    }

    iterator begin() const {
        return iterator(fNodes, fFirstChild);
    }

    iterator end() const {
        return iterator(fNodes, ID(-1));
    }

    String description() const;

    std::vector<ASTNode>* fNodes;

    NodeData fData;

    int fOffset;

    Kind fKind;

    ID fFirstChild;

    ID fLastChild;

    ID fNext;
};

} // namespace

#endif
