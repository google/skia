/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLModifiers.h"

#include <vector>

namespace SkSL {

struct Expression;
class  IRGenerator;
struct Statement;
class  Type;

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
struct IRNode {
    class ID {
    public:
        ID()
            : fIRGenerator(nullptr)
            , fValue(-1)
            , fNode(nullptr) {}

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

        IRNode& node() const;

        // FIXME remove this temporary function
        Type& typeNode() const {
            return (Type&) this->node();
        }

        // FIXME remove this temporary function
        Expression& expressionNode() const {
            return (Expression&) this->node();
        }

        // FIXME remove this temporary function
        Statement& statementNode() const {
            return (Statement&) this->node();
        }

    private:
        ID(IRGenerator* irGenerator, int value)
            : fIRGenerator(irGenerator)
            , fValue(value)
            , fNode(nullptr) {}

        ID(IRNode* node)
            : fIRGenerator(node->fIRGenerator)
            , fValue(-1)
            , fNode(node) {}

        IRGenerator* fIRGenerator;
        int fValue;
        // FIXME: temporary, remove when we finish the rearchitecture
        IRNode* fNode;

        friend class IRGenerator;
        friend struct IRNode;
        friend struct Program;
    };

    enum class Kind {
        kBool,
        kFloat,
        // data: isStatic(bool), children: test, ifTrue, ifFalse?
        kIf,
        kInt,
        kLegacy,
        kNull,
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

        IRNode& operator*() {
            SkASSERT(fID);
            return fID.fNode ? *fID.fNode : *(*fNodes)[fID.fValue];
        }

        IRNode* operator->() {
            SkASSERT(fID);
            return fID.fNode ? fID.fNode : (*fNodes)[fID.fValue].get();
        }

    private:
        iterator(std::vector<std::unique_ptr<IRNode>>* nodes, ID id)
            : fNodes(nodes)
            , fID(id) {}

        std::vector<std::unique_ptr<IRNode>>* fNodes;

        ID fID;

        friend struct IRNode;
    };

    struct NodeData {
        char fBytes[max(sizeof(bool),
                    max(sizeof(SKSL_INT),
                        sizeof(SKSL_FLOAT)))];

        enum class Kind {
            kBool,
            kInt,
            kFloat,
        } fKind;

        NodeData() = default;

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
    };

    IRNode(IRGenerator* irGenerator)
        : fIRGenerator(irGenerator)
        , fOffset(-1)
        , fNodeKind(Kind::kNull) {}

    IRNode(IRGenerator* irGenerator, int offset)
        : fIRGenerator(irGenerator)
        , fOffset(offset)
        , fNodeKind(Kind::kLegacy) {}

    IRNode(IRGenerator* irGenerator, int offset, Kind kind, bool b)
        : fIRGenerator(irGenerator)
        , fData(b)
        , fOffset(offset)
        , fNodeKind(kind) {}

    IRNode(IRGenerator* irGenerator, int offset, Kind kind, SKSL_INT i)
        : fIRGenerator(irGenerator)
        , fData(i)
        , fOffset(offset)
        , fNodeKind(kind) {}

    IRNode(IRGenerator* irGenerator, int offset, Kind kind, SKSL_FLOAT f)
        : fIRGenerator(irGenerator)
        , fData(f)
        , fOffset(offset)
        , fNodeKind(kind) {}

    operator bool() const {
        return fNodeKind != Kind::kNull;
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

    void addChild(ID id) {
        /*SkASSERT(!(*fIRGenerator.fNodes)[id.fValue]->fNext);
        if (fLastChild) {
            SkASSERT(!(*fIRGenerator.fNodes)[fLastChild.fValue]->fNext);
            (*fNodes)[fLastChild.fValue]->fNext = id;
        } else {
            fFirstChild = id;
        }
        fLastChild = id;
        SkASSERT(!(*fIRGenerator.fNodes)[fLastChild.fValue]->fNext);*/
        abort();
    }

    iterator begin() const {
        //return iterator(fIRGenerator.fNodes, fFirstChild);
        abort();
    }

    iterator end() const {
        //return iterator(fIRGenerator.fNodes, ID(-1));
        abort();
    }

    virtual String description() const {
        return "<irnode>";
    }

    virtual ~IRNode() {
    }

    virtual bool isEmpty() const {
        return false;
    }

    virtual bool isConstant() const {
        return false;
    }

    virtual bool hasSideEffects() const {
        return false;
    }

    IRGenerator* fIRGenerator;

    NodeData fData;

    int fOffset;

    Kind fNodeKind;

    ID fFirstChild;

    ID fLastChild;

    ID fNext;
};

} // namespace

#endif
