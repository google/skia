/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLPosition.h"

#include <string>

namespace SkSL {

// The fKind field of IRNode could contain any of these values.
enum class ProgramElementKind {
    kExtension = 0,
    kFunction,
    kFunctionPrototype,
    kGlobalVar,
    kInterfaceBlock,
    kModifiers,
    kStructDefinition,

    kFirst = kExtension,
    kLast = kStructDefinition
};

enum class SymbolKind {
    kExternal = (int) ProgramElementKind::kLast + 1,
    kField,
    kFunctionDeclaration,
    kType,
    kVariable,

    kFirst = kExternal,
    kLast = kVariable
};

enum class StatementKind {
    kBlock = (int) SymbolKind::kLast + 1,
    kBreak,
    kContinue,
    kDiscard,
    kDo,
    kExpression,
    kFor,
    kIf,
    kNop,
    kReturn,
    kSwitch,
    kSwitchCase,
    kVarDeclaration,

    kFirst = kBlock,
    kLast = kVarDeclaration,
};

enum class ExpressionKind {
    kBinary = (int) StatementKind::kLast + 1,
    kChildCall,
    kConstructorArray,
    kConstructorArrayCast,
    kConstructorCompound,
    kConstructorCompoundCast,
    kConstructorDiagonalMatrix,
    kConstructorMatrixResize,
    kConstructorScalarCast,
    kConstructorSplat,
    kConstructorStruct,
    kEmpty,
    kFieldAccess,
    kFunctionReference,
    kFunctionCall,
    kIndex,
    kLiteral,
    kMethodReference,
    kPoison,
    kPostfix,
    kPrefix,
    kSetting,
    kSwizzle,
    kTernary,
    kTypeReference,
    kVariableReference,

    kFirst = kBinary,
    kLast = kVariableReference
};

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
class IRNode : public Poolable {
public:
    virtual ~IRNode() {}

    virtual std::string description() const = 0;

    // No copy construction or assignment
    IRNode(const IRNode&) = delete;
    IRNode& operator=(const IRNode&) = delete;

    // Position of this element within the program being compiled, for error reporting purposes.
    Position fPosition;

    Position position() const {
        return fPosition;
    }

    void setPosition(Position p) {
        fPosition = p;
    }

    /**
     *  Use is<T> to check the type of an IRNode.
     *  e.g. replace `s.kind() == Statement::Kind::kReturn` with `s.is<ReturnStatement>()`.
     */
    template <typename T>
    bool is() const {
        return this->fKind == (int)T::kIRNodeKind;
    }

    /**
     *  Use as<T> to downcast IRNodes.
     *  e.g. replace `(ReturnStatement&) s` with `s.as<ReturnStatement>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
    }

protected:
    IRNode(Position position, int kind)
        : fPosition(position)
        , fKind(kind) {}

    int fKind;
};

}  // namespace SkSL

#endif
