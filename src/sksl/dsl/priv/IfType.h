/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_IFTYPE
#define SKSL_DSL_IFTYPE

#include "src/sksl/dsl/Bool.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLIfStatement.h"

namespace skslcode {

template<class Test, class IfTrue, class IfFalse>
class IfType {
public:
    IfType(Test test, IfTrue ifTrue, IfFalse ifFalse)
        : fTest(std::move(test))
        , fIfTrue(std::move(ifTrue))
        , fIfFalse(std::move(ifFalse)) {}

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::IfStatement>(/*offset=*/-1,
                                                   /*isStatic=*/false,
                                                   fTest.expression(),
                                                   fIfTrue.statement(),
                                                   fIfFalse.statement());
    }

private:
    const Test fTest;
    const IfTrue fIfTrue;
    const IfFalse fIfFalse;
};

} // namespace skslcode

#endif
