/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

IRNode::ID Setting::constantPropagate(const DefinitionMap& definitions) {
    if (fIRGenerator->fSettings->fReplaceSettings) {
        return VariableReference::CopyConstant(fIRGenerator, fValue.expression());
    }
    return IRNode::ID();
}

} // namespace
