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

// std::max isn't constexpr in some compilers
static constexpr size_t Max(size_t a, size_t b) {
    return a > b ? a : b;
}

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
        // data: ParameterData, children: type, arraySize1, arraySize2, ..., value?
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

        bool operator==(const iterator& other) const {
            return fID == other.fID;
        }

        bool operator!=(const iterator& other) const {
            return fID != other.fID;
        }

        ASTNode& operator*() {
            SkASSERT(fID);
            return (*fNodes)[fID.fValue];
        }

        ASTNode* operator->() {
            SkASSERT(fID);
            return &(*fNodes)[fID.fValue];
        }

    private:
        iterator(std::vector<ASTNode>* nodes, ID id)
            : fNodes(nodes)
            , fID(id) {}

        std::vector<ASTNode>* fNodes;

        ID fID;

        friend struct ASTNode;
    };

    struct TypeData {
        TypeData() {}

        TypeData(StringFragment name, bool isStructDeclaration, bool isNullable)
            : fName(name)
            , fIsStructDeclaration(isStructDeclaration)
            , fIsNullable(isNullable) {}

        StringFragment fName;
        bool fIsStructDeclaration;
        bool fIsNullable;
    };

    struct ParameterData {
        ParameterData() {}

        ParameterData(Modifiers modifiers, StringFragment name, size_t sizeCount)
            : fModifiers(modifiers)
            , fName(name)
            , fSizeCount(sizeCount) {}

        Modifiers fModifiers;
        StringFragment fName;
        size_t fSizeCount;
    };

    struct VarData {
        VarData() {}

        VarData(StringFragment name, size_t sizeCount)
            : fName(name)
            , fSizeCount(sizeCount) {}

        StringFragment fName;
        size_t fSizeCount;
    };

    struct FunctionData {
        FunctionData() {}

        FunctionData(Modifiers modifiers, StringFragment name, size_t parameterCount)
            : fModifiers(modifiers)
            , fName(name)
            , fParameterCount(parameterCount) {}

        Modifiers fModifiers;
        StringFragment fName;
        size_t fParameterCount;
    };

    struct InterfaceBlockData {
        InterfaceBlockData() {}

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
        SectionData() {}

        SectionData(StringFragment name, StringFragment argument, StringFragment text)
            : fName(name)
            , fArgument(argument)
            , fText(text) {}

        StringFragment fName;
        StringFragment fArgument;
        StringFragment fText;
    };

    struct NodeData {
        char fBytes[Max(sizeof(Token),
                    Max(sizeof(StringFragment),
                    Max(sizeof(bool),
                    Max(sizeof(SKSL_INT),
                    Max(sizeof(SKSL_FLOAT),
                    Max(sizeof(Modifiers),
                    Max(sizeof(TypeData),
                    Max(sizeof(FunctionData),
                    Max(sizeof(ParameterData),
                    Max(sizeof(VarData),
                    Max(sizeof(InterfaceBlockData),
                        sizeof(SectionData))))))))))))];

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
            , fKind(kind) {
        switch (kind) {
            case Kind::kBinary:
            case Kind::kPostfix:
            case Kind::kPrefix:
                fData.fKind = NodeData::Kind::kToken;
                break;

            case Kind::kBool:
            case Kind::kIf:
            case Kind::kSwitch:
                fData.fKind = NodeData::Kind::kBool;
                break;

            case Kind::kEnum:
            case Kind::kEnumCase:
            case Kind::kExtension:
            case Kind::kField:
            case Kind::kIdentifier:
                fData.fKind = NodeData::Kind::kStringFragment;
                break;

            case Kind::kFloat:
                fData.fKind = NodeData::Kind::kFloat;
                break;

            case Kind::kFunction:
                fData.fKind = NodeData::Kind::kFunctionData;
                break;

            case Kind::kInt:
                fData.fKind = NodeData::Kind::kInt;
                break;

            case Kind::kInterfaceBlock:
                fData.fKind = NodeData::Kind::kInterfaceBlockData;
                break;

            case Kind::kModifiers:
                fData.fKind = NodeData::Kind::kModifiers;
                break;

            case Kind::kParameter:
                fData.fKind = NodeData::Kind::kParameterData;
                break;

            case Kind::kVarDeclaration:
                fData.fKind = NodeData::Kind::kVarData;
                break;

            case Kind::kType:
                fData.fKind = NodeData::Kind::kTypeData;
                break;

            default:
                break;
        }
    }

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

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, TypeData td)
        : fNodes(nodes)
        , fData(td)
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
        Token result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    bool getBool() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBool);
        bool result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    SKSL_INT getInt() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInt);
        SKSL_INT result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    SKSL_FLOAT getFloat() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFloat);
        SKSL_FLOAT result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    StringFragment getString() const {
        SkASSERT(fData.fKind == NodeData::Kind::kStringFragment);
        StringFragment result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    Modifiers getModifiers() const {
        SkASSERT(fData.fKind == NodeData::Kind::kModifiers);
        Modifiers result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setModifiers(const Modifiers& m) {
        memcpy(fData.fBytes, &m, sizeof(m));
    }

    TypeData getTypeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeData);
        TypeData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setTypeData(const ASTNode::TypeData& td) {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeData);
        memcpy(fData.fBytes, &td, sizeof(td));
    }

    ParameterData getParameterData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kParameterData);
        ParameterData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setParameterData(const ASTNode::ParameterData& pd) {
        SkASSERT(fData.fKind == NodeData::Kind::kParameterData);
        memcpy(fData.fBytes, &pd, sizeof(pd));
    }

    VarData getVarData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVarData);
        VarData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setVarData(const ASTNode::VarData& vd) {
        SkASSERT(fData.fKind == NodeData::Kind::kVarData);
        memcpy(fData.fBytes, &vd, sizeof(vd));
    }

    FunctionData getFunctionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionData);
        FunctionData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setFunctionData(const ASTNode::FunctionData& fd) {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionData);
        memcpy(fData.fBytes, &fd, sizeof(fd));
    }

    InterfaceBlockData getInterfaceBlockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlockData);
        InterfaceBlockData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
    }

    void setInterfaceBlockData(const ASTNode::InterfaceBlockData& id) {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlockData);
        memcpy(fData.fBytes, &id, sizeof(id));
    }

    SectionData getSectionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSectionData);
        SectionData result;
        memcpy(&result, fData.fBytes, sizeof(result));
        return result;
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

#ifdef SK_DEBUG
    String description() const;
#endif

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
