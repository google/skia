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

namespace SkSL {

struct Expression;
struct Statement;
struct Symbol;

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
struct IRNode {
    enum Kind {
        kBinary_Kind,
        kBlock_Kind,
        kBoolLiteral_Kind,
        kBreak_Kind,
        kConstructor_Kind,
        kContinue_Kind,
        kDefined_Kind,
        kDiscard_Kind,
        kDo_Kind,
        kEnum_Kind,
        kExpression_Kind,
        kExtension_Kind,
        kExternal_Kind,
        kExternalFunctionCall_Kind,
        kExternalValue_Kind,
        kField_Kind,
        kFieldAccess_Kind,
        kFloatLiteral_Kind,
        kFor_Kind,
        kFunction_Kind,
        kFunctionCall_Kind,
        kFunctionDeclaration_Kind,
        kFunctionReference_Kind,
        kGlobalVar_Kind,
        kIf_Kind,
        kIndex_Kind,
        kInterfaceBlock_Kind,
        kIntLiteral_Kind,
        kModifiers_Kind,
        kNop_Kind,
        kNullLiteral_Kind,
        kPostfix_Kind,
        kPrefix_Kind,
        kReturn_Kind,
        kSection_Kind,
        kSetting_Kind,
        kSwitch_Kind,
        kSwizzle_Kind,
        kTernary_Kind,
        kType_Kind,
        kTypeReference_Kind,
        kUnresolvedFunction_Kind,
        kVarDeclaration_Kind,
        kVarDeclarations_Kind,
        kVariable_Kind,
        kVariableReference_Kind,
        kWhile_Kind,
    };

    IRNode(int offset, Kind kind)
    : fOffset(offset)
    , fKind(kind) {}

    virtual ~IRNode() {}

    /**
     *  Use as<T> to downcast nodes.
     *  e.g. replace `(ReturnStatement&) s` with `s.as<ReturnStatement>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->fKind == T::kIRNodeKind);
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->fKind == T::kIRNodeKind);
        return static_cast<T&>(*this);
    }


    virtual String description() const = 0;

    virtual std::unique_ptr<IRNode> clone() const = 0;

    std::unique_ptr<Expression> cloneExpression() const;

    std::unique_ptr<Statement> cloneStatement() const;

    std::unique_ptr<Symbol> cloneSymbol() const;

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    int fOffset;

    Kind fKind;
};

}  // namespace SkSL

#endif
