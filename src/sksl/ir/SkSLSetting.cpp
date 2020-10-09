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

std::unique_ptr<Expression> Setting::constantPropagate(const IRGenerator& irGenerator,
                                                       const DefinitionMap& definitions) {
    if (irGenerator.settings()->fReplaceSettings) {
        return irGenerator.valueForSetting(this->fOffset, this->name());
    }
    return nullptr;
}

}  // namespace SkSL
