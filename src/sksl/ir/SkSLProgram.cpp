/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLProgram.h"

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

IRNode::ID Program::Settings::Value::literal(IRGenerator* irGenerator, int offset) const {
    switch (fKind) {
        case Program::Settings::Value::kBool_Kind:
            return irGenerator->createNode(new BoolLiteral(irGenerator, offset, fValue));
        case Program::Settings::Value::kFloat_Kind:
            return irGenerator->createNode(new FloatLiteral(irGenerator, offset, fValue));
        case Program::Settings::Value::kInt_Kind:
            return irGenerator->createNode(new IntLiteral(irGenerator, offset, fValue));
        default:
            SkASSERT(false);
            return IRNode::ID();
    }
}

} // namespace
