/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIRNode.h"

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

IRNode::IRNode(int offset, int kind, const Type* data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(int offset, int kind, TypeTokenData data)
: fOffset(offset)
, fKind(kind)
, fData(data) {}

IRNode::IRNode(const IRNode& other)
    : fOffset(other.fOffset)
    , fData(other.fData) {
    SkASSERT(other.fExpressionChildren.empty());
}

IRNode::~IRNode() {}

} // namespace SkSL
