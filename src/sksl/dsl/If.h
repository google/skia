/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_IF
#define SKSL_DSL_IF

#include "src/sksl/dsl/Block.h"
#include "src/sksl/dsl/Bool.h"
#include "src/sksl/dsl/priv/Empty.h"
#include "src/sksl/dsl/priv/IfType.h"
#include "src/sksl/dsl/priv/Statement.h"

namespace skslcode {

template<class Test, class IfTrue>
IfType<Test, IfTrue, Empty> If(Test test, IfTrue ifTrue) {
    return IfType<Test, IfTrue, Empty>(std::move(test), std::move(ifTrue), Empty());
}

template<class Test, class IfTrue, class IfFalse>
IfType<Test, IfTrue, IfFalse> If(Test test, IfTrue ifTrue, IfFalse ifFalse) {
    return IfType<Test, IfTrue, IfFalse>(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

} // namespace skslcode

#endif
