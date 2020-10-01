/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIRNode.h"

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

IRNode::IRNode(int offset, int kind, const BlockData& data,
               std::vector<std::unique_ptr<Statement>> stmts)
: fOffset(offset)
, fKind(kind)
, fData(data)
, fStatementChildren(std::move(stmts)) {}

IRNode::IRNode(int offset, int kind, const BoolLiteralData& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const EnumData& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const ExternalValueData& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const IntLiteralData& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const String& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const Type* data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, const TypeTokenData& data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(const IRNode& other)
    : fOffset(other.fOffset)
    , fKind(other.fKind)
    , fData(other.fData) {
    // For now, we can't use a default copy constructor because of the std::unique_ptr children.
    // Since we never copy nodes containing children, it's easiest just to assert we don't have any
    // than bother with cloning them.
    SkASSERT(other.fExpressionChildren.empty());
}

IRNode::~IRNode() {}

} // namespace SkSL
