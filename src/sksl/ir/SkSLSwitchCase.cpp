/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchCase.h"

namespace SkSL {

std::unique_ptr<SwitchCase> SwitchCase::Make(Position pos,
                                             SKSL_INT value,
                                             std::unique_ptr<Statement> statement) {
    return std::unique_ptr<SwitchCase>(new SwitchCase(pos, /*isDefault=*/false, value,
                                                      std::move(statement)));
}

std::unique_ptr<SwitchCase> SwitchCase::MakeDefault(Position pos,
                                                    std::unique_ptr<Statement> statement) {
    return std::unique_ptr<SwitchCase>(new SwitchCase(pos, /*isDefault=*/true, /*value=*/-1,
                                                      std::move(statement)));
}

std::string SwitchCase::description() const {
    return fDefault ? "default: \n" + fStatement->description()
                    : "case " + std::to_string(fValue) + ": \n" + fStatement->description();
}

}  // namespace SkSL
