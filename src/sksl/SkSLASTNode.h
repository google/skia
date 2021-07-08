/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTNODE
#define SKSL_ASTNODE

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLString.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLOperators.h"

#include <algorithm>
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
        // data: operator, children: left, right
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
        // data: name(string_view)
        kExtension,
        // data: field(string_view), children: base
        kField,
        // children: declarations
        kFile,
        // data: value(float)
        kFloat,
        // children: init, test, next, statement
        kFor,
        // data: FunctionData, children: returnType, parameters, statement?
        kFunction,
        // data: name(string_view)
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
        // data: operator, children: operand
        kPostfix,
        // data: operator, children: operand
        kPrefix,
        // children: value
        kReturn,
        // children: value, statement 1, statement 2...
        kSwitchCase,
        // children: value, case 1, case 2...
        kSwitch,
        // children: test, ifTrue, ifFalse
        kTernary,
        // data: name(string_view), children: sizes
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

    struct ParameterData {
        ParameterData() {}

        ParameterData(Modifiers modifiers, skstd::string_view name, bool isArray)
            : fModifiers(modifiers)
            , fName(name)
            , fIsArray(isArray) {}

        Modifiers fModifiers;
        skstd::string_view fName;
        bool fIsArray;
    };

    struct VarData {
        VarData() {}

        VarData(skstd::string_view name, bool isArray)
            : fName(name)
            , fIsArray(isArray) {}

        skstd::string_view fName;
        bool fIsArray;
    };

    struct FunctionData {
        FunctionData() {}

        FunctionData(Modifiers modifiers, skstd::string_view name, size_t parameterCount)
            : fModifiers(modifiers)
            , fName(name)
            , fParameterCount(parameterCount) {}

        Modifiers fModifiers;
        skstd::string_view fName;
        size_t fParameterCount;
    };

    struct InterfaceBlockData {
        InterfaceBlockData() {}

        InterfaceBlockData(Modifiers modifiers, skstd::string_view typeName,
                           size_t declarationCount, skstd::string_view instanceName, bool isArray)
            : fModifiers(modifiers)
            , fTypeName(typeName)
            , fDeclarationCount(declarationCount)
            , fInstanceName(instanceName)
            , fIsArray(isArray) {}

        Modifiers fModifiers;
        skstd::string_view fTypeName;
        size_t fDeclarationCount;
        skstd::string_view fInstanceName;
        bool fIsArray;
    };

    struct NodeData {
        // We use fBytes as a union which can hold any type of AST node, and use placement-new to
        // copy AST objects into fBytes. Note that none of the AST objects have interesting
        // destructors, so we do not bother doing a placement-delete on any of them in ~NodeData.
        char fBytes[std::max({sizeof(Operator),
                              sizeof(skstd::string_view),
                              sizeof(bool),
                              sizeof(SKSL_INT),
                              sizeof(SKSL_FLOAT),
                              sizeof(Modifiers),
                              sizeof(FunctionData),
                              sizeof(ParameterData),
                              sizeof(VarData),
                              sizeof(InterfaceBlockData)})];

        enum class Kind {
            kOperator,
            kStringView,
            kBool,
            kInt,
            kFloat,
            kModifiers,
            kFunctionData,
            kParameterData,
            kVarData,
            kInterfaceBlockData,
        } fKind;

        NodeData() = default;

        NodeData(Operator op)
            : fKind(Kind::kOperator) {
            new (fBytes) Operator(op);
        }

        NodeData(const skstd::string_view& data)
            : fKind(Kind::kStringView) {
            new (fBytes) skstd::string_view(data);
        }

        NodeData(bool data)
            : fKind(Kind::kBool) {
            new (fBytes) bool(data);
        }

        NodeData(SKSL_INT data)
            : fKind(Kind::kInt) {
            new (fBytes) SKSL_INT(data);
        }

        NodeData(SKSL_FLOAT data)
            : fKind(Kind::kFloat) {
            new (fBytes) SKSL_FLOAT(data);
        }

        NodeData(const Modifiers& data)
            : fKind(Kind::kModifiers) {
            new (fBytes) Modifiers(data);
        }

        NodeData(const FunctionData& data)
            : fKind(Kind::kFunctionData) {
            new (fBytes) FunctionData(data);
        }

        NodeData(const VarData& data)
            : fKind(Kind::kVarData) {
            new (fBytes) VarData(data);
        }

        NodeData(const ParameterData& data)
            : fKind(Kind::kParameterData) {
            new (fBytes) ParameterData(data);
        }

        NodeData(const InterfaceBlockData& data)
            : fKind(Kind::kInterfaceBlockData) {
            new (fBytes) InterfaceBlockData(data);
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
                fData.fKind = NodeData::Kind::kOperator;
                break;

            case Kind::kBool:
            case Kind::kIf:
            case Kind::kSwitch:
                fData.fKind = NodeData::Kind::kBool;
                break;

            case Kind::kExtension:
            case Kind::kField:
            case Kind::kIdentifier:
            case Kind::kType:
                fData.fKind = NodeData::Kind::kStringView;
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

            default:
                break;
        }
    }

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, Operator op)
        : fNodes(nodes)
        , fData(op)
        , fOffset(offset)
        , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, skstd::string_view s)
        : fNodes(nodes)
        , fData(s)
        , fOffset(offset)
        , fKind(kind) {}

    ASTNode(std::vector<ASTNode>* nodes, int offset, Kind kind, const char* s)
        : fNodes(nodes)
        , fData(skstd::string_view(s))
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

    operator bool() const {
        return fKind != Kind::kNull;
    }

    Operator getOperator() const {
        SkASSERT(fData.fKind == NodeData::Kind::kOperator);
        return *reinterpret_cast<const Operator*>(fData.fBytes);
    }

    bool getBool() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBool);
        return *reinterpret_cast<const bool*>(fData.fBytes);
    }

    SKSL_INT getInt() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInt);
        return *reinterpret_cast<const SKSL_INT*>(fData.fBytes);
    }

    SKSL_FLOAT getFloat() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFloat);
        return *reinterpret_cast<const SKSL_FLOAT*>(fData.fBytes);
    }

    const skstd::string_view& getStringView() const {
        SkASSERT(fData.fKind == NodeData::Kind::kStringView);
        return *reinterpret_cast<const skstd::string_view*>(fData.fBytes);
    }

    const Modifiers& getModifiers() const {
        SkASSERT(fData.fKind == NodeData::Kind::kModifiers);
        return *reinterpret_cast<const Modifiers*>(fData.fBytes);
    }

    void setModifiers(const Modifiers& m) {
        new (fData.fBytes) Modifiers(m);
    }

    const ParameterData& getParameterData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kParameterData);
        return *reinterpret_cast<const ParameterData*>(fData.fBytes);
    }

    void setParameterData(const ASTNode::ParameterData& pd) {
        new (fData.fBytes) ParameterData(pd);
    }

    const VarData& getVarData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVarData);
        return *reinterpret_cast<const VarData*>(fData.fBytes);
    }

    void setVarData(const ASTNode::VarData& vd) {
        new (fData.fBytes) VarData(vd);
    }

    const FunctionData& getFunctionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionData);
        return *reinterpret_cast<const FunctionData*>(fData.fBytes);
    }

    void setFunctionData(const ASTNode::FunctionData& fd) {
        new (fData.fBytes) FunctionData(fd);
    }

    const InterfaceBlockData& getInterfaceBlockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlockData);
        return *reinterpret_cast<const InterfaceBlockData*>(fData.fBytes);
    }

    void setInterfaceBlockData(const ASTNode::InterfaceBlockData& id) {
        new (fData.fBytes) InterfaceBlockData(id);
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

}  // namespace SkSL

#endif
